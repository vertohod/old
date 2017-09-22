#include <sstream>

#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>

#include "audio_tags.h"
#include "log.h"

using namespace std;

std::shared_ptr<audio_tags> get_audio_tags(const std::string& path)
{
	lo::l(lo::TRASH) << "get_audio_tags, path: " << path;

	TagLib::FileRef audio_file(path.c_str());

	std::shared_ptr<audio_tags> res(new audio_tags());

	if (!audio_file.isNull() && audio_file.tag()) {

		TagLib::Tag *tag = audio_file.tag();

		std::ostringstream st_title;
		std::ostringstream st_artist;

		st_title << tag->title();
		res->title = st_title.str();

		st_artist << tag->artist();
		res->artist = st_artist.str();

	} else lo::l(lo::TRASH) << "get_audio_tags, can't determine type of file";

	return res;
}
