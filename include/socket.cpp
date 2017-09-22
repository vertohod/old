#include "socket.h"
#include "log.h"

namespace net
{

handle_close_exception::handle_close_exception(int socket_number) : m_socket_number(socket_number) {}

const char* handle_close_exception::what() const noexcept
{
	return NULL;
}

int handle_close_exception::socket_number() const noexcept
{
	return m_socket_number;
}

int socket::m_socket_counter = 0;
boost::mutex socket::m_mutex;

socket::socket(boost_socket_ptr arg_socket) : 
	m_socket(arg_socket), m_data_size(0), m_status(ST_EMPTY)
{
	counter_increment();
}

void socket::counter_increment()
{
	{
		boost::unique_lock<boost::mutex> lock(m_mutex);
		++m_socket_counter;
	}
	m_socket_number = m_socket_counter;
}

int socket::get_status()
{
	return m_status;
}
	
int socket::get_number()
{
	return m_socket_number;
}
	
void socket::set_status(int arg_status)
{
	m_status = arg_status;
}

void socket::_handle_receive(const boost::system::error_code &err, size_t bytes)
{
	m_data_size += bytes;

	if (err == boost::asio::error::eof ||
		err == boost::asio::error::connection_reset) {

		throw handle_close_exception(get_number());
	} else {
		handle_receive(err, bytes);
	}
}

void socket::send(const std::string& data)
{
	m_socket->async_send(boost::asio::buffer(data.c_str(), data.size()), boost::bind(&socket::handle_send, this, _1));
}	

void socket::receive()
{
	if (m_data->size() == 0) m_data->resize(EXTENSION_BUFFER);
	else if (m_data->size() - m_data_size == 0) m_data->resize(m_data->size() * 1.5);

	size_t length = m_data->size() - m_data_size;

	m_socket->async_receive(boost::asio::buffer(&m_data->at(m_data_size), length), boost::bind(&socket::_handle_receive, this, _1, _2));
}
	
size_t socket::get_data_size()
{
	return m_data_size;
}

sptr_str socket::get_data()
{
	m_data->resize(m_data_size);
	return m_data;
}

void socket::close()
{
	throw handle_close_exception(get_number());
}

socket::~socket()
{
	try {
		m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    } catch (const boost::system::system_error& e) {
		lo::l(lo::ERROR) << e.what();
	}

	try {
		m_socket->close();
    } catch (const boost::system::system_error& e) {
		lo::l(lo::ERROR) << e.what();
	}
}

} // namespace net
