#ifndef ADMIN_CHATD_H
#define ADMIN_CHATD_H

#include "stat_daemon.h"
#include "admin.h"

class admin_chatd : public admin<stat_daemon>
{
public:
	admin_chatd(net::boost_socket_ptr);
	virtual void handle_process(const request&);
};

#endif
