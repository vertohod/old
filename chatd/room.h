#ifndef ROOM_H
#define ROOM_H

#include <deque>
#include <map>

#include "message.h"
#include "types.h"

namespace chatd
{

#ifndef func_add
	#define func_add    "add"
#endif

#ifndef func_del
	#define func_del    "del"
#endif

typedef std::map<sptr_cstr, OID, comparer_sptr_cstr> users_t;
typedef std::deque<sptr<message>> messages_t;

class room_store;

class room : public object
{
private:
    bool            m_private;
	time_t			m_last_out;

	//settings
	sptr_cstr		m_background_path;
	int				m_type;
	int				m_transparence;
	sptr_cstr		m_background_color;
	sptr_cstr		m_textsize;
	sptr_cstr		m_description;

	users_t			m_users_inside; // Ключ - имя пользователя, значение - его oid
	messages_t		m_messages;

	std::mutex		m_mutex;

public:
	room(OID oid, sptr_cstr name, OID owner, bool arg_private);
	room(sptr_cmap data);

	sptr<const messages_t> get_messages(OID user, OID last_message);
	void user_in(sptr_cstr user_name, OID user_oid, sptr_cstr user_name_old = sptr_cstr(), bool save_dump = true);
	bool user_out(sptr_cstr user_name, sptr_cstr message = sptr_cstr(), bool save_dump = true);
	sptr<const users_t> get_users();
	size_t size();
	time_t get_last_out() const;
	bool is_private() const;
	void add_allow(OID, bool save_dump = true);
	void del_allow(OID, bool save_dump = true);

	void set_bpath(sptr_cstr, bool save_dump = true);
	void set_btype(int);
	void set_btransparence(int);
	void set_bcolor(sptr_cstr);
	void set_textsize(sptr_cstr);
	void set_description(sptr_cstr);
	void set_options(sptr_cstr, int, int, sptr_cstr, sptr_cstr, sptr_cstr, bool save_dump = true);

	sptr_cstr get_bpath();
	int get_btype();
	int get_btransparence();
	sptr_cstr get_bcolor();
	sptr_cstr get_textsize();
	sptr_cstr get_description();

    void add_message(
		sptr_cstr arg_message,
        sptr_cstr name_from = sptr_cstr(new std::string(DEFAULT_USER)),
        OID oid_from = 0,
		sptr_cstr name_to = sptr_cstr(),
		OID oid_to = 0,
		sptr_cstr avatar = sptr_cstr(),
		bool arg_private = false,
        sptr_cstr images = sptr_cstr(),
		sptr_cstr audios = sptr_cstr(),
		sptr_cstr videos = sptr_cstr(),
		sptr_cstr archives = sptr_cstr(),
        unsigned char color = 0,
		OID poid = 0);

private:
    void add_message_wl(
		sptr_cstr arg_message,
        sptr_cstr name_from = sptr_cstr(new std::string(DEFAULT_USER)),
        OID oid_from = 0,
		sptr_cstr name_to = sptr_cstr(),
		OID oid_to = 0,
		sptr_cstr avatar = sptr_cstr(),
		bool arg_private = false,
        sptr_cstr images = sptr_cstr(),
		sptr_cstr audios = sptr_cstr(),
		sptr_cstr videos = sptr_cstr(),
		sptr_cstr archives = sptr_cstr(),
        unsigned char color = 0,
		OID poid = 0);

	void add_message(sptr<message>);
	void clear_messages();

public:
	sptr_cstr to_string(const std::string& func = func_add) const;
	sptr_cstr options_to_string() const;
	sptr_cstr inside_to_string(OID user_oid, sptr_cstr user_name, const std::string& func) const;
	sptr_cstr change_name_to_string() const;
	sptr_cstr message_to_string(const message& mess) const;

	friend room_store;
};

sptr_cstr allow_to_string(OID user, OID room, const std::string& func);

typedef std::map<sptr_cstr, sptr<room>, comparer_sptr_cstr> rooms_name_t;

class room_store : public store<room>
{
private:
	rooms_name_t	m_rooms_name; // В качестве ключа - имя комнаты
	std::mutex		m_mutex;

public:
	room_store();
	void create_dr();
	void set_settings_dr();
	void set_settings_one_room(OID oid, const std::string& config_value);
	size_t last_message() const;
	OID add(sptr_cstr name, OID owner, bool arg_private, bool save_dump = true);
	void del(OID oid);
	void clear();
	void load(sptr_cmap data);
	sptr<const rooms_name_t> get_rooms();

private:
	bool check_name_wl(sptr_cstr name) const;
};

#ifndef OB_room
	#define OB_room		"room"
#endif

#ifndef OB_allow
	#define OB_allow	"allow"
#endif

#ifndef OB_inside
	#define OB_inside	"inside"
#endif

#ifndef OB_options
	#define OB_options	"options"
#endif

#ifndef OB_message
	#define OB_message	"message"
#endif

} // end of namespace chatd

#endif
