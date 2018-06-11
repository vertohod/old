#include <boost/thread/detail/singleton.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <random>

#include "configure.h"
#include "stat_daemon.h"
#include "response.h"
#include "service.h"
#include "util.h"
#include "log.h"

using namespace boost::detail::thread;

service::service(net::boost_socket_ptr socket) : net::http_server(socket) {}

std::string get_numbers(size_t max, size_t amount)
{
    std::random_device      rd;
    std::mt19937            gen(rd());

    std::vector<size_t> source;
    for (size_t i = 1; i <= max; ++i) source.push_back(i);

    std::vector<size_t> result;
    for (size_t j = 0; j < amount; ++j) {
        std::uniform_int_distribution<> dis(0, source.size() - 1);
        auto random_index = dis(gen);

        result.push_back(source[random_index]);
        for (size_t k = random_index; k < (source.size() - 1); ++k) {
            source[k] = source[k + 1];
        }
        source.resize(source.size());
    }

    std::string result_str;
    for (size_t i = 0; i < result.size(); ++i) {
        if (!result_str.empty()) result_str += ",";
        result_str += util::itoa(result[i]);
    }

    return result_str;
}

void service::handle_process(const request& req)
{
	response resp;

    auto numbers_str = get_numbers(50, 5);
    auto extra_str = get_numbers(10, 2);

	try {

		if (*req.path() == "/lottery.json") {

			resp.add_header("Content-Type", "application/json; charset=utf-8");
            resp.data()->assign(std::string("{\n\t\"numbers\":\"") + numbers_str + "\"\n\t\"extra\":\"" + extra_str + "\"\n}\n");
			resp.set_status(S200);

		}

	} catch (const std::exception& e) {

		lo::l(lo::ERROR) << e.what();

	}

	resp.set_ver(req.ver());
	http_server::send(*resp.to_raw());
}
