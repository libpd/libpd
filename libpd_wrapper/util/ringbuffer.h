/*
 *  Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 *  For information on usage and redistribution, and for a DISCLAIMER OF ALL
 *  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#ifndef __OPENSL_RING_BUFFER_H__
#define __OPENSL_RING_BUFFER_H__

// Simple lock-free ring buffer implementation for one writer thread and one
// consumer thread.
typedef struct ring_buffer {
    int size;
    char *buf_ptr;
    int write_idx;
    int write_readable_idx;
    int read_idx;
    int is_in_write_group;
} ring_buffer;

// Creates a ring buffer (returns NULL on failure).
ring_buffer *rb_create(int size);

// Deletes a ring buffer.
void rb_free(ring_buffer *buffer);

// Returns the number of bytes that can currently be written; safe to be called
// from any thread.
int rb_available_to_write(ring_buffer *buffer);

// Returns the number of bytes that can currently be read; safe to be called
// from any thread.
int rb_available_to_read(ring_buffer *buffer);

// Writes the given number of bytes from src to the ring buffer if the ring
// buffer has enough space. Only to be called from a single writer thread.
// Returns 0 on success.
int rb_write_to_buffer(ring_buffer *buffer, const char *src, int len);

// Reads the given number of bytes fromthe ring buffer to dest if the ring
// buffer has enough data. Only to be called from a single reader thread.
// Returns 0 on success.
int rb_read_from_buffer(ring_buffer *buffer, char *dest, int len);

// Wrap these functions around a series of rb_write_to_buffer operations,
// to prevent reading between the writes.
int rb_begin_write_group(ring_buffer *buffer);
int rb_end_write_group(ring_buffer *buffer);

#endif
