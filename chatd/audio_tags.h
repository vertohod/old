#ifndef AUDIO_TAGS_H
#define AUDIO_TAGS_H

// need to link libtag (/usr/local/lib), libz (/usr/lib/x86_64-linux-gnu)

#include <cwchar>
#include <string>
#include <memory>


struct audio_tags
{
	std::string	title;
	std::string	artist;
};

std::shared_ptr<audio_tags> get_audio_tags(const std::string& path);

#endif
