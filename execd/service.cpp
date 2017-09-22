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

		if (*req.path() == "/stat.json") {

			auto& prof = singleton<profile<stat_daemon>>::instance();

			resp.data()->assign(prof.get());
			resp.add_header("Content-Type", "application/json; charset=utf-8");
			resp.set_status(S200);

		} else if (*req.path() == "/config.json") {
		} else if (*req.path() == "/log.json") {

			std::string level = req.get<std::string>("l", "");
			lo::set_log_level(lo::trans_level(level));

			resp.data()->assign("{\n\t\"current_level\":\"" + lo::get_level() + "\"\n}\n");
			resp.add_header("Content-Type", "application/json; charset=utf-8");
			resp.set_status(S200);

		} else if (*req.path() == "/dump.json") {
		} else if (*req.path() == "/proc.json") {

			sptr_cstr comm = req.post("comm")->val<sptr_cstr>();
			sptr_cstr upstream = req.post("upstream")->val<sptr_cstr>();
			sptr_cstr cb = req.post("cb")->val<sptr_cstr>();

			lo::l(lo::WARNING) << "Command: " << *comm;
			lo::l(lo::DEBUG) << "Upstream: " << *upstream;
			lo::l(lo::DEBUG) << "Callback: " << *cb;

			exec(comm, upstream, cb);

		}
	} catch (const std::exception& e) {

		lo::l(lo::ERROR) << e.what();

	}

	resp.set_ver(req.ver());
	http_server::send(*resp.to_raw());
}

void service::exec(sptr_cstr comm, sptr_cstr upstream, sptr_cstr cb)
{
	auto& cfg = config();

	auto ip_port = cfg.upstream[*upstream];
	if (ip_port.empty()) return;

	auto& threads = singleton<boost::thread_group>::instance();
	threads.create_thread(boost::bind(&thread_function_for_waiting, comm, ip_port, cb));
}

void thread_function_for_waiting(sptr_cstr comm, const std::string& ip_port, sptr_cstr cb)
{
	lo::l(lo::DEBUG) << "Start thread";

	auto& procs = singleton<proc_manager>::instance();

	auto pid = procs.start(comm);

	lo::l(lo::DEBUG) << "Waiting...";
	int status = 0;
	waitpid(pid, &status, 0);	
	lo::l(lo::DEBUG) << "Process " << pid << " was finished";

	if (!cb->empty()) {

		request req;
		req.set_ver(ver_t("HTTP/1.1"));
		req.set_method(GET);
		req.set_path(*cb);

		lo::l(lo::WARNING) << "Callback (" << ip_port << "): " << *cb;
		auto resp = get_html(ip_port, req);
	}

	lo::l(lo::DEBUG) << "Finish thread";
}
