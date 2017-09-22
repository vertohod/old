#include <boost/thread/detail/singleton.hpp>
#include <boost/regex.hpp>
#include <functional>
#include <algorithm>
#include <fstream>

#include "sys/stat.h"
#include "unistd.h"

#include "service_messages.h"
#include "serialization.h"
#include "stat_daemon.h"
#include "configure.h"
#include "alphabet.h"
#include "service.h"
#include "message.h"
#include "archive.h"
#include "writer.h"
#include "answer.h"
#include "invite.h"
#include "avatar.h"
#include "smile.h"
#include "types.h"
#include "image.h"
#include "audio.h"
#include "video.h"
#include "error.h"
#include "front.h"
#include "room.h"
#include "util.h"
#include "log.h"

using namespace boost::detail::thread;

service::service(net::boost_socket_ptr socket) : net::http_server(socket) {}

void service::handle_process(const request& req)
{
	context cnt;

	try {
		if (req.method() == OPTIONS) {
			cnt.resp.add_header("Allow", "GET, POST, HEAD, OPTIONS");
			throw error<200>(sptr_cstr());
		}
		if (req.method() != GET && req.method() != POST && req.method() != HEAD) {
			throw error<501>(http_proto::method_to_str(req.method()));
		}

		fill_context(req, cnt);

		auto& wr = singleton<chatd::writers>::instance();
		if (cnt.typing) {
			wr.add_writer(cnt.sess_ptr->get_oid(), cnt.sess_ptr->get_name(), cnt.room);
		} else {
			wr.del_writer(cnt.sess_ptr->get_name());
		}

		const std::string& path = *req.path();

		if (path == "/" || path == "/view.json") {
			view_tick::t();
		} else if (path == "/send.json") {
			send_interval::start();
			send(cnt);
		} else if (path == "/login.json") {
			login(cnt);
		} else if (path == "/logout.json") {
			logout(cnt);
		} else if (path == "/comein.json") {
			comein(cnt, cnt.room);
		} else if (path == "/getout.json") {
			getout(cnt);
		} else if (path == "/change.json") {
			change(cnt);
		} else if (path == "/upload.json") {
			upload(req, cnt);
		} else if (path == "/remove.json") {
			remove(req, cnt);
		} else if (path == "/image.json") {
			image_tick::t();
			image(req, cnt);
		} else if (path == "/audio.json") {
			audio_tick::t();
			audio(req, cnt);
		} else if (path == "/video.json") {
			video(req, cnt);
		} else if (path == "/archive.json") {
			archive(req, cnt);
		} else if (path == "/personal.json") {
			personal(cnt);
		} else if (path == "/add.json") {
			add(req, cnt);
		} else if (path == "/activate.json") {
			activate(req);
		} else if (path == "/settings.json") {
			settings(req, cnt);
		} else if (path == "/smiles.json") {
			auto& cfg = config();
			auto& smile = singleton<smile_store>::instance();
			throw error<200>(smile.to_json(cfg.dir_smiles_short));
		} else if (path == "/avatars.json") {
			auto& cfg = config();
			auto& avatar = singleton<avatar_store>::instance();
			throw error<200>(avatar.to_json(cfg.dir_avatars_short));
		} else if (path == "/invite.json") {
			invite(req, cnt);
		} else {
			unknown_tick::t();
			throw error<204>(sptr_cstr(new std::string("Was asked unknown file: " + path)));
		}

		view_interval::start();
		generate_answer(cnt);
		view_interval::finish();

	} catch (const error<200>& e) {
		cnt.resp.set_status(S200);
		cnt.resp.data()->assign(e.what());
		if (!cnt.resp.data()->empty()) {
			cnt.resp.add_header("Content-type", "application/json; charset=utf-8");
		}
		lo::l(lo::INFO) << "answer 200: " <<  e.what();
	} catch (const error200_info& e) {
        cnt.sess_ptr->add_info(sptr_cstr(new std::string(e.what())));
		generate_short_answer(cnt);
		lo::l(lo::INFO) << "answer 200: " <<  e.what();
	} catch (const error200_error& e) {
        cnt.sess_ptr->add_error(sptr_cstr(new std::string(e.what())));
		generate_short_answer(cnt);
		lo::l(lo::INFO) << "answer 200: " <<  e.what();
	} catch (const error200_personal& e) {
		cnt.resp.set_status(S200);
		cnt.resp.add_header("Content-Type", "application/json; charset=utf-8");
		lo::l(lo::INFO) << "answer 200: " <<  e.what();
	} catch (const redirect_image& e) {
		cnt.resp.set_status(S200);
		cnt.resp.add_header("X-Accel-Redirect", e.what());
		cnt.resp.add_header("Content-Type", e.content_type());
		lo::l(lo::INFO) << "make redirect to: " << e.what();
	} catch (const redirect_empty& e) {
		cnt.resp.set_status(S200);
		cnt.resp.add_header("X-Accel-Redirect", e.what());
		cnt.resp.add_header("Content-Type", "image/png");
		lo::l(lo::INFO) << "make redirect to: " << e.what();
	} catch (const redirect_media& e) {
		cnt.resp.set_status(S200);
		cnt.resp.add_header("X-Accel-Redirect", e.what());
		cnt.resp.add_header("Content-Type", e.content_type());
		lo::l(lo::INFO) << "make redirect to: " << e.what();
	} catch (const error<204>& e) {
		cnt.resp.set_status(S204);
		lo::l(lo::WARNING) << "answer 204: " <<  e.what();
	} catch (const error<302>& e) {
		cnt.resp.set_status(S302);
		cnt.resp.add_header("Location", e.what());
		lo::l(lo::WARNING) << "answer 302: " <<  e.what();
	} catch (const error<403>& e) {
		cnt.resp.set_status(S403);
		lo::l(lo::ERROR) << "answer 403: " << e.what();
	} catch (const error<404>& e) {
		cnt.resp.set_status(S404);
		lo::l(lo::ERROR) << "answer 404: " << e.what();
	} catch (const error<405>& e) {
		cnt.resp.set_status(S405);
        cnt.resp.add_header("Allow", "GET, POST, HEAD, OPTIONS");
		lo::l(lo::ERROR) << "answer 405: " << e.what();
	} catch (const error<500>& e) {
		cnt.resp.set_status(S500);
		lo::l(lo::ERROR) << "answer 500: " << e.what();
	} catch (const error<501>& e) {
		cnt.resp.set_status(S501);
        cnt.resp.add_header("Allow", "GET, POST, HEAD, OPTIONS");
		lo::l(lo::ERROR) << "answer 501: " << e.what();
	} catch (const std::exception& e) {
		cnt.resp.set_status(S500);
		lo::l(lo::ERROR) << "answer 500: " << e.what();
	}

	send_interval::finish();

	cnt.resp.set_ver(req.ver());
	http_server::send(*cnt.resp.to_raw(req.method() != HEAD));
}

void service::fill_context(const request& req, context& cnt)
{
	std::string uuid_str = req.cookie("UUID", std::string());
	if (!uuid_str.empty()) {
		lo::l(lo::TRASH) << "Received UUID: " << uuid_str;
		cnt.uuid_ptr = session::gen(uuid_str);
	} else lo::l(lo::TRASH) << "Received empty UUID";

	// Пустой uuid не будет найден, и будет создан новый
	cnt.sess_ptr = singleton<sessions>::instance().get(cnt.uuid_ptr);

	// Если в GET и POST не указана комната используется последняя из сессии
	// если же и там 0, то устанавливается первая существующая
	{
		auto& fr = singleton<front>::instance();

		cnt.room = std::max(req.get<OID>("room", 0), req.post("room")->val<OID>());
		if (cnt.room == 0) cnt.room = cnt.sess_ptr->get_current_room();
		if (cnt.room == 0) cnt.room = fr.first_room;
		cnt.sess_ptr->set_current_room(cnt.room);
	}

	cnt.last_mess = std::max(req.get<OID>("last_message", 0), req.post("last_message")->val<OID>());

	cnt.typing = req.get<OID>("typing", 0);

	cnt.to = req.post("to")->val<OID>();
	cnt.poid = req.post("poid")->val<OID>();
	cnt.priv = req.post("private")->val<int>();
	cnt.personal = req.post("personal")->val<int>();
	cnt.color = req.post("color")->val<unsigned char>();
	cnt.room_old = req.post("room_old")->val<OID>();
	cnt.room_new = req.post("room_new")->val<OID>();

	cnt.answer_obj_mask = std::max(req.get<int>("mask", 0), req.post("mask")->val<int>());

	cnt.mess = req.post("message")->val<sptr_cstr>();
	cnt.images = req.post("images")->val<sptr_cstr>();
	cnt.audios = req.post("audios")->val<sptr_cstr>();
	cnt.videos = req.post("videos")->val<sptr_cstr>();
	cnt.archives = req.post("archives")->val<sptr_cstr>();
	cnt.nick.reset(new std::string(util::trim(*req.post("nickname")->val<sptr_cstr>())));

	lo::l(lo::TRASH) << "Received typing: " << cnt.typing;
	lo::l(lo::TRASH) << "Received to: " << cnt.to;
	lo::l(lo::TRASH) << "Received poid: " << cnt.poid;
	lo::l(lo::TRASH) << "Received priv: " << cnt.priv;
	lo::l(lo::TRASH) << "Received personal: " << cnt.personal;
	lo::l(lo::TRASH) << "Received color: " << static_cast<int>(cnt.color);
	lo::l(lo::TRASH) << "Received room_old: " << cnt.room_old;
	lo::l(lo::TRASH) << "Received room_new: " << cnt.room_new;
	lo::l(lo::TRASH) << "Received mess: " << *cnt.mess;
	lo::l(lo::TRASH) << "Received images: " << *cnt.images;
	lo::l(lo::TRASH) << "Received audios: " << *cnt.audios;
	lo::l(lo::TRASH) << "Received videos: " << *cnt.videos;
	lo::l(lo::TRASH) << "Received archives: " << *cnt.archives;
	lo::l(lo::TRASH) << "Received nick: " << *cnt.nick;

	try {

		auto& cfg = config();
		auto& avatar = singleton<avatar_store>::instance();

		if (cnt.sess_ptr->get_avatar() > 0 && cnt.sess_ptr->get_avatar() <= avatar.size()) {

			cnt.avatar = sptr_cstr(new std::string(cfg.dir_avatars_short + avatar.get(cnt.sess_ptr->get_avatar())));

		} else {
//FIXME
//			auto& image_store = singleton<chatd::image_store>::instance();
//			auto image_ptr = image_store.get(cnt.sess_ptr->get_avatar());
//			cnt.avatar = sptr_cstr(new std::string(cfg.path_image_web + "m" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));

		}

	} catch (...) {}
}

void service::login(context& cnt)
{
	static boost::regex reg(std::string("^[") + ALPH_ALL + "]*$");

	auto& sess = singleton<sessions>::instance();
	auto& rm = singleton<chatd::room_store>::instance();

	auto& cfg = config();

	size_t nickname_length = util::length_utf8(*cnt.nick);
	if (nickname_length >= cfg.nickname_min && nickname_length <= cfg.nickname_max) {

		if (!boost::regex_match(*cnt.nick, reg)) {
			cnt.sess_ptr->add_error(smessage(SMESS111_INVALID_CHARACTERS));
			return;
		}

		sptr_cstr nickname_old = cnt.sess_ptr->get_name();

		if (!sess.set_name(cnt.uuid_ptr, cnt.sess_ptr, cnt.nick)) {
			cnt.sess_ptr->add_error(smessage(SMESS105_NICKNAME_EXIST));
			return;
		}

		auto rooms = cnt.sess_ptr->get_rooms();

		if (rooms->size() == 0) {
			rm.get(cnt.room)->user_in(cnt.sess_ptr->get_name(), cnt.sess_ptr->get_oid(), nickname_old);
			cnt.sess_ptr->comein_room(cnt.room);
		} else {
			// Если пользователь переименовался, надо отпавить сообщение во все комнаты в которых он находится
			for (auto room_oid : *rooms) {
				rm.get(room_oid)->user_in(cnt.sess_ptr->get_name(), cnt.sess_ptr->get_oid(), nickname_old);
			}	
		}

	} else cnt.sess_ptr->add_error(smessage(SMESS104_NICKNAME_WRONG, format(cfg.nickname_min), format(cfg.nickname_max)));
}

void service::comein(context& cnt, OID room)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& rm = singleton<chatd::room_store>::instance();

	if (!cnt.sess_ptr->is_inside(room)) {
		rm.get(room)->user_in(cnt.sess_ptr->get_name(), cnt.sess_ptr->get_oid());
		cnt.sess_ptr->comein_room(room);

		cnt.answer_obj_mask |= SHOW_WHOLE_ROOM;
	}
}

void service::getout(context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& rm = singleton<chatd::room_store>::instance();

	if (cnt.sess_ptr->is_inside(cnt.room)) {
		rm.get(cnt.room)->user_out(cnt.sess_ptr->get_name());
		cnt.sess_ptr->getout_room(cnt.room);
	}
}

void service::logout(context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& sess = singleton<sessions>::instance();
	auto& rm = singleton<chatd::room_store>::instance();

	auto rooms = cnt.sess_ptr->get_rooms();
	for (auto room_oid : *rooms) {
		rm.get(room_oid)->user_out(cnt.sess_ptr->get_name());
	}
	cnt.sess_ptr->clear_rooms();

	// НЕ удаляем все загруженные картинки пользователя
	// они удалятся при истечении времени хранения сессии

	// ... после этого разавторизовываем его
	sess.del_session(cnt.sess_ptr);
}

void service::send(context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	if (check_message(cnt)) {

		if (cnt.personal) {
			auto& sess = singleton<sessions>::instance();
			sess.add_message(
				cnt.sess_ptr,
				cnt.mess,
				cnt.sess_ptr->get_name(),
				cnt.sess_ptr->get_oid(),
				cnt.to,
				cnt.avatar,
				cnt.images,
				cnt.audios,
				cnt.videos,
				cnt.archives,
				cnt.color
			);

			// Сообщение для разработчиков, пишем ответ
			if (cnt.to == 0) {
				sess.add_message_one_user(cnt.sess_ptr, smessage(SMESS118_THANKS_FOR_MESSAGE), sptr_cstr(), sptr_cstr(), sptr_cstr(), sptr_cstr(), 0);
			}
		} else {
			if (!cnt.sess_ptr->is_inside(cnt.room)) {
				cnt.sess_ptr->add_error(smessage(SMESS202_USER_NOT_INSIDE));
				return;
			}

			// Заполняем поле name_to если надо/возможно
			sptr_cstr name_to;
			if (cnt.priv) {
				if (cnt.to != 0) {
					auto& sess = singleton<sessions>::instance();
					auto sess_ptr = sess.get(cnt.to);
					name_to = sess_ptr->get_name();
				} else if (cnt.priv) name_to = sptr_cstr(new std::string(DEFAULT_USER));
			} else {
				// Исправим багу интерфейса
				cnt.to = 0;
			}
			// ----------------------------------------

			auto& rm = singleton<chatd::room_store>::instance();
			rm.get(cnt.room)->add_message(
				cnt.mess,
				cnt.sess_ptr->get_name(),
				cnt.sess_ptr->get_oid(),
				name_to,
				cnt.to,
				cnt.avatar,
				cnt.priv,
				cnt.images,
				cnt.audios,
				cnt.videos,
				cnt.archives,
				cnt.color,
				cnt.poid);

			// Сообщение для разработчиков, пишем ответ
			if (cnt.priv && cnt.to == 0) {
				rm.get(cnt.room)->add_message(
					smessage(SMESS118_THANKS_FOR_MESSAGE),
					sptr_cstr(new std::string(DEFAULT_USER)),
					0,
					cnt.sess_ptr->get_name(),
					cnt.sess_ptr->get_oid(),
					sptr_cstr(),
					true,
					sptr_cstr(), // images
					sptr_cstr(), // audios
					sptr_cstr(), // videos
					sptr_cstr(), // archives
					0,  // color
					0); // poid
			}
		}

		// Сообщение для разработчиков
		if ((cnt.priv || cnt.personal) && cnt.to == 0) {
			std::ofstream ofs;
			auto& cfg = config();
			ofs.open(cfg.path_messages.c_str(), std::ofstream::out | std::ofstream::app);
			ofs << lo::get_time() << " (" << cnt.sess_ptr->get_oid() << ") " << *cnt.sess_ptr->get_name() << ": " << *cnt.mess << "\n";
			ofs.close();
		}
	}
}

bool service::check_message(context& cnt)
{
	auto& cfg = config();

	if ((!cnt.images->empty() || !cnt.audios->empty() || !cnt.videos->empty() || !cnt.archives->empty() || !cnt.mess->empty() || cnt.poid != 0) &&
		util::length_utf8(*cnt.mess) <= cfg.message_length_max) {

		sptr_cstr str_for_test(new std::string(*cnt.mess + *cnt.images + *cnt.audios + *cnt.videos + *cnt.archives));

		if ((cnt.mess->empty() && cnt.poid != 0) || cnt.sess_ptr->check_queue(cnt.room, str_for_test)) {
			return true;
		} else cnt.sess_ptr->add_error(smessage(SMESS109_MESSAGE_FLOOD));
	} else cnt.sess_ptr->add_error(smessage(SMESS107_MESSAGE_WRONG, format(cfg.message_length_max)));

	return false;
}

void service::change(context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& rm = singleton<chatd::room_store>::instance();

	if (cnt.room_old != cnt.room_new) {
		cnt.room = cnt.room_new;

		if (cnt.sess_ptr->is_inside(cnt.room_old)) {
			// сообщение в старую комнату о переходе в новую
			sptr_str mess_temp;

			*mess_temp = *smessage(SMESS110_CHANGED_ROOM, *cnt.sess_ptr->get_name(), *rm.get(cnt.room)->get_name());

			rm.get(cnt.room_old)->user_out(cnt.sess_ptr->get_name(), mess_temp);
			cnt.sess_ptr->getout_room(cnt.room_old);
		}

		rm.get(cnt.room)->user_in(cnt.sess_ptr->get_name(), cnt.sess_ptr->get_oid());
		cnt.sess_ptr->comein_room(cnt.room);

		cnt.answer_obj_mask |= SHOW_WHOLE_ROOM;
	}
}

void service::add_room(const request& req, context& cnt)
{
	static boost::regex reg(std::string("^[") + ALPH_ALL + "]*$");

	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& rm = singleton<chatd::room_store>::instance();
	auto& cfg = config();

	sptr_cstr name(new std::string(util::trim(*req.post("name")->val<sptr_cstr>())));

	size_t name_length = util::length_utf8(*name);
	if (name_length >= cfg.roomname_min && name_length <= cfg.roomname_max) {

		if (!boost::regex_match(*name, reg)) throw error200_error(smessage(SMESS111_INVALID_CHARACTERS));

		auto oid_room_new = rm.add(name, cnt.sess_ptr->get_oid(), cnt.priv);
		comein(cnt, oid_room_new);

		cnt.answer_obj_mask |= SHOW_ROOMS;

		cnt.sess_ptr->add_info(smessage(SMESS124_ROOM_ADDED));

	} else throw error200_error(smessage(SMESS106_ROOMNAME_WRONG, format(cfg.roomname_min), format(cfg.roomname_max)));
}

void service::upload(const request& req, context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto file_field = req.post("file");

	if (*file_field->ct() == "image/jpeg" ||
		*file_field->ct() == "image/gif" ||
		*file_field->ct() == "image/pjpeg" ||
		*file_field->ct() == "image/png" ||
		*file_field->ct() == "image/svg+xml" ||
		*file_field->ct() == "image/tiff" ||
		*file_field->ct() == "image/vnd.microsoft.icon" ||
		*file_field->ct() == "image/vnd.wap.wbmp") {

		upload_image(req, cnt);

	} else if (*file_field->ct() == "audio/basic" ||
		*file_field->ct() == "audio/L24" ||
		*file_field->ct() == "audio/mp4" ||
		*file_field->ct() == "audio/mp3" ||
		*file_field->ct() == "audio/aac" ||
		*file_field->ct() == "audio/x-aac" ||
		*file_field->ct() == "audio/mpeg" ||
		*file_field->ct() == "audio/ogg" ||
		*file_field->ct() == "audio/vorbis" ||
		*file_field->ct() == "audio/x-ms-wma" ||
		*file_field->ct() == "audio/x-ms-wax" ||
		*file_field->ct() == "audio/vnd.rn-realaudio" ||
		*file_field->ct() == "audio/vnd.wave" ||
		*file_field->ct() == "audio/aiff" ||
		*file_field->ct() == "audio/x-aiff" ||
		*file_field->ct() == "audio/basic" ||
		*file_field->ct() == "audio/x-au" ||
		*file_field->ct() == "audio/make" ||
		*file_field->ct() == "audio/x-gsm" ||
		*file_field->ct() == "audio/it" ||
		*file_field->ct() == "audio/x-jam" ||
		*file_field->ct() == "audio/midi" ||
		*file_field->ct() == "audio/nspaudio" ||
		*file_field->ct() == "audio/x-nspaudio" ||
		*file_field->ct() == "audio/x-liveaudio" ||
		*file_field->ct() == "audio/x-mpequrl" ||
		*file_field->ct() == "audio/x-mid" ||
		*file_field->ct() == "audio/x-midi" ||
		*file_field->ct() == "audio/x-vnd.audioexplosion.mjuicemediafile" ||
		*file_field->ct() == "audio/mod" ||
		*file_field->ct() == "audio/x-mod" ||
		*file_field->ct() == "audio/x-mpeg" ||
		*file_field->ct() == "audio/mpeg3" ||
		*file_field->ct() == "audio/x-mpeg-3" ||
		*file_field->ct() == "audio/make.my.funk" ||
		*file_field->ct() == "audio/vnd.qcelp" ||
		*file_field->ct() == "audio/x-pn-realaudio" ||
		*file_field->ct() == "audio/x-pn-realaudio-plugin" ||
		*file_field->ct() == "audio/x-realaudio" ||
		*file_field->ct() == "audio/x-pn-realaudio" ||
		*file_field->ct() == "audio/webm") {

		upload_audio(req, cnt);

	} else if (*file_field->ct() == "video/animaflex" ||
		*file_field->ct() == "video/x-ms-asf" ||
		*file_field->ct() == "video/x-ms-asf-plugin" ||
		*file_field->ct() == "video/avi" ||
		*file_field->ct() == "video/msvideo" ||
		*file_field->ct() == "video/x-msvideo" ||
		*file_field->ct() == "video/avs-video" ||
		*file_field->ct() == "video/x-dv" ||
		*file_field->ct() == "video/dl" ||
		*file_field->ct() == "video/x-dl" ||
		*file_field->ct() == "video/x-dv" ||
		*file_field->ct() == "video/fli" ||
		*file_field->ct() == "video/x-fli" ||
		*file_field->ct() == "video/x-atomic3d-feature" ||
		*file_field->ct() == "video/gl" ||
		*file_field->ct() == "video/x-gl" ||
		*file_field->ct() == "video/x-isvideo" ||
		*file_field->ct() == "video/mpeg" ||
		*file_field->ct() == "video/x-motion-jpeg" ||
		*file_field->ct() == "video/quicktime" ||
		*file_field->ct() == "video/x-sgi-movie" ||
		*file_field->ct() == "video/x-mpeg" ||
		*file_field->ct() == "video/x-mpeq2a" ||
		*file_field->ct() == "video/x-sgi-movie" ||
		*file_field->ct() == "video/x-qtc" ||
		*file_field->ct() == "video/vnd.rn-realvideo" ||
		*file_field->ct() == "video/x-scm" ||
		*file_field->ct() == "video/vdo" ||
		*file_field->ct() == "video/vivo" ||
		*file_field->ct() == "video/vnd.vivo" ||
		*file_field->ct() == "video/vosaic" ||
		*file_field->ct() == "video/x-amt-demorun" ||
		*file_field->ct() == "video/x-amt-showrun") {

		upload_video(req, cnt);

	} else {

		upload_file(req, cnt);

		// Вероятно, больше не актуально
//		throw error200_error(smessage(SMESS119_NOT_SUPPORTED));
	}
}

void service::upload_image(const request& req, context& cnt)
{
	auto image_field = req.post("file");

	if (image_field->size() == 0) throw error200_error(smessage(SMESS123_UPLOAD_NOT_SUCCESS));

	std::string extension;
	auto ext_pos = image_field->file()->rfind(".");
	if (ext_pos != std::string::npos) {
		extension = image_field->file()->substr(ext_pos, image_field->file()->size() - ext_pos);
	} else throw error200_error(smessage(SMESS122_UNKNOWN_EXTENSION));

	auto& cfg = config();

	std::string path_temp = cfg.dir_temp + format(std::hash<std::string>()(format(time(NULL)) + *image_field->file()));

	// Записываем полученные байты в файл
	std::ofstream ofs;
	ofs.open(path_temp, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	auto val = image_field->val_pr();
	for (auto it = val.first; it != val.second; ++it) ofs << *it;
	ofs.close();

	int image_mode = strtoul("666", 0, 8);
	if (0 > chmod(path_temp.c_str(), image_mode)) throw error<500>(smessage(SMESS216_CHANGE_UNABLE, path_temp));

//FIXME
//	auto& image_store = singleton<chatd::image_store>::instance();
//	image_store.add(cnt.sess_ptr->get_oid(), path_temp, *image_field->file(), extension, *image_field->ct());

	cnt.sess_ptr->add_info(smessage(SMESS120_IMAGE_UPLOADED));
}

void service::activate(const request& req)
{
	auto object = req.get<sptr_cstr>("object", 0);
	OID user_oid = req.get<OID>("user", 0);

	if (*object == "image") {

		OID image_oid = req.get<OID>("oid", 0);

		if (image_oid > 0 && user_oid > 0) {

			auto& sess = singleton<sessions>::instance();
			auto sess_ptr = sess.get(user_oid);
//FIXME
//			auto& image_store = singleton<chatd::image_store>::instance();
//			auto image_ptr = image_store.get(image_oid);

			// Устанавливаем права файлам картинок
//			image_ptr->set_privilege();

			// Добавим OID картинки в сессию пользователю 
			sess_ptr->add_image(image_oid);

			// Удаляем исходник
//			unlink(image_ptr->get_path_temp()->c_str());
		}

		throw error<204>(sptr_cstr(new std::string("Was asked activation for image: " + format(image_oid))));

	} else if (*object == "archive") {

		OID archive_oid = req.get<OID>("oid", 0);

		if (archive_oid > 0 && user_oid > 0) {

			auto& sess = singleton<sessions>::instance();
			auto sess_ptr = sess.get(user_oid);

			auto& archive_store = singleton<chatd::archive_store>::instance();
			auto archive_ptr = archive_store.get(archive_oid);

			// Устанавливаем права файлам
			archive_ptr->set_privilege();

			// Добавим OID в сессию пользователю 
			sess_ptr->add_archive(archive_oid);

			// Удаляем исходник
			unlink(archive_ptr->get_path_temp()->c_str());
		}

		throw error<204>(sptr_cstr(new std::string("Was asked activation for archive: " + format(archive_oid))));
	}
}

void service::settings(const request& req, context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& cfg = config();

    auto object = req.post("object")->val<sptr_cstr>();

	if (*object == "room") {

		OID oid = req.post("oid")->val<OID>();

		OID background_image = req.post("background_image")->val<OID>();
		int type = req.post("type")->val<int>();
		int transparence = req.post("transparence")->val<int>();
		// FIXME дыра в безопасности
		sptr_cstr background_color = req.post("background_color")->val<sptr_cstr>();
		sptr_cstr textsize = req.post("textsize")->val<sptr_cstr>();
		sptr_cstr description = req.post("description")->val<sptr_cstr>();

		auto& rm = singleton<chatd::room_store>::instance();
		auto room_ptr = rm.get(oid);

		if (!room_ptr->check_owner(cnt.sess_ptr->get_oid())) throw error<403>(smessage(SMESS204_ROOM_DN_BELONG));

		sptr_cstr background_path;

		if (background_image) {
//			auto& img = singleton<chatd::image_store>::instance();
//			auto image_ptr = img.get(background_image);
//			background_path = new std::string(cfg.path_image_web + "n" + format(image_ptr->get_oid()) + *image_ptr->get_extension());
		}

		room_ptr->set_options(
			background_path,
			type,
			transparence,
			util::escape_html(*background_color),
			util::escape_html(*textsize),
			util::escape_html(*description)
		);
	}
}

void service::upload_audio(const request& req, context& cnt)
{
	auto audio_field = req.post("file");

	auto& cfg = config();

	if (audio_field->size() == 0) throw error200_error(smessage(SMESS123_UPLOAD_NOT_SUCCESS));

	std::string extension;
	auto ext_pos = audio_field->file()->rfind(".");
	if (ext_pos != std::string::npos) {
		extension = audio_field->file()->substr(ext_pos, audio_field->file()->size() - ext_pos);
	} else throw error200_error(smessage(SMESS122_UNKNOWN_EXTENSION));

	std::string path_temp = cfg.dir_temp + format(std::hash<std::string>()(format(time(NULL)) + *audio_field->file())) + extension;

	// Записываем полученные байты в файл
	std::ofstream ofs;
	ofs.open(path_temp, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	auto val = audio_field->val_pr();
	for (auto it = val.first; it != val.second; ++it) ofs << *it;
	ofs.close();

	auto& ad = singleton<chatd::audio_store>::instance();
	OID audio_oid = ad.add(cnt.sess_ptr->get_oid(), path_temp, *audio_field->file(), extension, *audio_field->ct());

	// Добавим OID аудио-файла в сессию пользователю 
	if (audio_oid > 0) cnt.sess_ptr->add_audio(audio_oid);

	unlink(path_temp.c_str());

	cnt.sess_ptr->add_info(smessage(SMESS121_AUDIO_UPLOADED));
}

void service::upload_video(const request& req, context& cnt)
{
	auto video_field = req.post("file");

	auto& cfg = config();

	if (video_field->size() == 0) throw error200_error(smessage(SMESS123_UPLOAD_NOT_SUCCESS));

	std::string extension;
	auto ext_pos = video_field->file()->rfind(".");
	if (ext_pos != std::string::npos) {
		extension = video_field->file()->substr(ext_pos, video_field->file()->size() - ext_pos);
	} else throw error200_error(smessage(SMESS122_UNKNOWN_EXTENSION));

	std::string path_temp = cfg.dir_temp + format(std::hash<std::string>()(format(time(NULL)) + *video_field->file())) + extension;

	// Записываем полученные байты в файл
	std::ofstream ofs;
	ofs.open(path_temp, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	auto val = video_field->val_pr();
	for (auto it = val.first; it != val.second; ++it) ofs << *it;
	ofs.close();

	auto& vd = singleton<chatd::video_store>::instance();
	OID video_oid = vd.add(cnt.sess_ptr->get_oid(), path_temp, *video_field->file(), extension, *video_field->ct());

	// Добавим OID видео-файла в сессию пользователю
	if (video_oid > 0) cnt.sess_ptr->add_video(video_oid);

	unlink(path_temp.c_str());

	cnt.sess_ptr->add_info(smessage(SMESS135_VIDEO_UPLOADED));
}

void service::upload_file(const request& req, context& cnt)
{
	auto file_field = req.post("file");

	if (file_field->size() == 0) throw error200_error(smessage(SMESS123_UPLOAD_NOT_SUCCESS));

	auto& cfg = config();

	// Заменяем опасные символы
	auto file_temp = quote(*file_field->file());
	std::string path_temp = cfg.dir_temp + *file_temp;

	// Записываем полученные байты в файл
	std::ofstream ofs;
	ofs.open(path_temp, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	auto val = file_field->val_pr();
	for (auto it = val.first; it != val.second; ++it) ofs << *it;
	ofs.close();

	int file_mode = strtoul("666", 0, 8);
	if (0 > chmod(path_temp.c_str(), file_mode)) throw error<500>(smessage(SMESS216_CHANGE_UNABLE, path_temp));

	auto& archive_store = singleton<chatd::archive_store>::instance();
	archive_store.add(cnt.sess_ptr->get_oid(), path_temp, *file_field->file(), *file_field->ct());

	cnt.sess_ptr->add_info(smessage(SMESS137_ARCHIVE_UPLOADED));
}

void service::remove(const request& req, context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

    auto object = req.post("object")->val<sptr_cstr>();
	auto oids_str = req.post("oids")->val<sptr_cstr>();

	lo::l(lo::TRASH) << "service::remove, " << "object: " << *object << ", oids: " << *oids_str;

    auto oids_vec_str = util::split(*oids_str, ",");
    sptr_vec oids;

    for (auto oid_str : *oids_vec_str) (*oids).push_back(format<OID>(oid_str));

    if (*object == "image") {
        remove_images(cnt, oids);
    } else if (*object == "audio") {
        remove_audios(cnt, oids);
    } else if (*object == "video") {
        remove_videos(cnt, oids);
    } else if (*object == "archive") {
        remove_archives(cnt, oids);
    } else if (*object == "rallow") {
        remove_room_allows(cnt, oids);
    } else if (*object == "pmessage") {
        remove_personal_messages(cnt, oids);
    } else if (*object == "password") {
        remove_password(cnt);
    } else if (*object == "alias") {
		remove_alias(cnt);
    }
}

void service::remove_images(context& cnt, sptr_cvec oids)
{
//FIXME
//	auto& img = singleton<chatd::image_store>::instance();

    for (auto oid : *oids) {
	    cnt.sess_ptr->del_image(oid);
//	    img.del(oid, cnt.sess_ptr->get_oid());
    }

	cnt.sess_ptr->add_info(smessage(SMESS125_IMAGE_REMOVED));
}

void service::remove_audios(context& cnt, sptr_cvec oids)
{
	auto& ad = singleton<chatd::audio_store>::instance();

    for (auto oid : *oids) {
	    cnt.sess_ptr->del_audio(oid);
	    ad.del(oid, cnt.sess_ptr->get_oid());
    }

	cnt.sess_ptr->add_info(smessage(SMESS126_AUDIO_REMOVED));
}

void service::remove_videos(context& cnt, sptr_cvec oids)
{
	auto& vd = singleton<chatd::video_store>::instance();

    for (auto oid : *oids) {
	    cnt.sess_ptr->del_video(oid);
	    vd.del(oid, cnt.sess_ptr->get_oid());
    }

	cnt.sess_ptr->add_info(smessage(SMESS136_VIDEO_REMOVED));
}

void service::remove_archives(context& cnt, sptr_cvec oids)
{
	auto& arch = singleton<chatd::archive_store>::instance();

    for (auto oid : *oids) {
	    cnt.sess_ptr->del_archive(oid);
	    arch.del(oid, cnt.sess_ptr->get_oid());
    }

	cnt.sess_ptr->add_info(smessage(SMESS138_ARCHIVE_REMOVED));
}

void service::remove_room_allows(context& cnt, sptr_cvec oids)
{
	auto& ss = singleton<sessions>::instance();
	auto& rm = singleton<chatd::room_store>::instance();

    // У всех перечисленных пользователей отбираем доступ к комнате
	auto room_ptr = rm.get(cnt.room);
	if (room_ptr->get_owner() == cnt.sess_ptr->get_oid()) {

		for (auto oid : *oids) {
            room_ptr->del_allow(oid);

			auto user_ptr = ss.get(oid);
            // Выгоняем из комнаты, если он внутри
			room_ptr->user_out(user_ptr->get_name());
			user_ptr->getout_room(cnt.room);
			// Пишем обидное сообщение в комнату
			room_ptr->add_message(smessage(SMESS114_USER_WAS_DRIVEN, *user_ptr->get_name()));

            // Пишем уведомительные сообщения в личку
            ss.add_message_one_user(user_ptr, smessage(SMESS113_ACCESS_REMOVED, *room_ptr->get_name()), sptr_cstr(), sptr_cstr(), sptr_cstr(), sptr_cstr(), 0);
        }
    }

	cnt.sess_ptr->add_info(smessage(SMESS127_ALLOW_REMOVED));
}

void service::remove_personal_messages(context& cnt, sptr_cvec oids)
{
    for (auto oid : *oids) cnt.sess_ptr->del_message(oid);

	cnt.sess_ptr->add_info(smessage(SMESS128_MESSAGE_REMOVED));

    // Возвращаем обновленный список сообщений
	personal(cnt);
}

void service::remove_password(context& cnt)
{
    cnt.sess_ptr->clear_password();
}

void service::remove_alias(context& cnt)
{
    auto& ss = singleton<sessions>::instance();

	cnt.sess_ptr = ss.remove_alias(cnt.uuid_ptr);
}

void service::add(const request& req, context& cnt)
{
    auto object = req.post("object")->val<sptr_cstr>();

    if (*object == "alias") {
		add_alias(req, cnt);
		return;
	}

	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

    if (*object == "rallow") {

		auto oids_str = req.post("oids")->val<sptr_cstr>();
		auto oids_vec_str = util::split(*oids_str, ",");
		sptr_vec oids;
		for (auto oid_str : *oids_vec_str) (*oids).push_back(format<OID>(oid_str));

        add_room_allows(cnt, oids);

    } else if (*object == "room") {

		add_room(req, cnt);

	} else if (*object == "password") {

		add_password(req, cnt);

	} else if (*object == "avatar") {

		add_avatar(req, cnt);

	} else if (*object == "invite") {

		add_invite(req, cnt);

	}
}

void service::add_alias(const request& req, context& cnt)
{
    sptr_cstr password = req.post("password")->val<sptr_cstr>();

    auto& ss = singleton<sessions>::instance();
    cnt.sess_ptr = ss.add_alias(cnt.uuid_ptr, cnt.nick, password);
}

void service::add_password(const request& req, context& cnt)
{
    sptr_cstr password = req.post("password")->val<sptr_cstr>();

    cnt.sess_ptr->set_password(password);
	cnt.sess_ptr->add_info(smessage(SMESS117_PASSWORD_ADDED));
}

void service::add_avatar(const request& req, context& cnt)
{
	auto oid = req.post("oid")->val<OID>();

	auto& cfg = config();
	auto& avatar = singleton<avatar_store>::instance();

	if (oid == 0) {

		cnt.sess_ptr->set_avatar(oid);

	} else if (oid <= avatar.size()) {

		cnt.avatar = sptr_cstr(new std::string(cfg.dir_avatars_short + avatar.get(oid)));
		cnt.sess_ptr->set_avatar(oid);
		cnt.sess_ptr->add_info(smessage(SMESS131_AVATAR_INSTALLED));

	} else {

//FIXME
//		auto& image_store = singleton<chatd::image_store>::instance();
//		auto image_ptr = image_store.get(oid);
/*
		if (image_ptr->check_owner(cnt.sess_ptr->get_oid())) {

			image_ptr->add_allow(0); // разрешаем картинку видеть всем
			cnt.avatar = sptr_cstr(new std::string(cfg.path_image_web + "m" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));
			cnt.sess_ptr->set_avatar(oid);

			cnt.sess_ptr->add_info(smessage(SMESS131_AVATAR_INSTALLED));

		} else throw error<403>(smessage(SMESS203_IMAGE_DN_BELONG));
*/

	}
}

void service::add_invite(const request& req, context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& rm = singleton<chatd::room_store>::instance();
	auto& invite_store = singleton<chatd::invite_store>::instance();

	auto room_oid = req.post("room")->val<OID>();
	auto room_ptr = rm.get(room_oid);

	if (room_ptr->check_owner(cnt.sess_ptr->get_oid())) {

		auto name = req.post("name")->val<sptr_cstr>();
		auto start = req.post("start")->val<time_t>();
		auto expire = req.post("expire")->val<time_t>();
		auto really_times = req.post("really_times")->val<size_t>();
		auto password_str = req.post("password")->val<sptr_cstr>();
		auto permission_str = req.post("permission")->val<sptr_cstr>();

		auto invite_oid = invite_store.add(
			name,
			cnt.sess_ptr->get_oid(),
			room_oid,
			start,
			expire,
			really_times,
			password_str,
			permission_str
		);

		auto& cfg = config();
		std::string link(cfg.invite_path + format(invite_oid));

		cnt.sess_ptr->add_info(smessage(SMESS132_INVITATION_ADDED, *room_ptr->get_name(), format(invite_oid), link));

	} else throw error<403>(smessage(SMESS204_ROOM_DN_BELONG));
}

void service::invite(const request& req, context& cnt)
{
	auto oid = req.get<OID>("oid", 0);
	auto password = req.get<sptr_cstr>("password", sptr_cstr());

	if (oid == 0) return;

	auto& cfg = config();

	auto& invite_store = singleton<chatd::invite_store>::instance();
	auto invite_ptr = invite_store.get(oid);

	try {

		if (invite_ptr->check(cnt.sess_ptr->get_oid(), *password)) {

			auto& rm = singleton<chatd::room_store>::instance();
			auto room_ptr = rm.get(invite_ptr->get_room());

			if (!cnt.sess_ptr->is_inside(room_ptr->get_oid())) {

				// У пользователя нет имени. Добавляем случайное.
				if (cnt.sess_ptr->get_name()->empty()) {

					auto& sess = singleton<sessions>::instance();

					while (true) {
						auto random_nickname = util::get_random_string(ALPH_ENGLISH, cfg.nickname_max);
						if (sess.set_name(cnt.uuid_ptr, cnt.sess_ptr, random_nickname)) break;
					}
				}

				// Проверяем разрешение на вход в комнату. Если его нет - добавляем.
				if (!room_ptr->check_allow(cnt.sess_ptr->get_oid())) {
					room_ptr->add_allow(cnt.sess_ptr->get_oid());
				}

				room_ptr->user_in(cnt.sess_ptr->get_name(), cnt.sess_ptr->get_oid());
				cnt.sess_ptr->comein_room(room_ptr->get_oid());
			}

			cnt.room = room_ptr->get_oid();
			cnt.sess_ptr->set_current_room(cnt.room);

			cnt.sess_ptr->add_info(smessage(SMESS133_USED_INVITATION, *room_ptr->get_name()));

		} else cnt.sess_ptr->add_error(smessage(SMESS134_FAILUSE_INVITATION));

	} catch (const error<302>& e) {

		throw error<302>(sptr_cstr(new std::string(cfg.path_form_password)));

	}

	throw error<302>(sptr_cstr(new std::string(cfg.invite_redirect)));
}

void service::add_room_allows(context& cnt, sptr_cvec oids)
{
	auto& rm = singleton<chatd::room_store>::instance();
    auto& ss = singleton<sessions>::instance();

	auto room_ptr = rm.get(cnt.room);
    if (room_ptr->get_owner() == cnt.sess_ptr->get_oid()) {

		// Всем перечисленным пользователям раздаем доступ к комнате
		for (auto oid : *oids) {
            room_ptr->add_allow(oid);

            auto user_ptr = ss.get(oid);

            // Пишем уведомительные сообщения в личку
            ss.add_message_one_user(user_ptr, smessage(SMESS112_ACCESS_ADDED, *cnt.sess_ptr->get_name(), *room_ptr->get_name()), sptr_cstr(), sptr_cstr(), sptr_cstr(), sptr_cstr(), 0);
        }
    }
}

void service::image(const request& req, context& cnt)
{
	OID oid_img = req.get<OID>("oid", 0);
	std::string size = req.get<std::string>("size", "");

	auto& cfg = config();
//FIXME
//	auto& img = singleton<chatd::image_store>::instance();

//	sptr<chatd::image> image_ptr;

	try {
//		image_ptr = img.get(oid_img);
	} catch (const error<500>& e) {
		lo::l(lo::ERROR) << e.what();
		throw redirect_empty(cfg.path_empty_image);
	}
/*
	if (image_ptr->check_allow(cnt.sess_ptr->get_oid())) {
		std::string path = (size == "n" ? cfg.dir_normal_short : cfg.dir_miniature_short) + format(image_ptr->get_oid()) + *image_ptr->get_extension();
		throw redirect_image(path, *image_ptr->get_content_type());
	} else {
		throw redirect_empty(cfg.path_empty_image);
	}
*/
}

void service::audio(const request& req, context& cnt)
{
	OID audio_oid = req.get<OID>("oid", 0);

	auto& ad = singleton<chatd::audio_store>::instance();
	auto audio_ptr = ad.get(audio_oid);

	auto& cfg = config();
	std::string path = cfg.dir_audio_short + format(audio_ptr->get_oid()) + *audio_ptr->get_extension();

	if (audio_ptr->check_allow(cnt.sess_ptr->get_oid())) {
		throw redirect_media(path, *audio_ptr->get_content_type());
	} else throw error<403>(smessage(SMESS209_AUDIO_ACCESS_DENIED, path));
}

void service::video(const request& req, context& cnt)
{
	OID video_oid = req.get<OID>("oid", 0);

	auto& vd = singleton<chatd::video_store>::instance();
	auto video_ptr = vd.get(video_oid);

	auto& cfg = config();
	std::string path = cfg.dir_video_short + format(video_ptr->get_oid()) + *video_ptr->get_extension();

	if (video_ptr->check_allow(cnt.sess_ptr->get_oid())) {
		throw redirect_media(path, *video_ptr->get_content_type());
	} else throw error<403>(smessage(SMESS210_VIDEO_ACCESS_DENIED, path));
}

void service::archive(const request& req, context& cnt)
{
	OID archive_oid = req.get<OID>("oid", 0);

	auto& arch = singleton<chatd::archive_store>::instance();
	auto archive_ptr = arch.get(archive_oid);

	auto& cfg = config();
	std::string path = cfg.dir_archive_short + format(archive_ptr->get_oid()) + ".zip";

	if (archive_ptr->check_allow(cnt.sess_ptr->get_oid())) {
		throw redirect_media(path, "application/zip");
	} else throw error<403>(smessage(SMESS211_ARCHIVE_ACCESS_DENIED, path));
}

void service::personal(context& cnt)
{
	if (cnt.sess_ptr->get_name()->empty()) throw error<403>(smessage(SMESS201_UNAUTHORIZED));

	auto& audio_store = singleton<chatd::audio_store>::instance();
//	auto& image_store = singleton<chatd::image_store>::instance();
	auto& video_store = singleton<chatd::video_store>::instance();
	auto& archive_store = singleton<chatd::archive_store>::instance();

	answer_personal answer_temp;

	answer_temp.m_oid = cnt.sess_ptr->get_oid();
	answer_temp.m_name = cnt.sess_ptr->get_name();

	auto& cfg = config();

	auto pmessages = cnt.sess_ptr->get_personal();
    for (auto list_pr : *pmessages) {
        for (auto mess : list_pr.second) {

			auto images_vec = mess->get_images();
			sptr<std::vector<an_mimage>> images_res;
			for (auto image_oid : *images_vec) {
				try {
/*
					auto image_ptr = image_store.get(image_oid);

					if (image_ptr->get_oid() > 0) {
						sptr_cstr path1(new std::string(cfg.path_image_web + "m" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));
						sptr_cstr path2(new std::string(cfg.path_image_web + "n" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));

						images_res->push_back(an_mimage(path1, path2));
					}
*/
				} catch (...) { continue; }
			}

			auto audio_vec = mess->get_audios();
			sptr<std::vector<an_maudio>> audios_res;
			for (auto audio_oid : *audio_vec) {
				try {

					auto audio_ptr = audio_store.get(audio_oid);

					if (audio_ptr->get_oid()) {
						sptr_cstr path(new std::string(cfg.path_audio_web + "a" + format(audio_ptr->get_oid()) + *audio_ptr->get_extension()));
						audios_res->push_back(an_maudio(audio_ptr->get_name(), audio_ptr->get_title(), audio_ptr->get_artist(), path));
					}

				} catch (...) { continue; }
			}

			auto video_vec = mess->get_videos();
			sptr<std::vector<an_mvideo>> videos_res;
			for (auto video_oid : *video_vec) {
				try {

					auto video_ptr = video_store.get(video_oid);

					if (video_ptr->get_oid()) {
						sptr_cstr path(new std::string(cfg.path_video_web + "v" + format(video_ptr->get_oid()) + *video_ptr->get_extension()));
						videos_res->push_back(an_mvideo(video_ptr->get_name(), path));
					}

				} catch (...) { continue; }
			}

			auto archive_vec = mess->get_archives();
			sptr<std::vector<an_marchive>> archives_res;
			for (auto archive_oid : *archive_vec) {
				try {

					auto archive_ptr = archive_store.get(archive_oid);

					if (archive_ptr->get_oid()) {
						sptr_cstr path(new std::string(cfg.path_archive_web + "h" + format(archive_ptr->get_oid()) + ".zip"));
						archives_res->push_back(an_marchive(archive_ptr->get_name(), path));
					}

				} catch (...) { continue; }
			}

			auto youtube_vec = mess->get_youtube();
			sptr<std::vector<an_myoutube>> youtube_res;
			for (auto youtube_url : *youtube_vec) {
				youtube_res->push_back(an_myoutube(youtube_url));
			}

			answer_temp.m_messages.push_back(
				an_mess(
					mess->get_oid(),
					0, // комната всегда 0
					mess->get_poid(),
					mess->get_time(),
					false, // appeal всегда false для лички
					mess->is_private(),
					mess->get_avatar(),
					mess->get_oid_from(),
					mess->get_name_from(),
					mess->get_oid_to(),
					mess->get_name_to(),
					mess->get_message(),
					mess->get_raw(),
					images_res,
					audios_res,
					videos_res,
					archives_res,
					youtube_res,
					mess->get_color()
				)
			);
        }
    }

	serialization::s(*cnt.resp.data(), answer_temp, 0);

	throw error200_personal(sptr_cstr(new std::string("Отправляем ответ с сообщениями в личке")));
}

void service::generate_answer(context& cnt)
{
	answer answer_temp;

	answer_temp.oid = cnt.sess_ptr->get_oid();
	answer_temp.name = cnt.sess_ptr->get_name();
	answer_temp.avatar = cnt.avatar;
	answer_temp.room = cnt.room;

//	auto& image_store = singleton<chatd::image_store>::instance();
	auto& audio_store = singleton<chatd::audio_store>::instance();
	auto& video_store = singleton<chatd::video_store>::instance();
	auto& archive_store = singleton<chatd::archive_store>::instance();

	answer_temp.mask = cnt.answer_obj_mask;

	answer_temp.hash_personal = cnt.sess_ptr->get_hash_personal();
	answer_temp.hash_image = cnt.sess_ptr->get_hash_image();
	answer_temp.hash_audio = cnt.sess_ptr->get_hash_audio();
	answer_temp.hash_video = cnt.sess_ptr->get_hash_video();
	answer_temp.hash_archive = cnt.sess_ptr->get_hash_archive();

	auto& cfg = config();

    auto& rm = singleton<chatd::room_store>::instance();

	answer_temp.last_message = rm.last_message();

	auto rooms = cnt.sess_ptr->get_rooms();
	// Если пользователь не заходил ни в одну из комнат, показать ему текущую
	if (rooms->size() == 0) rooms->insert(cnt.room);

	for (auto room_oid : *rooms) {

		// FIXME Убрать вместе со строкой: if (rooms->size() == 0) rooms->insert(cnt.room);
		try {

			auto messages_res = rm.get(room_oid)->get_messages(
				cnt.sess_ptr->get_oid(),
				((room_oid == cnt.room) && (cnt.answer_obj_mask & SHOW_WHOLE_ROOM)) ? 0 : cnt.last_mess
			);

			for (auto mess : *messages_res) {

				bool appeal = false;
				if (!(cnt.sess_ptr->get_name()->empty())
					&& mess->get_message()->find(*(cnt.sess_ptr->get_name())) != std::string::npos) {
					appeal = true;
				}

				auto images_vec = mess->get_images();
				sptr<std::vector<an_mimage>> images_res;
				for (auto image_oid : *images_vec) {
					try {
/*
						// Удаленные картинки будут генерировать здесь исключение
						auto image_ptr = image_store.get(image_oid);

						sptr_cstr path1(new std::string(cfg.path_image_web + "m" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));
						sptr_cstr path2(new std::string(cfg.path_image_web + "n" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));

						images_res->push_back(an_mimage(path1, path2));
*/

					} catch (...) { continue; }
				}

				auto audio_vec = mess->get_audios();
				sptr<std::vector<an_maudio>> audios_res;
				for (auto audio_oid : *audio_vec) {
					try {
						// Удаленные аудио-файлы будут генерировать здесь исключение
						auto audio_ptr = audio_store.get(audio_oid);

						sptr_cstr path(new std::string(cfg.path_audio_web + "a" + format(audio_ptr->get_oid()) + *audio_ptr->get_extension()));
						audios_res->push_back(an_maudio(audio_ptr->get_name(), audio_ptr->get_title(), audio_ptr->get_artist(), path));

					} catch (...) { continue; }
				}

				auto video_vec = mess->get_videos();
				sptr<std::vector<an_mvideo>> videos_res;
				for (auto video_oid : *video_vec) {
					try {
						auto video_ptr = video_store.get(video_oid);

						sptr_cstr path(new std::string(cfg.path_video_web + "a" + format(video_ptr->get_oid()) + *video_ptr->get_extension()));
						videos_res->push_back(an_mvideo(video_ptr->get_name(), path));

					} catch (...) { continue; }
				}

				auto archive_vec = mess->get_archives();
				sptr<std::vector<an_marchive>> archives_res;
				for (auto archive_oid : *archive_vec) {
					try {
						auto archive_ptr = archive_store.get(archive_oid);

						sptr_cstr path(new std::string(cfg.path_archive_web + "h" + format(archive_ptr->get_oid()) + ".zip"));
						archives_res->push_back(an_marchive(archive_ptr->get_name(), path));

					} catch (...) { continue; }
				}

				auto youtube_vec = mess->get_youtube();
				sptr<std::vector<an_myoutube>> youtube_res;
				for (auto youtube_url : *youtube_vec) {
					youtube_res->push_back(an_myoutube(youtube_url));
				}

				answer_temp.messages.push_back(
					an_mess(
						mess->get_oid(),
						room_oid,
						mess->get_poid(),
						mess->get_time(),
						appeal,
						mess->is_private(),
						mess->get_avatar(),
						mess->get_oid_from(),
						mess->get_name_from(),
						mess->get_oid_to(),
						mess->get_name_to(),
						mess->get_message(),
						mess->get_raw(),
						images_res,
						audios_res,
						videos_res,
						archives_res,
						youtube_res,
						mess->get_color()
					)
				);
			}

		} catch (...) {
			continue;
		}
	}
	// -------------------

	// Добавляем пользователей
	if (cnt.answer_obj_mask & SHOW_USERS && cnt.sess_ptr->is_inside(cnt.room)) {
//		auto& image_store = singleton<chatd::image_store>::instance();
		auto& sess = singleton<sessions>::instance();
		auto users = rm.get(cnt.room)->get_users();
		for (auto& user_pr : *users) {
			if (user_pr.second == cnt.sess_ptr->get_oid()) continue;
			auto sess_ptr = sess.get(user_pr.second);

			sptr_cstr user_avatar;
			try {

//				auto image_ptr = image_store.get(sess_ptr->get_avatar());
//				user_avatar = sptr_cstr(new std::string(cfg.path_image_web + "m" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));

			} catch (...) {}

			answer_temp.users.push_back(an_user(user_pr.second, sess_ptr->get_name(), user_avatar));
		}
	}
	// -------------------

	// Добавляем комнаты
	if (cnt.answer_obj_mask & SHOW_ROOMS) {
		auto rooms_all = rm.get_rooms();
		for (auto& room_pr : *rooms_all) {
			answer_temp.rooms.push_back(
				an_room(room_pr.second->get_oid(),
					room_pr.second->get_name(),
					cnt.sess_ptr->is_inside(room_pr.second->get_oid()),
					room_pr.second->is_private(),
					room_pr.second->check_allow(cnt.sess_ptr->get_oid()),
					room_pr.second->check_owner(cnt.sess_ptr->get_oid()),
					room_pr.second->size(),
					room_pr.second->get_bpath(),
					room_pr.second->get_btype(),
					room_pr.second->get_btransparence(),
					room_pr.second->get_bcolor(),
					room_pr.second->get_textsize(),
					room_pr.second->get_description()
				)
			);
		}
	}
	// -------------------

	// Добавляем писателей
	auto& wr = singleton<chatd::writers>::instance();
	auto writers = wr.get_writers(cnt.room);
	for (auto& wr_pr : *writers) {
		answer_temp.writers.push_back(
			an_writer(wr_pr.second->get_oid(), wr_pr.second->get_name())
		);
	}
	// -------------------

	// Добавляем картинки
	if (cnt.answer_obj_mask & SHOW_IMAGES) {
		auto images = cnt.sess_ptr->get_images();
		for (OID image_oid : *images) {
/*
			auto image_ptr = image_store.get(image_oid);
			sptr_cstr path(new std::string(cfg.path_image_web + "m" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));
			sptr_cstr norm(new std::string(cfg.path_image_web + "n" + format(image_ptr->get_oid()) + *image_ptr->get_extension()));
			answer_temp.images.push_back(
				an_image(image_ptr->get_oid(), image_ptr->get_name(), path, norm)
			);
*/
		}
	}
	// -------------------

	// Добавляем музыку
	if (cnt.answer_obj_mask & SHOW_AUDIOS) {
		auto audios = cnt.sess_ptr->get_audios();
		for (OID audio_oid : *audios) {
			auto audio_ptr = audio_store.get(audio_oid);
			sptr_cstr path(new std::string(cfg.path_audio_web + "a" + format(audio_ptr->get_oid()) + *audio_ptr->get_extension()));
			answer_temp.audios.push_back(
				an_audio(audio_ptr->get_oid(), audio_ptr->get_name(), audio_ptr->get_title(), audio_ptr->get_artist(), path)
			);
		}
	}
	// -------------------

	// Добавляем видео
	if (cnt.answer_obj_mask & SHOW_VIDEOS) {
		auto videos = cnt.sess_ptr->get_videos();
		for (OID video_oid : *videos) {
			auto video_ptr = video_store.get(video_oid);
			sptr_cstr path(new std::string(cfg.path_video_web + "v" + format(video_ptr->get_oid()) + *video_ptr->get_extension()));
			answer_temp.videos.push_back(
				an_video(video_ptr->get_oid(), video_ptr->get_name(), path)
			);
		}
	}
	// -------------------

	// Добавляем архивы
	if (cnt.answer_obj_mask & SHOW_ARCHIVES) {
		auto archives = cnt.sess_ptr->get_archives();
		for (OID archive_oid : *archives) {
			auto archive_ptr = archive_store.get(archive_oid);
			sptr_cstr path(new std::string(cfg.path_archive_web + "h" + format(archive_ptr->get_oid()) + ".zip"));
			answer_temp.archives.push_back(
				an_archive(archive_ptr->get_oid(), archive_ptr->get_name(), path)
			);
		}
	}
	// -------------------

	answer_temp.info = *cnt.sess_ptr->get_info();
	answer_temp.error = *cnt.sess_ptr->get_error();

	serialization::s(*cnt.resp.data(), answer_temp, 0);

	cnt.resp.set_status(S200);
	cnt.resp.add_header("Content-Type", "application/json; charset=utf-8");
	cnt.resp.add_cookie("UUID", to_string(*(cnt.uuid_ptr)));
}

void service::generate_short_answer(context& cnt)
{
	answer answer_temp(true);

	answer_temp.oid = cnt.sess_ptr->get_oid();
	answer_temp.name = cnt.sess_ptr->get_name();
	answer_temp.avatar = cnt.avatar;
	answer_temp.room = cnt.room;

	answer_temp.info = *cnt.sess_ptr->get_info();
	answer_temp.error = *cnt.sess_ptr->get_error();

	serialization::s(*cnt.resp.data(), answer_temp, 0);

	cnt.resp.set_status(S200);
	cnt.resp.add_header("Content-Type", "application/json; charset=utf-8");
	cnt.resp.add_cookie("UUID", to_string(*(cnt.uuid_ptr)));
}
