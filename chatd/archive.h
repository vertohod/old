#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "store.h"
#include "types.h"

namespace chatd
{

#ifndef func_add
    #define func_add    "add"
#endif

#ifndef func_del
    #define func_del    "del"
#endif

class archive : public object
{
private:
	sptr_cstr	m_path_temp;
	sptr_cstr	m_ctype;

public:
	archive();
	archive(OID oid, sptr_cstr name, OID owner, sptr_cstr path_temp, sptr_cstr ctype);
	archive(sptr_cmap data);

	sptr_cstr get_path_temp() const;
    sptr_cstr get_content_type() const;
	void set_privilege() const;
	void add_allow(OID, bool save_dump = true);
	void del_allow(OID, bool save_dump = true);

	sptr_cstr to_string(const std::string& func = func_add) const;
	static sptr_cstr allow_to_string(OID user, OID archive, const std::string& func);
};

class archive_store : public store<archive>
{
private:
	std::mutex      m_mutex;

public:
	archive_store();

	OID add(
		OID user,
		const std::string& path,
		const std::string& original_name,
		const std::string& content_type
	);

	void del(OID oid, OID user);
	void clear(sptr_cvec oids, OID user);
	void load(sptr_cmap data);

private:
	bool add(OID, sptr_cstr, OID, sptr_cstr, sptr_cstr);
	static void del_file(sptr<archive>);
};

#ifndef OB_archive
    #define OB_archive		"archive"
#endif

#ifndef OB_archive_allow
    #define OB_archive_allow	"archive_allow"
#endif

} // end of namespace

#endif
