#include "answer.h"

an_mimage::an_mimage(sptr_cstr min, sptr_cstr norm) :
	m_min(min), m_norm(norm)
{
}

an_maudio::an_maudio(sptr_cstr name, sptr_cstr title, sptr_cstr artist, sptr_cstr path) :
	m_name(name), m_title(title), m_artist(artist), m_path(path)
{
}

an_mvideo::an_mvideo(sptr_cstr name, sptr_cstr path) : m_name(name), m_path(path)
{
}

an_marchive::an_marchive(sptr_cstr name, sptr_cstr path) : m_name(name), m_path(path)
{
}

an_myoutube::an_myoutube(sptr_cstr url) : m_url(url)
{
}

an_mess::an_mess(OID oid, OID room, OID poid, time_t time, bool appeal, bool arg_private, sptr_cstr avatar,
	OID oid_from,
	sptr_cstr name_from, OID oid_to, sptr_cstr name_to, sptr_cstr message, sptr_cstr raw,
	sptr<const std::vector<an_mimage>> images,
	sptr<const std::vector<an_maudio>> audios,
	sptr<const std::vector<an_mvideo>> videos,
	sptr<const std::vector<an_marchive>> archives,
	sptr<const std::vector<an_myoutube>> youtube,
	unsigned char color) :
	m_oid(oid), m_room(room), m_poid(poid), m_time(time), m_appeal(appeal), m_private(arg_private),
	m_avatar(avatar), m_oid_from(oid_from),
	m_name_from(name_from), m_oid_to(oid_to), m_name_to(name_to), m_message(message), m_raw(raw),
	m_images(images), m_audios(audios), m_videos(videos), m_archives(archives), m_youtube(youtube), m_color(color)
{
}

an_user::an_user(OID oid, sptr_cstr name, sptr_cstr avatar) :
	m_oid(oid), m_name(name), m_avatar(avatar)
{
}

an_room::an_room(OID oid, sptr_cstr name, bool inside, bool arg_private, bool allow, bool owner, size_t amount,
	sptr_cstr background_path, int type, int transparence, sptr_cstr background_color, sptr_cstr textsize,
	sptr_cstr description) :
	m_oid(oid), m_name(name), m_inside(inside), m_private(arg_private), m_allow(allow), m_owner(owner), m_amount(amount),
	m_background_path(background_path), m_type(type), m_transparence(transparence), m_background_color(background_color),
	m_textsize(textsize), m_description(description)
{
}

an_writer::an_writer(OID oid, sptr_cstr name) :
	m_oid(oid), m_name(name)
{
}

an_image::an_image(OID oid, sptr_cstr name, sptr_cstr path, sptr_cstr norm) :
	m_oid(oid), m_name(name), m_path(path), m_norm(norm)
{
}

an_audio::an_audio(OID oid, sptr_cstr name, sptr_cstr title, sptr_cstr artist, sptr_cstr path) :
	m_oid(oid), m_name(name), m_title(title), m_artist(artist), m_path(path)
{
}

an_video::an_video(OID oid, sptr_cstr name, sptr_cstr path) :
	m_oid(oid), m_name(name), m_path(path)
{
}

an_archive::an_archive(OID oid, sptr_cstr name, sptr_cstr path) :
	m_oid(oid), m_name(name), m_path(path)
{
}

answer::answer(bool arg_short) : oid(0), room(0), last_message(0), m_short(arg_short)
{
}

answer_personal::answer_personal() : m_oid(0)
{
}

namespace serialization
{

template <>
void s<an_mimage>(std::string& buf, const an_mimage& obj, int level)
{
	begin(buf);
	s(buf, "min", *obj.m_min, 0);
	comma(buf);
	s(buf, "norm", *obj.m_norm, 0);
	end(buf);
}

template <>
void s<an_maudio>(std::string& buf, const an_maudio& obj, int level)
{
	begin(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "title", *obj.m_title, 0);
	comma(buf);
	s(buf, "artist", *obj.m_artist, 0);
	comma(buf);
	s(buf, "path", *obj.m_path, 0);
	end(buf);
}

template <>
void s<an_mvideo>(std::string& buf, const an_mvideo& obj, int level)
{
	begin(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "path", *obj.m_path, 0);
	end(buf);
}

template <>
void s<an_marchive>(std::string& buf, const an_marchive& obj, int level)
{
	begin(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "path", *obj.m_path, 0);
	end(buf);
}

template <>
void s<an_myoutube>(std::string& buf, const an_myoutube& obj, int level)
{
	begin(buf);
	s(buf, "url", *obj.m_url, 0);
	end(buf);
}

template <>
void s<an_mess>(std::string& buf, const an_mess& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "room", obj.m_room, 0);
	comma(buf);
	s(buf, "poid", obj.m_poid, 0);
	comma(buf);
	s(buf, "time", obj.m_time, 0);
	comma(buf);
	s(buf, "appeal", obj.m_appeal, 0);
	comma(buf);
	s(buf, "private", obj.m_private, 0);
	comma(buf);
	s(buf, "avatar", *obj.m_avatar, 0);
	comma(buf);
	s(buf, "oid_from", obj.m_oid_from, 0);
	comma(buf);
	s(buf, "name_from", *obj.m_name_from, 0);
	comma(buf);
	s(buf, "oid_to", obj.m_oid_to, 0);
	comma(buf);
	s(buf, "name_to", *obj.m_name_to, 0);
	comma(buf);
	s(buf, "html", *obj.m_message, 0);
	comma(buf);
	s(buf, "raw", *obj.m_raw, 0);
	comma(buf);
	s(buf, "images", *obj.m_images, 0);
	comma(buf);
	s(buf, "audios", *obj.m_audios, 0);
	comma(buf);
	s(buf, "videos", *obj.m_videos, 0);
	comma(buf);
	s(buf, "archives", *obj.m_archives, 0);
	comma(buf);
	s(buf, "youtube", *obj.m_youtube, 0);
	comma(buf);
	s(buf, "color", obj.m_color, 0);
	end(buf);
}

template <>
void s<an_user>(std::string& buf, const an_user& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "avatar", *obj.m_avatar, 0);
	end(buf);
}

template <>
void s<an_room>(std::string& buf, const an_room& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "inside", obj.m_inside, 0);
	comma(buf);
	s(buf, "private", obj.m_private, 0);
	comma(buf);
	s(buf, "allow", obj.m_allow, 0);
	comma(buf);
	s(buf, "owner", obj.m_owner, 0);
	comma(buf);
	s(buf, "amount", obj.m_amount, 0);
	comma(buf);
	s(buf, "background_path", *obj.m_background_path, 0);
	comma(buf);
	s(buf, "type", obj.m_type, 0);
	comma(buf);
	s(buf, "transparence", obj.m_transparence, 0);
	comma(buf);
	s(buf, "background_color", *obj.m_background_color, 0);
	comma(buf);
	s(buf, "textsize", *obj.m_textsize, 0);
	comma(buf);
	s(buf, "description", *obj.m_description, 0);
	end(buf);
}

template <>
void s<an_writer>(std::string& buf, const an_writer& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	end(buf);
}

template <>
void s<an_image>(std::string& buf, const an_image& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "path", *obj.m_path, 0);
	comma(buf);
	s(buf, "norm", *obj.m_norm, 0);
	end(buf);
}

template <>
void s<an_audio>(std::string& buf, const an_audio& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "title", *obj.m_title, 0);
	comma(buf);
	s(buf, "artist", *obj.m_artist, 0);
	comma(buf);
	s(buf, "path", *obj.m_path, 0);
	end(buf);
}

template <>
void s<an_video>(std::string& buf, const an_video& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "path", *obj.m_path, 0);
	end(buf);
}

template <>
void s<an_archive>(std::string& buf, const an_archive& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "path", *obj.m_path, 0);
	end(buf);
}

template <>
void s<answer>(std::string& buf, const answer& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.oid, 0);
	comma(buf);
	s(buf, "name", *obj.name, 0);
	comma(buf);
	s(buf, "avatar", *obj.avatar, 0);
	comma(buf);
	s(buf, "room", obj.room, 0);

	if (!obj.m_short) {

		comma(buf);
		s(buf, "last_message", obj.last_message, 0);
		comma(buf);
		s(buf, "hash_personal", obj.hash_personal, 0);
		comma(buf);
		s(buf, "hash_image", obj.hash_image, 0);
		comma(buf);
		s(buf, "hash_audio", obj.hash_audio, 0);
		comma(buf);
		s(buf, "hash_video", obj.hash_video, 0);
		comma(buf);
		s(buf, "hash_archive", obj.hash_archive, 0);
		comma(buf);
		s(buf, "messages", obj.messages, 0);
		comma(buf);
		s(buf, "writers", obj.writers, 0);

		if (obj.mask & SHOW_USERS) {
			comma(buf);
			s(buf, "users", obj.users, 0);
		}

		if (obj.mask & SHOW_ROOMS) {
			comma(buf);
			s(buf, "rooms", obj.rooms, 0);
		}

		if (obj.mask & SHOW_IMAGES) {
			comma(buf);
			s(buf, "images", obj.images, 0);
		}

		if (obj.mask & SHOW_AUDIOS) {
			comma(buf);
			s(buf, "audios", obj.audios, 0);
		}

		if (obj.mask & SHOW_VIDEOS) {
			comma(buf);
			s(buf, "videos", obj.videos, 0);
		}

		if (obj.mask & SHOW_ARCHIVES) {
			comma(buf);
			s(buf, "archives", obj.archives, 0);
		}
	}

	comma(buf);
	s(buf, "info", obj.info, 0);
	comma(buf);
	s(buf, "error", obj.error, 0);
	end(buf);
}

template <>
void s<answer_personal>(std::string& buf, const answer_personal& obj, int level)
{
	begin(buf);
	s(buf, "oid", obj.m_oid, 0);
	comma(buf);
	s(buf, "name", *obj.m_name, 0);
	comma(buf);
	s(buf, "messages", obj.m_messages, 0);
	end(buf);
}

} // end of namespace
