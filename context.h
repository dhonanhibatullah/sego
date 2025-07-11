#ifndef __SEGO_CONTEXT_H
#define __SEGO_CONTEXT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include "enums.h"

    typedef struct
    {
        pthread_mutex_t lock;
        sgContextFlag flag;
        int fd[2];
    } sgContext;

    /*
     * @brief   Creates new context.
     * @param   none
     * @return  The pointer to the context (`sgContext`) instance.
     */
    sgContext *sgContextCreate()
    {
        sgContext *ctx = (sgContext *)malloc(sizeof(sgContext));
        if (!ctx)
            return NULL;

        if (pthread_mutex_init(&ctx->lock, NULL) != 0)
        {
            free(ctx);
            return NULL;
        }

        if (pipe(ctx->fd) == -1)
        {
            pthread_mutex_destroy(&ctx->lock);
            free(ctx);
            return NULL;
        }

        ctx->flag = SG_CTX_LOWERED;
        return ctx;
    }

    /*
     * @brief   Pushes a value to the pipe
     * @param   ctx the context instance
     * @return  None.
     */
    void __sgContextPipePush(sgContext *ctx)
    {
        static uint8_t val = 0xFF;
        write(ctx->fd[1], &val, 1);
    }

    /*
     * @brief   Pops a value to the pipe
     * @param   ctx the context instance
     * @return  None.
     */
    void __sgContextPipePop(sgContext *ctx)
    {
        static uint8_t tmp;
        read(ctx->fd[0], &tmp, 1);
    }

    /*
     * @brief   Raises the context flag.
     * @param   ctx the context instance
     * @return  `SG_ERR_NULLPTR` if the argument is a `NULL`. `SG_OK` if ok.
     */
    sgReturnType sgContextRaise(sgContext *ctx)
    {
        if (ctx == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ctx->lock);
        if (ctx->flag != SG_CTX_RAISED)
        {
            ctx->flag = SG_CTX_RAISED;
            __sgContextPipePush(ctx);
        }
        pthread_mutex_unlock(&ctx->lock);

        return SG_OK;
    }

    /*
     * @brief   Lowers the context flag.
     * @param   ctx the context instance
     * @return  `SG_ERR_NULLPTR` if the argument is a `NULL`. `SG_OK` if ok.
     */
    sgReturnType sgContextLower(sgContext *ctx)
    {
        if (ctx == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ctx->lock);
        if (ctx->flag != SG_CTX_LOWERED)
        {
            ctx->flag = SG_CTX_LOWERED;
            __sgContextPipePop(ctx);
        }
        pthread_mutex_unlock(&ctx->lock);

        return SG_OK;
    }

    /*
     * @brief   Retrieves the context flag.
     * @param   ctx the context instance
     * @return  `SG_CTX_ERROR` if the argument is a `NULL`. Otherwise, `SG_CTX_LOWERED` or `SG_CTX_RAISED`.
     */
    sgContextFlag sgContextGetFlag(sgContext *ctx)
    {
        if (ctx == NULL)
            return SG_CTX_ERROR;

        pthread_mutex_lock(&ctx->lock);
        sgContextFlag flag = ctx->flag;
        pthread_mutex_unlock(&ctx->lock);
        return flag;
    }

    /*
     * @brief   Destroys the context instance.
     * @param   ctx the context instance
     * @return  None.
     */
    void sgContextDestroy(sgContext *ctx)
    {
        if (ctx == NULL)
            return;

        pthread_mutex_destroy(&ctx->lock);
        free(ctx);
    }

    typedef struct
    {
        uint8_t raise;
        sgContext *ctx;
    } __sgContextToggleAfterArgs;

    /*
     * @brief   Context's flag toggling routine.
     * @param   a the routine argument
     * @return  A void pointer.
     */
    void __sgContextToggleAfterHandler(union sigval sv)
    {
        __sgContextToggleAfterArgs *args = (__sgContextToggleAfterArgs *)sv.sival_ptr;

        if (args->raise)
            sgContextRaise(args->ctx);
        else
            sgContextLower(args->ctx);

        free(args);
    }

    /*
     * @brief   Raises context's flag after a period of time.
     * @param   ctx the context instance
     * @param   time the time
     * @return  `SG_ERR_NULLPTR` if the argument is a `NULL`. `SG_ERR_ALLOC` if memory allocation is somehow failed. `SG_ERR_PTHREAD` if there is a problem with `pthread`. `SG_OK` if ok.
     * @note    Multiply the time with the desired time unit, e.g. 500L * `SG_TIME_MS` for flag raised after 500ms.
     */
    sgReturnType sgContextRaiseAfter(sgContext *ctx, long time)
    {
        if (ctx == NULL)
            return SG_ERR_NULLPTR;

        __sgContextToggleAfterArgs *args = (__sgContextToggleAfterArgs *)malloc(sizeof(__sgContextToggleAfterArgs));
        if (args == NULL)
            return SG_ERR_ALLOC;

        args->raise = 0x01;
        args->ctx = ctx;

        struct sigevent sev = {0};
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_notify_function = __sgContextToggleAfterHandler;
        sev.sigev_value.sival_ptr = args;

        timer_t timerId;
        timer_create(CLOCK_REALTIME, &sev, &timerId);

        struct itimerspec its = {0};
        its.it_value.tv_sec = time / SG_TIME_S;
        its.it_value.tv_nsec = time % SG_TIME_S;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
        timer_settime(timerId, 0, &its, NULL);

        return SG_OK;
    }

    /*
     * @brief   Lowers context's flag after a period of time.
     * @param   ctx the context instance
     * @param   time the time
     * @return  `SG_ERR_NULLPTR` if the argument is a `NULL`. `SG_ERR_ALLOC` if memory allocation is somehow failed. `SG_ERR_PTHREAD` if there is a problem with `pthread`. `SG_OK` if ok.
     * @note    Multiply the time with the desired time unit, e.g. 500L * `SG_TIME_MS` for flag lowered after 500ms.
     */
    sgReturnType sgContextLowerAfter(sgContext *ctx, long time)
    {
        if (ctx == NULL)
            return SG_ERR_NULLPTR;

        __sgContextToggleAfterArgs *args = (__sgContextToggleAfterArgs *)malloc(sizeof(__sgContextToggleAfterArgs));
        if (args == NULL)
            return SG_ERR_ALLOC;

        args->raise = 0x00;
        args->ctx = ctx;

        struct sigevent sev = {0};
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_notify_function = __sgContextToggleAfterHandler;
        sev.sigev_value.sival_ptr = args;

        timer_t timerId;
        timer_create(CLOCK_REALTIME, &sev, &timerId);

        struct itimerspec its = {0};
        its.it_value.tv_sec = time / SG_TIME_S;
        its.it_value.tv_nsec = time % SG_TIME_S;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
        timer_settime(timerId, 0, &its, NULL);

        return SG_OK;
    }

#ifdef __cplusplus
}
#endif

#endif