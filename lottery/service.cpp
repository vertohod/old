#include <boost/thread/detail/singleton.hpp>
#include <sys/types.h>
#include <sys/wait.h>

#include "processmanager.h"
#include "configure.h"
#include "stat_daemon.h"
#include "get_html.h"
#include "response.h"
#include "service.h"
#include "util.h"
#include "log.h"

using namespace boost::detail::thread;

service::service(net::boost_socket_ptr socket) : net::http_server(socket) {}

void service::handle_process(const request& req)
{
	response resp;

	try {

		if (*req.path() == "/get.json") {

			auto& prof = singleton<profile<stat_daemon>>::instance();

			resp.data()->assign(prof.get());
			resp.add_header("Content-Type", "application/json; charset=utf-8");
            resp.data()->assign("{\n\t\"number\":\"" + "1234" + "\"\n}\n");
			resp.set_status(S200);

		}

	} catch (const std::exception& e) {

		lo::l(lo::ERROR) << e.what();

	}

	resp.set_ver(req.ver());
	http_server::send(*resp.to_raw());
}
