#ifndef SERVICE_MESSAGES_H
#define SERVICE_MESSAGES_H

#include <unordered_map>
#include <string>

#include "types.h"

sptr_cstr smessage(int message_code, const std::string& str1, const std::string& str2, const std::string& str3);
sptr_cstr smessage(int message_code, const std::string& str1, const std::string& str2);
sptr_cstr smessage(int message_code, const std::string& str);
sptr_cstr smessage(int message_code);

#define DEFAULT_USER "#Robot"

#define DEFAULT_ROOM0 "#Courtyard"
#define DEFAULT_ROOM1 "#Bedroom 1"
#define DEFAULT_ROOM2 "#Bedroom 2"
#define DEFAULT_ROOM3 "#Bedroom 3"
#define DEFAULT_ROOM4 "#Bedroom 4"
#define DEFAULT_ROOM5 "#Parlor"
#define DEFAULT_ROOM6 "#Kitchen"
#define DEFAULT_ROOM7 "#Bathroom"
#define DEFAULT_ROOM8 "#WC"
#define DEFAULT_ROOM9 "#Balcony small"
#define DEFAULT_ROOM10 "#Balcony big"
#define DEFAULT_ROOM11 "#The farthest dark corner"

#define SMESS100_WELCOME			100
#define SMESS101_USER_CAME_IN		101
#define SMESS102_USER_GOT_OUT		102
#define SMESS103_LEFT_WITHOUT_GB	103
#define SMESS104_NICKNAME_WRONG		104
#define SMESS105_NICKNAME_EXIST		105
#define SMESS106_ROOMNAME_WRONG		106
#define SMESS107_MESSAGE_WRONG		107
#define SMESS108_CHANGE_NICKNAME	108
#define SMESS109_MESSAGE_FLOOD		109
#define SMESS110_CHANGED_ROOM		110
#define SMESS111_INVALID_CHARACTERS	111
#define SMESS112_ACCESS_ADDED		112
#define SMESS113_ACCESS_REMOVED		113
#define SMESS114_USER_WAS_DRIVEN	114
#define SMESS115_NICKNAME_IS_FREE	115
#define SMESS116_PASSWORD_WRONG		116
#define SMESS117_PASSWORD_ADDED		117
#define SMESS118_THANKS_FOR_MESSAGE	118
#define SMESS119_NOT_SUPPORTED		119
#define SMESS120_IMAGE_UPLOADED		120
#define SMESS121_AUDIO_UPLOADED		121
#define SMESS122_UNKNOWN_EXTENSION	122
#define SMESS123_UPLOAD_NOT_SUCCESS	123
#define SMESS124_ROOM_ADDED			124
#define SMESS125_IMAGE_REMOVED		125
#define SMESS126_AUDIO_REMOVED		126
#define SMESS127_ALLOW_REMOVED		127
#define SMESS128_MESSAGE_REMOVED	128
#define SMESS129_USER_BLOCKED		129
#define SMESS130_ROOMNAME_EXIST		130
#define SMESS131_AVATAR_INSTALLED	131
#define SMESS132_INVITATION_ADDED	132
#define SMESS133_USED_INVITATION	133
#define SMESS134_FAILUSE_INVITATION	134
#define SMESS135_VIDEO_UPLOADED		135
#define SMESS136_VIDEO_REMOVED		136
#define SMESS137_ARCHIVE_UPLOADED	137
#define SMESS138_ARCHIVE_REMOVED	138

#define SMESS201_UNAUTHORIZED			201
#define SMESS202_USER_NOT_INSIDE		202
#define SMESS203_IMAGE_DN_BELONG		203
#define SMESS204_ROOM_DN_BELONG			204
#define SMESS205_IMAGE_NOT_FOUND		205
#define SMESS206_USER_NOT_FOUND			206
#define SMESS207_ROOM_ACCESS_DENIED		207
#define SMESS208_IMAGE_ACCESS_DENIED	208
#define SMESS209_AUDIO_ACCESS_DENIED	209
#define SMESS210_VIDEO_ACCESS_DENIED	210
#define SMESS211_ARCHIVE_ACCESS_DENIED	211
#define SMESS212_SESSION_NOT_FOUND		212
#define SMESS213_USERID_NOT_FOUND		213
#define SMESS214_GROUPID_NOT_FOUND		214
#define SMESS215_SET_UNABLE				215
#define SMESS216_CHANGE_UNABLE			216
#define SMESS217_DELETE_UNABLE			217
#define SMESS218_AUDIO_DN_BELONG		218
#define SMESS219_ADD_USER_UNABLE		219
#define SMESS220_OBJECT_NOT_FOUND		220
#define SMESS221_VIDEO_DN_BELONG		221
#define SMESS222_ARCHIVE_DN_BELONG		222

#endif
