#ifndef __SEGO_H
#define __SEGO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "enums.h"
#include "queue.h"
#include "list.h"
#include "channel.h"
#include "context.h"
#include "select.h"
#include "moment.h"
#include "handler.h"
#include "map.h"

    __sgHandler *sgh;

    /*
     * @brief   Starts sego handler.
     * @param   none
     * @return  None.
     */
    void sgInit()
    {
        sgh = (__sgHandler *)malloc(sizeof(__sgHandler));
        if (sgh == NULL)
        {
            perror("Failed to allocate memory for Sego handler.");
            exit(EXIT_FAILURE);
        }

        sgh->startCh = sgChanMake(sizeof(__sgRoutineWrapperArgs), 4);
        if (sgh->startCh == NULL)
        {
            perror("Failed to allocate memory for Sego handler's start channel.");
            free(sgh);
            exit(EXIT_FAILURE);
        }

        sgh->stopCh = sgChanMake(sizeof(pthread_t), 4);
        if (sgh->stopCh == NULL)
        {
            perror("Failed to allocate memory for Sego handler's stop channel.");
            sgChanDestroy(sgh->startCh);
            free(sgh);
            exit(EXIT_FAILURE);
        }

        sgh->closeCtx = sgContextCreate();
        if (sgh->closeCtx == NULL)
        {
            perror("Failed to allocate memory for Sego handler's close context.");
            sgChanDestroy(sgh->startCh);
            sgChanDestroy(sgh->stopCh);
            free(sgh);
            exit(EXIT_FAILURE);
        }

        sgh->table = NULL;

        if (pthread_create(&sgh->sgHandlerThread, NULL, __sgHandlerRoutine, NULL) != 0)
        {
            sgChanDestroy(sgh->startCh);
            sgChanDestroy(sgh->stopCh);
            sgContextDestroy(sgh->closeCtx);
            free(sgh);
            perror("Failed to start Sego handler routine.");
            exit(EXIT_FAILURE);
        }
    }

    /*
     * @brief   Stops sego handler.
     * @param   none
     * @return  None.
     */
    void sgClose()
    {
        uint8_t term = 0xFF;
        sgContextRaise(sgh->closeCtx);
        pthread_join(sgh->sgHandlerThread, NULL);
        free(sgh);
    }

    /*
     * @brief   Starts a sego routine.
     * @param   fn the routine function
     * @param   arg the argument to be passed to the routine
     * @return  None.
     */
    void sego(sgRoutine fn, void *arg)
    {
        if (fn == NULL)
            return;

        __sgRoutineWrapperArgs args;
        args.fn = fn;
        args.arg = arg;
        sgChanIn(sgh->startCh, &args);
    }

#ifdef __cplusplus
}
#endif

#endif