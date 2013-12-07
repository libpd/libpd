/* -*- Mode: C -*- */
/*
 * File: squeue.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 *
 * Copyright (c) 2003 Bryan Jurish.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 */

#ifndef SQUEUE_H
#define SQUEUE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/**
 * \file squeue.h
 * \brief Headers for static queues.
 */

/// Queue structure.
typedef struct {
  void          **data;      // data array
  unsigned        size;      // number of allocated elements - 1 (index-max)
  void          **head;      // head of queue
  void          **tail;      // tail of queue
} squeue_t, *squeue_ptr;

typedef void **squeue_iter_t;

/*----------------------------------------------------------------------
 * Creation / Deletion
 *----------------------------------------------------------------------*/

/// Create and initialize a new squeue. Returns NULL on failure.
extern squeue_ptr squeue_new(unsigned int size);

/// Destroy an squeue -- implicitly calls clear().
extern void squeue_destroy(squeue_ptr sq);

/*----------------------------------------------------------------------
 * Predicates
 *----------------------------------------------------------------------*/

#ifdef SQUEUE_DEBUG

/// Returns true if the squeue has no elements.
extern char squeue_empty(squeue_ptr sq);

/// Returns true if the squeue has no empty slots remaining.
extern char squeue_full(squeue_ptr sq);

#else

/// Returns true if the squeue has no elements.
# define squeue_empty(sq) (!sq->head)

/// Returns true if the squeue has no empty slots remaining.
# define squeue_full(sq) (sq->head && sq->head == squeue_next(sq,sq->tail))

#endif


/*----------------------------------------------------------------------
 * Manipulation
 *----------------------------------------------------------------------*/

#ifdef SQUEUE_DEBUG

/// Clear all elements from an squeue.  User data is not freed.
extern void squeue_clear(squeue_ptr sq);

#else

/// Clear all elements from an squeue.  User data will not be freed.
# define squeue_clear(sq) sq->head = sq->tail = NULL

#endif

/// Prepend data to the front of an squeue.
/// Returns new head or NULL if queue is full.
extern void **squeue_prepend(squeue_ptr sq, void *data);

/// Append data to the end of an squeue.
/// Returns new tail or NULL if queue is full.
extern void **squeue_append(squeue_ptr sq, void *data);

/*----------------------------------------------------------------------
 * Access
 *----------------------------------------------------------------------*/

/// Shift an element off the front of an squeue.
/// Returns a pointer to the element's data, or NULL if queue is empty.
extern void *squeue_shift(squeue_ptr sq);

/// Pop an element off the back of an squeue.
/// Returns a pointer to the element's data, or NULL if queue is empty.
extern void *squeue_pop(squeue_ptr sq);

#ifdef SQUEUE_DEBUG

/// Returns the first datum in the queue, or NULL if queue is empty.
extern void *squeue_peek_head(squeue_ptr sq);

/// Returns the final datum in the queue, or NULL if queue is empty.
extern void *squeue_peek_tail(squeue_ptr sq);

#else

/// Returns the first datum in the queue, or NULL if queue is empty.
# define squeue_peek_head(sq) (sq->head ? *(sq->head) : NULL)

/// Returns the final datum in the queue, or NULL if queue is empty.
# define squeue_peek_tail(sq) (sq->tail ? *(sq->tail) : NULL)

#endif /* SQUEUE_DEBUG */

/*----------------------------------------------------------------------
 * Utilities
 *----------------------------------------------------------------------*/
#ifdef SQUEUE_DEBUG

/// Returns the index immediately preceeding 'p' (wrapped)
extern void **squeue_prev(squeue_t *sq, void **p);

/// Returns the index immediately follwoing 'p' (wrapped)
extern void **squeue_next(squeue_t *sq, void **p);

#else

/// Returns the index immediately preceeding 'p' (wrapped)
# define squeue_prev(sq,p) (p && p > sq->data ? p-1 : sq->data+sq->size)

/// Returns the index immediately follwoing 'p' (wrapped)
# define squeue_next(sq,p) (p && p < sq->data+sq->size ? p+1 : sq->data)

#endif /* SQUEUE_DEBUG */


/*----------------------------------------------------------------------
 * Iteration
 *----------------------------------------------------------------------*/
#ifdef SQUEUE_DEBUG

/// Returns an iterator (pointer-pointer) for the first datum in the queue,
/// or NULL if queue is empty.
extern squeue_iter_t squeue_iter_first(squeue_t *sq);

/// Returns an iterator for the next datum in the queue, or
/// NULL if already at end-of-queue.
extern squeue_iter_t squeue_iter_next(squeue_t *sq, squeue_iter_t sqi);

/// Returns an iterator (pointer-pointer) for the final datum in the queue,
/// or NULL if queue is empty.
extern squeue_iter_t squeue_iter_last(squeue_t *sq);

/// Returns an iterator for the previous datum in the queue,
/// or NULL if already at beginning-of-queue.
extern squeue_iter_t squeue_iter_prev(squeue_t *sq, squeue_iter_t sqi);


/// Returns a true value if p is a valid iterator value, false otherwise.
/// If you have initialized an incremented your iterator with the
/// squeue_iter() functions, a simple 'p != NULL' will suffice.
extern char squeue_iter_valid(squeue_t *sq, squeue_iter_t sqi);


/// Get the datum from an iterator or NULL if the iterator is invalid.
extern void *squeue_iter_data(squeue_iter_t sqi);


#else

# define squeue_iter_first(sq) (sq->head)

# define squeue_iter_last(sq) (sq->tail)

# define squeue_iter_next(sq,sqi) \
  (sqi == sq->tail ? NULL : (sqi && sqi < sq->data+sq->size ? sqi+1 : sq->data))

# define squeue_iter_prev(sq,sqi) \
  (sqi == sq->head ? NULL : (sqi && sqi > sq->data ? sqi-1 : sqi+sq->size))

# define squeue_iter_valid(sq,sqi) \
  (sqi && sqi >= sq->data && sqi <= sq->data+sq->size)

# define squeue_iter_data(sqi) (sqi ? *sqi : NULL)

#endif /* SQUEUE_DEBUG */



#endif /* SQUEUE_H */
