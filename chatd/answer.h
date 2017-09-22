#ifndef ANSWER_H
#define ANSWER_H

#include <string>
#include <vector>
#include <memory>
#include "types.h"
#include "serialization.h"

enum SHOW_MASK
{
	SHOW_USERS		= 1,
	SHOW_ROOMS		= 2,
	SHOW_IMAGES		= 4,
	SHOW_AUDIOS		= 8,
	SHOW_WHOLE_ROOM = 16,
	SHOW_VIDEOS		= 32,
	SHOW_ARCHIVES	= 64,
};

struct an_mimage
{
	sptr_cstr	m_min;
	sptr_cstr	m_norm;

	an_mimage(sptr_cstr min, sptr_cstr norm);
};

namespace serialization
{
template <>
void s<an_mimage>(std::string& buf, const an_mimage& obj, int level);
} // end of namespace

struct an_maudio
{
	sptr_cstr	m_name;
	sptr_cstr	m_title;
	sptr_cstr	m_artist;
	sptr_cstr	m_path;

	an_maudio(sptr_cstr name, sptr_cstr title, sptr_cstr artist, sptr_cstr path);
};

namespace serialization
{
template <>
void s<an_maudio>(std::string& buf, const an_maudio& obj, int level);
} // end of namespace

struct an_mvideo
{
	sptr_cstr	m_name;
	sptr_cstr	m_path;

	an_mvideo(sptr_cstr name, sptr_cstr path);
};

namespace serialization
{
template <>
void s<an_mvideo>(std::string& buf, const an_mvideo& obj, int level);
} // end of namespace

struct an_marchive
{
	sptr_cstr	m_name;
	sptr_cstr	m_path;

	an_marchive(sptr_cstr name, sptr_cstr path);
};

namespace serialization
{
template <>
void s<an_marchive>(std::string& buf, const an_marchive& obj, int level);
} // end of namespace

struct an_myoutube
{
	sptr_cstr	m_url;

	an_myoutube(sptr_cstr url);
};

namespace serialization
{
template <>
void s<an_myoutube>(std::string& buf, const an_myoutube& obj, int level);
} // end of namespace

struct an_mess
{
	OID				m_oid;
	OID				m_room;
	OID				m_poid;
	time_t			m_time;
	bool			m_appeal;
	bool			m_private;
	sptr_cstr		m_avatar;
	OID				m_oid_from;
	sptr_cstr		m_name_from;
	OID				m_oid_to;
	sptr_cstr		m_name_to;
	sptr_cstr		m_message;
	sptr_cstr		m_raw;

	sptr<const std::vector<an_mimage>>		m_images;
	sptr<const std::vector<an_maudio>>		m_audios;
	sptr<const std::vector<an_mvideo>>		m_videos;
	sptr<const std::vector<an_marchive>>	m_archives;
	sptr<const std::vector<an_myoutube>>	m_youtube;
	unsigned char m_color;

	an_mess(OID oid, OID room, OID poid, time_t time, bool appeal, bool arg_private, sptr_cstr avatar,
		OID oid_from,
		sptr_cstr name_from, OID oid_to, sptr_cstr name_to, sptr_cstr message, sptr_cstr raw,
		sptr<const std::vector<an_mimage>> images,
		sptr<const std::vector<an_maudio>> audios,
		sptr<const std::vector<an_mvideo>> videos,
		sptr<const std::vector<an_marchive>> archives,
		sptr<const std::vector<an_myoutube>> youtube,
		unsigned char color);
};

namespace serialization
{
template <>
void s<an_mess>(std::string& buf, const an_mess& obj, int level);
} // end of namespace


struct an_user
{
	OID				m_oid;
	sptr_cstr		m_name;
	sptr_cstr		m_avatar;

	an_user(OID oid, sptr_cstr name, sptr_cstr avatar);
};

namespace serialization
{
template <>
void s<an_user>(std::string& buf, const an_user& obj, int level);
} // end of namespace

struct an_room
{
	OID				m_oid;
	sptr_cstr		m_name;
	bool			m_inside;
    bool            m_private;
    bool            m_allow;
	bool			m_owner;
	size_t			m_amount;

	sptr_cstr		m_background_path;
	int				m_type;
	int				m_transparence;
	sptr_cstr		m_background_color;
	sptr_cstr		m_textsize;
	sptr_cstr		m_description;

	an_room(OID oid, sptr_cstr name, bool inside, bool arg_private, bool allow, bool owner, size_t amount,
		sptr_cstr background_path, int type, int transparence, sptr_cstr background_color, sptr_cstr textsize,
		sptr_cstr description);
};

namespace serialization
{
template <>
void s<an_room>(std::string& buf, const an_room& obj, int level);
} // end of namespace

struct an_writer
{
	OID				m_oid;
	sptr_cstr		m_name;

	an_writer(OID oid, sptr_cstr name);
};

namespace serialization
{
template <>
void s<an_writer>(std::string& buf, const an_writer& obj, int level);
} // end of namespace

struct an_image
{
	OID				m_oid;
	sptr_cstr		m_name;
	sptr_cstr		m_path;
	sptr_cstr		m_norm;

	an_image(OID oid, sptr_cstr name, sptr_cstr path, sptr_cstr norm);
};

namespace serialization
{
template <>
void s<an_image>(std::string& buf, const an_image& obj, int level);
} // end of namespace

struct an_audio
{
	OID				m_oid;
	sptr_cstr		m_name;
	sptr_cstr		m_title;
	sptr_cstr		m_artist;
	sptr_cstr		m_path;

	an_audio(OID oid, sptr_cstr name, sptr_cstr title, sptr_cstr artist, sptr_cstr path);
};

namespace serialization
{
template <>
void s<an_audio>(std::string& buf, const an_audio& obj, int level);
} // end of namespace

struct an_video
{
	OID				m_oid;
	sptr_cstr		m_name;
	sptr_cstr		m_path;

	an_video(OID oid, sptr_cstr name, sptr_cstr path);
};

namespace serialization
{
template <>
void s<an_video>(std::string& buf, const an_video& obj, int level);
} // end of namespace

struct an_archive
{
	OID				m_oid;
	sptr_cstr		m_name;
	sptr_cstr		m_path;

	an_archive(OID oid, sptr_cstr name, sptr_cstr path);
};

namespace serialization
{
template <>
void s<an_archive>(std::string& buf, const an_archive& obj, int level);
} // end of namespace

struct answer
{
	OID				oid;
	sptr_cstr		name;
	sptr_cstr		avatar;
	OID				room;
	OID				last_message;
	OID				hash_personal;
	OID				hash_image;
	OID				hash_audio;
	OID				hash_video;
	OID				hash_archive;
	int				mask;

	std::vector<an_mess>	messages;
	std::vector<an_user>	users;
	std::vector<an_room>	rooms;
	std::vector<an_writer>	writers;
	std::vector<an_image>	images;
	std::vector<an_audio>	audios;
	std::vector<an_video>	videos;
	std::vector<an_archive>	archives;
	std::string				info;
	std::string				error;

	bool m_short;

	answer(bool arg_short = false);
};

namespace serialization
{
template <>
void s<answer>(std::string& buf, const answer& obj, int level);
} // end of namespace

struct answer_personal
{
	OID				m_oid;
	sptr_cstr		m_name;

	std::vector<an_mess> m_messages;

	answer_personal();
};

namespace serialization
{
template <>
void s<answer_personal>(std::string& buf, const answer_personal& obj, int level);
} // end of namespace

#endif
