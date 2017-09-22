#include <stdexcept>
#include <exception>
#include <fstream>
#include "dumper.h"
#include "util.h"
#include "log.h"

dumper::dumper()
{
}

void dumper::init(const std::string& path)
{
	m_path = path;
}

size_t dumper::load()
{
    std::ifstream   ifs;
    std::string     str_in;

    ifs.open(m_path.c_str(), std::ifstream::in);
    if (ifs.fail()) throw std::runtime_error("Failed to open dump file: " + m_path);

	size_t count = 0;

    for (char ch = ifs.get(); ifs.good(); ch = ifs.get()) {
		if (ch == '\n') {

			++count;

			try {
				// Парсим строку, и вызываем все обработчики для нее
				auto record = parse(str_in);
				for (auto handler : m_handlers) handler(record);
			} catch (const std::exception& e) {
				lo::l(lo::ERROR) << "line: " << str_in;
				throw std::runtime_error(e.what());
			}

			str_in.clear();
		} else str_in.push_back(ch);
    }

    ifs.close();

	return count;
}

void dumper::add_handler(handler_t hand)
{
	m_handlers.push_back(hand);
}

sptr_cmap dumper::parse(const std::string& line)
{
	auto fields = util::split(line, "\t");

	sptr_map res;

	for (auto& field : *fields) {

		auto pos = field.find("=");
		if (pos != std::string::npos) {
			res->insert(std::make_pair(field.substr(0, pos), field.substr(pos + 1, field.size() - pos - 1)));
		}
	}

	return res;
}

void dumper::save(sptr_cstr object)
{
    std::ofstream   ofs;

    ofs.open(m_path.c_str(), std::ofstream::out | std::ofstream::app);
	ofs << *object << "\n";
	ofs.close();
}
