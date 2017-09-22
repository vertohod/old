#ifndef SESSIONS_H
#define SESSIONS_H

#include <boost/circular_buffer.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <queue>
#include <list>

#include "service_messages.h"
#include "message.h"
#include "dumper.h"
#include "types.h"

#ifndef func_add
	#define func_add	"add"
#endif

#ifndef func_del
	#define func_del	"del"
#endif

class sessions;

typedef std::list<sptr<chatd::message>> list_message_t;
typedef std::unordered_map<OID, list_message_t> personal_t; // OID - oid отправителя
typedef std::queue<sptr_cstr> system_messages_t;

#define DAY_MSECONDS 86400000
#define ITERATION_MIN 100

class session
{
private:
	OID					m_oid; // по этой OID пользователи будут идентифицировать друг-друга
	sptr_cstr			m_name;
	OID					m_avatar;
    size_t              m_password;
	OID					m_current_room;
	personal_t			m_personal;
	system_messages_t	m_sysmess_info;
	system_messages_t	m_sysmess_error;

	size_t				m_hash_personal;	
	size_t				m_hash_image;
	size_t				m_hash_audio;
	size_t				m_hash_video;
	size_t				m_hash_archive;

	time_t		m_created;
	time_t		m_last_access;
	bool		m_block; // флаг устанавливается, если сессия заблокирована
	time_t		m_block_time; // время снятия блокировки
	size_t		m_violations_amount; // при N нарушениях блокировать по IP

	std::mutex	m_mutex;

	struct room_mess
	{
		room_mess(OID room, size_t hash);
		room_mess(const room_mess&);
		room_mess operator=(const room_mess&);
		OID		m_room;
		size_t	m_hash;
	};

	boost::circular_buffer_space_optimized<room_mess> m_messages;
	std::unordered_set<OID> m_rooms; // хранит комнаты, в которые пользователь заходил
	std::set<OID> m_images; // хранит загруженные картинки
	std::set<OID> m_audios; // хранит загруженные мелодии
	std::set<OID> m_videos;
	std::set<OID> m_archives;

	session& operator=(const session&);

public:
	session();
	~session();

	sptr_cstr get_name() const;	
	OID get_oid() const;
	OID get_avatar() const;
	time_t get_created() const;
	time_t get_last_access() const;
	size_t get_hash_personal() const;
	size_t get_hash_image() const;
	size_t get_hash_audio() const;
	size_t get_hash_video() const;
	size_t get_hash_archive() const;
	OID get_current_room() const;

	// проверяет очередь последних сообщений
	bool check_queue(OID, sptr_cstr);

	void comein_room(OID);
	void getout_room(OID);
	bool is_anywhere();
	sptr<std::unordered_set<OID>> get_rooms();
	bool is_inside(OID);
	void clear_rooms();
	void add_image(OID);
	void del_image(OID);
	sptr_cvec get_images();
	void clear_images();

	void add_audio(OID);
	void del_audio(OID);
	sptr_cvec get_audios();
	void clear_audios();

	void add_video(OID);
	void del_video(OID);
	sptr_cvec get_videos();
	void clear_videos();

	void add_archive(OID);
	void del_archive(OID);
	sptr_cvec get_archives();
	void clear_archives();

    void add_allow(OID);
    void del_allow(OID);
    bool check_allow(OID); // Функция используется для пометки разрешенных комнат. Не используется для проверки доступа при принятии решения во время входа 

	void set_avatar(OID, bool save_dump = true);
    void set_password(sptr_cstr);
    bool check_password(sptr_cstr);
    void clear_password();
	void set_current_room(OID);

	void add_message(OID, sptr<chatd::message>);
	void del_message(OID);
	sptr<const personal_t> get_personal();

	void add_info(sptr_cstr);
	void add_error(sptr_cstr);
	sptr_cstr get_info();
	sptr_cstr get_error();

	static sptr<const UUID> gen(const std::string&);

	session(sptr_cmap);
	sptr_cstr to_string(sptr<const UUID>, const std::string& func = func_add) const;
	sptr_cstr change_name_to_string() const;
	sptr_cstr avatar_to_string() const;
	sptr_cstr password_to_string() const;
	sptr_cstr current_to_string() const;

private:
	void set_name(sptr_cstr name = "");
	void set_oid(OID);

	void add_message_wl(OID, sptr<chatd::message>);
	void del_message_wl(OID, bool save_dump = true);
    void set_password_wl(size_t);
    void set_current_room_wl(OID);
	sptr_cstr personal_delete(OID);

	static sptr<const UUID> gen_random();

	friend sessions;
};

namespace std {
	template<>
	class hash<UUID>
	{
	public:
		size_t operator()(const UUID&) const;
	};

	template<>
	class hash<sptr<const UUID>>
	{
	public:
		size_t operator()(sptr<const UUID>) const;
	};
}

class sessions
{
private:
	typedef std::unordered_map<sptr<const UUID>, sptr<session>, std::hash<sptr<const UUID>>, equal_sptr<const UUID>> uuid_session_t;
	typedef std::unordered_map<sptr_cstr, sptr<session>, std::hash<sptr_cstr>, equal_sptr<const std::string>> name_session_t;
	typedef std::unordered_map<OID, sptr<session>> oid_session_t;

	uuid_session_t	m_sessions_uuid;
	name_session_t	m_sessions_name;
	oid_session_t	m_sessions_oid;

	std::mutex m_mutex;
	boost::thread m_thread;

public:
	sessions();

	sptr<session> get(sptr<const UUID>&);
	sptr<session> get(OID);

	bool set_name(sptr<const UUID>, sptr<session>, sptr_cstr);
	void del_session(sptr<session>);
    void add_message(sptr<session>, sptr_cstr arg_message, sptr_cstr name_from, OID oid_from, OID oid_to,
		sptr_cstr avatar, sptr_cstr images, sptr_cstr audios, sptr_cstr videos, sptr_cstr archives, unsigned char color);
    void add_message_one_user(sptr<session>, sptr_cstr arg_message, sptr_cstr images, sptr_cstr audios,
		sptr_cstr videos, sptr_cstr archives, unsigned char color);
	sptr<session> add_alias(sptr<const UUID>, sptr_cstr, sptr_cstr);
	sptr<session> remove_alias(sptr<const UUID>);

	void load(sptr_cmap);

	template <typename T, typename IT>
	IT clear(T& map_with_sessions, IT it_current, size_t frequency)
	{
		std::vector<IT> sessions_for_remove;

		size_t iterations_amount = map_with_sessions.size() / frequency;
		if (iterations_amount * frequency < map_with_sessions.size()) ++iterations_amount;

		for (size_t count = 0; count < iterations_amount; ++count, ++it_current) {

			if (it_current == map_with_sessions.end()) {

				remove(map_with_sessions, sessions_for_remove);

				it_current = map_with_sessions.begin();
				if (it_current == map_with_sessions.end()) break;

			}

			auto sess_ptr = it_current->second;
			auto& cfg = config();

			if (sess_ptr->get_name()->empty() && time(NULL) - sess_ptr->get_last_access() > cfg.timeout_session_empty) {
				// Удаляем старые сессии без логинов
				sessions_for_remove.push_back(it_current);
			} else if (time(NULL) - sess_ptr->get_last_access() > cfg.timeout_session_full) {
				 // Удаляем старые сессии с логинами
				sessions_for_remove.push_back(it_current);
			}
		}

		remove(map_with_sessions, sessions_for_remove);

		return it_current;
	}

	template <typename T, typename ITS>
	void remove(T& map_with_sessions, ITS& iterators)
	{
		auto& dm = singleton<dumper>::instance();

		for (auto sess_it : iterators) {
			// Не пишем в дамп удаление нулевых сессий, т.к. в нем нет их добавления
			if (sess_it->second->get_oid() > 0) {
// FIXME
//				dm.save(sess_it->second->to_string(sess_it->first, func_del));
			}

			m_sessions_name.erase(sess_it->second->get_name());
			map_with_sessions.erase(sess_it);

			sess_size::s(m_sessions_uuid.size());
		}

		iterators.clear();
	}

private:
	sptr<session> get_wl(sptr<const UUID>&);
	sptr<session> get_wl(OID);

	bool set_name_wl(sptr<const UUID>, sptr<session>, sptr_cstr);
	void del_name_wl(sptr<session>);
	sptr<session> create(sptr<const UUID>&);

	static sptr_cstr alias_to_string(sptr<const UUID> alias, OID oid, const std::string& func = func_add);

	sessions(const sessions&) = delete;
	sessions& operator=(const sessions&) = delete;

	void thread_function();
};

#ifndef OB_session
	#define OB_session	"session"
#endif

#ifndef OB_alias
	#define OB_alias	"alias"
#endif

#ifndef OB_allow
	#define OB_allow	"allow"
#endif

#ifndef OB_personal
	#define OB_personal	"personal"
#endif

#ifndef OB_name
	#define OB_name		"name"
#endif

#ifndef OB_inside
	#define OB_inside	"inside"
#endif

#ifndef OB_password
	#define OB_password	"password"
#endif

#ifndef OB_audio
	#define OB_audio	"audio"
#endif

#ifndef OB_video
	#define OB_video	"video"
#endif

#ifndef OB_archive
	#define OB_archive	"archive"
#endif

#ifndef OB_image
	#define OB_image	"image"
#endif

#ifndef OB_avatar
	#define OB_avatar	"avatar"
#endif

#ifndef OB_current_room
	#define OB_current_room	"current_room"
#endif

#endif
