#include "admin_chatd.h"
#include "configure.h"
#include "domain.h"
#include "avatar.h"
#include "smile.h"
#include "room.h"

admin_chatd::admin_chatd(net::boost_socket_ptr socket) : admin(socket) {}

void admin_chatd::handle_process(const request& req)
{
	try {

		admin::handle_process(req);

	} catch (const path_not_supported& e) {

		response resp;

		if (*req.path() == "/config.json" && req.get<std::string>("func", "") == "print") {


		} else if (*req.path() == "/config.json" && req.get<std::string>("func", "") == "update") {

			auto& cfg = config();

			cfg.update();

			auto& rm = singleton<chatd::room_store>::instance();
			rm.set_settings_dr();

            resp.data()->assign("{\n\t\"config\":\"updated\"\n}\n");
            resp.add_header("Content-type", "application/json; charset=utf-8");
            resp.set_status(S200);

		} else if (*req.path() == "/avatars.json" && req.get<std::string>("func", "") == "update") {

	 		auto& cfg = config();

			auto& avatar = singleton<avatar_store>::instance();
			avatar.init(cfg.dir_avatars);

            resp.data()->assign(*avatar.to_json(cfg.dir_avatars_short));
            resp.add_header("Content-type", "application/json; charset=utf-8");
            resp.set_status(S200);

		} else if (*req.path() == "/smiles.json" && req.get<std::string>("func", "") == "update") {

	 		auto& cfg = config();

			auto& smile = singleton<smile_store>::instance();
			smile.init(cfg.path_smiles_list, cfg.smiles_extension);

            resp.data()->assign(*smile.to_json(cfg.dir_smiles_short));
            resp.add_header("Content-type", "application/json; charset=utf-8");
            resp.set_status(S200);

		} else if (*req.path() == "/domains.json" && req.get<std::string>("func", "") == "update") {

	 		auto& cfg = config();

			auto& domain = singleton<domain_store>::instance();
			domain.init(cfg.path_domains_list);

            resp.data()->assign("{\n\t\"domains\":\"updated\"\n}\n");
            resp.add_header("Content-type", "application/json; charset=utf-8");
            resp.set_status(S200);
		}

		resp.set_ver(req.ver());
		http_server::send(*resp.to_raw());
	}
}
