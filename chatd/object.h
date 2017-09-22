#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace blizzard
{

#define DESER(OBJECT, TYPE, NAME) [](blizzard::object* ptr, const std::string& val){dynamic_cast<OBJECT*>(ptr)->set_##NAME(format<TYPE>(val));}
#define SER(OBJECT, NAME) [](const blizzard::object* ptr){ return format(dynamic_cast<const OBJECT*>(ptr)->get_##NAME()); }

#define ADD_FUNCTION(OBJECT, TYPE, NAME) \
struct add_function_into_map_##OBJECT##_##NAME\
{add_function_into_map_##OBJECT##_##NAME(){\
OBJECT::m_serialization_functions.push_back(std::make_pair(#NAME, SER(OBJECT, NAME)));\
OBJECT::m_deserialization_functions.insert(std::make_pair(#NAME, DESER(OBJECT, TYPE, NAME)));\
}};\
add_function_into_map_##OBJECT##_##NAME object_for_add_function_##OBJECT##_##NAME;

// ------ добавление полей в объекты -----------------------------------------------------------
#define DECLARATION_FIELD(TYPE, NAME)\
private: TYPE m_field_##NAME;\
public: TYPE get_##NAME() const; void set_##NAME(const TYPE& val);

#define DEFINITION_FIELD(OBJECT, TYPE, NAME)\
TYPE OBJECT::get_##NAME() const {return m_field_##NAME;}\
void OBJECT::set_##NAME(const TYPE& val) {m_field_##NAME = val;}\
ADD_FUNCTION(OBJECT, TYPE, NAME)

#define FIELD(NAME) m_field_##NAME

#define DECLARATION_CONSTS private: static const std::string m_name; static const bool m_increment; static const int m_version;\
public: virtual const std::string& get_name() const override; int get_version() const;\
void set_name(const std::string&); void set_version(int);\
static const std::string& name();\
static bool increment();

#define DEFINITION_CONSTS(OBJECT, NAME, INCREMENT, VERSION)\
const std::string OBJECT::m_name = NAME;\
const bool OBJECT::m_increment = INCREMENT;\
const int OBJECT::m_version = VERSION;\
const std::string& OBJECT::get_name() const {return m_name;}\
int OBJECT::get_version() const {return m_version;}\
void OBJECT::set_name(const std::string& val){} void OBJECT::set_version(int val){}\
ADD_FUNCTION(OBJECT, const std::string, name)\
ADD_FUNCTION(OBJECT, int, version)\
const std::string& OBJECT::name(){return OBJECT::m_name;}\
bool OBJECT::increment(){return OBJECT::m_increment;}

#define DECLARATION_FIELDS_FUNCTIONS_STORES \
static deserialization_functions_t m_deserialization_functions;\
static serialization_functions_t m_serialization_functions;

#define DEFINITION_FIELDS_FUNCTIONS_STORES(OBJECT) \
OBJECT::deserialization_functions_t OBJECT::m_deserialization_functions;\
OBJECT::serialization_functions_t OBJECT::m_serialization_functions;

// ---------------------------------------------------------------------------------------------

// дружественный класс для object
template <typename T> class table;

#define SEPARATOR "\t"

struct key_interface
{
private:
	OID m_index;

public:
	key_interface(OID index);
	virtual ~key_interface();
	OID get_index() const;
	void set_index(OID);

	virtual const std::string& name() const = 0;
	virtual size_t hash() const = 0;
	virtual bool operator==(const key_interface&) const = 0;
	virtual bool operator<(const key_interface&) const = 0;
};

class object
{
protected:
	typedef std::unordered_map<std::string, std::function<void(object*, const std::string&)>> deserialization_functions_t;
	typedef std::vector<std::pair<std::string, std::function<const std::string(const object*)>>> serialization_functions_t;

public:
	DECLARATION_FIELDS_FUNCTIONS_STORES

public:
	enum EVENT {
		NONE	= 0,
		SELECT	= 1,
		INSERT	= 2,
		UPDATE	= 3,
		REMOVE	= 4,
		RESULT	= 5
	};

	// указатель на контейнейр с индексами
	// указательно на мьютекс таблицы

private:
	DECLARATION_FIELD(unsigned char, event)
	DECLARATION_FIELD(time_t, time)
	DECLARATION_FIELD(OID, index)
	DECLARATION_FIELD(OID, owner)

public:
	object();
	virtual ~object();

	virtual const std::string& get_name() const = 0;
	virtual std::shared_ptr<object> make_copy() const = 0;

	// дня хранения в таблице
	size_t hash() const;
	bool operator==(const object&);
	bool operator<(const object&);
	// --------------------

	virtual std::unique_ptr<const key_interface> get_key(const std::string& name);
	virtual uptr_str serialization() const;
	void deserialization(const std::string& line);

protected:
	// вызывается из производного класса
	virtual bool init_field(const std::string& key, const std::string& val);
	void copy_fields(object& obj) const;

public:
	static uptr_cstr get_field(const std::string& line, const std::string& field_name);

public:
	bool operator==(const object& obj) const;

public:
	// Пример ключа.
	// этот может пригодиться, если понадобится упорядочить по index
	struct key_index : public key_interface
	{
	public:
		static const std::string m_name;

		key_index();
		key_index(const object& obj);

		const std::string& name() const override;

		size_t hash() const override;
		bool operator==(const key_interface&) const override;
		bool operator<(const key_interface&) const override;

	private:
		key_index(const key_index&);
		key_index& operator=(const key_index&);
	};

	struct key_owner : public key_interface
	{
	private:
		OID	m_owner;

	public:
		static const std::string m_name;

		key_owner();
		key_owner(const object& obj);

		const std::string& name() const override;

		OID get_owner() const;
		void set_owner(OID);

		size_t hash() const override;
		bool operator==(const key_interface&) const override;
		bool operator<(const key_interface&) const override;

	private:
		key_owner(const key_owner&);
		key_owner& operator=(const key_owner&);
	};
};

template <typename T>
class factory
{
public:
	// здесь возвращается shared_ptr вместо unique_ptr, т.к. потом удобней использовать
	static std::shared_ptr<T> create()
	{
		return std::shared_ptr<T>(new T());
	}
};

} // end of namespace
#endif
