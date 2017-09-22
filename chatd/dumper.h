#ifndef DUMPER_H
#define DUMPER_H

#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <string>
#include "types.h"

typedef boost::function<void(sptr_cmap)> handler_t;

class dumper
{
private:
	std::string				m_path;
	std::vector<handler_t>	m_handlers;

public:
	dumper();
	void init(const std::string&);

	void add_handler(handler_t hand);
	void save(sptr_cstr);
	size_t load();

private:
	sptr_cmap parse(const std::string&);
};

#endif
