/* *********************************************+
 * iemnet
 *     networking for Pd
 *
 *  (c) 2010 IOhannes m zmölnig
 *           Institute of Electronic Music and Acoustics (IEM)
 *           University of Music and Dramatic Arts (KUG), Graz, Austria
 *
 *  data handling structures
 *   think of these as private, no need to worry about them outside the core lib
 */

/* ---------------------------------------------------------------------------- */

/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc.,                                                            */
/* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                  */
/*                                                                              */

/* ---------------------------------------------------------------------------- */
#ifndef INCLUDE__IEMNET_DATA_H_
#define INCLUDE__IEMNET_DATA_H_

/**
 * a resizable list of float-only atoms
 */
typedef struct _iemnet_floatlist {
  t_atom*argv;
  size_t argc;

  size_t size; // real size (might be bigger than argc)
} t_iemnet_floatlist;

/**
 * create a list of float-only atoms
 *
 * \param size initial size of the floatlist
 * \return pointer to a float-atom list or NULL of creation failed
 */
t_iemnet_floatlist*iemnet__floatlist_create(unsigned int size);
/**
 * destroy a list of float-only atoms
 *
 * \param  pointer to a float-atom list
 */
void iemnet__floatlist_destroy(t_iemnet_floatlist*cl);


/**
 * chunk of data as sent to a socket or received from it
 * for received data, this might additionally hold the originator (if available)
 */
typedef struct _iemnet_chunk {
  unsigned char* data;
  size_t size;

  long addr;
  unsigned short port;
} t_iemnet_chunk;

/**
 * free a "chunk" (de-allocate memory,...)
 */
void iemnet__chunk_destroy(t_iemnet_chunk*);

/**
 * initialize a "chunk" (allocate memory,...) of fixed size
 * receiver address will be set to 0
 *
 * \param size of the chunk (data will be zeroed out)
 * \return a new chunk of given size
 */
t_iemnet_chunk*iemnet__chunk_create_empty(int);
/**
 * initialize a "chunk" (allocate memory,...) with given data
 * receiver address will be set to 0
 *
 * \param size of data
 * \param data of size
 * \return a new chunk that holds a copy of data
 */
t_iemnet_chunk*iemnet__chunk_create_data(int size, unsigned char*data);
/**
 * initialize a "chunk" (allocate memory,...) with given data from specified address
 * \param size of data
 * \param data of size
 * \param addr originating address (can be NULL)
 * \return a new chunk that holds a copy of data
 */
t_iemnet_chunk*iemnet__chunk_create_dataaddr(int size, unsigned char*data, struct sockaddr_in*addr);
/**
 * initialize a "chunk" (allocate memory,...) with given data
 * receiver address will be set to 0
 *
 * \param argc size of list
 * \param argv list of atoms containing only "bytes" (t_floats [0..255])
 * \return a new chunk that holds a copy of the list data
 */
t_iemnet_chunk*iemnet__chunk_create_list(int argc, t_atom*argv);
/**
 * initialize a "chunk" (allocate memory,...) from another chunk
 *
 * \param src the source chunk
 * \return a new chunk that holds a copy of the source data
 */
t_iemnet_chunk*iemnet__chunk_create_chunk(t_iemnet_chunk*source);


/**
 * convert a data chunk to a Pd-list of A_FLOATs
 * the destination list will eventually be resized if it is too small to hold the chunk
 *
 * \param c the chunk to convert 
 * \param dest the destination list
 * \return the destination list if all went well, else NULL
 */
t_iemnet_floatlist*iemnet__chunk2list(t_iemnet_chunk*c, t_iemnet_floatlist*dest);


/**
 * opaque type for a thread safe queue (FIFO)
 */
typedef struct _iemnet_queue t_iemnet_queue;
EXTERN_STRUCT _iemnet_queue;

/**
 * push data to the FIFO (queue)
 *
 * \param q the queue to push to
 * \param d the pushed data (the queue will only store the pointer to the data; so don't free it yet)
 * \return the fill state of the queue after the push
 *
 * \note thread safe
 */
int queue_push(t_iemnet_queue* const q, t_iemnet_chunk* const d);
/**
 * \brief pop data from the FIFO (queue), blocking
 *
 *  pops data from the stack; 
 *  if the stack is empty, this function will block until data is pushed to the stack
 *  if the queue is finalized, this function will return immediately with NULL
 *
 * \param q the queue to pop from
 * \return pointer to the popped data; the caller is responsible for freeing the chunk
 *
 * \note thread safe
 */
t_iemnet_chunk* queue_pop_block(t_iemnet_queue* const q);
/**
 * pop data from the stack (queue), non-blocking
 *
 * pops data from the stack; if the stack is empty, this function will immediately return NULL
 *
 * \param q the queue to pop from
 * \return pointer to the popped data or NULL; the caller is responsible for freeing the chunk
 *
 * \note thread safe
 */
t_iemnet_chunk* queue_pop_noblock(t_iemnet_queue* const);
/**
 * get size if queue
 *
 * \param q the queue to get the size of
 * \return the fill state of the queue, -1 if something goes wrong
 *
 * \note thread safe
 */
int queue_getsize(t_iemnet_queue* const q);
/**
 * initiate cleanup process
 *
 * unblocks all blocking calls to queue_pop_block(t_iemnet_queue* const q);
 *
 * \param q the queue to unblock
 */
void queue_finish(t_iemnet_queue* q);
/**
 * destroy queue (FIFO)
 *
 * releases all data in the queue (by calling iemnet__chunk_destroy()) and then frees all other ressources attached to the queue
 *
 * \param q the queue to destroy
 */
void queue_destroy(t_iemnet_queue* q);
/**
 * create a queue (FIFO)
 *
 * \return the newly created queue; if something went wrong NULL is returned
 */
t_iemnet_queue* queue_create(void);


#endif /* INCLUDE__IEMNET_DATA_H_ */
