#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <boost/thread/detail/singleton.hpp>
#include <string>

#include "config.h"

using namespace boost::detail::thread;

class chatd_config : public config
{
public:
	chatd_config() {}

	chatd_config(const std::string& arPath) : config(arPath)
	{
		call_back();
	}

	void call_back()
	{
		// Необходим вызов для установки базовых параметров
		config::call_back();

		auto& op = get_options();

		op
			("listen", "Port for main server", listen)
			("admin-port", "Port for recieving command", admin_port)
			("user", "User name for UNIX-socket", user)
			("group", "Group name for UNIX-socket", group)
			("path-pid", "Path of file which content daemon's pid", path_pid)
			("length-queue", "Length of queue for last messages", length_queue)
			("nickname-min", "Min length for nickname", nickname_min)
			("nickname-max", "Max length for nickname", nickname_max)
			("roomname-min", "Min length for room's name", roomname_min)
			("roomname-max", "Max length for room's name", roomname_max)
			("message-length-max", "Max length of message which possible to send in the chat", message_length_max)
			("message-amount", "Max amount message which will be keep in memory", message_amount)
			("timeout-room", "Time after whick we can remove a room", timeout_room)
			("timeout-user-inside", "Time after whick we can expial user out a room", timeout_user_inside)
			("timeout-session-empty", "Time after whick we can remove a session", timeout_session_empty)
			("timeout-session-full", "Time after whick we can remove a session", timeout_session_full)
			("timeout-writer", "Time after whick we can remove a writer", timeout_writer)
			("command-resize", "Command line for resize images to normal size", command_resize)
			("command-miniature", "Command line for resize images to miniature size", command_miniature)
			("command-compress", "Command line for encoding file", command_compress)
			("dir-temp", "Temp directory for upload images", dir_temp)
			("dir-normal", "Directory for keeping images normal size", dir_normal)
			("dir-miniature", "Directory for keeping images miniature size", dir_miniature)
			("dir-normal-short", "Path for redirect to images normal size", dir_normal_short)
			("dir-miniature-short", "Path for redirect to images miniature size", dir_miniature_short)
			("path-image-web", "path which using browser", path_image_web)
			("path-empty-image", "path to image, whick can see wrong user", path_empty_image)
			("path-dump", "path to daemon's dump", path_dump)
			("image-mode", "File mode bits for Images", image_mode)
			("audio-mode", "File mode bits for Audios", audio_mode)
			("video-mode", "File mode bits for Videos", video_mode)
			("archive-mode", "File mode bits for Archives", archive_mode)
			("path-messages", "Messages for developers", path_messages)
			("message-is-old", "The message consider as old if it lives seconds", message_is_old)
			("dir-audio", "Directory for keeping audio files", dir_audio)
			("dir-audio-short", "Path for redirect to audio file", dir_audio_short)
			("path-audio-web", "Path which using browser for audio", path_audio_web)
			("dir-video", "Directory for keeping video files", dir_video)
			("dir-video-short", "Path for redirect to video file", dir_video_short)
			("path-video-web", "Path which using browser for video", path_video_web)
			("dir-archive", "Directory for keeping archive", dir_archive)
			("dir-archive-short", "Path for redirect to archive file", dir_archive_short)
			("path-archive-web", "Path which using browser for archive", path_archive_web)
			("upstream", "Map of upstreams to other daemons", upstream)
			("timeout-execd", "Time of waiting of answer from execd", timeout_execd)
			("background00", "Path to image for room's background", background00)
			("background01", "Path to image for room's background", background01)
			("background02", "Path to image for room's background", background02)
			("background03", "Path to image for room's background", background03)
			("background04", "Path to image for room's background", background04)
			("background05", "Path to image for room's background", background05)
			("background06", "Path to image for room's background", background06)
			("background07", "Path to image for room's background", background07)
			("background08", "Path to image for room's background", background08)
			("background09", "Path to image for room's background", background09)
			("background10", "Path to image for room's background", background10)
			("background11", "Path to image for room's background", background11)
			("robot-avatar", "Path to robot avatar", robot_avatar)
			("dir-avatars", "Path to dir with avatars", dir_avatars)
			("path-smiles-list", "Path to file with smiles' list", path_smiles_list)
			("path-domains-list", "Path to file with domains' list", path_domains_list)
			("dir-avatars-short", "Path to dir with avatars for web", dir_avatars_short)
			("dir-smiles-short", "Path to dir with smiles for web", dir_smiles_short)
			("smiles-extension", "Extension of smiles", smiles_extension)
			("invite-path", "Begining of invitation's path", invite_path)
			("invite-redirect", "Path for invitation's redirect", invite_redirect)
			("path-form-password", "Path to password's input form", path_form_password)
		;

		// Проверяем, нет ли лишних параметров в файле
		check();
	}


public:
	// OPTIONS --------------------------------------------------------

	std::string		listen;
	std::string		admin_port;
	std::string		user;
	std::string		group;
	std::string		path_pid;
	size_t			length_queue;
	size_t			nickname_min;
	size_t			nickname_max;
	size_t			roomname_min;
	size_t			roomname_max;
	size_t			message_length_max;
	size_t			message_amount;
	long			timeout_room;
	long			timeout_user_inside;
	long			timeout_session_empty;
	long			timeout_session_full;
	long			timeout_writer;
	std::string		command_resize;
	std::string		command_miniature;
	std::string		command_compress;
	std::string		dir_temp;
	std::string		dir_normal;
	std::string		dir_miniature;
	std::string		dir_normal_short;
	std::string		dir_miniature_short;
	std::string		path_image_web;
	std::string		path_empty_image;
	std::string		path_dump;
	std::string		image_mode;
	std::string		audio_mode;
	std::string		video_mode;
	std::string		archive_mode;
	std::string		path_messages;
	long			message_is_old;
	std::string		dir_audio;
	std::string		dir_audio_short;
	std::string		path_audio_web;
	std::string		dir_video;
	std::string		dir_video_short;
	std::string		path_video_web;
	std::string		dir_archive;
	std::string		dir_archive_short;
	std::string		path_archive_web;
	map_str_t		upstream;
	size_t			timeout_execd;

	std::string		background00;
	std::string		background01;
	std::string		background02;
	std::string		background03;
	std::string		background04;
	std::string		background05;
	std::string		background06;
	std::string		background07;
	std::string		background08;
	std::string		background09;
	std::string		background10;
	std::string		background11;

	std::string		robot_avatar;
	std::string		dir_avatars;
	std::string		path_smiles_list;
	std::string		path_domains_list;

	std::string		dir_avatars_short;
	std::string		dir_smiles_short;

	std::string		smiles_extension;
	std::string		invite_path;
	std::string		invite_redirect;
	std::string		path_form_password;

	// ----------------------------------------------------------------
};

inline chatd_config& cfg()
{
	return singleton<chatd_config>::instance();
}

#endif
