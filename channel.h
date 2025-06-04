#ifndef __SEGO_CHANNEL_H
#define __SEGO_CHANNEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "enums.h"
#include "queue.h"

    typedef struct
    {
        pthread_mutex_t lock;
        pthread_cond_t cond;
        sgQueue *queue;
        int fd[2];
    } sgChan;

    /*
     * @brief   Makes new channel.
     * @param   itemSize the item of the transported data in the channel
     * @param   bufferSize the channel's buffer size
     * @return  The pointer to the channel (`sgChan`) instance.
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

        if (pipe(ch->fd) == -1)
        {
            pthread_mutex_destroy(&ch->lock);
            pthread_cond_destroy(&ch->cond);
            sgQueueDestroy(ch->queue);
            free(ch);
            return NULL;
        }

        return ch;
    }

    /*
     * @brief   Pushes a value to the pipe
     * @param   ch the channel instance
     * @return  None.
     */
    void __sgChanPipePush(sgChan *ch)
    {
        static uint8_t val = 0xFF;
        write(ch->fd[1], &val, 1);
    }

    /*
     * @brief   Pops a value to the pipe
     * @param   ch the channel instance
     * @return  None.
     */
    void __sgChanPipePop(sgChan *ch)
    {
        static uint8_t tmp;
        read(ch->fd[0], &tmp, 1);
    }

    /*
     * @brief   Sends data to the channel.
     * @param   ch the channel instance
     * @param   data the data
     * @return  `SG_ERR_NULLPTR` if one argument is `NULL`. `SG_ERR_ALLOC` if memory allocation is somehow failed. `SG_OK` if ok.
     */
    sgReturnType sgChanIn(sgChan *ch, void *data)
    {
        if (ch == NULL || data == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ch->lock);

        sgReturnType ret = sgQueueQueue(ch->queue, data);
        if (ret == SG_OK)
        {
            pthread_cond_signal(&ch->cond);
            __sgChanPipePush(ch);
        }
        else if (ret == SG_QUEUE_FULL)
            pthread_cond_signal(&ch->cond);

        pthread_mutex_unlock(&ch->lock);

        return ret;
    }

    /*
     * @brief   Retrieves data from the channel.
     * @param   ch the channel instance
     * @param   buf the buffer to hold the data
     * @return  `SG_ERR_NULLPTR` if one argument is `NULL`. `SG_OK` if ok.
     * @note    This function is blocking.
     */
    sgReturnType sgChanOut(sgChan *ch, void *buf)
    {
        if (ch == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        pthread_mutex_lock(&ch->lock);

        while (ch->queue->waiting == 0)
            pthread_cond_wait(&ch->cond, &ch->lock);

        sgReturnType ret = sgQueueDequeue(ch->queue, buf);
        if (ret == SG_OK)
            __sgChanPipePop(ch);

        pthread_mutex_unlock(&ch->lock);

        return ret;
    }

    /*
     * @brief   Retrieves data from the channel wit timeout.
     * @param   ch the channel instance
     * @param   buf the buffer to hold the data
     * @param   timeout the duration until timeout
     * @return  `SG_ERR_NULLPTR` if one argument is `NULL`. `SG_TIMEOUT` if timeout. `SG_OK` if ok.
     * @note    This function is blocking up until timeout. Multiply the timeout with the desired time unit, e.g. 500L * `SG_TIME_MS` for 500ms timeout duration.
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
        if (ret == SG_OK)
            __sgChanPipePop(ch);

        pthread_mutex_unlock(&ch->lock);

        return ret;
    }

    /*
     * @brief   Destroys the channel instance.
     * @param   ch the channel instance
     * @return  None.
     */
    void sgChanDestroy(sgChan *ch)
    {
        if (ch == NULL)
            return;

        sgQueueDestroy(ch->queue);
        pthread_mutex_destroy(&ch->lock);
        pthread_cond_destroy(&ch->cond);
        close(ch->fd[0]);
        close(ch->fd[1]);
        free(ch);
    }

#ifdef __cplusplus
}
#endif

#endif