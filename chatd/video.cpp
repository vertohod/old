#include <sys/stat.h>
#include "service_messages.h"
#include "stat_daemon.h"
#include "configure.h"
#include "dumper.h"
#include "video.h"
#include "error.h"
#include "util.h"
#include "log.h"

namespace chatd
{


video::video() : object() {}

video::video(OID oid, sptr_cstr name, OID owner, sptr_cstr extension, sptr_cstr ctype) :
	object(oid, name, owner), m_extension(extension), m_ctype(ctype)
{
}

video::video(sptr_cmap data) : object(data),
	m_extension(new std::string(util::get_val<std::string>(*data, "extension"))),
	m_ctype(new std::string(util::get_val<std::string>(*data, "ctype")))
{
}

sptr_cstr video::get_extension() const
{
	return m_extension;
}

sptr_cstr video::get_content_type() const
{
	return m_ctype;
}

void video::add_allow(OID user, bool save_dump)
{
	if (object::add_allow(user) && save_dump) {
		singleton<dumper>::instance().save(allow_to_string(user, object::get_oid(), func_add));
	}
}

void video::del_allow(OID user, bool save_dump)
{
	if (object::del_allow(user) && save_dump) {
		singleton<dumper>::instance().save(allow_to_string(user, object::get_oid(), func_del));
	}
}

sptr_cstr video::to_string(const std::string& func) const
{
	sptr_str res;
	sp::stream  st(*res);

	st << "object=" << OB_video;
	st << "\t";
	st << "func=" << func;
	st << "\t";
	st << *object::to_string();
	st << "\t";
	st << "extension=" << *m_extension;
	st << "\t";
	st << "ctype=" << *m_ctype;

	return res;
}

sptr_cstr video::allow_to_string(OID user, OID video, const std::string& func)
{
	sptr_str res;
	sp::stream st(*res);

	st << "object=" << OB_video_allow;
	st << "\t";
	st << "func=" << func;
	st << "\t";
	st << "user=" << user;
	st << "\t";
	st << "video=" << video;

	return res;
}

// ******** video_store ***********

video_store::video_store() {}

OID video_store::add(OID user, const std::string& path, const std::string& original_name, const std::string& extension, const std::string& content_type)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto& cfg = config();

	OID oid = store::oid();
	std::string path_out = cfg.dir_video + format(oid) + extension;

	// Переносим в постоянную папку
	util::rename(path, path_out);

	if (!geteuid()) {
		util::passwd	pwd;
		util::group		grp;

		pwd = util::getpwnam(cfg.user);
		if (pwd.pw_name.empty()) throw error<500>(smessage(SMESS213_USERID_NOT_FOUND, cfg.user));
		grp = util::getgrnam(cfg.group);
		if (grp.gr_name.empty()) throw error<500>(smessage(SMESS214_GROUPID_NOT_FOUND, cfg.group));

		uid_t uid = pwd.pw_uid;
		gid_t gid = grp.gr_gid;

		if (0 > chown(path_out.c_str(), uid, gid)) throw error<500>(smessage(SMESS215_SET_UNABLE, path_out));

		int video_mode = strtoul(cfg.video_mode.c_str(), 0, 8);
		if (0 > chmod(path_out.c_str(), video_mode)) throw error<500>(smessage(SMESS216_CHANGE_UNABLE, path_out));
	}

	bool res = add(oid, sptr_cstr(new std::string(original_name)), user,
		sptr_cstr(new std::string(extension)),
		sptr_cstr(new std::string(content_type)));

	return res ? oid : 0;
}

bool video_store::add(OID oid, sptr_cstr name, OID owner, sptr_cstr extension, sptr_cstr content_type)
{
	sptr<video> video_ptr(new video(oid, name, owner, extension, content_type));

	bool res = store::add(video_ptr);

	singleton<dumper>::instance().save(video_ptr->to_string(func_add));

	return res;
}

void video_store::del(OID oid, OID user)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto ad_ptr = store::get(oid);
	if (ad_ptr->check_owner(user)) {
		del_file(ad_ptr);
		store::del(ad_ptr->get_oid());

		singleton<dumper>::instance().save(ad_ptr->to_string(func_del));
	} else throw error<403>(smessage(SMESS218_AUDIO_DN_BELONG));
}

void video_store::del_file(sptr<video> ad_ptr)
{
	auto& cfg = config();

	std::string path = cfg.dir_video + format(ad_ptr->get_oid()) + *ad_ptr->get_extension();
	if (0 > unlink(path.c_str())) throw error<500>(smessage(SMESS217_DELETE_UNABLE, path));
}

void video_store::clear(sptr_cvec oids, OID user)
{
	for (auto oid : *oids) del(oid, user);
}

void video_store::load(sptr_cmap data)
{
	auto object = util::get_val<std::string>(*data, "object");
	auto func = util::get_val<std::string>(*data, "func");

	if (object == OB_video) {

		sptr<video> video_ptr(new video(data));

		if (func == func_add) {
			store::add(video_ptr);
		} else {
			store::del(video_ptr->get_oid());
		}

	} else if (object == OB_video_allow) {

		auto user = util::get_val<OID>(*data, "user");
		auto video = util::get_val<OID>(*data, "video");

		auto video_ptr = store::get(video);

		if (func == func_add) video_ptr->add_allow(user, false);
		else video_ptr->del_allow(user, false);

	}
}

} // end of namespace
