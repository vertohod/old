#ifndef SERVICE_H
#define SERVICE_H

#include "http_server.h"

class service : public net::http_server
{
public:
	service(net::boost_socket_ptr);

private:
	void handle_process(const request& req);
};

#endif
