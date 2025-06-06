#ifndef __SEGO_SELECT_H
#define __SEGO_SELECT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdarg.h>
#include <poll.h>
#include "context.h"
#include "channel.h"

    typedef uint64_t sgSel;

    /*
     * @brief   Select (listens) to several channels. This waits until a channel is receiving a data.
     * @param   n number of channels to be listened
     * @param   ... channels
     * @return  The successfully selected channel.
     * @note    This function is blocking.
     */
    sgSel sgSelect(int n, ...)
    {
        sgChan *ch[n];
        struct pollfd pfds[n];

        va_list args;
        va_start(args, n);

        for (int i = 0; i < n; ++i)
        {
            ch[i] = va_arg(args, sgChan *);
            pfds[i].fd = ch[i]->fd[0];
            pfds[i].events = POLLIN;
        }

        va_end(args);

        int ret = poll(pfds, n, -1);
        if (ret == -1)
            return (sgSel)NULL;
        else
        {
            for (int i = 0; i < n; ++i)
            {
                if (pfds[i].revents & POLLIN)
                    return (sgSel)ch[i];
            }
        }
    }

    /*
     * @brief   Selects (listens) to multiple channels. Skips if no incoming data is received among the channels.
     * @param   n number of channels to be listened
     * @param   ... channels
     * @return  The successfully selected channel. Otherwise, `0`.
     * @note    This function is non-blocking.
     * @note    Use `else` statement to implement the default case.
     */
    sgSel sgSelectDefault(int n, ...)
    {
        sgChan *ch[n];
        struct pollfd pfds[n];

        va_list args;
        va_start(args, n);

        for (int i = 0; i < n; ++i)
        {
            ch[i] = va_arg(args, sgChan *);
            pfds[i].fd = ch[i]->fd[0];
            pfds[i].events = POLLIN;
        }

        va_end(args);

        int ret = poll(pfds, n, 0);
        if (ret <= 0)
            return (sgSel)NULL;
        else
        {
            for (int i = 0; i < n; ++i)
            {
                if (pfds[i].revents & POLLIN)
                    return (sgSel)ch[i];
            }
        }
    }

    /*
     * @brief   Selects (listens) to multiple contexes and channels. This waits until a channel receives data or a context is raised.
     * @param   m number of contexts to be listened
     * @param   n number of channels to be listened
     * @param   ... contexes and channels (respectively)
     * @return  The successfully selected context/channel.
     * @note    This function is blocking.
     */
    sgSel sgSelectWithContext(int m, int n, ...)
    {
        sgContext *ctx[m];
        sgChan *ch[n];
        struct pollfd pfds[m + n];

        va_list args;
        va_start(args, n);

        for (int i = 0; i < m; ++i)
        {
            ctx[i] = va_arg(args, sgContext *);
            pfds[i].fd = ctx[i]->fd[0];
            pfds[i].events = POLLIN;
        }

        for (int i = 0; i < n; ++i)
        {
            ch[i] = va_arg(args, sgChan *);
            pfds[m + i].fd = ch[i]->fd[0];
            pfds[m + i].events = POLLIN;
        }

        va_end(args);

        int ret = poll(pfds, m + n, -1);
        if (ret == -1)
            return (sgSel)NULL;
        else
        {
            for (int i = 0; i < m; ++i)
            {
                if (pfds[i].revents & POLLIN)
                    return (sgSel)ctx[i];
            }

            for (int i = 0; i < n; ++i)
            {
                if (pfds[m + i].revents & POLLIN)
                    return (sgSel)ch[i];
            }
        }
    }

    /*
     * @brief   Selects (listens) to multiple contexes and channels. Skips if there is no data received and no context is raised.
     * @param   m number of contexts to be listened
     * @param   n number of channels to be listened
     * @param   ... contexes and channels (respectively)
     * @return  The successfully selected context/channel. Otherwise, `0`.
     * @note    This function is non-blocking.
     * @note    Use `else` statement to implement the default case.
     */
    sgSel sgSelectDefaultWithContext(int m, int n, ...)
    {
        sgContext *ctx[m];
        sgChan *ch[n];
        struct pollfd pfds[m + n];

        va_list args;
        va_start(args, n);

        for (int i = 0; i < m; ++i)
        {
            ctx[i] = va_arg(args, sgContext *);
            pfds[i].fd = ctx[i]->fd[0];
            pfds[i].events = POLLIN;
        }

        for (int i = 0; i < n; ++i)
        {
            ch[i] = va_arg(args, sgChan *);
            pfds[m + i].fd = ch[i]->fd[0];
            pfds[m + i].events = POLLIN;
        }

        va_end(args);

        int ret = poll(pfds, m + n, 0);
        if (ret <= 0)
            return (sgSel)NULL;
        else
        {
            for (int i = 0; i < m; ++i)
            {
                if (pfds[i].revents & POLLIN)
                    return (sgSel)ctx[i];
            }

            for (int i = 0; i < n; ++i)
            {
                if (pfds[m + i].revents & POLLIN)
                    return (sgSel)ch[i];
            }
        }
    }

#ifdef __cplusplus
}
#endif

#endif