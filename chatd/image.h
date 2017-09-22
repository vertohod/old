#ifndef IMAGE_H
#define IMAGE_H

#include "object.h"
#include "types.h"
#include "table.h"

namespace chatd
{

class image : public blizzard::object, public blizzard::factory<image>
{
public:
	DECLARATION_FIELDS_FUNCTIONS_STORES

private:
	DECLARATION_CONSTS

	DECLARATION_FIELD(sptr_cstr, file_name) // исходное название файла
	DECLARATION_FIELD(sptr_cstr, content_type) // полученные от браузера при загрузке
	DECLARATION_FIELD(sptr_cstr, path_temporary) // путь к временному файлу до обработки
	DECLARATION_FIELD(sptr_cstr, extension) // расширение
	DECLARATION_FIELD(bool, active) // false при добавлении в базу, true после обработки

private:
	bool m_flag_test;

public:
	image();
	~image();

	// Обязательные методы -----
	static std::unique_ptr<blizzard::keys_stores_t> init_keys();

	virtual uptr_str serialization() const override;
	virtual bool init_field(const std::string& key, const std::string& val) override;
	virtual std::shared_ptr<blizzard::object> make_copy() const override;
private:
	void copy_fields(blizzard::object& obj) const;
	// -------------------------

public:
	void set_flag_test();
	static sptr_cstr get_extension(const sptr_cstr& file_name);
	void resize() const;

private:
	void set_permission(const std::string& dir, const std::string& user, const std::string& group, const std::string& mode) const;
};

} // end of namespace
#endif
