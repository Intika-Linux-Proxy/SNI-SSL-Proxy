/*
 * tcprelay.c - tcp relay
 *
 * Copyright (C) 2014 - 2016, Xiaoxiao <i@pxx.io>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <libmill.h>
#include <sys/socket.h>
#include <unistd.h>


static int tcpsockfd(tcpsock sock)
{
    return ((int *)sock)[1];
}


static size_t tcprecvpkt(tcpsock sock, char *buf, size_t len, int64_t deadline)
{
    int fd = tcpsockfd(sock);

    int n = read(fd, buf, len);
    if (n < 0)
    {
        if (errno != EAGAIN)
        {
            return 0;
        }
    }
    else if (n == 0)
    {
        errno = ECONNRESET;
        return 0;
    }
    else
    {
        errno = 0;
        return n;
    }

    int events = fdwait(fd, FDW_IN, deadline);
    if ((events & (FDW_IN | FDW_ERR)) != 0)
    {
        errno = 0;
        int n = read(fd, buf, len);
        if (n < 0)
        {
            return 0;
        }
        else if (n == 0)
        {
            errno = ECONNRESET;
            return 0;
        }
        else
        {
            errno = 0;
            return n;
        }
    }

    errno = ETIMEDOUT;
    return 0;
}


static size_t tcpsendpkt(tcpsock sock, char *buf, size_t len, int64_t deadline)
{
    int fd = tcpsockfd(sock);

    int n = write(fd, buf, len);
    if (n < 0)
    {
        if (errno != EAGAIN)
        {
            return -1;
        }
    }
    else if (n == 0)
    {
        errno = ECONNRESET;
        return 0;
    }
    else
    {
        errno = 0;
        return n;
    }

    int events = fdwait(fd, FDW_OUT, deadline);
    if ((events & (FDW_OUT | FDW_ERR)) != 0)
    {
        int n = write(fd, buf, len);
        if (n < 0)
        {
            return 0;
        }
        else if (n == 0)
        {
            errno = ECONNRESET;
            return 0;
        }
        else
        {
            errno = 0;
            return n;
        }
    }

    errno = ETIMEDOUT;
    return 0;
}


coroutine void tcppipe(tcpsock from, tcpsock to, chan ch)
{
    char buf[4096];

    while (1)
    {
        size_t nbytes = tcprecvpkt(from, buf, sizeof(buf), -1);
        if (errno != 0)
        {
            chs(ch, int, 1);
            return;
        }

        size_t nsent = 0;
        while (nsent < nbytes)
        {
            size_t n = tcpsendpkt(to, buf + nsent, nbytes - nsent, -1);
            if (errno != 0) {
                chs(ch, int, 1);
                return;
            }
            nsent += n;
        }
    }
}


void tcprelay(tcpsock sock1, tcpsock sock2)
{
    chan ch = chmake(int, 0);
    go(tcppipe(sock1, sock2, ch));
    go(tcppipe(sock2, sock1, ch));
    (void)chr(ch, int);
    shutdown(tcpsockfd(sock1), SHUT_RDWR);
    shutdown(tcpsockfd(sock2), SHUT_RDWR);
    (void)chr(ch, int);
    tcpclose(sock1);
    tcpclose(sock2);
}
