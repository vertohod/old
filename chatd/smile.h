#ifndef SMILE_H
#define SMILE_H

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

#include "types.h"

struct smile_info
{
	std::string m_escape_sequence;
	std::string	m_file;

	smile_info(const std::string&, const std::string&);
};

class smile_store
{
private:
	std::unordered_map<std::string, std::string> m_smiles;
	std::vector<smile_info> m_for_out;

    std::mutex m_mutex;

public:
	smile_store();
	void init(const std::string& path, const std::string& extension);
	std::string get(const std::string& key);
	sptr_cstr to_json(const std::string&);

private:
	void add(const std::string&, const std::string&, int);
	static char hex_to_char(char ch);
};

#endif
