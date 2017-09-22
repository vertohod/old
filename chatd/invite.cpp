#include <boost/thread/detail/singleton.hpp>
#include <functional>

#include "invite.h"
#include "dumper.h"
#include "error.h"
#include "util.h"

using namespace boost::detail::thread;

namespace chatd
{

invite::invite(
	OID oid,
	sptr_cstr name,
	OID owner,
	OID room,
	time_t start,
	time_t expire,
	size_t really_times,
	sptr_cstr password,
	sptr_cstr permission
	) :
	object(oid, name, owner),
	m_room(room),
	m_start(start),
	m_expire(expire),
	m_really_times(really_times),
	m_count(0)
{
	if (!password->empty()) {
		m_password = std::hash<std::string>()(*password);
	} else m_password = 0;

	set_permission(*permission);
}

invite::invite(sptr_cmap data) : object(data),
	m_room(util::get_val<OID>(*data, "oid")),
	m_start(util::get_val<OID>(*data, "start")),
	m_expire(util::get_val<OID>(*data, "expire")),
	m_really_times(util::get_val<OID>(*data, "really_times")),
	m_password(util::get_val<OID>(*data, "password"))
{
	auto permission = util::get_val<std::string>(*data, "permission");
	set_permission(permission);
}

void invite::set_permission(const std::string& permission)
{
	auto oids_str = util::split(permission, ",");

	for (auto& oid_str : *oids_str) {
		object::add_allow(format<OID>(oid_str));
	}
}

bool invite::check(OID user, const std::string& password)
{
	if (m_start != 0 && m_start < time(NULL)) return false;
	if (m_expire != 0 && m_expire > time(NULL)) return false;
	if (m_really_times != 0 && m_really_times <= m_count) return false;

	if (object::check_allow(user)) {
		if (m_password != 0) {
			if (password.empty() || m_password != std::hash<std::string>()(password)) {
				// Редирект на форму ввода пароля
				throw error<302>(sptr_cstr());
			}
		}

		std::unique_lock<std::mutex> lock(m_mutex);

		++m_count;
		singleton<dumper>::instance().save(count_to_string());

		return true;
	}

	return false;
}

bool invite::valid() const
{
	if (m_expire != 0 && m_expire > time(NULL)) return false;
	if (m_really_times != 0 && m_really_times <= m_count) return false;

	return true;
}

size_t invite::count() const
{
	return m_count;
}

void invite::set_count(size_t count)
{
	m_count = count;
}

OID invite::get_room() const
{
	return m_room;
}

sptr_cstr invite::to_string(const std::string& func) const
{
    sptr_str res;
    sp::stream  st(*res);

    st << "object=" << OB_invite;
    st << "\t";
    st << "func=" << func;
    st << "\t";
    st << *object::to_string();
    st << "\t";
    st << "room=" << m_room;
    st << "\t";
    st << "start=" << m_start;
    st << "\t";
    st << "expire=" << m_expire;
    st << "\t";
    st << "really_times=" << m_really_times;
    st << "\t";
    st << "password=" << m_password;
    st << "\t";
    st << "permission=" << object::get_allows();

    return res;
}

sptr_cstr invite::count_to_string() const
{
    sptr_str res;
    sp::stream  st(*res);

    st << "object=" << OB_invite_count;
    st << "\t";
    st << "func=" << func_add;
    st << "\t";
    st << "oid=" << object::get_oid();
    st << "\t";
    st << "count=" << m_count;

    return res;
}


// -------------------------------------------------------

invite_store::invite_store()
{
}

OID invite_store::add(
	sptr_cstr name,
	OID owner,
	OID room,
	time_t start,
	time_t expire,
	size_t really_times,
	sptr_cstr password,
	sptr_cstr permission)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto res = store::oid(false, 19); // 19-ти битный. Меньше миллиона

	auto invite_ptr = sptr<invite>(new invite(
		res,
		name,
		owner,
		room,
		start,
		expire,
		really_times,
		password,
		permission
	));

	store::add(invite_ptr);
	singleton<dumper>::instance().save(invite_ptr->to_string(func_add));

	return res;
}

void invite_store::del(OID oid)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto invite_ptr = store::get(oid);
	singleton<dumper>::instance().save(invite_ptr->to_string(func_del));

	store::del(oid);
}

void invite_store::load(sptr_cmap data)
{
	auto object = util::get_val<std::string>(*data, "object");
	auto func = util::get_val<std::string>(*data, "func");

	if (object == OB_invite) {

		sptr<invite> invite_ptr(new invite(data));

		if (func == func_add) {
			store::add(invite_ptr);
		} else {
			store::del(invite_ptr->get_oid());
		}

	} else if (object == OB_invite_count) {

		auto oid = util::get_val<OID>(*data, "oid");
		auto count = util::get_val<size_t>(*data, "count");

		auto invite_ptr = store::get(oid);
		invite_ptr->set_count(count);

	}
}

} // end of namespace chatd
