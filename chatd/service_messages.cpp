#include "service_messages.h"
#include "util.h"

std::unordered_map<int, std::string> service_messages = {
	{SMESS100_WELCOME, "Welcome to Rupor.chat"},
	{SMESS101_USER_CAME_IN, "Let's welcome <b>%s</b> to the room."},
	{SMESS102_USER_GOT_OUT, "The user <b>%s</b> has left the room."},
	{SMESS103_LEFT_WITHOUT_GB, "The user <b>%s</b> has left without saying goodbye."},
	{SMESS104_NICKNAME_WRONG, "A nickname can't contain less than <b>%s</b> or more <b>%s</b> characters."},
	{SMESS105_NICKNAME_EXIST, "This nickname is taken already. Choose another one please."},
	{SMESS106_ROOMNAME_WRONG, "A room's name can't contain less than <b>%s</b> or more <b>%s</b> characters."},
	{SMESS107_MESSAGE_WRONG, "A message can't be empty or exceed <b>%s</b> characters."},
	{SMESS108_CHANGE_NICKNAME, "The user <b>%s</b> has changed his nickname to <b>%s</b>."},
	{SMESS109_MESSAGE_FLOOD, "Don't send the same messages please."},
	{SMESS110_CHANGED_ROOM, "The user <b>%s</b> has joined the room <b>%s</b>."},
	{SMESS111_INVALID_CHARACTERS, "You have used invalid characters. The name can contain letters, numbers, dashes and underscores."},
	{SMESS112_ACCESS_ADDED, "The user <b>%s</b> has granted you the permission to access the private room <b>%s</b>."},
	{SMESS113_ACCESS_REMOVED, "Access to the room <b>%s</b> was closed."},
	{SMESS114_USER_WAS_DRIVEN, "The user <b>%s</b> was driven from room."},
	{SMESS115_NICKNAME_IS_FREE, "An alias for <b>%s</b> can’t be created as it doesn’t exist."},
	{SMESS116_PASSWORD_WRONG, "The password is incorrect."},
	{SMESS117_PASSWORD_ADDED, "A password on your session is installed. Now you can run it on another device."},
	{SMESS118_THANKS_FOR_MESSAGE, "Thank you for your message. We will read it."},
	{SMESS119_NOT_SUPPORTED, "Uploaded file is not supported."},
	{SMESS120_IMAGE_UPLOADED, "The image was uploaded successfully. Please wait while it's being processed."},
	{SMESS121_AUDIO_UPLOADED, "The audio file was uploaded successfully."},
	{SMESS122_UNKNOWN_EXTENSION, "Unknown file's extension."},
	{SMESS123_UPLOAD_NOT_SUCCESS, "Upload complete isn't successfully."},
	{SMESS124_ROOM_ADDED, "The room was added successfully."},
	{SMESS125_IMAGE_REMOVED, "The images were removed successfully."},
	{SMESS126_AUDIO_REMOVED, "The audio files were removed successfully."},
	{SMESS127_ALLOW_REMOVED, "The permission for room was removed successfully."},
	{SMESS128_MESSAGE_REMOVED, "The messages were removed successfully."},
	{SMESS129_USER_BLOCKED, "The user <b>%s</b> was blocked."},
	{SMESS130_ROOMNAME_EXIST, "This room's name is taken already. Choose another one please."},
	{SMESS131_AVATAR_INSTALLED, "An avatar installed successfully"},
	{SMESS132_INVITATION_ADDED, "Invitation for the room <b>%s</b> was added"},
	{SMESS133_USED_INVITATION, "You have used invatation for coming in the room <b>%s</b>"},
	{SMESS134_FAILUSE_INVITATION, "You can't use this invitation."},
	{SMESS135_VIDEO_UPLOADED, "The video file was uploaded successfully."},
	{SMESS136_VIDEO_REMOVED, "The video files were removed successfully."},
	{SMESS137_ARCHIVE_UPLOADED, "The file was uploaded successfully."},
	{SMESS138_ARCHIVE_REMOVED, "The archive files were removed successfully."},
	{SMESS201_UNAUTHORIZED, "An user is not logged."},
	{SMESS202_USER_NOT_INSIDE, "An user haven't joined a room, and can't sends messages here."},
	{SMESS203_IMAGE_DN_BELONG, "The picture with the specified name does not belong to user."},
	{SMESS204_ROOM_DN_BELONG, "The room with the specified name does not belong to user."},
	{SMESS205_IMAGE_NOT_FOUND, "The picture with the specified name is not found."},
	{SMESS206_USER_NOT_FOUND, "An user with the ID is not found."},
	{SMESS207_ROOM_ACCESS_DENIED, "The room <b>%s</b> is private you don't have permission to access it."},
	{SMESS208_IMAGE_ACCESS_DENIED, "Access denied to image %s"},
	{SMESS209_AUDIO_ACCESS_DENIED, "Access denied to audio file %s"},
	{SMESS210_VIDEO_ACCESS_DENIED, "Access denied to video file %s"},
	{SMESS211_ARCHIVE_ACCESS_DENIED, "Access denied to file %s"},
	{SMESS212_SESSION_NOT_FOUND, "The session was not found 8-/"},
	{SMESS213_USERID_NOT_FOUND, "System user's id was not found for %s"},
	{SMESS214_GROUPID_NOT_FOUND, "System group's id was not found for %s"},
	{SMESS215_SET_UNABLE, "Failed to establish rights for %s"},
	{SMESS216_CHANGE_UNABLE, "Failed to change mode for %s"},
	{SMESS217_DELETE_UNABLE, "Failed to delete file %s"},
	{SMESS218_AUDIO_DN_BELONG, "The audio file with the specified name does not belong to user"},
	{SMESS219_ADD_USER_UNABLE, "Failed to add new session"},
	{SMESS220_OBJECT_NOT_FOUND, "The object %s was not found"},
	{SMESS221_VIDEO_DN_BELONG, "The video file with the specified name does not belong to user"},
	{SMESS222_ARCHIVE_DN_BELONG, "The archive with the specified name does not belong to user"}
};

sptr_cstr smessage(int code, const std::string& str1, const std::string& str2, const std::string& str3)
{
	sptr_str res;

	auto it = service_messages.find(code);
	if (it != service_messages.end()) {
		*res = util::ins(it->second, str1);
		*res = util::ins(*res, str2);
		*res = util::ins(*res, str3);

		*res = format(code) + " " + *res + ";" + str1 + ";" + str2 + ";" + str3;
	}

	return res;
}

sptr_cstr smessage(int code, const std::string& str1, const std::string& str2)
{
	sptr_str res;

	auto it = service_messages.find(code);
	if (it != service_messages.end()) {
		*res = util::ins(it->second, str1);
		*res = util::ins(*res, str2);

		*res = format(code) + " " + *res + ";" + str1 + ";" + str2;
	}

	return res;
}

sptr_cstr smessage(int code, const std::string& str)
{
	sptr_str res;

	auto it = service_messages.find(code);
	if (it != service_messages.end()) {
		*res = format(code) + " " + util::ins(it->second, str) + ";" + str;
	}

	return res;
}

sptr_cstr smessage(int code)
{
	sptr_str res;

	auto it = service_messages.find(code);
	if (it != service_messages.end()) *res = format(code) + " " + it->second;

	return res;
}
