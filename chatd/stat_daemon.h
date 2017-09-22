#ifndef STAT_DAEMON_H
#define STAT_DAEMON_H

#include "stat_base.h"
#include "types.h"

class stat_daemon : public stat_base
{
public:
    virtual sptr_str create();
    virtual void clear();
};

STAT_TICK(tview, view_tick)
STAT_TICK(timage, image_tick)
STAT_TICK(taudio, audio_tick)
STAT_TICK(tunknown, unknown_tick)

STAT_INTERVAL(iview, view_interval)
STAT_INTERVAL(isend, send_interval)
STAT_INTERVAL(iprocm, proc_message_interval)
STAT_INTERVAL(iproci, proc_image_interval)
STAT_INTERVAL(iproca, proc_audio_interval)

STAT_SIZE(ssess, sess_size)
STAT_SIZE(simage, image_size)
STAT_SIZE(saudio, audio_size)

#endif
