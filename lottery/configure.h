#ifndef CONFIGURE_H
#define CONFIGURE_H

#include "config.h"
#include <string>
#include <boost/thread/detail/singleton.hpp>

using namespace boost::detail::thread;

class chatd_config : public config
{
public:
	chatd_config() {}

	chatd_config(const std::string& arPath) : config(arPath)
	{
		call_back();
	}

	void call_back()
	{
		// Необходим вызов для установки базовых параметров
		config::call_back();

		auto& op = get_options();

		op
			("listen", "Port for main server", listen)
			("upstream", "Upstream for callback", upstream)
			("path-pid", "Path of file which content daemon's pid", path_pid)
		;

		// Проверяем, нет ли лишних параметров в файле
		check();
	}


public:
	// OPTIONS --------------------------------------------------------

	std::string		listen;
	map_str_t		upstream;
	std::string		path_pid;

	// ----------------------------------------------------------------
};

inline chatd_config& config()
{
	return singleton<chatd_config>::instance();
}

#endif
