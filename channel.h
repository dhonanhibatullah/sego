#ifndef __SEGO_CHANNEL_H
#define __SEGO_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "enums.h"
#include "queue.h"

    typedef struct
    {
        pthread_mutex_t lock;
        pthread_cond_t cond;
        sgQueue *queue;
    } sgChan;

    /*
     */
    sgChan *sgChanMake(size_t itemSize, size_t bufferSize)
    {
        sgChan *ch = (sgChan *)malloc(sizeof(sgChan));
        if (!ch)
            return NULL;

        if (pthread_mutex_init(&ch->lock, NULL) != 0)
        {
            free(ch);
            return NULL;
        }

        if (pthread_cond_init(&ch->cond, NULL) != 0)
        {
            pthread_mutex_destroy(&ch->lock);
            free(ch);
            return NULL;
        }

        ch->queue = sgQueueCreate(itemSize, bufferSize);
        if (!ch->queue)
        {
            pthread_mutex_destroy(&ch->lock);
            pthread_cond_destroy(&ch->cond);
            free(ch);
            return NULL;
        }

        return ch;
    }

    /*
     */
    sgReturnType sgChanIn(sgChan *ch, void *data)
    {
        if (ch == NULL || data == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ch->lock);

        sgReturnType ret = sgQueueQueue(ch->queue, data);
        if (ret == SG_OK)
            pthread_cond_signal(&ch->cond);

        pthread_mutex_unlock(&ch->lock);

        return ret;
    }

    /*
     */
    sgReturnType sgChanOut(sgChan *ch, void *buf)
    {
        if (ch == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ch->lock);

        while (ch->queue->waiting == 0)
            pthread_cond_wait(&ch->cond, &ch->lock);
        sgReturnType ret = sgQueueDequeue(ch->queue, buf);

        pthread_mutex_unlock(&ch->lock);

        return ret;
    }

    /*
     */
    sgReturnType sgChanOutTimed(sgChan *ch, void *buf, long timeout)
    {
        if (ch == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ch->lock);

        if (ch->queue->waiting == 0)
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);

            ts.tv_sec += timeout / SG_TIME_S;
            ts.tv_nsec += timeout % SG_TIME_S;
            if (ts.tv_nsec >= SG_TIME_S)
            {
                ts.tv_sec += 1;
                ts.tv_nsec -= SG_TIME_S;
            }

            while (ch->queue->waiting == 0)
            {
                int ret = pthread_cond_timedwait(&ch->cond, &ch->lock, &ts);
                if (ret == ETIMEDOUT)
                {
                    pthread_mutex_unlock(&ch->lock);
                    return SG_TIMEOUT;
                }
            }
        }
        sgReturnType ret = sgQueueDequeue(ch->queue, buf);

        pthread_mutex_unlock(&ch->lock);

        return ret;
    }

    /*
     */
    void sgChanDestroy(sgChan *ch)
    {
        if (ch == NULL)
            return;

        sgQueueDestroy(ch->queue);
        pthread_mutex_destroy(&ch->lock);
        pthread_cond_destroy(&ch->cond);
        free(ch);
    }

#ifdef __cplusplus
}
#endif

#endif