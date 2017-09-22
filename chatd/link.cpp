#include "link.h"

size_t std::hash<UUID>::operator()(const UUID& obj) const
{
	return boost::hash<UUID>()(obj);
}

size_t std::hash<sptr<const UUID>>::operator()(sptr<const UUID> obj) const
{
	return boost::hash<UUID>()(*obj);
}

size_t std::hash<sptr_cstr>::operator()(sptr_cstr obj) const
{
	return std::hash<std::string>()(*obj);
}


link::link(sptr<const UUID> uuid, OID oid) : m_oid(oid)
{
	m_uuids.insert(uuid);
}

OID link::oid()
{
	return m_oid;
}

uuid_set_t link::uuids()
{
	return m_uuids;
}

void link::add_uuid(sptr<const UUID> uuid)
{
	m_uuids.insert(uuid);
}

void link::del_uuid(sptr<const UUID> uuid)
{
	m_uuids.erase(uuid);
}

