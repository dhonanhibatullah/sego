#ifndef __SEGO_QUEUE_H
#define __SEGO_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "enums.h"

    typedef struct __sgQueueItem
    {
        void *ptr;
        struct __sgQueueItem *next;
    } sgQueueItem;

    typedef struct
    {
        uint32_t waiting;
        size_t itemSize;
        size_t bufferSize;
        sgQueueItem *head;
        sgQueueItem *tail;
    } sgQueue;

    /*
     * @brief   Creates a queue with fixed item size.
     * @param   itemSize the size for each item
     * @param   bufferSize the size of the queue
     * @return  the pointer to the queue (`sgQueue`) instance
     * @note    If the queue is full when a new item is being queued, the earliest one gets dequeued.
     */
    sgQueue *sgQueueCreate(size_t itemSize, size_t bufferSize)
    {
        if (itemSize == 0 || bufferSize == 0)
            return NULL;

        sgQueue *q = (sgQueue *)malloc(sizeof(sgQueue));
        if (!q)
            return NULL;

        q->waiting = 0;
        q->itemSize = itemSize;
        q->bufferSize = bufferSize;
        q->head = NULL;
        q->tail = NULL;
        return q;
    }

    /*
     * @brief   Queues new item.
     * @param   q the queue instance
     * @param   data the new item
     * @return  `SG_ERR_NULLPTR` if one argument is a `NULL`. `SG_ERR_ALLOC` if a new memory allocation is somehow failed. `SG_OK` if ok.
     */
    sgReturnType sgQueueQueue(sgQueue *q, void *data)
    {
        if (q == NULL || data == NULL)
            return SG_ERR_NULLPTR;

        if (q->waiting == q->bufferSize)
        {
            sgQueueItem *prevHead = q->head;
            q->head = q->head->next;

            if (q->head == NULL)
                q->tail = NULL;

            free(prevHead->ptr);
            free(prevHead);
            q->waiting -= 1;
        }

        sgQueueItem *newItem = (sgQueueItem *)malloc(sizeof(sgQueueItem));
        if (!newItem)
            return SG_ERR_ALLOC;

        newItem->ptr = malloc(q->itemSize);
        if (!newItem->ptr)
        {
            free(newItem);
            return SG_ERR_ALLOC;
        }
        memcpy(newItem->ptr, data, q->itemSize);
        newItem->next = NULL;

        if (q->waiting == 0)
        {
            q->head = newItem;
            q->tail = newItem;
        }
        else
        {
            q->tail->next = newItem;
            q->tail = newItem;
        }

        q->waiting += 1;
        return SG_OK;
    }

    /*
     * @brief   Dequeues the first item.
     * @param   q the queue instance
     * @param   buf buffer to hold the item
     * @return  `SG_ERR_NULLPTR` if one argument is a `NULL`. `SG_NOTHING` if queue is empty. `SG_OK` if ok.
     */
    sgReturnType sgQueueDequeue(sgQueue *q, void *buf)
    {
        if (q == NULL || buf == NULL)
            return SG_ERR_NULLPTR;

        if (q->waiting == 0)
            return SG_NOTHING;

        sgQueueItem *prevHead = q->head;
        memcpy(buf, q->head->ptr, q->itemSize);
        q->head = q->head->next;

        if (q->head == NULL)
            q->tail = NULL;

        free(prevHead->ptr);
        free(prevHead);
        q->waiting -= 1;
        return SG_OK;
    }

    /*
     * @brief   Destroys the queue instance.
     * @param   q the queue instance
     * @return  none
     */
    void sgQueueDestroy(sgQueue *q)
    {
        if (q == NULL)
            return;

        while (q->head != NULL)
        {
            sgQueueItem *prevHead = q->head;
            q->head = q->head->next;

            if (q->head == NULL)
                q->tail = NULL;

            free(prevHead->ptr);
            free(prevHead);
        }

        free(q);
    }

#ifdef __cplusplus
}
#endif

#endif