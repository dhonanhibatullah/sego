#ifndef __SEGO_HANDLER_H
#define __SEGO_HANDLER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include "uthash.h"
#include "enums.h"
#include "channel.h"

#include <stdio.h>

    typedef void *(*sgRoutine)(void *);

    typedef struct
    {
        pthread_t id;
        uint8_t sts;
        UT_hash_handle hh;
    } sgRoutineHash;

    typedef struct
    {
        sgChan *startCh;
        sgChan *stopCh;
        sgChan *closeCh;
        sgRoutineHash *table;
        pthread_t sgHandlerThread;
    } sgHandler;
    extern sgHandler *sgh;

    /*
     * @brief   Adds new routine to the table.
     * @param   id the thread ID
     * @return  None.
     */
    void sgHandlerAddRoutine(pthread_t id)
    {
        sgRoutineHash *r = (sgRoutineHash *)malloc(sizeof(sgRoutineHash));
        r->id = id;
        r->sts = 0x00;
        HASH_ADD_INT(sgh->table, id, r);
    }

    /*
     * @brief   Finds routine from the table.
     * @param   id the thread ID
     * @return  The hash.
     */
    sgRoutineHash *sgHandlerFindRoutine(pthread_t id)
    {
        sgRoutineHash *r;
        HASH_FIND_INT(sgh->table, &id, r);
        return r;
    }

    /*
     * @brief   Removes routine from the table.
     * @param   id the thread ID
     * @return  None.
     */
    void sgHandlerRemoveRoutine(pthread_t id)
    {
        sgRoutineHash *r = sgHandlerFindRoutine(id);
        if (r)
        {
            HASH_DEL(sgh->table, r);
            free(r);
        }
    }

    /*
     * @brief   Terminates all routines inside the table.
     * @param   none
     * @return  None.
     */
    void sgHandlerTerminateRoutines()
    {
        sgRoutineHash *r, *tmp;
        HASH_ITER(hh, sgh->table, r, tmp)
        {
            pthread_cancel(r->id);
            HASH_DEL(sgh->table, r);
            free(r);
        }
    }

    typedef struct
    {
        pthread_t id;
        sgRoutine fn;
        void *arg;
    } __sgRoutineWrapperArgs;

    /*
     * @brief   Wraps the routine function to fit the sego handler scheme.
     * @param   a the routine argument
     * @return  A void pointer.
     */
    void *__sgRoutineWrapper(void *a)
    {
        pthread_t id = ((__sgRoutineWrapperArgs *)a)->id;
        sgRoutine fn = ((__sgRoutineWrapperArgs *)a)->fn;
        void *arg = ((__sgRoutineWrapperArgs *)a)->arg;

        fn(arg);
        sgChanIn(sgh->stopCh, &id);

        return NULL;
    }

    /*
     * @brief   Handles all the sego routines. This runs in the background after `sgInit()`.
     * @param   arg the routine argument
     * @return  A void pointer.
     */
    void *sgHandlerRoutine(void *arg)
    {
        __sgRoutineWrapperArgs startChBuf;
        pthread_t stopChBuf;
        uint8_t closeChBuf;

        while (1)
        {
            if (sgChanOutTimed(sgh->startCh, &startChBuf, 25L * SG_TIME_MS) == SG_OK)
            {
                pthread_create(&startChBuf.id, NULL, __sgRoutineWrapper, (void *)&startChBuf);
                sgHandlerAddRoutine(startChBuf.id);
            }
            if (sgChanOutTimed(sgh->stopCh, &stopChBuf, 25L * SG_TIME_MS) == SG_OK)
            {
                pthread_join(stopChBuf, NULL);
                sgHandlerRemoveRoutine(stopChBuf);
            }
            if (sgChanOutTimed(sgh->closeCh, &closeChBuf, 25L * SG_TIME_MS) == SG_OK)
            {
                sgHandlerTerminateRoutines();
                sgChanDestroy(sgh->startCh);
                sgChanDestroy(sgh->stopCh);
                sgChanDestroy(sgh->closeCh);
                break;
            }
        }
    }

#ifdef __cplusplus
}
#endif

#endif