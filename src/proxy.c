/*
 * proxy.c - proxy worker
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

#define _GNU_SOURCE
#include <errno.h>
#include <libmill.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buffer.h"
#include "log.h"
#include "tcprelay.h"


static int tcpsockfd(tcpsock sock)
{
    return ((int *)sock)[1];
}


coroutine static void proxy(tcpsock sock, buffer_t *buf, const char *host, int port)
{
    int64_t deadline = now() + 10000;

    LOG("connect %s:%d", host, port);
    ipaddr addr = ipremote(host, port, IPADDR_PREF_IPV4, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    tcpsock conn = tcpconnect(addr, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    tcpsend(conn, buffer_data(buf), buffer_len(buf), -1);
    tcpflush(conn, -1);
    buffer_del(buf);

    tcprelay(sock, conn);
    return;

  cleanup:
    tcpclose(sock);
    buffer_del(buf);
}


static const char *get_http_host(buffer_t *buf)
{
    static char host[256];

    const uint8_t *data = buffer_data(buf);
    int len = buffer_len(buf);
    host[0] = '\0';

    char *pos = (char *)memmem(data, len, "\r\nHost:", strlen("\r\nHost:"));
    if (pos != NULL)
    {
        pos += strlen("\r\nHost:");
        while (*pos == ' ')
        {
            pos++;
        }
        len -= (int)(ptrdiff_t)(pos - (char *)data);
        char *end_pos = (char *)memmem(pos, len, "\r\n", strlen("\r\n"));
        if (end_pos != NULL)
        {
            len = (int)(ptrdiff_t)(end_pos - pos);
            if (len <= 0)
            {
                LOG("invalid Host in request");
                return NULL;
            }
            if (len + 1 > (int)sizeof(host))
            {
                return NULL;
            }
            memcpy(host, pos, len);
            host[len] = '\0';
            return host;
        }
        else
        {
            return host;
        }
    }
    pos = (char *)memmem(buffer_data(buf), len, "\r\n\r\n", strlen("\r\n\r\n"));
    if (pos != NULL)
    {
        LOG("no Host in request");
        return NULL;
    }
    return host;
}


coroutine void http_worker(tcpsock sock)
{
    int64_t deadline = now() + 10000;

    uint8_t line[4096];
    buffer_t buffer;
    buffer_init(&buffer);

    int fd = tcpsockfd(sock);
    while (1)
    {
        int events = fdwait(fd, FDW_IN, deadline);
        int n;
        if ((events & (FDW_IN | FDW_ERR)) == 0)
        {
            goto cleanup;
        }
        else
        {
            n = read(fd, line, sizeof(line));
            if (n <= 0)
            {
                goto cleanup;
            }
        }
        int r = buffer_append(&buffer, line, n);
        if (r < 0)
        {
            LOG("no enough memory");
            goto cleanup;
        }
        const char *host = get_http_host(&buffer);
        if (host == NULL)
        {
            goto cleanup;
        }
        else if (*host != '\0')
        {
            go(proxy(sock, &buffer, host, 80));
            return;
        }
    }

  cleanup:
    tcpclose(sock);
    buffer_del(&buffer);
}


static const char *get_https_host(buffer_t *buf)
{
    /* 1   TLS_HANDSHAKE_CONTENT_TYPE
     * 1   TLS major version
     * 1   TLS minor version
     * 2   TLS Record length
     * --------------
     * 1   Handshake type
     * 3   Length
     * 2   Version
     * 32  Random
     * 1   Session ID length
     * ?   Session ID
     * 2   Cipher Suites length
     * ?   Cipher Suites
     * 1   Compression Methods length
     * ?   Compression Methods
     * 2   Extensions length
     * ---------------
     * 2   Extension data length
     * 2   Extension type (0x0000 for server_name)
     * ---------------
     * 2   server_name list length
     * 1   server_name type (0)
     * 2   server_name length
     * ?   server_name
     */
    const int TLS_HEADER_LEN = 5;
    const int FIXED_LENGTH_RECORDS = 38;
    const int TLS_HANDSHAKE_CONTENT_TYPE = 0x16;
    const int TLS_HANDSHAKE_TYPE_CLIENT_HELLO = 0x01;

    static char host[256];

    const uint8_t *data = buffer_data(buf);
    int length = buffer_len(buf);
    host[0] = '\0';

    int pos = 0;
    if (length < TLS_HEADER_LEN + FIXED_LENGTH_RECORDS)
    {
        // not enough data
        return host;
    }

    if ((data[0] & 0x80) && (data[2] == 1))
    {
        // SSL 2.0, does not support SNI
        return NULL;
    }
    if (data[0] != TLS_HANDSHAKE_CONTENT_TYPE)
    {
        return NULL;
    }
    if (data[1] < 3)
    {
        // TLS major version < 3, does not support SNI
        return NULL;
    }
    int record_len = (data[3] << 8) + data[4] + TLS_HEADER_LEN;
    if (length < record_len)
    {
        // not enough data
        return host;
    }
    if (data[TLS_HEADER_LEN] != TLS_HANDSHAKE_TYPE_CLIENT_HELLO)
    {
        // invalid handshake type
        return NULL;
    }
    pos += TLS_HEADER_LEN + FIXED_LENGTH_RECORDS;

    // skip session ID
    if (pos + 1 > length || pos + 1 + data[pos] > length)
    {
        // not enough data
        return host;
    }
    pos += 1 + data[pos];
    // skip cipher suites
    if (pos + 2 > length || pos + 2 + (data[pos] << 8) + data[pos+1] > length)
    {
        // not enough data
        return host;
    }
    pos += 2 + (data[pos] << 8) + data[pos+1];
    // skip compression methods
    if (pos + 1 > length || pos + 1 + data[pos] > length)
    {
        // not enough data
        return host;
    }
    pos += 1 + data[pos];
    // skip extension length
    if (pos + 2 > length)
    {
        return host;
    }
    pos += 2;

    // parse extension data
    while (1)
    {
        if (pos + 4 > record_len)
        {
            // buffer more than one record, SNI still not found
            return NULL;
        }
        if (pos + 4 > length)
        {
            return host;
        }
        int ext_data_len = (data[pos+2] << 8) + data[pos+3];
        if (data[pos] == 0 && data[pos+1] == 0)
        {
            // server_name extension type
            pos += 4;
            if (pos + 5 > length)
            {
                // server_name list header
                return host;
            }
            int server_name_len = (data[pos+3] << 8) + data[pos+4];
            if (pos + 5 + server_name_len > length)
            {
                return host;
            }
            // return server_name
            if (server_name_len + 1 > (int)sizeof(host))
            {
                return NULL;
            }
            memcpy(host, data + pos + 5, server_name_len);
            host[server_name_len] = '\0';
            return host;
        }
        else
        {
            // skip
            pos += 4 + ext_data_len;
        }
    }
}


coroutine void https_worker(tcpsock sock)
{
    int64_t deadline = now() + 10000;

    uint8_t line[4096];
    buffer_t buffer;
    buffer_init(&buffer);

    int fd = tcpsockfd(sock);
    while (1)
    {
        int events = fdwait(fd, FDW_IN, deadline);
        int n;
        if ((events & (FDW_IN | FDW_ERR)) == 0)
        {
            goto cleanup;
        }
        else
        {
            n = read(fd, line, sizeof(line));
            if (n <= 0)
            {
                goto cleanup;
            }
        }
        int r = buffer_append(&buffer, line, n);
        if (r < 0)
        {
            LOG("no enough memory");
            goto cleanup;
        }
        const char *host = get_https_host(&buffer);
        if (host == NULL)
        {
            goto cleanup;
        }
        else if (*host != '\0')
        {
            go(proxy(sock, &buffer, host, 443));
            return;
        }
    }

  cleanup:
    tcpclose(sock);
    buffer_del(&buffer);
}
