#include <boost/thread/detail/singleton.hpp>
#include <exception>
#include "stat_daemon.h"
#include "configure.h"
#include "service.h"
#include "server.h"
#include "front.h"
#include "util.h"
#include "log.h"

using namespace boost::detail::thread;

int main(int argc, char* argv[])
{
	try {
		lo::set_log_level(lo::WARNING);

		auto& cfg = config();
		cfg.parse_cmd(argc, argv);

		// Перенаплавляем STDOUT в файл
		lo::l() << "Forwarding STDOUT to " << cfg.path_to_stdout;
		lo::set_output(cfg.path_to_stdout);
		// Демонизируемся
		util::daemonize();
		util::savepid(cfg.path_pid.c_str());

		// Создаем объект профилировщика
		auto& prof = singleton<profile<stat_daemon>>::instance();
		prof.clear();

		typedef singleton<server::server<service>> main_srv;
		main_srv::instance().init(cfg.listen);

        auto& fr = singleton<front>::instance();
        fr.ready = true;

		lo::l(lo::WARNING) << "Daemon execd is started";
		main_srv::instance().run();

	} catch (const std::exception& e) {
		lo::l(lo::FATAL) << e.what();	
	}

	return 0;
}
