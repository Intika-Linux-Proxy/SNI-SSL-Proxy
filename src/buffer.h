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

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

typedef struct
{
    uint8_t *data;
    int used;
    int allocated;
    uint8_t _initial[4096];
} buffer_t;

extern void buffer_init(buffer_t *buf);
extern void buffer_del(buffer_t *buf);
extern void buffer_empty(buffer_t *buf);
extern uint8_t *buffer_data(const buffer_t *buf);
extern int buffer_len(const buffer_t *buf);
extern int buffer_append(buffer_t *buf, uint8_t *data, int size);



#endif
