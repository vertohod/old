#ifndef AVATAR_H
#define AVATAR_H

#include <map>
#include <string>
#include <mutex>

#include "types.h"

class avatar_store
{
private:
	std::map<OID, std::string> m_avatars;

	std::mutex m_mutex;

public:
	avatar_store();
	void init(const std::string& path);
	std::string get(OID key);
	sptr_cstr to_json(const std::string& path);
	size_t size();
};

#endif
