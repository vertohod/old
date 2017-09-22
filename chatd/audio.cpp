#include <sys/stat.h>
#include "service_messages.h"
#include "stat_daemon.h"
#include "audio_tags.h"
#include "configure.h"
#include "audio.h"
#include "error.h"
#include "util.h"
#include "log.h"

namespace chatd
{

DEFINITION_FIELDS_FUNCTIONS_STORES(audio)

DEFINITION_CONSTS(audio, "audio", true, 1)

DEFINITION_FIELD(audio, sptr_cstr, title)
DEFINITION_FIELD(audio, sptr_cstr, artist)
DEFINITION_FIELD(audio, sptr_cstr, extension)
DEFINITION_FIELD(audio, sptr_cstr, content_type)

std::unique_ptr<blizzard::keys_stores_t> audio::init_keys()
{
	std::unique_ptr<blizzard::keys_stores_t> res(new blizzard::keys_stores_t());

	// Добавляем ключи
	// owner
	{
		auto key_store = std::make_shared<blizzard::key_store_umset<const blizzard::object::key_owner>>();
		res->insert(std::make_pair(key_store->name(), key_store));
	}

	return res;
}

uptr_str audio::serialization() const
{
	auto res = object::serialization();
    sp::stream st(*res);

    for (auto& pr : m_serialization_functions) {
        st << pr.first << "=" << pr.second(this) << SEPARATOR;
    }

    return res;
}

bool audio::init_field(const std::string& key, const std::string& val)
{
    auto it = m_deserialization_functions.find(key);

    if (it != m_deserialization_functions.end()) {
        it->second(this, val);
        return true;
    }

	return object::init_field(key, val);
}

std::shared_ptr<blizzard::object> audio::make_copy() const
{
	std::shared_ptr<blizzard::object> obj_new = create();

	copy_fields(*obj_new);

	return obj_new;
}

void audio::copy_fields(blizzard::object& obj) const
{
	object::copy_fields(obj);

	for (auto& pr : m_serialization_functions) {
		dynamic_cast<audio*>(&obj)->init_field(pr.first, pr.second(this));
	}
}
} // end of namespace
