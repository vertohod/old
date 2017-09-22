#ifndef SERVICE_H
#define SERVICE_H

#include "http_server.h"

class service : public net::http_server
{
public:
	service(net::boost_socket_ptr);

private:
	void handle_process(const request& req);
	void exec(sptr_cstr comm, sptr_cstr upstream, sptr_cstr cb);
};

void thread_function_for_waiting(sptr_cstr comm, const std::string& ip_port, sptr_cstr cb);

#endif
