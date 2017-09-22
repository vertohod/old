#ifndef AUDIO_H
#define AUDIO_H

#include "object.h"
#include "types.h"
#include "table.h"

namespace chatd
{

class audio : public blizzard::object, public blizzard::factory<audio>
{
public:
	DECLARATION_FIELDS_FUNCTIONS_STORES

private:
	DECLARATION_CONSTS

	DECLARATION_FIELD(sptr_cstr, title)
	DECLARATION_FIELD(sptr_cstr, artist)
	DECLARATION_FIELD(sptr_cstr, extension)
	DECLARATION_FIELD(sptr_cstr, content_type)

public:
	// Обязательные методы -----
	static std::unique_ptr<blizzard::keys_stores_t> init_keys();

	virtual uptr_str serialization() const override;
	virtual bool init_field(const std::string& key, const std::string& val) override;
	virtual std::shared_ptr<blizzard::object> make_copy() const override;
private:
	void copy_fields(blizzard::object& obj) const;
	// -------------------------
};

} // end of namespace
#endif
