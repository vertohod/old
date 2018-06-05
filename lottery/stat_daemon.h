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

#endif
