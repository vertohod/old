#include <fstream>

#include "format.h"
#include "domain.h"

domain_store::domain_store()
{
}

void domain_store::init(const std::string& path)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	std::ifstream	ifs;
	std::string		str_in;

	m_domains.clear();

    ifs.open(path.c_str(), std::ifstream::in);
    if (ifs.fail()) throw std::runtime_error("Failed to open file with domains: " + path);

    for (char ch = ifs.get(); ifs.good(); ch = ifs.get()) {

        if (ch == '\n') {

			if (!str_in.empty()) m_domains.insert(str_in);
			str_in.clear();

		} else str_in.push_back(ch);
	}

	if (!str_in.empty()) m_domains.insert(str_in);
}

bool domain_store::check(const std::string& key)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto it = m_domains.find(key);
	return it != m_domains.end();
}
