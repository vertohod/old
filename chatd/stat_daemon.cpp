#include <boost/thread/detail/singleton.hpp>

#include "stat_daemon.h"
#include "front.h"
#include "log.h"

STAT_TICK_DEF(view_tick)
STAT_TICK_DEF(image_tick)
STAT_TICK_DEF(audio_tick)
STAT_TICK_DEF(unknown_tick)

STAT_INTERVAL_DEF(view_interval)
STAT_INTERVAL_DEF(send_interval)
STAT_INTERVAL_DEF(proc_message_interval)
STAT_INTERVAL_DEF(proc_image_interval)
STAT_INTERVAL_DEF(proc_audio_interval)

STAT_SIZE_DEF(sess_size)
STAT_SIZE_DEF(image_size)
STAT_SIZE_DEF(audio_size)

using namespace boost::detail::thread;

sptr_str stat_daemon::create()
{   
	auto res = stat_base::create();

	auto& fr = singleton<front>::instance();

	serialization::s(*res, "daemon", fr, 1);

	serialization::s(*res, "view_rps", view_tick::get(), 1);
	serialization::s(*res, "image_rps", image_tick::get(), 1);
	serialization::s(*res, "audio_rps", audio_tick::get(), 1);
	serialization::s(*res, "unknown_rps", unknown_tick::get(), 1);

	serialization::s(*res, "view_time", view_interval::get(corr()), 1);
	serialization::s(*res, "send_time", send_interval::get(corr()), 1);
	serialization::s(*res, "proc_message_time", proc_message_interval::get(corr()), 1);
	serialization::s(*res, "proc_image_time", proc_image_interval::get(corr()), 1);
	serialization::s(*res, "proc_audio_time", proc_audio_interval::get(corr()), 1);

	serialization::s(*res, "session_size_buffer", sess_size::get(), 1);
	serialization::s(*res, "image_size_buffer", image_size::get(), 1);
	serialization::s(*res, "audio_size_buffer", audio_size::get(), 1);

	return res;
}

void stat_daemon::clear()
{   
	stat_base::clear();

	view_tick::clear();
	image_tick::clear();
	audio_tick::clear();
	unknown_tick::clear();

	view_interval::clear();
	send_interval::clear();
	proc_message_interval::clear();
	proc_image_interval::clear();
	proc_audio_interval::clear();

	sess_size::clear();
	image_size::clear();
	audio_size::clear();
}
