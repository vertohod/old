#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>

#include "object.h"
#include "types.h"
#include "table.h"

namespace chatd
{

class message : public blizzard::object, public blizzard::factory<message>
{
public:
	DECLARATION_FIELDS_FUNCTIONS_STORES

private:
	DECLARATION_CONSTS

	DECLARATION_FIELD(OID, poid)
	DECLARATION_FIELD(OID, oid_from)
	DECLARATION_FIELD(OID, oid_to)
	DECLARATION_FIELD(bool, private)
	DECLARATION_FIELD(time_t, time)
	DECLARATION_FIELD(unsigned char, color)
	DECLARATION_FIELD(sptr_cstr, avatar)
	DECLARATION_FIELD(sptr_cstr, name_from)
	DECLARATION_FIELD(sptr_cstr, name_to)
	DECLARATION_FIELD(sptr_cstr, message)
	DECLARATION_FIELD(sptr_cstr, raw)

	DECLARATION_FIELD(std::shared_ptr<std::vector<OID>>, images)
//	DECLARATION_FIELD(std::shared_ptr<std::vector<OID>>, audios)
//	DECLARATION_FIELD(std::shared_ptr<std::vector<OID>>, videos)
//	DECLARATION_FIELD(std::shared_ptr<std::vector<OID>>, archives)

	DECLARATION_FIELD(std::shared_ptr<std::vector<sptr_str>>, youtube)

public:
	message();

	// Обязательные методы -----
	static std::unique_ptr<blizzard::keys_stores_t> init_keys();

	virtual uptr_str serialization() const override;
	virtual bool init_field(const std::string& key, const std::string& val) override;
	virtual std::shared_ptr<blizzard::object> make_copy() const override;
	void copy_fields(blizzard::object& obj) const;
	// -------------------------

private:
	void processing_service_message(sptr_cstr);
	void processing_message(sptr_cstr);

	template <typename T>
	std::shared_ptr<std::vector<OID>> filter_medias(OID from, OID to, sptr_cstr oids_list_str);

	static size_t split_message(const std::string&, std::vector<std::string>&, bool bracket = false);
	static sptr_str make_youtube_url(const std::string& url);
};

} // end of namespace chatd

#endif
