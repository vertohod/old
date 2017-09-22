#ifndef INVITE_H
#define INVITE_H

#include <mutex>

#include "store.h"
#include "types.h"

#ifndef func_add
	#define func_add	"add"
#endif

#ifndef func_del
	#define func_del	"del"
#endif

namespace chatd
{

class invite : public object
{
private:
	OID			m_room;
	time_t		m_start;			// действителен начиная с этого времени
	time_t		m_expire;			// истекает
	size_t		m_really_times;		// действителен раз
	size_t		m_password;			// пароль
	size_t		m_count;			// сколько раз воспользовались

    mutable std::mutex	m_mutex;

public:
	invite(
		OID oid,
		sptr_cstr name,
		OID owner,
		OID room,
		time_t start,
		time_t expire,
		size_t really_times,
		sptr_cstr password,
		sptr_cstr permission
	);
	invite(sptr_cmap data);

	bool check(OID user, const std::string& password);
	bool valid() const;				// действительно ли еще приглашение или можно удалить
	size_t count() const;
	void set_count(size_t);
	OID get_room() const;

	sptr_cstr to_string(const std::string& func) const;

private:
	sptr_cstr count_to_string() const;
	void set_permission(const std::string& permission);
};

class invite_store : public store<invite>
{
private:
    std::mutex      m_mutex;

public:
	invite_store();

	OID add(
		sptr_cstr name, // Свободная строка. Может содержать описание приглашения
		OID owner,
		OID room,
		time_t start,
		time_t expire,
		size_t really_times,
		sptr_cstr password,
		sptr_cstr permission
	);

	void del(OID oid);

	void load(sptr_cmap data);
};

#ifndef OB_invite
	#define OB_invite			"invite"
#endif

#ifndef OB_invite_count
	#define OB_invite_count		"invite_count"
#endif

} // end of namespace chatd

#endif
