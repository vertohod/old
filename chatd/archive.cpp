#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "service_messages.h"
#include "stat_daemon.h"
#include "configure.h"
#include "get_html.h"
#include "dumper.h"
#include "archive.h"
#include "error.h"
#include "util.h"

namespace chatd
{


archive::archive() : object() {}

archive::archive(OID oid, sptr_cstr name, OID owner, sptr_cstr path_temp, sptr_cstr ctype) :
	object(oid, name, owner), m_path_temp(path_temp), m_ctype(ctype)
{
}

archive::archive(sptr_cmap data) : object(data),
	m_path_temp(new std::string(util::get_val<std::string>(*data, "path_temp"))),
	m_ctype(new std::string(util::get_val<std::string>(*data, "content_type")))
{
}

sptr_cstr archive::get_path_temp() const
{
	return m_path_temp;
}

sptr_cstr archive::get_content_type() const
{
    return m_ctype;
}

void archive::set_privilege() const
{
	auto& cfg = config();

	std::string path_archive = cfg.dir_archive + format(get_oid()) + ".zip";

	// Установим права на файлы
	if (!geteuid()) {
		util::passwd	pwd;
		util::group		grp;

		pwd = util::getpwnam(cfg.user);
		if (pwd.pw_name.empty()) throw error<500>(smessage(SMESS213_USERID_NOT_FOUND, cfg.user));
		grp = util::getgrnam(cfg.group);
		if (grp.gr_name.empty()) throw error<500>(smessage(SMESS214_GROUPID_NOT_FOUND, cfg.group));

		uid_t uid = pwd.pw_uid;
		gid_t gid = grp.gr_gid;

		if (0 > chown(path_archive.c_str(), uid, gid)) throw error<500>(smessage(SMESS215_SET_UNABLE, path_archive));

		int archive_mode = strtoul(cfg.archive_mode.c_str(), 0, 8);
		if (0 > chmod(path_archive.c_str(), archive_mode)) throw error<500>(smessage(SMESS216_CHANGE_UNABLE, path_archive));

		singleton<dumper>::instance().save(to_string(func_add));
	}
}

void archive::add_allow(OID user, bool save_dump)
{
	if (object::add_allow(user) && save_dump) {
		singleton<dumper>::instance().save(allow_to_string(user, object::get_oid(), func_add));
	}
}

void archive::del_allow(OID user, bool save_dump)
{
	if (object::del_allow(user) && save_dump) {
		singleton<dumper>::instance().save(allow_to_string(user, object::get_oid(), func_del));
	}
}

sptr_cstr archive::to_string(const std::string& func) const
{
    sptr_str res;
    sp::stream  st(*res);

    st << "object=" << OB_archive;
    st << "\t";
    st << "func=" << func;
    st << "\t";
    st << *object::to_string();
    st << "\t";
    st << "path_temp=" << *m_path_temp;
    st << "\t";
    st << "content_type=" << *m_ctype;

    return res;
}

sptr_cstr archive::allow_to_string(OID user, OID archive, const std::string& func)
{
    sptr_str res;
    sp::stream st(*res);

    st << "object=" << OB_archive_allow;
    st << "\t";
    st << "func=" << func;
    st << "\t";
    st << "user=" << user;
    st << "\t";
    st << "archive=" << archive;

    return res;
}

// ******** archive_store ***********

archive_store::archive_store() {}

OID archive_store::add(OID user, const std::string& path, const std::string& original_name, const std::string& content_type)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	OID oid = 0;

	auto& cfg = config();

	auto ip_port = cfg.upstream["execd"];
	if (!ip_port.empty()) {

		oid = store::oid();
		std::string path_archive = cfg.dir_archive + format(oid);

		std::string command_compress(util::ins(cfg.command_compress, path_archive));
		command_compress = util::ins(command_compress, path);

		// Параметры, которые отправляются в execd
		std::string comm = command_compress;
		std::string upstream = "chatd";
		std::string cb = "/activate.json?object=archive&oid=" + format(oid) + "&user=" + format(user);

		request req;
		req.set_ver(ver_t("HTTP/1.1"));
		req.set_method(POST);
		req.set_path("/proc.json");
		req.add_post("comm", comm);
		req.add_post("upstream", upstream);
		req.add_post("cb", cb);
		auto resp = get_html(ip_port, req, cfg.timeout_execd);

		// Добавили архив в базу
		add(
			oid,
			sptr_cstr(new std::string(original_name)),
			user,
			sptr_cstr(new std::string(path)),
			sptr_cstr(new std::string(content_type))
		);
	}

	return oid;
}

bool archive_store::add(OID oid, sptr_cstr name, OID owner, sptr_cstr path_temp, sptr_cstr content_type)
{
	bool res = store::add(sptr<archive>(new archive(oid, name, owner, path_temp, content_type)));

	return res; 
}

void archive_store::del(OID oid, OID user)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto arch_ptr = store::get(oid);
	if (arch_ptr->check_owner(user)) {
		store::del(arch_ptr->get_oid());
		del_file(arch_ptr);

		singleton<dumper>::instance().save(arch_ptr->to_string(func_del));
	} else throw error<403>(smessage(SMESS222_ARCHIVE_DN_BELONG));
}

void archive_store::del_file(sptr<archive> arch_ptr)
{
	auto& cfg = config();

	std::string file_name = format(arch_ptr->get_oid()) + ".zip"; 
	if (0 > unlink((cfg.dir_archive + file_name).c_str()))
		throw error<500>(smessage(SMESS217_DELETE_UNABLE, cfg.dir_archive + file_name));
}

void archive_store::clear(sptr_cvec oids, OID user)
{
	for (auto oid : *oids) del(oid, user);
}

void archive_store::load(sptr_cmap data)
{
    auto object = util::get_val<std::string>(*data, "object");
    auto func = util::get_val<std::string>(*data, "func");

    if (object == OB_archive) {

		sptr<archive> archive_ptr(new archive(data));

        if (func == func_add) {
			store::add(archive_ptr);
		} else {
			store::del(archive_ptr->get_oid());
		}

	} else if (object == OB_archive_allow) {

        auto user = util::get_val<OID>(*data, "user");
        auto archive = util::get_val<OID>(*data, "archive");

        auto archive_ptr = store::get(archive);

        if (func == func_add) archive_ptr->add_allow(user, false);
        else archive_ptr->del_allow(user, false);

	}
}

} // end of namespace
