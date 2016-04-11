/*
 * socks5.c - SOCKS5 client
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

#include <arpa/inet.h>
#include <errno.h>
#include <libmill.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "socks5.h"


tcpsock socks5_connect(const char *socks5_host, int socks5_port, const char *host, int port)
{
    int64_t deadline = now() + 10000;
    uint8_t buf[272];

    ipaddr addr = ipremote(socks5_host, socks5_port, IPADDR_PREF_IPV4, deadline);
    if (errno != 0)
    {
        return NULL;
    }
    tcpsock sock = tcpconnect(addr, deadline);
    if (sock == 0)
    {
        return NULL;
    }

    // SOCKS5 CLIENT HELLO
    // +-----+----------+----------+
    // | VER | NMETHODS | METHODS  |
    // +-----+----------+----------+
    // |  1  |    1     | 1 to 255 |
    // +-----+----------+----------+
    buf[0] = 0x05;
    buf[1] = 0x01;
    buf[2] = 0x00;
    tcpsend(sock, buf, 3, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    tcpflush(sock, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }

    // SOCKS5 SERVER HELLO
    // +-----+--------+
    // | VER | METHOD |
    // +-----+--------+
    // |  1  |   1    |
    // +-----+--------+
    uint8_t ver;
    tcprecv(sock, &ver, 1, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    // version must be 5
    if (ver != 5)
    {
        goto cleanup;
    }
    uint8_t method;
    tcprecv(sock, &method, 1, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    // method must be 0
    if (method != 0)
    {
        goto cleanup;
    }

    // SOCKS5 CLIENT REQUEST
    // +-----+-----+-------+------+----------+----------+
    // | VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
    // +-----+-----+-------+------+----------+----------+
    // |  1  |  1  | X'00' |  1   | Variable |    2     |
    // +-----+-----+-------+------+----------+----------+
    buf[0] = 0x05;  // VER 5
    buf[1] = 0x01;  // CONNECT
    buf[2] = 0x00;
    buf[3] = 0x03;  // Domain name
    buf[4] = (uint8_t)strlen(host);
    memcpy(buf + 5, host, buf[4]);
    *(uint16_t *)(buf + 5 + buf[4]) = htons(port);
    tcpsend(sock, buf, 5 + buf[4] + 2, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    tcpflush(sock, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }

    // SOCKS5 SERVER REPLY
    // +-----+-----+-------+------+----------+----------+
    // | VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
    // +-----+-----+-------+------+----------+----------+
    // |  1  |  1  | X'00' |  1   | Variable |    2     |
    // +-----+-----+-------+------+----------+----------+
    tcprecv(sock, &ver, 1, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    // version must be 5
    if (ver != 5)
    {
        goto cleanup;
    }

    uint8_t rep;
    tcprecv(sock, &rep, 1, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    // success
    if (rep != 0x00)
    {
        goto cleanup;
    }

    uint8_t rsv;
    tcprecv(sock, &rsv, 1, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }

    uint8_t atyp;
    tcprecv(sock, &atyp, 1, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }
    if (atyp == 0x01)
    {
        // IPv4 address
        uint8_t addr[4];
        tcprecv(sock, addr, 4, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
    }
    else if (atyp == 0x03)
    {
        // Domain name
        uint8_t len;
        tcprecv(sock, &len, 1, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        uint8_t addr[256];
        tcprecv(sock, addr, len, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
    }
    else if (atyp == 0x04)
    {
        // IPv6 address
        uint8_t addr[16];
        tcprecv(sock, addr, 16, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
    }
    else
    {
        // unsupported address type
        goto cleanup;
    }

    tcprecv(sock, &port, 2, deadline);
    if (errno != 0)
    {
        goto cleanup;
    }

    return sock;

  cleanup:
    tcpclose(sock);
    return NULL;
}
