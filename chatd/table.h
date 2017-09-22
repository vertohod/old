#ifndef TABLE_H
#define TABLE_H

#include <unordered_set>
#include <random>
#include <memory>
#include <set>

#include "service_messages.h"
#include "object.h"
#include "format.h"
#include "types.h"
#include "error.h"

namespace blizzard
{

// В общем случае используется для любых ключей
class key
{
public:

	template <typename T>
	class hash
	{
	public:
		size_t operator()(const T& obj) const
		{
			return obj.hash();
		}
		size_t operator()(const std::shared_ptr<T>& obj_ptr) const
		{
			return obj_ptr->hash();
		}
	};

	template <typename T>
	class equal
	{
	public:
		bool operator()(const T& obj1, const T& obj2) const
		{
			return obj1 == obj2;
		}
		bool operator()(const std::shared_ptr<T>& obj_ptr1, const std::shared_ptr<T>& obj_ptr2) const
		{
			return *obj_ptr1 == *obj_ptr2;
		}
	};

	template <typename T>
	class less
	{
	public:
		bool operator()(const T& obj1, const T& obj2) const
		{
			return obj1 < obj2;
		}
		bool operator()(const std::shared_ptr<T>& obj_ptr1, const std::shared_ptr<T>& obj_ptr2) const
		{
			return *obj_ptr1 < *obj_ptr2;
		}
	};
};

enum key_type {
	UNORDERED			= 1,
	UNORDERED_UNIQUE	= 2,
	ORDERED				= 3,
	ORDERED_UNIQUE		= 4
};

// Неупорядоченные ключи
// Неуникальный
typedef std::unordered_multiset<std::shared_ptr<const key_interface>, key::hash<const key_interface>, key::equal<const key_interface>> key_umset_t;
// Уникальный
typedef std::unordered_set<std::shared_ptr<const key_interface>, key::hash<const key_interface>, key::equal<const key_interface>> key_uset_t;
// Упорядоченные ключи
// Неуникальный
typedef std::multiset<std::shared_ptr<const key_interface>, key::less<const key_interface>> key_mset_t;
// Уникальный
typedef std::set<std::shared_ptr<const key_interface>, key::less<const key_interface>> key_set_t;

// результат поиска по ключал (множество OID-ов)
// порядок следования OID-ов важен, по этому vector
typedef std::vector<OID> result_vector_t;
typedef std::unique_ptr<result_vector_t> result_t;

// вставка ключа происходит в два этапа
// 1) проходим по всем хранилищам и вставляем ключи, при этом сохраняем итераторы
// 2) если добавление прошло успешно, делаем commit (теряем итераторы)
// 3) иначе по сохраненным итераторам удаляем записи

class key_store_interface
{
public:
	virtual bool add(const object&) = 0;
	virtual bool del(const object&) = 0;
	virtual void rollback() = 0;
	virtual void commit() = 0;
	virtual result_t find(std::shared_ptr<const key_interface>&) = 0;
	virtual const std::string& name() = 0;

	virtual ~key_store_interface();
};

template <typename KEY>
class key_store_umset : public key_store_interface
{
private:
	key_umset_t m_store;
	key_umset_t::iterator m_it_rollback;

public:
	typedef KEY key_type;

	key_store_umset()
	{
		m_it_rollback = m_store.end();
	}

	virtual bool add(const object& obj) override
	{
		m_it_rollback = m_store.insert(std::shared_ptr<const key_interface>(new key_type(obj)));
		return true;
	}

	virtual bool del(const object& obj) override
	{
		std::shared_ptr<const key_interface> key(new key_type(obj));

		auto pr = m_store.equal_range(key);
		for (auto it = pr.first; it != pr.second; ++it) {
			if ((*it)->get_index() == key->get_index()) {
				m_store.erase(it);
				return true;
			}
		}
		return false;
	}

	virtual void rollback() override
	{
		if (m_it_rollback != m_store.end()) m_store.erase(m_it_rollback);
		m_it_rollback = m_store.end();
	}

	virtual void commit() override
	{
		m_it_rollback = m_store.end();
	}

	virtual result_t find(std::shared_ptr<const key_interface>& key) override
	{
		result_t res(new result_vector_t());

		auto pr = m_store.equal_range(key);
		for_each(pr.first, pr.second, [&res](const std::shared_ptr<const key_interface>& val) { res->push_back(val->get_index()); });

		return res;
	}

	virtual const std::string& name() override
	{
		return key_type::m_name;
	}
};

template <typename KEY>
class key_store_uset : public key_store_interface
{
private:
	key_uset_t m_store;

public:
	virtual result_t find(std::shared_ptr<const key_interface>& key) override
	{
		result_t res(new result_vector_t());

		auto it = m_store.find(key);
		if (it != m_store.end()) {
			res->push_back((*it)->get_index());
		}

		return res;
	}
};

template <typename KEY>
class key_store_mset : public key_store_interface
{
private:
	key_mset_t m_store;

public:
	virtual result_t find(std::shared_ptr<const key_interface>& key) override
	{
		result_t res(new result_vector_t());

		auto pr = m_store.equal_range(key);
		for_each(pr.first, pr.second, [&res](const std::shared_ptr<const key_interface>& val) { res->push_back(val->get_index()); });

		return res;
	}
};

template <typename KEY>
class key_store_set : public key_store_interface
{
private:
	key_set_t m_store;
	key_set_t::iterator m_it_rollback;

public:
	typedef KEY key_type;

	key_store_set()
	{
		m_it_rollback = m_store.end();
	}

	virtual bool add(const object& obj) override
	{
		auto pr = m_store.insert(std::shared_ptr<const key_interface>(new key_type(obj)));

		if (pr.second) m_it_rollback = pr.first;

		return pr.second;
	}

	virtual bool del(const object& obj) override
	{
		std::shared_ptr<const key_interface> key(new key_type(obj));

		auto it = m_store.find(key);
		if (it != m_store.end()) {
			m_store.erase(it);
			return true;
		}

		return false;
	}

	virtual void rollback() override
	{
		if (m_it_rollback != m_store.end()) m_store.erase(m_it_rollback);
		m_it_rollback = m_store.end();
	}

	virtual void commit() override
	{
		m_it_rollback = m_store.end();
	}

	virtual result_t find(std::shared_ptr<const key_interface>& key) override
	{
		result_t res(new result_vector_t());

		auto it = m_store.find(key);
		if (it != m_store.end()) {
			res->push_back((*it)->get_index());
		}

		return res;
	}

	virtual const std::string& name() override
	{
		return key_type::m_name;
	}
};

class table_interface
{
public:
	virtual void process(const std::string& line) = 0;

	virtual std::weak_ptr<const object> get(OID oid) = 0;
	virtual OID insert(std::shared_ptr<const object>& obj, bool set_index = true) = 0;
	virtual bool remove(std::shared_ptr<const object>& obj) = 0;
	virtual bool update(std::shared_ptr<const object>& obj) = 0;
	virtual size_t size() = 0;

	template<typename T>
	result_t find(std::shared_ptr<T>& key)
	{
		std::shared_ptr<const key_interface> key_temp = key;
		return _find(key_temp);
	}

	virtual ~table_interface();

private:
	virtual result_t _find(std::shared_ptr<const key_interface>& key) = 0;
};

typedef std::unordered_map<std::string, std::shared_ptr<key_store_interface>> keys_stores_t;

// Наследуется всеми таблицами. По умолчанию не имеет ключей,
// но производные таблицы могут иметь любое количество ключей и должны обрабатывать их
template <typename T>
class table : public table_interface
{
private:
	enum state_t
	{
		START	= 0,
		LOAD	= 1,
		SEND	= 2,
		READY	= 3,
	};

	// Индексы должны быть уникальными
	typedef std::unordered_map<OID, std::shared_ptr<const object>> object_store_t;

	object_store_t			m_object_store; // основное хранилище. хранит только свои записи
	object_store_t			m_object_cache; // хранит чужие записи на протяжении короткого времени

	keys_stores_t			m_keys_stores;

	const bool				m_increment;
	const size_t&			m_server_number;
	const size_t&			m_max_number;

	state_t					m_state;
	OID						m_last_oid;
	const std::string&		m_name;

	//boost::shared_mutex		m_mutex;

public:
	table(const size_t& server_number, const size_t& max_number) :
		m_name(T::name()),
		m_increment(T::increment()),
		m_server_number(server_number),
		m_max_number(max_number),
		m_state(START),
		m_last_oid(max_number)
	{
		init_keys();
	}

public:
	virtual void process(const std::string& line) override
	{
		std::shared_ptr<const object> object_temp(new T());
		const_cast<object*>(object_temp.get())->deserialization(line);

		if (object_temp->get_event() == object::SELECT) {

//			object_select(object_temp);

		} else if (object_temp->get_event() == object::INSERT) {

			insert(object_temp);

		} else if (object_temp->get_event() == object::UPDATE) {

			update(object_temp);

		} else if (object_temp->get_event() == object::REMOVE) {

			remove(object_temp);

		}
	}

private:
	void init_keys()
	{
		m_keys_stores.swap(*T::init_keys());
	}

	bool keys_insert(const object& object, keys_stores_t::iterator it)
	{
		if (it == m_keys_stores.end()) return true;

		auto res = it->second->add(object);

		if (res && keys_insert(object, ++it)) return true;
		else {
			if (res) it->second->rollback();
			return false;
		}
		it->second->commit();

		return true;
	}

	bool keys_delete(const object& object, keys_stores_t::iterator it)
	{
		if (it == m_keys_stores.end()) return true;

		it->second->del(object);
		keys_delete(object, ++it);

		return true;
	}

	size_t get_mask(size_t bits, size_t sn_bits)
	{
		size_t res = 0;
		for (size_t count = 0; count < (bits - sn_bits); ++count) {
			res = res << 1 | 1;
		}
		return res;
	}

	size_t get_server_number(size_t bits, size_t sn_bits)
	{
		size_t res = m_server_number;
		for (size_t count = 0; count < (bits - sn_bits); ++count) {
			res = res << 1;
		}
		return res;
	}

	OID oid(bool increment = false, size_t bits = 52, size_t sn_bits = 12)
	{
		// Ограничение количества бит в OID (для корректной работы в JS)
		static const size_t mask = get_mask(bits, sn_bits);
		// В старшие биты зашиваем номер сервера
		static const size_t server_number = get_server_number(bits, sn_bits);

		static std::mt19937_64 generator;
		static std::uniform_int_distribution<size_t> distribution(1, mask);

		while (true) {
			OID res = increment ? (++m_last_oid * m_max_number): distribution(generator);
			res |= server_number;
			res = res - (res % m_max_number) + m_server_number;
			if (check(res)) return res;
		}

		return 0;
	}

public:
	virtual OID insert(std::shared_ptr<const object>& obj_ptr, bool set_index = true) override
	{
		if (set_index) {
			const_cast<object*>(obj_ptr.get())->set_index(oid(m_increment));
		}

		if (!keys_insert(*obj_ptr, m_keys_stores.begin())) return 0;

		auto pr = m_object_store.insert(std::make_pair(obj_ptr->get_index(), obj_ptr));

		if (!pr.second) {
			keys_delete(*obj_ptr, m_keys_stores.begin());
			return 0;
		}

		return obj_ptr->get_index();
	}

	virtual bool remove(std::shared_ptr<const object>& obj_ptr) override
	{
		keys_delete(*obj_ptr, m_keys_stores.begin());

		return m_object_store.erase(obj_ptr->get_index());
	}

	virtual bool update(std::shared_ptr<const object>& obj_ptr) override
	{
		auto obj_temp = get(obj_ptr->get_index()).lock();
		remove(obj_temp);

		return insert(obj_ptr, false);
	}

	virtual std::weak_ptr<const object> get(OID oid) override
	{
		auto it = m_object_store.find(oid);

		if (it == m_object_store.end()) throw error<500>(smessage(SMESS220_OBJECT_NOT_FOUND, format(oid)));

		return it->second;
	}

	virtual size_t size() override
	{
		return m_object_store.size();
	}

	bool check(OID oid)
	{
		auto it = m_object_store.find(oid);
		return it == m_object_store.end();
	}

private:
	result_t _find(std::shared_ptr<const key_interface>& key) override
	{
		auto it = m_keys_stores.find(key->name());
		if (it == m_keys_stores.end()) throw std::runtime_error("Unknown key: " + key->name());

		return it->second->find(key);
	}
};

} // end of namespace
#endif
