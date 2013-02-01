/*
 *  Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 *  For information on usage and redistribution, and for a DISCLAIMER OF ALL
 *  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#ifndef __OPENSL_RING_BUFFER_H__
#define __OPENSL_RING_BUFFER_H__

typedef struct ring_buffer {
    int size;
    char *buf_ptr;
    volatile int write_idx;
    volatile int read_idx;
} ring_buffer;

ring_buffer *rb_create(int size);
void rb_free(ring_buffer *buffer);
int rb_available_to_write(ring_buffer *buffer);
int rb_available_to_read(ring_buffer *buffer);
int rb_write_to_buffer(ring_buffer *buffer, const char *src, int len);
int rb_read_from_buffer(ring_buffer *buffer, char *dest, int len);

#endif
