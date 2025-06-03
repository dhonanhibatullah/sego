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

    sgReturnType sgContextRaise(sgContext *ctx)
    {
        if (ctx == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ctx->lock);
        ctx->flag = SG_CTX_RAISED;
        pthread_mutex_unlock(&ctx->lock);

        return SG_OK;
    }

    sgReturnType sgContextLower(sgContext *ctx)
    {
        if (ctx == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ctx->lock);
        ctx->flag = SG_CTX_LOWERED;
        pthread_mutex_unlock(&ctx->lock);

        return SG_OK;
    }

    sgContextFlag sgContextGetFlag(sgContext *ctx)
    {
        if (ctx == NULL)
            return 0xFF;

        pthread_mutex_lock(&ctx->lock);
        sgContextFlag flag = ctx->flag;
        pthread_mutex_unlock(&ctx->lock);
        return flag;
    }

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