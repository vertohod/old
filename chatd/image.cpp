#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "service_messages.h"
#include "stat_daemon.h"
#include "configure.h"
#include "get_html.h"
#include "request.h"
#include "stream.h"
#include "image.h"
#include "error.h"
#include "util.h"

namespace chatd
{

DEFINITION_FIELDS_FUNCTIONS_STORES(image)

DEFINITION_CONSTS(image, "image", true, 1)

DEFINITION_FIELD(image, sptr_cstr, file_name)
DEFINITION_FIELD(image, sptr_cstr, content_type)
DEFINITION_FIELD(image, sptr_cstr, path_temporary)
DEFINITION_FIELD(image, sptr_cstr, extension)
DEFINITION_FIELD(image, bool, active)

image::image() : m_flag_test(false) {}

image::~image()
{
	if (m_flag_test) return;

	std::string file_name = format(get_index()) + "." + *FIELD(extension);

	if (0 > unlink((cfg().dir_normal + file_name).c_str()))
		lo::l(lo::ERROR) << "Can't remove the file: " << cfg().dir_normal << file_name;

	if (0 > unlink((cfg().dir_miniature + file_name).c_str()))
		lo::l(lo::ERROR) << "Can't remove the file: " << cfg().dir_miniature << file_name;
}

std::unique_ptr<blizzard::keys_stores_t> image::init_keys()
{
	std::unique_ptr<blizzard::keys_stores_t> res(new blizzard::keys_stores_t());

	// Добавляем ключи
	// index
	{
		auto key_store = std::make_shared<blizzard::key_store_set<const blizzard::object::key_index>>();
		res->insert(std::make_pair(key_store->name(), key_store));
	}

	// owner
	{
		auto key_store = std::make_shared<blizzard::key_store_umset<const blizzard::object::key_owner>>();
		res->insert(std::make_pair(key_store->name(), key_store));
	}

	return res;
}

void image::set_flag_test()
{
	m_flag_test = true;
}

sptr_cstr image::get_extension(const sptr_cstr& file_name)
{
    auto ext_pos = file_name->rfind(".");
    if (ext_pos != std::string::npos) {
		return sptr_cstr(file_name->substr(ext_pos + 1, file_name->size() - ext_pos));
    }
	return sptr_cstr();
}

void image::resize() const
{
	auto ip_port = cfg().upstream["execd"];
	if (!ip_port.empty()) {
		std::string path_source = cfg().dir_temp + *FIELD(file_name);
		std::string name_out = format(get_index()) + *FIELD(extension);

		std::string path_normal = cfg().dir_normal + name_out;
		std::string path_miniature = cfg().dir_miniature + name_out;

		std::string command_resize(util::ins(cfg().command_resize, path_source));
		command_resize = util::ins(command_resize, path_normal);

		std::string command_miniature(util::ins(cfg().command_miniature, path_source));
		command_miniature = util::ins(command_miniature, path_miniature);

		// Параметры, которые отправляются в execd
		std::string comm = command_miniature + " && " + command_resize;
		std::string upstream = "chatd";
		std::string cb = "/activate.json?object=image&oid=" + format(get_index()) + "&user=" + format(get_owner());

		request req;
		req.set_ver(ver_t("HTTP/1.1"));
		req.set_method(POST);
		req.set_path("/proc.json");
		req.add_post("comm", comm);
		req.add_post("upstream", upstream);
		req.add_post("cb", cb);
		auto resp = get_html(ip_port, req, cfg().timeout_execd);
	}
}

uptr_str image::serialization() const
{
	auto res = object::serialization();
    sp::stream st(*res);

    for (auto& pr : m_serialization_functions) {
        st << pr.first << "=" << pr.second(this) << SEPARATOR;
    }

    return res;
}

bool image::init_field(const std::string& key, const std::string& val)
{
    auto it = m_deserialization_functions.find(key);

    if (it != m_deserialization_functions.end()) {
        it->second(this, val);
        return true;
    }

	return object::init_field(key, val);
}

std::shared_ptr<blizzard::object> image::make_copy() const
{
	std::shared_ptr<blizzard::object> obj_new = create();

	copy_fields(*obj_new);

	return obj_new;
}

void image::copy_fields(blizzard::object& obj) const
{
	object::copy_fields(obj);

	for (auto& pr : m_serialization_functions) {
		dynamic_cast<image*>(&obj)->init_field(pr.first, pr.second(this));
	}
}

void image::set_permission(const std::string& dir,
	const std::string& user,
	const std::string& group,
	const std::string& mode) const
{
	std::string file_name = format(get_index()) + *FIELD(extension);

	std::string path_file = dir + file_name;

	util::passwd pwd = util::getpwnam(user);
	if (pwd.pw_name.empty()) throw error<500>(smessage(SMESS213_USERID_NOT_FOUND, user));

	util::group grp = util::getgrnam(group);
	if (grp.gr_name.empty()) throw error<500>(smessage(SMESS214_GROUPID_NOT_FOUND, group));

	uid_t uid = pwd.pw_uid;
	gid_t gid = grp.gr_gid;

	if (0 > chown(path_file.c_str(), uid, gid)) throw error<500>(smessage(SMESS215_SET_UNABLE, path_file));

	int image_mode = strtoul(mode.c_str(), 0, 8);
	if (0 > chmod(path_file.c_str(), image_mode)) throw error<500>(smessage(SMESS216_CHANGE_UNABLE, path_file));
}
} // end of namespace
