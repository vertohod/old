#ifndef VIDEO_H
#define VIDEO_H

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

class video : public object
{
private:
	sptr_cstr	m_extension;
	sptr_cstr	m_ctype;

public:
	video();
	video(OID oid, sptr_cstr name, OID owner, sptr_cstr extension, sptr_cstr ctype);
	video(sptr_cmap data);

	sptr_cstr	get_extension() const;
	sptr_cstr	get_content_type() const;

	void add_allow(OID, bool save_dump = true);
	void del_allow(OID, bool save_dump = true);

	sptr_cstr to_string(const std::string& func = func_add) const;
	static sptr_cstr allow_to_string(OID user, OID video, const std::string& func);
};

class video_store : public store<video>
{
private:
	std::mutex      m_mutex;

public:
	video_store();

	OID add(OID user, const std::string& path, const std::string& original_name, const std::string& extension, const std::string& content_type);
	void del(OID oid, OID user);
	void clear(sptr_cvec oids, OID user);
	void load(sptr_cmap data);

private:
	bool add(OID oid, sptr_cstr name, OID owner, sptr_cstr extension, sptr_cstr content_type);
	static void del_file(sptr<video> ad_ptr);
};

#ifndef OB_video
	#define OB_video           "video"
#endif

#ifndef OB_video_allow
	#define OB_video_allow     "video_allow"
#endif

} // end of namespace

#endif
