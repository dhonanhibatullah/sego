#ifndef __SEGO_MOMENT_H
#define __SEGO_MOMENT_H

#include <time.h>
#include "enums.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void sgMomentSleep(long time)
    {
        struct timespec ts = {
            .tv_sec = time / SG_TIME_S,
            .tv_nsec = time % SG_TIME_S};

        nanosleep(&ts, NULL);
    }

#ifdef __cplusplus
}
#endif

#endif