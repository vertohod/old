#include "blizzard.h"

namespace blizzard
{

table_interface* blizzard::table(const std::string& table_name)
{
	auto it = m_table_map.find(table_name);

	if (it == m_table_map.end()) throw std::runtime_error(""/*TODO*/);

	return it->second.get();
}

OID blizzard::insert(std::shared_ptr<const object> object)
{
	auto res = table(object->get_name())->insert(object);
	shard(object);

	return res;
}

bool blizzard::update(std::shared_ptr<const object> object)
{
	auto res = table(object->get_name())->update(object);
	shard(object);

	return res;
}

bool blizzard::remove(std::shared_ptr<const object> object)
{
	auto res = table(object->get_name())->remove(object);
	shard(object);

	return res;
}

void blizzard::shard(std::shared_ptr<const object> object)
{
	// TODO
}

} // end of namespace

blizzard::blizzard& bl()
{
	return singleton<blizzard::blizzard>::instance();
}
