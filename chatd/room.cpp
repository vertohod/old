#include <boost/thread/detail/singleton.hpp>

#include "configure.h"
#include "message.h"
#include "stream.h"
#include "dumper.h"
#include "error.h"
#include "front.h"
#include "room.h"

namespace chatd
{

room::room(
	OID oid,
	sptr_cstr name,
	OID owner,
	bool arg_private
) : object(oid, name, owner), m_private(arg_private), m_last_out(0), m_type(0), m_transparence(0)
{
}

room::room(sptr_cmap data) : object(data),
    m_private(util::get_val<bool>(*data, "private")),
	m_last_out(util::get_val<time_t>(*data, "last_out")),
	m_type(0),
	m_transparence(0)
{
}

void room::add_message(
	sptr_cstr arg_message,
    sptr_cstr name_from,
	OID oid_from,
	sptr_cstr name_to,
	OID oid_to,
	sptr_cstr avatar,
	bool arg_private,
    sptr_cstr images,
	sptr_cstr audios,
	sptr_cstr videos,
	sptr_cstr archives,
	unsigned char color,
    OID poid)
{
    std::unique_lock<std::mutex> lock(m_mutex);

	add_message_wl(arg_message, name_from, oid_from, name_to, oid_to, avatar, arg_private, images, audios, videos, archives, color, poid);
}

void room::add_message_wl(
	sptr_cstr arg_message,
    sptr_cstr name_from,
	OID oid_from,
	sptr_cstr name_to,
	OID oid_to,
	sptr_cstr avatar,
	bool arg_private,
    sptr_cstr images,
	sptr_cstr audios,
	sptr_cstr videos,
	sptr_cstr archives,
	unsigned char color,
    OID poid)
{
    // Если запрос на редактирование сообщения, надо проверить автора
    if (poid != 0) {
        for (auto mess_ptr : m_messages) {
            if (poid == mess_ptr->get_oid() && oid_from == mess_ptr->get_oid_from()) {
                // Записываем сообщение, которое в интерфейсе отредактирует старое
                sptr<message> new_mess_ptr(
                    new message(
						poid,
						mess_ptr->get_oid_from(),
						mess_ptr->get_oid_to(),
						mess_ptr->is_private(),
						mess_ptr->get_color(),
                        mess_ptr->get_avatar(),
                        mess_ptr->get_name_from(),
						mess_ptr->get_name_to(),
						arg_message,
						images,
						audios,
						videos,
						archives
					)
                );

				// Редактируем поля в старом сообщении
				*mess_ptr = *new_mess_ptr;

                m_messages.push_back(new_mess_ptr);
				singleton<dumper>::instance().save(message_to_string(*new_mess_ptr));
                break;
            }
        }
    } else {

		if (avatar->empty()) {
			auto& cfg = config();
			avatar = sptr_cstr(new std::string(oid_from == 0 ? cfg.robot_avatar : ""));
		}

        // Добавить новое сообщение
        sptr<message> new_mess_ptr(
            new message(
				poid,
				oid_from,
				oid_to,
				arg_private,
				color,
				avatar,
                name_from,
				name_to,
				arg_message,
				images,
				audios,
				videos,
				archives
			)
        );

        m_messages.push_back(new_mess_ptr);
		singleton<dumper>::instance().save(message_to_string(*new_mess_ptr));
    }

	clear_messages();
}

void room::add_message(sptr<message> mess_ptr)
{
	m_messages.push_back(mess_ptr);

	clear_messages();
}

void room::clear_messages()
{
    auto& cfg = config();
    // Не удаляем из очереди сообщения моложе заданного в конфиге количества секунд
    while ((m_messages.size() > cfg.message_amount) &&
        (util::get_gtime() > (m_messages.front()->get_time() + cfg.message_is_old))) {

        m_messages.pop_front();
    }
}

sptr<const messages_t> room::get_messages(OID user, OID last_message)
{
	std::unique_lock<std::mutex> lock(m_mutex);

    sptr<messages_t> res;

	for (auto mess_it = m_messages.rbegin(); mess_it != m_messages.rend(); ++mess_it) {
		// Добавляем только новые сообщения
		if (last_message >= (*mess_it)->get_oid()) break;

		if ((*mess_it)->is_private()) {
			// Если не авторизованный пользователь, то впринципе не может видеть приваты
			if (user == 0) continue;
			// Если пользователь не имеет отношения к сообщению, то он его не видит
			if ((*mess_it)->get_oid_from() != user &&
				(*mess_it)->get_oid_to() != user) continue;
		}
		res->push_front(*mess_it);
    }

	return res;
}

void room::user_in(sptr_cstr user_name, OID user_oid, sptr_cstr user_name_old, bool save_dump)
{
	std::unique_lock<std::mutex> lock(m_mutex);

    if (m_private && !object::check_allow(user_oid))  throw error<403>(smessage(SMESS207_ROOM_ACCESS_DENIED, *get_name()));

	auto& dm = singleton<dumper>::instance();

	if (user_name_old->empty()) {
		auto status_pr = m_users_inside.insert(std::make_pair(user_name, user_oid));
		// При считывании дампа, сообщения не пишем
		if (status_pr.second && save_dump) {
			add_message_wl(smessage(SMESS101_USER_CAME_IN, *user_name));
		}
	} else {
		m_users_inside.erase(user_name_old);
		auto status_pr = m_users_inside.insert(std::make_pair(user_name, user_oid));
		// При считывании дампа, сообщения не пишем
		if (status_pr.second && save_dump) {
			add_message_wl(smessage(SMESS108_CHANGE_NICKNAME, *user_name_old, *user_name));
		}

		// Если обрабатываем дамп, то записывать его не надо
		if (save_dump) dm.save(inside_to_string(user_oid, user_name_old, func_del));
	}
	if (save_dump) dm.save(inside_to_string(user_oid, user_name, func_add));
}

bool room::user_out(sptr_cstr user_name, sptr_cstr message, bool save_dump)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	OID user_oid = 0;

	auto it = m_users_inside.find(user_name);

	if (it != m_users_inside.end()) {
		user_oid = it->second;
		m_users_inside.erase(it);
		m_last_out = time(NULL);
	}

	// При считывании дампа, сообщения не пишем
	if (user_oid != 0 && save_dump) {
		add_message_wl(message->empty() ? smessage(SMESS102_USER_GOT_OUT, *user_name) : message);
		singleton<dumper>::instance().save(inside_to_string(user_oid, user_name, func_del));
	}

    return (user_oid != 0);
}

sptr<const users_t> room::get_users()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return sptr<const users_t>(new users_t(m_users_inside));
}

size_t room::size()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return m_users_inside.size();
}

time_t room::get_last_out() const
{
	return m_last_out;
}

bool room::is_private() const
{
	return m_private;
}

void room::add_allow(OID oid, bool save_dump)
{
	object::add_allow(oid);

	if (save_dump) {
		auto& dm = singleton<dumper>::instance();
		dm.save(allow_to_string(oid, object::get_oid(), func_add));
	}
}

void room::del_allow(OID oid, bool save_dump)
{
	object::del_allow(oid);

	if (save_dump) {
		auto& dm = singleton<dumper>::instance();
		dm.save(allow_to_string(oid, object::get_oid(), func_del));
	}
}

void room::set_bpath(sptr_cstr background_path, bool save_dump)
{
	m_background_path = background_path;

    if (save_dump) singleton<dumper>::instance().save(options_to_string());
}

void room::set_btype(int type)
{
	m_type = type;

    singleton<dumper>::instance().save(options_to_string());
}

void room::set_btransparence(int transparence)
{
	m_transparence = transparence;

    singleton<dumper>::instance().save(options_to_string());
}

void room::set_bcolor(sptr_cstr background_color)
{
	m_background_color = background_color;

    singleton<dumper>::instance().save(options_to_string());
}

void room::set_textsize(sptr_cstr textsize)
{
	m_textsize = textsize;

    singleton<dumper>::instance().save(options_to_string());
}

void room::set_description(sptr_cstr description)
{
	m_description = description;

    singleton<dumper>::instance().save(options_to_string());
}

void room::set_options(sptr_cstr background_path, int type, int transparence, sptr_cstr background_color,
	sptr_cstr textsize, sptr_cstr description, bool save_dump)
{
	m_background_path = background_path;
	m_type = type;
	m_transparence = transparence;
	m_background_color = background_color;
	m_textsize = textsize;
	m_description = description;

	if (save_dump) singleton<dumper>::instance().save(options_to_string());
}

sptr_cstr room::get_bpath()
{
	return m_background_path;
}

int room::get_btype()
{
	return m_type;
}

int room::get_btransparence()
{
	return m_transparence;
}

sptr_cstr room::get_bcolor()
{
	return m_background_color;
}

sptr_cstr room::get_textsize()
{
	return m_textsize;
}

sptr_cstr room::get_description()
{
	return m_description;
}

sptr_cstr room::to_string(const std::string& func) const
{
    sptr_str res;
    sp::stream  st(*res);

    st << "object=" << OB_room;
    st << "\t";
    st << "func=" << func;
    st << "\t";
	st << *object::to_string();
    st << "\t";
    st << "private=" << m_private;
    st << "\t";
    st << "last_out=" << m_last_out;

    return res;
}

sptr_cstr room::options_to_string() const
{
    sptr_str res;
    sp::stream  st(*res);

    st << "object=" << OB_options;
    st << "\t";
    st << "func=" << func_add; // для сохранения формата
    st << "\t";
    st << "oid=" << object::get_oid();
    st << "\t";
    st << "background_path=" << *m_background_path;
    st << "\t";
    st << "type=" << m_type;
    st << "\t";
    st << "transparence=" << m_transparence;
    st << "\t";
    st << "background_color=" << *m_background_color;
    st << "\t";
    st << "textsize=" << *m_textsize;
    st << "\t";
    st << "description=" << *m_description;

    return res;
}

sptr_cstr room::inside_to_string(OID user_oid, sptr_cstr user_name, const std::string& func) const
{   
    sptr_str res;
    sp::stream st(*res);

    st << "object=" << OB_inside;
    st << "\t";
    st << "func=" << func;
    st << "\t";
    st << "room=" << object::get_oid();
    st << "\t";
    st << "user_oid=" << user_oid;
    st << "\t";
    st << "user_name=" << *user_name;

    return res;
}

sptr_cstr room::message_to_string(const message& mess) const
{
    sptr_str res;
    sp::stream st(*res);

	st << *mess.to_string(OB_message, func_add);
    st << "\t";
    st << "room=" << object::get_oid();

    return res;
}

sptr_cstr allow_to_string(OID user, OID room, const std::string& func)
{   
    sptr_str res;
    sp::stream st(*res);

    st << "object=" << OB_allow;
    st << "\t";
    st << "func=" << func;
    st << "\t";
    st << "user=" << user;
    st << "\t";
    st << "room=" << room;

    return res;
}

room_store::room_store() {}

void room_store::create_dr()
{
	{
		auto& fr = singleton<front>::instance();

		sptr_cstr room_temp(new std::string(DEFAULT_ROOM0));
		fr.first_room = add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM1));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM2));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM3));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM4));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM5));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM6));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM7));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM8));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM9));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM10));
		add(room_temp, 0, false, true);
	}
	{
		sptr_cstr room_temp(new std::string(DEFAULT_ROOM11));
		add(room_temp, 0, false, true);
	}
}

void room_store::set_settings_one_room(OID oid, const std::string& config_value)
{
	auto vec = util::split(config_value, ",");

	if (vec->size() == 6) {
		sptr_cstr background_path(new std::string(vec->at(0)));
		int type = format<int>(vec->at(1));
		int transparence = format<int>(vec->at(2));
		sptr_cstr background_color(new std::string("#" + vec->at(3)));
		sptr_cstr textsize(new std::string(vec->at(4)));
		sptr_cstr description(new std::string(vec->at(5)));

		store::get(oid)->set_options(background_path, type, transparence, background_color, textsize, description);
	}
}

void room_store::set_settings_dr()
{
	try {
		auto& cfg = config();

		auto& fr = singleton<front>::instance();
		auto room_oid = fr.first_room;

		set_settings_one_room(room_oid, cfg.background00);
		set_settings_one_room(++room_oid, cfg.background01);
		set_settings_one_room(++room_oid, cfg.background02);
		set_settings_one_room(++room_oid, cfg.background03);
		set_settings_one_room(++room_oid, cfg.background04);
		set_settings_one_room(++room_oid, cfg.background05);
		set_settings_one_room(++room_oid, cfg.background06);
		set_settings_one_room(++room_oid, cfg.background07);
		set_settings_one_room(++room_oid, cfg.background08);
		set_settings_one_room(++room_oid, cfg.background09);
		set_settings_one_room(++room_oid, cfg.background10);
		set_settings_one_room(++room_oid, cfg.background11);

	} catch (const std::runtime_error& e) {
		lo::l(lo::ERROR) << "room_store::room_store: " << e.what();
	}
}

bool room_store::check_name_wl(sptr_cstr name) const
{   
    auto it = m_rooms_name.find(name);
    return it == m_rooms_name.end();
}

OID room_store::add(sptr_cstr name, OID owner, bool arg_private, bool save_dump)
{   
    std::unique_lock<std::mutex> lock(m_mutex);

	if (!check_name_wl(name)) throw error200_error(smessage(SMESS130_ROOMNAME_EXIST));

    sptr<room> room_new(new room(store::oid(true), name, owner, arg_private));

    auto res_pr = m_rooms_name.insert(std::make_pair(name, room_new));

    if (res_pr.second) {
		store::add(room_new);

		if (save_dump) {
			auto& dm = singleton<dumper>::instance();
			dm.save(room_new->to_string(func_add));

			room_new->add_message(smessage(SMESS100_WELCOME));
		}

        return room_new->get_oid();
    }

    return 0;
}

void room_store::del(OID oid)
{
    std::unique_lock<std::mutex> lock(m_mutex);

	auto room = store::get(oid);
	m_rooms_name.erase(room->get_name());

    singleton<dumper>::instance().save(room->to_string(func_del));

	store::del(oid);
}

size_t room_store::last_message() const
{
	return message::m_last_message;
}

void room_store::clear()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    auto& dm = singleton<dumper>::instance();

    auto& cfg = config();

    rooms_name_t m_rooms_temp;
    m_rooms_temp.swap(m_rooms_name);

    for (auto& room_pr : m_rooms_temp) {

		// Служебные комнаты инкогда не удаляем
        if (room_pr.second->get_owner() != 0 &&
			room_pr.second->size() == 0 &&
            (time(NULL) - room_pr.second->get_last_out()) > cfg.timeout_room)
		{
			store::del(room_pr.second->get_oid());

			auto allows = room_pr.second->get_allows();
            // Пишем в дамп удаление доступа для всех пользователей
            // а потом и само удаление комнаты
            for (auto user_oid : *allows) {
                dm.save(allow_to_string(user_oid, room_pr.second->get_oid(), func_del));
            }
            dm.save(room_pr.second->to_string(func_del));

        } else m_rooms_name.insert(room_pr);
    }
}

void room_store::load(sptr_cmap data)
{   
    auto object = util::get_val<std::string>(*data, "object");
    auto func = util::get_val<std::string>(*data, "func");

    if (object == OB_room) {
        sptr<room> room_ptr(new room(data));

        if (func == func_add) {
            m_rooms_name.insert(std::make_pair(room_ptr->get_name(), room_ptr));
			store::add(room_ptr);
            store::set_oid(room_ptr->get_oid() + 1);

			auto& fr = singleton<front>::instance();

			if (fr.first_room == 0) {
				fr.first_room = room_ptr->get_oid();
			} else {
				fr.first_room = std::min(fr.first_room, room_ptr->get_oid());
			}

        } else {
            m_rooms_name.erase(room_ptr->get_name());
            store::del(room_ptr->get_oid());
        }

    } else if (object == OB_allow) {

        auto user = util::get_val<OID>(*data, "user");
        auto room = util::get_val<OID>(*data, "room");

        auto room_ptr = store::get(room);

        if (func == func_add) room_ptr->add_allow(user, false);
        else room_ptr->del_allow(user, false);

    } else if (object == OB_inside) {

        auto room = util::get_val<OID>(*data, "room");
        auto user_oid = util::get_val<OID>(*data, "user_oid");
        auto user_name = sptr_cstr(new std::string(util::get_val<std::string>(*data, "user_name")));

        auto room_ptr = store::get(room);

        if (func == func_add) room_ptr->user_in(user_name, user_oid, sptr_cstr(), false);
        else room_ptr->user_out(user_name, sptr_cstr(), false);

    } else if (object == OB_options) {

        auto oid = util::get_val<OID>(*data, "oid");
		auto background_path = sptr_cstr(new std::string(util::get_val<std::string>(*data, "background_path")));
		auto type = util::get_val<int>(*data, "type");
		auto transparence = util::get_val<int>(*data, "transparence");
		auto background_color = sptr_cstr(new std::string(util::get_val<std::string>(*data, "background_color")));
		auto textsize = sptr_cstr(new std::string(util::get_val<std::string>(*data, "textsize")));
		auto description = sptr_cstr(new std::string(util::get_val<std::string>(*data, "description")));

        auto room_ptr = store::get(oid);
		room_ptr->set_options(background_path, type, transparence, background_color, textsize, description, false);

	} else if (object == OB_message) {

        auto room = util::get_val<OID>(*data, "room");

        auto room_ptr = store::get(room);
		room_ptr->add_message(sptr<message>(new message(data)));

	}
}

sptr<const rooms_name_t> room_store::get_rooms()
{
    std::unique_lock<std::mutex> lock(m_mutex);
	
	return sptr<const rooms_name_t>(new rooms_name_t(m_rooms_name));
}

} // end of namespace chatd
