#include <boost/thread/detail/singleton.hpp>
#include <boost/filesystem.hpp>
#include <exception>
#include <unistd.h>
#include <sys/stat.h>

#include "stat_daemon.h"
#include "admin_chatd.h"
#include "configure.h"
#include "service.h"
#include "archive.h"
#include "dumper.h"
#include "server.h"
#include "domain.h"
#include "avatar.h"
#include "invite.h"
#include "smile.h"
#include "front.h"
#include "image.h"
#include "audio.h"
#include "video.h"
#include "room.h"
#include "util.h"
#include "log.h"

using namespace boost::detail::thread;

void load_dump()
{
	auto& cfg = config();
	auto& dm = singleton<dumper>::instance();

	dm.init(cfg.path_dump);

    auto& sess = singleton<sessions>::instance();
    auto& rm = singleton<chatd::room_store>::instance();
//    auto& image = singleton<chatd::image_store>::instance();
    auto& audio = singleton<chatd::audio_store>::instance();
    auto& video = singleton<chatd::video_store>::instance();
    auto& archive = singleton<chatd::archive_store>::instance();
    auto& invite = singleton<chatd::invite_store>::instance();

	dm.add_handler(boost::bind(&sessions::load, &sess, _1));
	dm.add_handler(boost::bind(&chatd::room_store::load, &rm, _1));
//	dm.add_handler(boost::bind(&chatd::image_store::load, &image, _1));
	dm.add_handler(boost::bind(&chatd::audio_store::load, &audio, _1));
	dm.add_handler(boost::bind(&chatd::video_store::load, &video, _1));
	dm.add_handler(boost::bind(&chatd::archive_store::load, &archive, _1));
	dm.add_handler(boost::bind(&chatd::invite_store::load, &invite, _1));

	auto size = dm.load();

	if (size == 0) {
		rm.create_dr();
		rm.set_settings_dr();
	}
}

void admin_thread_function()
{
	auto& cfg = config();

	typedef singleton<server::server<admin_chatd>> admin_server;
	admin_server::instance().init(cfg.admin_port);

	lo::l(lo::WARNING) << "Admin port listen: " << cfg.admin_port;
	admin_server::instance().run();
}

boost::thread admin_thread;

void start_admin_thread()
{
    boost::thread thread(boost::bind(&admin_thread_function));
    admin_thread.swap(thread);
}

template <typename CFG>
void create_dirs(const CFG& cfg)
{
	auto path = util::remove_end_slash(cfg.dir_temp);

	boost::system::error_code ec;
	boost::filesystem::create_directories(path, ec);

	if (ec.value() == boost::system::errc::success) {
		lo::l(lo::TRASH) << "Was created the dir: " << path;
		chmod(path.c_str(), 0755);
	} else {
		lo::l(lo::ERROR) << "Wasn't created the dir: " << path << ", error: " << ec.value() << ", message: " << ec.message();
		throw std::runtime_error("Failed to create all directories");
	}
}

int main(int argc, char* argv[])
{
	try {
		lo::set_log_level(lo::TRASH);

		auto& cfg = config();
		cfg.parse_cmd(argc, argv);

		// Перенаплавляем STDOUT в файл
		lo::l(lo::WARNING) << "Forwarding STDOUT to " << cfg.path_to_stdout;
		lo::set_output(cfg.path_to_stdout);
		// Демонизируемся
		util::daemonize();
		util::savepid(cfg.path_pid.c_str());

		auto& fr = singleton<front>::instance();
		fr.ready = false;

		create_dirs(cfg);

		// Создаем объект профилировщика
		auto& prof = singleton<profile<stat_daemon>>::instance();
		prof.clear();

		start_admin_thread();

		// Заполняем структуры аватарами и смайлами
		auto& avatar = singleton<avatar_store>::instance();
		auto& smile = singleton<smile_store>::instance();
		auto& domain = singleton<domain_store>::instance();

		avatar.init(cfg.dir_avatars);
		smile.init(cfg.path_smiles_list, cfg.smiles_extension);
		domain.init(cfg.path_domains_list);

		// Заполняем объекты из дампа
		load_dump();

		typedef singleton<server::server<service>> main_srv;
		main_srv::instance().init(cfg.listen);

		lo::l(lo::WARNING) << "Daemon chatd is started, listen: " << cfg.listen;
		lo::set_log_level(lo::WARNING);

		fr.ready = true;
		main_srv::instance().run();

	} catch (const std::exception& e) {
		lo::l(lo::FATAL) << "main: " << e.what();
	} catch (...) {
		lo::l(lo::FATAL) << "main: Unknown exception";
	}

	return 0;
}
