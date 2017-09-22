#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <map>

#include "http_server.h"
#include "log.h"

namespace server
{

template <typename REQ_PROC>
class server
{
	typedef boost::shared_ptr<REQ_PROC> req_proc_ptr;
	typedef std::map<int, req_proc_ptr> map_sockets;
	map_sockets m_sockets;

	struct boost_asio
	{
		boost_asio(const std::string& ip, int port) :
			m_service(),
			m_ep(boost::asio::ip::address_v4::from_string(ip), port),
			m_acc(m_service, m_ep) {}

		boost::asio::io_service m_service;
		boost::asio::ip::tcp::endpoint m_ep;
		boost::asio::ip::tcp::acceptor m_acc;
	};
	typedef boost::shared_ptr<boost_asio> boost_asio_ptr;

	boost_asio_ptr	m_ba_ptr;

public:
	void init(const std::string& addr)
	{
		std::string ip;
		int port;

		auto pos = addr.find(":");
		if (pos != std::string::npos) {
			ip = addr.substr(0, pos);
			port = atoi(addr.substr(pos + 1, addr.size() - pos - 1).c_str());
		} else {
			ip = "0.0.0.0";
			port = atoi(addr.c_str());
		}

		m_ba_ptr = boost_asio_ptr(new boost_asio(ip, port));
	}

	void run()
	{
		if (m_ba_ptr) {
			net::boost_socket_ptr socket_accept(new net::boost_socket(m_ba_ptr->m_service));
			start_accept(socket_accept);

			while (true) {
				try {
					m_ba_ptr->m_service.run();
				} catch (const net::handle_close_exception& e) {
					handle_close(e.socket_number());
				} catch (const boost::system::system_error& e) {
					lo::l(lo::ERROR) << "server::run: " << e.what();
					m_sockets.clear();
				} catch (...) {
					lo::l(lo::FATAL) << "server::run";
					throw 0;
				}
			}
		}
	}

private:
	void start_accept(net::boost_socket_ptr arg_socket)
	{
		m_ba_ptr->m_acc.async_accept(*arg_socket, boost::bind(&server::handle_accept, this, arg_socket, _1));
	}

	void handle_accept(net::boost_socket_ptr arg_socket, const boost::system::error_code &err)
	{
		if (err) return;

		net::boost_socket_ptr socket_accept(new net::boost_socket(m_ba_ptr->m_service));
		start_accept(socket_accept);

		req_proc_ptr s_tl(new REQ_PROC(arg_socket));
		while (check_number(s_tl->get_number())) s_tl->counter_increment();
		m_sockets.insert(std::pair<int, req_proc_ptr>(s_tl->get_number(), s_tl));
		s_tl->handle_accept(err);
	}

	void handle_close(int number)
	{
		auto it = m_sockets.find(number);
		if (it != m_sockets.end()) m_sockets.erase(it);
	}

	bool check_number(int number)
	{
		auto it = m_sockets.find(number);
		return it != m_sockets.end();
	}

public:
	size_t size()
	{
		return m_sockets.size();
	}
};

} // namespace server

#endif
