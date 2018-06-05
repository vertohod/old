#include <boost/thread/detail/singleton.hpp>

#include "stat_daemon.h"
#include "front.h"
#include "log.h"

using namespace boost::detail::thread;

sptr_str stat_daemon::create()
{   
	auto res = stat_base::create();

    auto& fr = singleton<front>::instance();

    serialization::s(*res, "daemon", fr, 1);

	return res;
}

void stat_daemon::clear()
{   
	stat_base::clear();
}
