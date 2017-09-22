#ifndef LINK_H
#define LINK_H

#include <unordered_map>
#include <unordered_set>

#include "types.h"

namespace std {
	template<>
	class hash<UUID>
	{
	public:
		size_t operator()(const UUID&) const;
	};

	template<>
	class hash<sptr<const UUID>>
	{
	public:
		size_t operator()(sptr<const UUID>) const;
	};
}

class link_store;

typedef std::unordered_set<sptr<const UUID>, std::hash<sptr<const UUID>>, equal_sptr<const UUID>> uuid_set_t;

class link
{
private:
	OID			m_oid; // oid пользователя

	uuid_set_t	m_uuids;

public:
	link(sptr<const UUID> uuid, OID oid);

	OID oid();
	uuid_set_t uuids();

private:
	void add_uuid(sptr<const UUID> uuid);
	void del_uuid(sptr<const UUID> uuid);

	friend link_store;
};

class link_store
{
private:
	typedef std::unordered_map<sptr<const UUID>, sptr<link>, std::hash<sptr<const UUID>>, equal_sptr<const UUID>> uuid_oid_t;

	uuid_oid_t								m_uuid_oid;
	std::unordered_map<OID, sptr<link>>		m_oid_oid;

public:
	add_uuid(sptr<const UUID> uuid, OID oid);
	del(sptr<const UUID> uuid);
	del(OID oid);
	get_oid(sptr<const UUID> uuid);
};


#endif
