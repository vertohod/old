#include "configure.h"
#include "writer.h"

namespace chatd
{

writer::writer() : m_oid(0), m_begin(0)
{
}

writer::writer(OID oid, sptr_cstr name, time_t begin, OID room) :
	m_oid(oid), m_name(name), m_begin(begin), m_room(room)
{
}

writer::writer(const writer& obj)
{
	m_oid = obj.m_oid;
	m_name = obj.m_name;
	m_begin = obj.m_begin;
	m_room = obj.m_room;
}

writer& writer::operator=(const writer& obj)
{
	m_oid = obj.m_oid;
	m_name = obj.m_name;
	m_begin = obj.m_begin;
	m_room = obj.m_room;

	return *this;
}

OID writer::get_oid() const
{
	return m_oid;
}

sptr_cstr writer::get_name() const
{
	return m_name;
}

time_t writer::get_begin() const
{
	return m_begin;
}

OID writer::get_room() const
{
	return m_room;
}

writers::writers()
{
}

void writers::add_writer(OID oid, sptr_cstr name, OID room)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (name->empty()) return;

	m_writers[name].reset(new writer(oid, name, time(NULL), room));
}

void writers::del_writer(sptr_cstr name)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (name->empty()) return;

	m_writers.erase(name);
}

sptr<const writers_t> writers::get_writers(OID room)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr<writers_t> res;

	for (auto& pr : m_writers) {
		if (pr.second->get_room() != room) continue;
		res->insert(pr);
	}

	return res; 
}

void writers::clear()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto& cfg = config();

	writers_t wr_temp;
	wr_temp.swap(m_writers);

	for (auto wr_pr : wr_temp) {
		if (time(NULL) - wr_pr.second->get_begin() > cfg.timeout_writer)
			continue;

		m_writers.insert(std::make_pair(wr_pr.first, wr_pr.second));
	}
}

} // end of namespace chatd
