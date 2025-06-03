#ifndef __SEGO_H
#define __SEGO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <pthread.h>
#include "enums.h"
#include "queue.h"
#include "channel.h"
#include "context.h"
#include "moment.h"
#include "handler.h"

    sgHandler *sgh;

    /*
     */
    void sgInit()
    {
        sgh = (sgHandler *)malloc(sizeof(sgHandler));
        if (sgh == NULL)
        {
            perror("Failed to allocate memory for Sego handler.");
            exit(EXIT_FAILURE);
        }

        sgh->startCh = sgChanMake(sizeof(__sgRoutineWrapperArgs), 8);
        if (sgh->startCh == NULL)
        {
            perror("Failed to allocate memory for Sego handler start channel.");
            exit(EXIT_FAILURE);
        }

        sgh->stopCh = sgChanMake(sizeof(pthread_t), 8);
        if (sgh->stopCh == NULL)
        {
            perror("Failed to allocate memory for Sego handler stop channel.");
            exit(EXIT_FAILURE);
        }

        sgh->closeCh = sgChanMake(sizeof(uint8_t), 1);
        if (sgh->closeCh == NULL)
        {
            perror("Failed to allocate memory for Sego handler close channel.");
            exit(EXIT_FAILURE);
        }

        sgh->table = NULL;

        if (pthread_create(&sgh->sgHandlerThread, NULL, sgHandlerRoutine, NULL) != 0)
        {
            perror("Failed to start Sego handler routine.");
            exit(EXIT_FAILURE);
        }
    }

    /*
     */
    void sgClose()
    {
        uint8_t term = 0xFF;
        sgChanIn(sgh->closeCh, &term);
        pthread_join(sgh->sgHandlerThread, NULL);
        free(sgh);
    }

    /*
     */
    void sego(sgRoutine fn, void *arg)
    {
        __sgRoutineWrapperArgs args;
        args.fn = fn;
        args.arg = arg;
        sgChanIn(sgh->startCh, &args);
    }

#ifdef __cplusplus
}
#endif

#endif