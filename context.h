#ifndef __SEGO_CONTEXT_H
#define __SEGO_CONTEXT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "enums.h"

    typedef struct
    {
        pthread_mutex_t lock;
        sgContextFlag flag;
    } sgContext;

    /*
     * @brief   Creates new context.
     * @param   none
     * @return  the pointer to the context (`sgContext`) instance
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

        ctx->flag = SG_CTX_LOWERED;
        return ctx;
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
        ctx->flag = SG_CTX_RAISED;
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
        ctx->flag = SG_CTX_LOWERED;
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
     * @return  none
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
        struct timespec ts;
    } __sgContextToggleAfterArgs;

    /*
     * @brief   Context's flag toggling routine.
     * @param   a the routine argument
     * @return  a void pointer
     */
    void *__sgContextToggleAfterRoutine(void *a)
    {
        __sgContextToggleAfterArgs *args = (__sgContextToggleAfterArgs *)a;

        nanosleep(&args->ts, NULL);
        if (args->raise)
            sgContextRaise(args->ctx);
        else
            sgContextLower(args->ctx);

        free(args);
        return NULL;
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
        args->ts.tv_sec = time / SG_TIME_S;
        args->ts.tv_nsec = time % SG_TIME_S;

        pthread_t tid;
        if (pthread_create(&tid, NULL, __sgContextToggleAfterRoutine, (void *)args) != 0)
        {
            free(args);
            return SG_ERR_PTHREAD;
        }

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
        args->ts.tv_sec = time / SG_TIME_S;
        args->ts.tv_nsec = time % SG_TIME_S;

        pthread_t tid;
        if (pthread_create(&tid, NULL, __sgContextToggleAfterRoutine, (void *)args) != 0)
        {
            free(args);
            return SG_ERR_PTHREAD;
        }

        return SG_OK;
    }

#ifdef __cplusplus
}
#endif

#endif