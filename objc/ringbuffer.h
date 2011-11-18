#ifndef __LIBPD_RING_BUFFER_H__
#define __LIBPD_RING_BUFFER_H__

typedef struct ring_buffer {
  size_t size;
  void *buf_ptr;
  volatile size_t write_idx;
  volatile size_t read_idx;
} ring_buffer;

ring_buffer *rb_create(size_t size);
void rb_free(ring_buffer *buffer);
size_t rb_available_to_write(ring_buffer *buffer);
size_t rb_available_to_read(ring_buffer *buffer);
int rb_write_to_buffer(ring_buffer *buffer, const void *src, size_t len);
int rb_read_from_buffer(ring_buffer *buffer, void *dest, size_t len);

#endif
