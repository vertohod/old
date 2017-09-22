#ifndef WRITER_H
#define WRITER_H

#include <mutex>
#include <map>

#include "types.h"

namespace chatd
{

class writer
{
private:
	OID				m_oid;
	sptr_cstr		m_name;
	time_t			m_begin;
	OID				m_room;

public:
	writer();
	writer(OID oid, sptr_cstr name, time_t begin, OID room);
	writer(const writer&);
	writer& operator=(const writer&);

	OID get_oid() const;
	sptr_cstr get_name() const;
	time_t get_begin() const;
	OID get_room() const;
};

typedef std::map<sptr_cstr, sptr<const writer>, comparer_sptr_cstr> writers_t;

class writers
{
private:
	writers_t	m_writers;
	std::mutex	m_mutex;

public:
	writers();
	void add_writer(OID oid, sptr_cstr name, OID room);
	void del_writer(sptr_cstr name);
	sptr<const writers_t> get_writers(OID room);
	void clear();
};

} // end of namespace chatd

#endif
