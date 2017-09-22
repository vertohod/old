#include <fstream>
#include <dirent.h>
#include <stdio.h>

#include "format.h"
#include "avatar.h"
#include "types.h"

avatar_store::avatar_store()
{
}

void avatar_store::init(const std::string& path)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_avatars.clear();
	OID count = 0;

	DIR *dir = opendir(path.c_str());
	if (dir == NULL) throw std::runtime_error("Failed to read directory contents" + path);
		
	struct dirent *entry;
	while (NULL != (entry = readdir(dir))) {
		std::string file_name(entry->d_name);
		if (file_name == "." || file_name == "..") continue;

		m_avatars.insert(std::make_pair(++count, file_name));
	}
	closedir(dir);
}

std::string avatar_store::get(OID key)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto it = m_avatars.find(key);
	if (it != m_avatars.end()) return it->second;

	return std::string();
}
	
sptr_cstr avatar_store::to_json(const std::string& path)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr_str res;

	*res = "{";
	for (auto it = m_avatars.begin(); it != m_avatars.end(); ++it) {
		if (it != m_avatars.begin()) *res += ",";
		*res += "\"" + format(it->first) + "\":\"" + path + it->second + "\"";
	}
	*res += "}";

	return res;
}

size_t avatar_store::size()
{
	return m_avatars.size();
}
