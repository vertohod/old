#ifndef SOCKET_H
#define SOCKET_H

#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#include <map>

#include "types.h"

namespace net
{

class handle_close_exception : public std::exception
{
    int m_socket_number;

public:
    explicit handle_close_exception(int socket_number);
    const char* what() const noexcept;
	int socket_number() const noexcept;
};


typedef boost::asio::ip::tcp::socket		boost_socket;
typedef boost::shared_ptr<boost_socket>		boost_socket_ptr;

class socket
{
private:
	static int m_socket_counter;
	static boost::mutex m_mutex;

protected:
	#define EXTENSION_BUFFER 1024

private:
	boost_socket_ptr	m_socket;
	sptr_str			m_data;
	size_t				m_data_size;
	int					m_status;
	int					m_socket_number;

public:	
	enum {
		ST_EMPTY = 0,
		ST_RECEIVE,
		ST_SEND,
		ST_CLOSE,
		ST_RECEIVE_RECEIVE,
		ST_RECEIVE_SEND,
		ST_SEND_SEND,
		ST_SEND_RECEIVE,
		ST_RECEIVE_CLOSE,
		ST_SEND_CLOSE
	};

	socket(boost_socket_ptr arg_socket);
	virtual ~socket();

	void counter_increment();
	int get_status();
	int get_number();
	void set_status(int arg_status);

	virtual void handle_receive(const boost::system::error_code &err, size_t bytes) = 0;
	virtual void handle_send(const boost::system::error_code &err) = 0;
	void send(const std::string& data);
	void receive();
	size_t get_data_size();
	sptr_str get_data();
	void close();

private:
	void _handle_receive(const boost::system::error_code &err, size_t bytes);
};

}

#endif
