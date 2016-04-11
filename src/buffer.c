/*
 * buffer.c - binary buffer
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"


void buffer_init(buffer_t *buf)
{
    assert(buf != NULL);

    buf->data = buf->_initial;
    buf->used = 0;
    buf->allocated = sizeof(buf->_initial);
}


void buffer_del(buffer_t *buf)
{
    assert(buf != NULL);
    assert(buf->data != NULL);
    assert(buf->allocated > 0);

    if (buf->allocated > (int)sizeof(buf->_initial))
    {
        free(buf->data);
    }

    buffer_init(buf);
}


uint8_t *buffer_data(const buffer_t *buf)
{
    assert(buf != NULL);
    assert(buf->data != NULL);
    assert(buf->allocated > 0);
    assert(buf->used <= buf->allocated);

    return buf->data;
}


int buffer_len(const buffer_t *buf)
{
    assert(buf != NULL);
    assert(buf->data != NULL);
    assert(buf->allocated > 0);
    assert(buf->used <= buf->allocated);

    return buf->used;
}


int buffer_append(buffer_t *buf, uint8_t *data, int size)
{
    assert(buf != NULL);
    assert(buf->data != NULL);
    assert(buf->allocated > 0);
    assert(buf->used <= buf->allocated);

    if (buf->used + size > buf->allocated)
    {
        // resize
        uint8_t *p;
        int new_size = buf->allocated + size + 2048;
        if (buf->allocated <= (int)sizeof(buf->_initial))
        {
            p = malloc(new_size);
            if (p == NULL)
            {
                return -1;
            }
            memcpy(p, buf->data, buf->used);
        }
        else
        {
            p = realloc(buf->data, new_size);
            if (p == NULL)
            {
                return -1;
            }
        }
        buf->data = p;
        buf->allocated = new_size;
    }
    memcpy(buf->data + buf->used, data, size);
    buf->used += size;
    return 0;
}
