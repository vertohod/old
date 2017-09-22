#include <boost/functional/hash.hpp>
#include <boost/thread/thread.hpp>
#include <functional>
#include <stdexcept>
#include <vector>
#include <memory>
#include <string>
#include <ctime>

#include "service_messages.h"
#include "stat_daemon.h"
#include "configure.h"
#include "sessions.h"
#include "message.h"
#include "archive.h"
#include "format.h"
#include "stream.h"
#include "writer.h"
#include "dumper.h"
#include "error.h"
#include "image.h"
#include "audio.h"
#include "video.h"
#include "room.h"
#include "util.h"
#include "log.h"

size_t std::hash<UUID>::operator()(const UUID& obj) const
{
	return boost::hash<UUID>()(obj);
}

size_t std::hash<sptr<const UUID>>::operator()(sptr<const UUID> obj) const
{
	return boost::hash<UUID>()(*obj);
}

size_t std::hash<sptr_cstr>::operator()(const sptr_cstr& obj) const
{
	return std::hash<std::string>()(*obj);
}

session::session() :
	m_oid(0),
	m_password(0),
	m_current_room(0),
	m_hash_personal(0),
	m_hash_image(0),
	m_hash_audio(0),
	m_hash_video(0),
	m_hash_archive(0),
	m_block(false),
	m_block_time(0),
	m_violations_amount(0)
{
	auto& cfg = config();
	m_messages.set_capacity(cfg.length_queue);

	time_t t = time(NULL);
	m_created = t;
	m_last_access = t;
}

session::session(sptr_cmap data) :
	m_password(0),
	m_current_room(0),
	m_hash_personal(0),
	m_hash_image(0),
	m_hash_audio(0),
	m_hash_video(0),
	m_hash_archive(0)
{
	auto& cfg = config();
	m_messages.set_capacity(cfg.length_queue);

	m_oid = util::get_val<OID>(*data, "oid");
	m_name = sptr_cstr(new std::string(util::get_val<std::string>(*data, "name")));

	m_created = util::get_val<size_t>(*data, "created");
	m_last_access = util::get_val<time_t>(*data, "last_access");

	m_block = util::get_val<bool>(*data, "block");
	m_block_time = util::get_val<time_t>(*data, "block_time");
	m_violations_amount = util::get_val<size_t>(*data, "violations_amount");
}

session::~session()
{
//    auto& img = singleton<chatd::image_store>::instance();
    auto& ad = singleton<chatd::audio_store>::instance();
    auto& vd = singleton<chatd::video_store>::instance();
    auto& arch = singleton<chatd::archive_store>::instance();

	try {
// FIXME
//		img.clear(get_images(), get_oid());
	} catch (...) {
		lo::l(lo::WARNING) << "Exception into session::~session, image oid: " << get_oid();
	}

	try {
		ad.clear(get_audios(), get_oid());
	} catch (...) {
		lo::l(lo::WARNING) << "Exception into session::~session, audio oid: " << get_oid();
	}

	try {
		vd.clear(get_videos(), get_oid());
	} catch (...) {
		lo::l(lo::WARNING) << "Exception into session::~session, video oid: " << get_oid();
	}

	try {
		arch.clear(get_archives(), get_oid());
	} catch (...) {
		lo::l(lo::WARNING) << "Exception into session::~session, archive oid: " << get_oid();
	}
}

sptr_cstr session::to_string(sptr<const UUID> uuid_ptr, const std::string& func) const
{
	sptr_str res;
	sp::stream st(*res);

	st << "object=" << OB_session;
	st << "\t";
	st << "func=" << func;
	st << "\t";
	st << "uuid=" << boost::uuids::to_string(*uuid_ptr);
	st << "\t";
	st << "oid=" << m_oid;
	st << "\t";
	st << "name=" << *m_name;
	st << "\t";
	st << "created=" << m_created;
	st << "\t";
	st << "last_access=" << m_last_access;
	st << "\t";
	st << "block=" << m_block;
	st << "\t";
	st << "block_time=" << m_block_time;
	st << "\t";
	st << "violations_amount=" << m_violations_amount;

	return res;
}

sptr_cstr session::change_name_to_string() const
{
	sptr_str res;
	sp::stream st(*res);

	st << "object=" << OB_name;
	st << "\t";
	st << "func=" << func_add; // для сохранения формата
	st << "\t";
	st << "oid=" << m_oid;
	st << "\t";
	st << "name=" << *m_name;

	return res;
}

sptr_cstr session::avatar_to_string() const
{
	sptr_str res;
	sp::stream st(*res);

	st << "object=" << OB_avatar;
	st << "\t";
	st << "func=" << func_add; // для сохранения формата
	st << "\t";
	st << "oid=" << m_oid;
	st << "\t";
	st << "avatar=" << m_avatar;

	return res;
}

sptr_cstr session::password_to_string() const
{
	sptr_str res;
	sp::stream st(*res);

	st << "object=" << OB_password;
	st << "\t";
	st << "func=" << func_add; // для сохранения формата
	st << "\t";
	st << "oid=" << m_oid;
	st << "\t";
	st << "password=" << m_password;

	return res;
}

sptr_cstr session::current_to_string() const
{
	sptr_str res;
	sp::stream st(*res);

	st << "object=" << OB_current_room;
	st << "\t";
	st << "func=" << func_add; // для сохранения формата
	st << "\t";
	st << "oid=" << m_oid;
	st << "\t";
	st << "room=" << m_current_room;

	return res;
}

sptr_cstr session::get_name() const
{
	return m_name;
}

OID session::get_oid() const
{
	return m_oid;
}

OID session::get_avatar() const
{
	return m_avatar;
}

time_t session::get_created() const
{
	return m_created;
}

time_t session::get_last_access() const
{
	return m_last_access;
}

size_t session::get_hash_personal() const
{
	return m_hash_personal;
}

size_t session::get_hash_image() const
{
	return m_hash_image;
}

size_t session::get_hash_audio() const
{
	return m_hash_audio;
}

size_t session::get_hash_video() const
{
	return m_hash_video;
}

size_t session::get_hash_archive() const
{
	return m_hash_archive;
}

OID session::get_current_room() const
{
	return m_current_room;
}

void session::set_name(sptr_cstr name)
{
	m_name = name;
}

void session::set_oid(OID oid)
{
	m_oid = oid;
}

session::room_mess::room_mess(OID room, size_t hash) :
	m_room(room), m_hash(hash)
{
}

session::room_mess::room_mess(const room_mess& obj)
{
	m_room = obj.m_room;
	m_hash = obj.m_hash;
}

session::room_mess session::room_mess::operator=(const room_mess& obj)
{
	m_room = obj.m_room;
	m_hash = obj.m_hash;

	return *this;
}

bool session::check_queue(OID room, sptr_cstr mess)
{
	// проверка отключена
	return true;

	std::unique_lock<std::mutex> lock(m_mutex);

	size_t hash = std::hash<std::string>()(*mess);

	for (auto rm : m_messages) {
		if (room == rm.m_room && hash == rm.m_hash) return false;
	}

	m_messages.push_front(room_mess(room, hash));

	return true;
}

void session::comein_room(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_rooms.insert(oid);
}

void session::getout_room(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_rooms.erase(oid);
}

bool session::is_anywhere()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return m_rooms.size() > 0;
}

sptr<std::unordered_set<OID>> session::get_rooms()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return sptr<std::unordered_set<OID>>(new std::unordered_set<OID>(m_rooms));
}

bool session::is_inside(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto it = m_rooms.find(oid);
	if (it != m_rooms.end()) return true;

	return false;
}

void session::clear_rooms()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_rooms.clear();
}

void session::add_image(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_images.insert(oid);
	m_hash_image = std::hash<std::string>()(format(oid) + format(time(NULL)));
}

void session::del_image(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_images.erase(oid);
	m_hash_image = std::hash<long>()(time(NULL));
}

sptr_cvec session::get_images()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr_vec res;
	for (auto oid : m_images) res->push_back(oid);

	return res;
}

void session::clear_images()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_images.clear();
}

void session::add_audio(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_audios.insert(oid);
	m_hash_audio = std::hash<std::string>()(format(oid) + format(time(NULL)));
}

void session::del_audio(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_audios.erase(oid);
	m_hash_audio = std::hash<long>()(time(NULL));
}

sptr_cvec session::get_audios()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr_vec res;
	for (auto oid : m_audios) res->push_back(oid);

	return res;
}

void session::clear_audios()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_audios.clear();
}

void session::add_video(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_videos.insert(oid);
	m_hash_video = std::hash<std::string>()(format(oid) + format(time(NULL)));
}

void session::del_video(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_videos.erase(oid);
	m_hash_video = std::hash<long>()(time(NULL));
}

sptr_cvec session::get_videos()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr_vec res;
	for (auto oid : m_videos) res->push_back(oid);

	return res;
}

void session::clear_videos()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_videos.clear();
}

void session::add_archive(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_archives.insert(oid);
	m_hash_archive = std::hash<std::string>()(format(oid) + format(time(NULL)));
}

void session::del_archive(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_archives.erase(oid);
	m_hash_archive = std::hash<long>()(time(NULL));
}

sptr_cvec session::get_archives()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr_vec res;
	for (auto oid : m_archives) res->push_back(oid);

	return res;
}

void session::clear_archives()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_archives.clear();
}

void session::set_avatar(OID image_oid, bool save_dump)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_avatar = image_oid;

	if (save_dump) {
		auto& dm = singleton<dumper>::instance();
		dm.save(avatar_to_string());
	}
}

void session::set_password(sptr_cstr password)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto password_temp = std::hash<std::string>()(*password);

	set_password_wl(password_temp);

	auto& dm = singleton<dumper>::instance();
	dm.save(password_to_string());
}

void session::set_password_wl(size_t password)
{
	m_password = password;
}

bool session::check_password(sptr_cstr password)
{
	std::unique_lock<std::mutex> lock(m_mutex);

    return m_password == std::hash<std::string>()(*password);
}

void session::clear_password()
{
	std::unique_lock<std::mutex> lock(m_mutex);

    m_password = 0;

	auto& dm = singleton<dumper>::instance();
	dm.save(password_to_string());
}

void session::set_current_room(OID room)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_oid != 0 && m_current_room != room) {

		set_current_room_wl(room);

		auto& dm = singleton<dumper>::instance();
		dm.save(current_to_string());
	}
}

void session::set_current_room_wl(OID room)
{
	m_current_room = room;
}

void session::add_message(OID user, sptr<chatd::message> mess_ptr)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	add_message_wl(user, mess_ptr);
}

void session::add_message_wl(OID user, sptr<chatd::message> mess_ptr)
{
	m_personal[user].push_back(mess_ptr);		

	m_hash_personal = std::hash<std::string>()(*mess_ptr->get_raw() + format(time(NULL)));
}

void session::del_message(OID message_oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	del_message_wl(message_oid);
}

void session::del_message_wl(OID message_oid, bool save_dump)
{
	for (auto& list_pr : m_personal) {
		for (auto it = list_pr.second.begin(); it != list_pr.second.end(); ++it) {
			if ((*it)->get_oid() == message_oid) {
				list_pr.second.erase(it);	

				if (save_dump) {
					auto& dm = singleton<dumper>::instance();
					dm.save(personal_delete((*it)->get_oid()));
				}

				if (list_pr.second.size() == 0) m_personal.erase(list_pr.first);
				return;
			}
		}
	}
}

sptr_cstr session::personal_delete(OID message)
{
    sptr_str res;
    sp::stream st(*res);

    st << "object=" << OB_personal;
    st << "\t";
    st << "func=" << func_del;
    st << "\t";
    st << "user=" << get_oid();
    st << "\t";
    st << "message=" << message;

    return res;
}

sptr<const personal_t> session::get_personal()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr<personal_t> res(new personal_t());

	for (auto& pr : m_personal) {

		auto it = pr.second.end();
		for (int count = 50; count > 0 && it != pr.second.begin(); --count, --it) {}

		res->insert(std::make_pair(pr.first, list_message_t(it, pr.second.end())));
	}

	return res;
}

sptr<const UUID> session::gen(const std::string& arg_uuid_str)
{
	return sptr<const UUID>(new UUID(boost::uuids::string_generator()(arg_uuid_str)));
}

sptr<const UUID> session::gen_random()
{
	return sptr<const UUID>(new UUID(boost::uuids::random_generator()()));
}

void session::add_info(sptr_cstr mess)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_sysmess_info.push(mess);
}

void session::add_error(sptr_cstr mess)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_sysmess_error.push(mess);
}

sptr_cstr session::get_info()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_sysmess_info.size() > 0) {

		auto res = m_sysmess_info.front();
		m_sysmess_info.pop();

		return res;
	}

	return sptr_cstr();
}

sptr_cstr session::get_error()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_sysmess_error.size() > 0) {

		auto res = m_sysmess_error.front();
		m_sysmess_error.pop();

		return res;
	}

	return sptr_cstr();
}

// methods of class sessions
sessions::sessions()
{
	boost::thread thread(boost::bind(&sessions::thread_function, this));
	m_thread.swap(thread);
}

sptr<session> sessions::get(sptr<const UUID>& uuid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return get_wl(uuid);
}

sptr<session> sessions::get_wl(sptr<const UUID>& uuid)
{
	if (!uuid) return create(uuid);

	sess_size::s(m_sessions_uuid.size());

	auto it = m_sessions_uuid.find(uuid);
	if (it == m_sessions_uuid.end()) return create(uuid);

	it->second->m_last_access = time(NULL);

	return it->second;
}

sptr<session> sessions::get(OID user)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return get_wl(user);
}

sptr<session> sessions::get_wl(OID user)
{
	auto it = m_sessions_oid.find(user);
	if (it == m_sessions_oid.end()) throw error<404>(smessage(SMESS206_USER_NOT_FOUND));
	return it->second;
}

sptr<session> sessions::create(sptr<const UUID>& uuid)
{
	while (true) {
		uuid = session::gen_random();
		auto it = m_sessions_uuid.find(uuid);
		if (it == m_sessions_uuid.end()) break;
	}

	auto ss = sptr<session>(new session());
	auto it_ins = m_sessions_uuid.insert(std::make_pair(uuid, ss));
	if (!it_ins.second) throw error<500>(smessage(SMESS219_ADD_USER_UNABLE));

	sess_size::s(m_sessions_uuid.size());

	return it_ins.first->second;
}

bool sessions::set_name(sptr<const UUID> uuid_ptr, sptr<session> sess_ptr, sptr_cstr name)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return set_name_wl(uuid_ptr, sess_ptr, name);
}

void sessions::del_session(sptr<session> sess_ptr)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	del_name_wl(sess_ptr);
}

void sessions::del_name_wl(sptr<session> sess_ptr)
{
	m_sessions_name.erase(sess_ptr->get_name());
	sess_ptr->set_name();

	// Записываем в лог пустой ник
	auto& dm = singleton<dumper>::instance();
	dm.save(sess_ptr->change_name_to_string());
}

bool sessions::set_name_wl(sptr<const UUID> uuid_ptr, sptr<session> sess_ptr, sptr_cstr name)
{
	if (name->empty()) del_name_wl(sess_ptr); 
	else {
		// Проверяем, нет ли уже такого имени в системе		
		{
			auto it = m_sessions_name.find(name);
			if (it != m_sessions_name.end()) return false;
		}

		// Удаляем старое имя
		m_sessions_name.erase(sess_ptr->get_name());
		// Устанавливаем и добавляем новое имя
		sess_ptr->set_name(name);
		m_sessions_name.insert(std::make_pair(sess_ptr->get_name(), sess_ptr));

		auto& dm = singleton<dumper>::instance();

		if (sess_ptr->get_oid() == 0) {

			OID oid_temp;

			// Стандартная процедура для подбора уникального OID
			while (true) {
				oid_temp = std::hash<std::string>()(*name + format(time(NULL)));
				oid_temp = oid_temp & 0xFFFFFFFFFFFFF;

				auto it = m_sessions_oid.find(sess_ptr->get_oid());
				if (it == m_sessions_oid.end()) break;
			}

			sess_ptr->set_oid(oid_temp);
			m_sessions_oid.insert(std::make_pair(sess_ptr->get_oid(), sess_ptr));

			// Считаем, что это новая сессия и пишем ее целиком
			dm.save(sess_ptr->to_string(uuid_ptr));

		} else {

			// Записываем только новое имя
			dm.save(sess_ptr->change_name_to_string());

		}
	}

	return true;
}

void sessions::add_message(sptr<session> sess_from_ptr, sptr_cstr arg_message, sptr_cstr name_from, OID oid_from, OID oid_to,
	sptr_cstr avatar, sptr_cstr images, sptr_cstr audios, sptr_cstr videos, sptr_cstr archives, unsigned char color)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	// Если сессия не найдена, вылетит по исключению c 404
	auto sess_to_ptr = get_wl(oid_to);
	auto name_to = sess_to_ptr->get_name();

	if (avatar->empty()) {
		auto& cfg = config();
		avatar = sptr_cstr(new std::string(oid_from == 0 ? cfg.robot_avatar : ""));
	}

	sptr<chatd::message> mess_ptr(new chatd::message(
		0, //  poid = 0
		oid_from,
		oid_to,
		true, // всегда private в нулевую комнату
		color,
		avatar,
		name_from,
		name_to,
		arg_message,
		images,
		audios,
		videos,
		archives
	));

	sess_from_ptr->add_message(oid_to, mess_ptr);
	sess_to_ptr->add_message(oid_from, mess_ptr);

	singleton<dumper>::instance().save(mess_ptr->to_string(OB_personal, func_add));
}

void sessions::add_message_one_user(sptr<session> sess_to_ptr, sptr_cstr arg_message, sptr_cstr images, sptr_cstr audios, sptr_cstr videos, sptr_cstr archives, unsigned char color)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto& cfg = config();
	sptr_cstr avatar = sptr_cstr(new std::string(cfg.robot_avatar));

	sptr<chatd::message> mess_ptr(new chatd::message(
		0, // poid = 0
		0, // oid_from - системное сообщение
        sess_to_ptr->get_oid(),
		true,
		color,
		avatar,
		sptr_cstr(new std::string(DEFAULT_USER)),
		sess_to_ptr->get_name(),
		arg_message,
		images,
		audios,
		videos,
		archives
	));

	sess_to_ptr->add_message(0, mess_ptr);

	// Записываем только уведомительные сообщения в личку от робота
	singleton<dumper>::instance().save(mess_ptr->to_string(OB_personal, func_add));
}

sptr<session> sessions::add_alias(sptr<const UUID> uuid_ptr, sptr_cstr user_name, sptr_cstr password)
{
	std::unique_lock<std::mutex> lock(m_mutex);

    auto sess_it = m_sessions_name.find(user_name);
    if (sess_it != m_sessions_name.end()) {
        if (sess_it->second->check_password(password)) {
			auto current_it = m_sessions_uuid.find(uuid_ptr);
			if (current_it == m_sessions_uuid.end()) throw error<500>(smessage(SMESS206_USER_NOT_FOUND));
			// Сначала запишем в дамп, потом сделаем замены
			auto& dm = singleton<dumper>::instance();
			dm.save(alias_to_string(uuid_ptr, sess_it->second->get_oid()));

			// Заменили сессию
			current_it->second = sess_it->second;
			return sess_it->second;

        } else throw error200_error(smessage(SMESS116_PASSWORD_WRONG));
    } else throw error200_info(smessage(SMESS115_NICKNAME_IS_FREE, *user_name));

	return sptr<session>();
}

sptr<session> sessions::remove_alias(sptr<const UUID> uuid_ptr)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto current_it = m_sessions_uuid.find(uuid_ptr);
	if (current_it == m_sessions_uuid.end()) throw error<500>(smessage(SMESS212_SESSION_NOT_FOUND));

	// Пишем дамп
	auto& dm = singleton<dumper>::instance();
	dm.save(alias_to_string(uuid_ptr, current_it->second->get_oid(), func_del));

	current_it->second = sptr<session>();

	return current_it->second;
}

void sessions::load(sptr_cmap data)
{
	auto object = util::get_val<std::string>(*data, "object");
	auto func = util::get_val<std::string>(*data, "func");

	if (object == OB_session) {
	
		if (func == func_add) {
			auto uuid_str = util::get_val<std::string>(*data, "uuid");
			auto uuid_ptr = session::gen(uuid_str);

			sptr<session> sess_ptr(new session(data));

			m_sessions_uuid.insert(std::make_pair(uuid_ptr, sess_ptr));
			m_sessions_name.insert(std::make_pair(sess_ptr->get_name(), sess_ptr));
			m_sessions_oid.insert(std::make_pair(sess_ptr->get_oid(), sess_ptr));

			sess_size::s(m_sessions_uuid.size());
		} else {
			auto oid = util::get_val<OID>(*data, "oid");
			auto sess_ptr = get_wl(oid);

			// При удалении сессии (logout) удаляем только имя
			m_sessions_name.erase(sess_ptr->get_name());
			sess_ptr->set_name();
		}

	} else if (object == OB_alias) {

		auto alias = util::get_val<std::string>(*data, "uuid");
		auto uuid_alias_ptr = session::gen(alias);

		if (func == func_add) {

			auto oid = util::get_val<OID>(*data, "oid");

			auto sess_ptr = get_wl(oid);

			auto it = m_sessions_uuid.find(uuid_alias_ptr);
			if (it == m_sessions_uuid.end()) {
				m_sessions_uuid.insert(std::make_pair(uuid_alias_ptr, sess_ptr));
			} else {
				it->second = sess_ptr;
			}

			sess_size::s(m_sessions_uuid.size());

		} else {

			auto it = m_sessions_uuid.find(uuid_alias_ptr);
			if (it != m_sessions_uuid.end()) it->second = sptr<session>();

		}

	} else if (object == OB_personal) {

		if (func == func_add) {
			sptr<chatd::message> mess_ptr(new chatd::message(data));

			if (mess_ptr->get_oid_to()) {
				auto sess_ptr = get_wl(mess_ptr->get_oid_to());
				sess_ptr->add_message_wl(mess_ptr->get_oid_from(), mess_ptr);
			}
			if (mess_ptr->get_oid_from()) {
				auto sess_ptr = get_wl(mess_ptr->get_oid_from());
				sess_ptr->add_message_wl(mess_ptr->get_oid_to(), mess_ptr);
			}
		} else {
			auto user_oid = util::get_val<OID>(*data, "user");
			auto message_oid = util::get_val<OID>(*data, "message");
			auto sess_ptr = get_wl(user_oid);

			sess_ptr->del_message_wl(message_oid, false);
		}

	} else if (object == OB_name) {

		auto oid = util::get_val<OID>(*data, "oid");
		auto name = sptr_cstr(new std::string(util::get_val<std::string>(*data, "name")));

		auto sess_ptr = get_wl(oid);

		m_sessions_name.erase(sess_ptr->get_name());
		sess_ptr->set_name(name);
		m_sessions_name.insert(std::make_pair(sess_ptr->get_name(), sess_ptr));

	} else if (object == OB_inside) {

        auto room = util::get_val<OID>(*data, "room");
        auto user_oid = util::get_val<OID>(*data, "user_oid");

		auto sess_ptr = get_wl(user_oid);

        if (func == func_add) sess_ptr->comein_room(room);
        else sess_ptr->getout_room(room);

	} else if (object == OB_password) {

        auto oid = util::get_val<OID>(*data, "oid");
        auto password = util::get_val<size_t>(*data, "password");

		auto sess_ptr = get_wl(oid);
        sess_ptr->set_password_wl(password);

	} else if (object == OB_audio) {

        auto owner = util::get_val<OID>(*data, "owner");
        auto oid = util::get_val<OID>(*data, "oid");

		auto sess_ptr = get_wl(owner);
        if (func == func_add) sess_ptr->add_audio(oid);
        else sess_ptr->del_audio(oid);

	} else if (object == OB_video) {

        auto owner = util::get_val<OID>(*data, "owner");
        auto oid = util::get_val<OID>(*data, "oid");

		auto sess_ptr = get_wl(owner);
        if (func == func_add) sess_ptr->add_video(oid);
        else sess_ptr->del_video(oid);

	} else if (object == OB_archive) {

        auto owner = util::get_val<OID>(*data, "owner");
        auto oid = util::get_val<OID>(*data, "oid");

		auto sess_ptr = get_wl(owner);
        if (func == func_add) sess_ptr->add_archive(oid);
        else sess_ptr->del_archive(oid);

	} else if (object == OB_image) {

        auto owner = util::get_val<OID>(*data, "owner");
        auto oid = util::get_val<OID>(*data, "oid");

		auto sess_ptr = get_wl(owner);
        if (func == func_add) sess_ptr->add_image(oid);
        else sess_ptr->del_image(oid);

	} else if (object == OB_avatar) {

        auto oid = util::get_val<OID>(*data, "oid");
        auto avatar = util::get_val<OID>(*data, "avatar");

		auto sess_ptr = get_wl(oid);
        if (func == func_add) sess_ptr->set_avatar(avatar, false);

	} else if (object == OB_current_room) {

        auto oid = util::get_val<OID>(*data, "oid");
        auto room = util::get_val<OID>(*data, "room");

		auto sess_ptr = get_wl(oid);
        sess_ptr->set_current_room_wl(room);

	}
}

sptr_cstr sessions::alias_to_string(sptr<const UUID> alias, OID oid, const std::string& func)
{
	sptr_str res;
	sp::stream st(*res);

	st << "object=" << OB_alias;
	st << "\t";
	st << "func=" << func;
	st << "\t";
	st << "uuid=" << boost::uuids::to_string(*alias);
	st << "\t";
	st << "oid=" << oid;

	return res;
}

/* это надо делать другой функцией
		if (time(NULL) - sess_ptr->get_last_access() > cfg.timeout_user_inside) {
			auto rooms = sess_ptr->get_rooms();
			for (auto room_oid : *rooms) {
				rm.get(room_oid)->user_out(
					sess_ptr->get_name(),
					smessage(SMESS103_LEFT_WITHOUT_GB, *sess_ptr->get_name())
				);
			}
			sess_ptr->clear_rooms();
		}


			auto& rm = singleton<chatd::room_store>::instance();
			rm.clear();

			auto& wr = singleton<chatd::writers>::instance();
			wr.clear();

*/

void sessions::thread_function()
{
	auto it1 = m_sessions_uuid.begin();
	auto it2 = m_sessions_oid.begin();

    while (true) {

		size_t frequency = m_sessions_uuid.size() / ITERATION_MIN;
		if (frequency * ITERATION_MIN < m_sessions_uuid.size()) ++frequency;

        util::sleep(DAY_MSECONDS / frequency);

        try {

			std::unique_lock<std::mutex> lock(m_mutex);

			it1 = clear(m_sessions_uuid, it1, frequency);
			it2 = clear(m_sessions_oid, it2, frequency);

        } catch (std::exception& e) {

            lo::l(lo::ERROR) << "something happend in thread for clearing: " <<  e.what();

        }
    }
}
