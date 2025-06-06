#ifndef __SEGO_MOMENT_H
#define __SEGO_MOMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <signal.h>
#include <time.h>
#include "enums.h"

    typedef void (*sgTimerCallback)(void *);

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

    typedef struct
    {
        timer_t id;
        sgTimerCallback cb;
        void *args;
        uint64_t reps;
        uint64_t cnt;
    } sgMomentTimer;

    /*
     * @brief   The wrapper function for `sgMomentTimer`'s callback.
     * @param   sv the signal value, contains the signal event instances
     * @return  None.
     */
    void __sgMomentTimerHandlerWrapper(union sigval sv)
    {
        sgMomentTimer *args = (sgMomentTimer *)sv.sival_ptr;
        args->cb(args->args);
        args->cnt += 1;

        if (args->reps != 0)
            if (args->cnt == args->reps)
            {
                timer_delete(args->id);
                free(args);
            }
    }

    /*
     * @brief   Creates and starts the timer immediately.
     * @param   delay the delay before the timer starts
     * @param   interval the interval
     * @param   reps the repetition count, `0` for infinite repetitions
     * @param   cb the callback function
     * @param   args the argument to the callback function
     * @return  The pointer to the timer (`sgMomentTimer`) instance.
     * @note    Do not use `sgMomentTimerDestroy()` if `reps` is non-zero, since the timer will destroy itself.
     * @note    Multiply the time with the desired time unit, e.g. 500L * `SG_TIME_MS` for 500ms.
     */
    sgMomentTimer *sgMomentTimerCreate(long delay, long interval, uint64_t reps, sgTimerCallback cb, void *args)
    {
        if (cb == NULL)
            return NULL;

        sgMomentTimer *a = (sgMomentTimer *)malloc(sizeof(sgMomentTimer));
        if (a == NULL)
            return NULL;

        a->cb = cb;
        a->args = args;
        a->reps = reps;
        a->cnt = 0;

        struct sigevent sev = {0};
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_notify_function = __sgMomentTimerHandlerWrapper;
        sev.sigev_value.sival_ptr = a;
        timer_create(CLOCK_REALTIME, &sev, &a->id);

        struct itimerspec its = {0};
        its.it_value.tv_sec = delay / SG_TIME_S;
        its.it_value.tv_nsec = delay % SG_TIME_S;
        its.it_interval.tv_sec = interval / SG_TIME_S;
        its.it_interval.tv_nsec = interval % SG_TIME_S;
        timer_settime(a->id, 0, &its, NULL);

        return a;
    }

    /*
     * @brief   Stops and destroys the created timer.
     * @param   timer the timer (`sgMomentTimer`) instance
     * @return  None.
     * @note    Do not use this function if `reps` is non-zero, since the timer will destroy itself.
     */
    void sgMomentTimerDestroy(sgMomentTimer *timer)
    {
        timer_delete(timer->id);
        free(timer);
    }

#ifdef __cplusplus
}
#endif

#endif