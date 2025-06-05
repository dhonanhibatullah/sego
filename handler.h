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
#include "context.h"
#include "select.h"

#include <stdio.h>

    typedef void *(*sgRoutine)(void *);

    typedef struct
    {
        pthread_t id;
        uint8_t sts;
        UT_hash_handle hh;
    } __sgRoutineHash;

    typedef struct
    {
        sgChan *startCh;
        sgChan *stopCh;
        sgContext *closeCtx;
        __sgRoutineHash *table;
        pthread_t sgHandlerThread;
    } __sgHandler;
    extern __sgHandler *sgh;

    /*
     * @brief   Adds new routine to the table.
     * @param   id the thread ID
     * @return  None.
     */
    void __sgHandlerAddRoutine(pthread_t id)
    {
        __sgRoutineHash *r = (__sgRoutineHash *)malloc(sizeof(__sgRoutineHash));
        r->id = id;
        r->sts = 0x00;
        HASH_ADD_INT(sgh->table, id, r);
    }

    /*
     * @brief   Finds routine from the table.
     * @param   id the thread ID
     * @return  The hash.
     */
    __sgRoutineHash *__sgHandlerFindRoutine(pthread_t id)
    {
        __sgRoutineHash *r;
        HASH_FIND_INT(sgh->table, &id, r);
        return r;
    }

    /*
     * @brief   Removes routine from the table.
     * @param   id the thread ID
     * @return  None.
     */
    void __sgHandlerRemoveRoutine(pthread_t id)
    {
        __sgRoutineHash *r = __sgHandlerFindRoutine(id);
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
    void __sgHandlerTerminateRoutines()
    {
        __sgRoutineHash *r, *tmp;
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
        __sgRoutineWrapperArgs *args = (__sgRoutineWrapperArgs *)a;

        args->fn(args->arg);
        sgChanIn(sgh->stopCh, &args->id);

        free(args);
        return NULL;
    }

    /*
     * @brief   Handles all the sego routines. This runs in the background after `sgInit()`.
     * @param   arg the routine argument
     * @return  A void pointer.
     */
    void *__sgHandlerRoutine(void *arg)
    {
        while (1)
        {
            sgSel sel = sgSelectWithContext(1, 2, sgh->closeCtx, sgh->startCh, sgh->stopCh);

            if (sel == (sgSel)sgh->closeCtx)
            {
                __sgHandlerTerminateRoutines();
                sgChanDestroy(sgh->startCh);
                sgChanDestroy(sgh->stopCh);
                sgContextDestroy(sgh->closeCtx);
                break;
            }
            else if (sel == (sgSel)sgh->startCh)
            {
                __sgRoutineWrapperArgs *buf = (__sgRoutineWrapperArgs *)malloc(sizeof(__sgRoutineWrapperArgs));
                sgChanOut(sgh->startCh, buf);
                pthread_create(&buf->id, NULL, __sgRoutineWrapper, (void *)buf);
                __sgHandlerAddRoutine(buf->id);
            }
            else if (sel == (sgSel)sgh->stopCh)
            {
                pthread_t buf;
                sgChanOut(sgh->stopCh, &buf);
                pthread_join(buf, NULL);
                __sgHandlerRemoveRoutine(buf);
            }
        }
    }

#ifdef __cplusplus
}
#endif

#endif