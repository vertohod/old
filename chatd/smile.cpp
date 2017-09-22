#include <fstream>

#include "serialization.h"
#include "format.h"
#include "smile.h"

smile_info::smile_info(const std::string& es, const std::string& file) :
	m_escape_sequence(es), m_file(file)
{
}

smile_store::smile_store()
{
}

char smile_store::hex_to_char(char ch)
{
    if (ch >= '0' and ch <= '9')
        return static_cast<char>(static_cast<unsigned char>(ch) - static_cast<unsigned char>('0'));

    if (ch >= 'a' and ch <= 'f')
        return static_cast<char>(static_cast<unsigned char>(ch) - static_cast<unsigned char>('a') + 10);

    if (ch >= 'A' and ch <= 'F')
        return static_cast<char>(static_cast<unsigned char>(ch) - static_cast<unsigned char>('A') + 10);

	return ch;
}

void smile_store::init(const std::string& path, const std::string& extension)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	enum symb_state {
		symb_char = 0,
		symb_code1 = 1,
		symb_code2 = 2
	};

	std::ifstream	ifs;
	std::string		str_in;
	std::string		file_name;

	m_smiles.clear();
	m_for_out.clear();

    ifs.open(path.c_str(), std::ifstream::in);
    if (ifs.fail()) throw std::runtime_error("Failed to open file with smiles: " + path);

	size_t variant_count = 0;

	auto state = symb_char;
	char code = 0;

    for (char ch = ifs.get(); ifs.good(); ch = ifs.get()) {
		if (ch == '#') {

			state = symb_code1;
			continue;

		} else if (ch == ' ') {

			++variant_count;

			if (!str_in.empty()) add(str_in, file_name + extension, variant_count);

			str_in.clear();
			state = symb_char;

        } else if (ch == '\n') {

			++variant_count;

			if (!str_in.empty()) add(str_in, file_name + extension, variant_count);

			file_name.clear();
			str_in.clear();
			state = symb_char;

			variant_count = 0;

		} else {

			switch (state) {
				case symb_char: {
					str_in.push_back(ch);
					break;
				}
				case symb_code1: {
					code = hex_to_char(ch) << 4;
					file_name.push_back(ch);
					state = symb_code2;
					break;
				}
				case symb_code2: {
					code |= hex_to_char(ch);
					file_name.push_back(ch);
					str_in.push_back(code);
					state = symb_code1;
					break;
				}
			}

		}
	}

	if (!str_in.empty()) add(str_in, file_name + extension, ++variant_count);
}

void smile_store::add(const std::string& key, const std::string& val, int variant)
{
	m_smiles.insert(std::make_pair(key, val));
	if (variant == 2) m_for_out.push_back(smile_info(key, val)); 
}

std::string smile_store::get(const std::string& key)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	auto it = m_smiles.find(key);
	if (it != m_smiles.end()) return it->second;

	return std::string();
}
	
sptr_cstr smile_store::to_json(const std::string& path)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	sptr_str res;

	bool the_first = true;

	*res = "{";
	for (auto& val : m_for_out) {
		if (!the_first) *res += ","; else the_first = false;

		*res += "\"" + serialization::escape(val.m_escape_sequence) + "\":\"" + path + val.m_file + "\"";
	}
	*res += "}";

	return res;
}
