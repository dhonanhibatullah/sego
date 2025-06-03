#ifndef __SEGO_MOMENT_H
#define __SEGO_MOMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <time.h>
#include "enums.h"

    /*
     * @brief   Sleeps.
     * @param   time sleep duration
     * @return  none
     * @note    Multiply the time with the desired time unit, e.g. 500L * `SG_TIME_MS` for 500ms sleep.
     */
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