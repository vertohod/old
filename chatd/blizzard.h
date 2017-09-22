#ifndef BLIZZARD_H
#define BLIZZARD_H

#include <boost/thread/detail/singleton.hpp>
#include <unordered_map>
#include <memory>

#include "singleton.h"
#include "object.h"
#include "types.h"
#include "table.h"

#define FIELD_SERVER	"@server" // номер сервера, который сгенирировал дамп. RESULT содержит номер получателя
#define FIELD_NUMBER	"@number" // порядковые/случайные значения для сопоставления SELECT и RESULT
#define FIELD_KEY		"@key"

namespace blizzard
{

class blizzard
{
private:
	typedef std::unordered_map<std::string, std::shared_ptr<table_interface>> table_map_t;

	table_map_t m_table_map;
	table_map_t m_cache_map;

	const size_t& m_server_number;
	const size_t& m_max_number;

public:
	blizzard(const size_t& server_number, const size_t& max_number) : m_server_number(server_number), m_max_number(max_number) {}

	template <typename T>
	inline void add_table()
	{
		auto it = m_table_map.find(T::name());

		if (it == m_table_map.end()) {

			auto pr = m_table_map.insert(
				std::make_pair(
					T::name(),
					std::shared_ptr<table_interface>(static_cast<table_interface*>(new ::blizzard::table<T>(m_server_number, m_max_number)))
				)
			);

			if (pr.second) it = pr.first;
			else throw std::runtime_error("Error during adding storage with name: " + T::name());

		} else throw std::runtime_error("The storage with such name exist already (" + T::name() + ")");
	}

	table_interface* table(const std::string& table_name);

	// Эти методы нужны для вызова shard, таблица не должна об этом заботиться.
	OID insert(std::shared_ptr<const object> object);
	bool update(std::shared_ptr<const object> object);
	bool remove(std::shared_ptr<const object> object);

	template <typename T>
	std::weak_ptr<const object> get(OID oid)
	{
		return table(T::name())->get(oid);
	}

	template <typename T>
	result_t find(std::shared_ptr<const key_interface> key)
	{
		return table(T::name())->find(key);
	}
	// ---------

private:
	void shard(std::shared_ptr<const object> object);

};
} // end of namespace

blizzard::blizzard& bl();
#endif
