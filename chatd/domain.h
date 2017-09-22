#ifndef DOMAIN_H
#define DOMAIN_H

#include <unordered_set>
#include <string>
#include <mutex>

class domain_store
{
private:
	std::unordered_set<std::string> m_domains;

    std::mutex m_mutex;

public:
	domain_store();
	void init(const std::string& path);
	bool check(const std::string& key);
};

#endif
