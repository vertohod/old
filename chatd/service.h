#ifndef SERVICE_H
#define SERVICE_H

#include "http_server.h"
#include "context.h"
#include "types.h"

class service : public net::http_server
{
public:
	service(net::boost_socket_ptr socket);

private:
	void handle_process(const request& req);

	// методы обработки конкретных запросов пользователя
	void fill_context(const request&, context& cnt);
	void send(context& cnt);
	void login(context& cnt);
	void logout(context& cnt);
	void comein(context& cnt, OID room);
	void getout(context& cnt);
	void change(context& cnt);
	void upload(const request&, context& cnt);
	void remove(const request&, context& cnt);
	void image(const request&, context& cnt);
	void audio(const request&, context& cnt);
	void video(const request&, context& cnt);
	void archive(const request&, context& cnt);
	void personal(context& cnt);
	void add(const request&, context& cnt);

	void activate(const request& req);
	void upload_image(const request&, context& cnt);
	void upload_audio(const request&, context& cnt);
	void upload_video(const request& req, context& cnt);
	void upload_file(const request& req, context& cnt);
	void settings(const request& req, context& cnt);

    void remove_images(context& cnt, sptr_cvec oids);
    void remove_audios(context& cnt, sptr_cvec oids);
    void remove_videos(context& cnt, sptr_cvec oids);
    void remove_archives(context& cnt, sptr_cvec oids);
    void remove_room_allows(context& cnt, sptr_cvec oids);
    void remove_personal_messages(context& cnt, sptr_cvec oids);
	void remove_password(context& cnt);
	void remove_alias(context& cnt);

	void add_alias(const request&, context& cnt);
    void add_room_allows(context& cnt, sptr_cvec oids);
	void add_room(const request&, context& cnt);
	void add_password(const request&, context&);
	void add_avatar(const request&, context&);
	void add_invite(const request&, context&);
	void invite(const request& req, context& cnt);

	bool check_message(context& cnt);
	void generate_answer(context& cnt);
	void generate_short_answer(context& cnt);
};

#endif
