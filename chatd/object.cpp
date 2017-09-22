#include <ctime>

#include "object.h"
#include "format.h"
#include "stream.h"
#include "util.h"

namespace blizzard
{

// обязательно добно быть выше чем определение полей DEFINITION_FIELD
DEFINITION_FIELDS_FUNCTIONS_STORES(object)

DEFINITION_FIELD(object, unsigned char, event)
DEFINITION_FIELD(object, time_t, time)
DEFINITION_FIELD(object, OID, index)
DEFINITION_FIELD(object, OID, owner)

object::object() :
	FIELD(time)(0), FIELD(event)(object::NONE), FIELD(owner)(0), FIELD(index)(0)
{}

object::~object() {}

size_t object::hash() const
{
	return std::hash<OID>()(FIELD(index));
}

bool object::operator==(const object& obj)
{
	return FIELD(index) == obj.FIELD(index);
}

bool object::operator<(const object& obj)
{
	return FIELD(index) < obj.FIELD(index);
}

std::unique_ptr<const key_interface> object::get_key(const std::string& name)
{
	// этот класс содержит только один ключ
	// производные могут содержать больше
	if (name == object::key_index::m_name) {

		return std::unique_ptr<key_interface>(new key_index(*this));

	} else throw std::runtime_error("Unknown name of key: " + name);

	// эта строка никогда не сработает
	return std::unique_ptr<key_interface>(new key_index(*this));
}

uptr_str object::serialization() const
{
	uptr_str res;
	sp::stream st(*res);

	for (auto& pr : m_serialization_functions) {
		st << pr.first << "=" << pr.second(this) << SEPARATOR;
	}

	return res;
}

void object::deserialization(const std::string& line)
{
	static const std::string separator(SEPARATOR);
    auto fields = util::split(line, separator);

    for (auto& field : *fields) {
        auto pos = field.find("=");
        if (pos != std::string::npos) {
            init_field(field.substr(0, pos), field.substr(pos + 1, field.size() - pos - 1));
        }
    }
}

bool object::init_field(const std::string& key, const std::string& val)
{
	auto it = m_deserialization_functions.find(key);

	if (it != m_deserialization_functions.end()) {
		it->second(this, val);
		return true;
	}

	return false;
}

void object::copy_fields(object& obj) const
{
	for (auto& pr : m_serialization_functions) {
		obj.init_field(pr.first, pr.second(this));
	}
}

uptr_cstr object::get_field(const std::string& line, const std::string& field_name)
{
	auto pos = line.find(field_name);
	if (pos != std::string::npos) {

		auto pos_end = line.find(SEPARATOR, pos + field_name.size());
		// Если не найден разделитель, берем до конца строки
		if (pos_end == std::string::npos) pos_end = line.size();

		return line.substr(pos + field_name.size() + 1, pos_end - pos - field_name.size() - 1);

	} else throw std::runtime_error("The field wasn't found (" + field_name + "), line: " + line);

	return "";
}

bool object::operator==(const object& obj) const
{
	return FIELD(index) == obj.FIELD(index);
}


// ----------------------------- KEYS ---------------------------------


key_interface::key_interface(OID index) : m_index(index) {}
key_interface::~key_interface() {}
OID key_interface::get_index() const { return m_index; }
void key_interface::set_index(OID index) { m_index = index; }
// INDEX
const std::string object::key_index::m_name = "index";
object::key_index::key_index() : key_interface(0) {}
object::key_index::key_index(const object& obj) : key_interface(obj.get_index()) {}
const std::string& object::key_index::name() const
{
	return m_name;
}
size_t object::key_index::hash() const
{
	return std::hash<OID>()(get_index());
}
bool object::key_index::operator==(const key_interface& obj) const
{
	return get_index() == obj.get_index();
}
bool object::key_index::operator<(const key_interface& obj) const
{
	return get_index() < obj.get_index();
}

// OWNER
const std::string object::key_owner::key_owner::m_name = "owner";
object::key_owner::key_owner() : key_interface(0), m_owner(0) {}
object::key_owner::key_owner(const object& obj) : key_interface(obj.get_index()), m_owner(obj.get_owner()) {}
const std::string& object::key_owner::name() const
{
	return m_name;
}
OID object::key_owner::get_owner() const
{
	return m_owner;
}
void object::key_owner::set_owner(OID oid)
{
	m_owner = oid;
}
size_t object::key_owner::hash() const
{
	return std::hash<OID>()(m_owner);
}
bool object::key_owner::operator==(const key_interface& obj) const
{
	return m_owner == dynamic_cast<const key_owner*>(&obj)->get_owner();
}
bool object::key_owner::operator<(const key_interface& obj) const
{
	return m_owner < dynamic_cast<const key_owner*>(&obj)->get_owner();
}
// -----------------------------------------------------------------------
} // end of namespace
