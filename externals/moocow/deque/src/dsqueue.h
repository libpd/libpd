/* -*- Mode: C -*- */
/*
 * File: dsqueue.h
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

#ifndef DSQUEUE_H
#define DSQUEUE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "squeue.h"


/**
 * \file dsqueue.h
 * \brief Headers for linked-list queues.
 */

/// Queue block structure.
typedef struct dsqueue_block {
  squeue_t           *sq;
  struct dsqueue_block *next;
  struct dsqueue_block *prev;
} dsqueue_block_t, *dsqueue_block_ptr;

/// Queue structure.
typedef struct {
  dsqueue_block_t *head;     // head of the queue
  dsqueue_block_t *tail;     // tail of the queue
  dsqueue_block_t *trash;    // recycling bin
  unsigned      blocksize;
} dsqueue_t, *dsqueue_ptr;


/// Queue iterator structure
typedef struct {
  dsqueue_block_t *block;
  squeue_iter_t   sqi;
} dsqueue_iter_t;

/*----------------------------------------------------------------------
 * Creation / Deletion
 *----------------------------------------------------------------------*/

/// Create and initialize a new dsqueue. Returns NULL on failure.
extern dsqueue_ptr dsqueue_new(unsigned blocksize);

/// Destroy an dsqueue -- implicitly calls clear().
extern void dsqueue_destroy(dsqueue_ptr dsq);

/*----------------------------------------------------------------------
 * Predicates
 *----------------------------------------------------------------------*/

/// True if the dsqueue has no blocks.
extern char dsqueue_is_empty(dsqueue_ptr dsq);

#define dsqueue_empty(dsq) (!dsq->head || dsqueue_is_empty(dsq))

/*----------------------------------------------------------------------
 * Manipulation
 *----------------------------------------------------------------------*/

/// Clear all blocks from an dsqueue.  'data' members will not be freed.
extern void dsqueue_clear(dsqueue_ptr dsq);

/// Prepend data to the front of an dsqueue.
extern dsqueue_block_t *dsqueue_prepend(dsqueue_ptr dsq, void *data);

/// Append data to the end of an dsqueue.
extern dsqueue_block_t *dsqueue_append(dsqueue_ptr dsq, void *data);

/*----------------------------------------------------------------------
 * Access
 *----------------------------------------------------------------------*/

/// Shift an block off the front of an dsqueue.
/// Returns a pointer to the block's data, or NULL if dsqueue is empty.
extern void *dsqueue_shift(dsqueue_ptr dsq);

/// Pop an block off the back of an dsqueue.
/// Returns a pointer to the block's data, or NULL if dsqueue is empty.
extern void *dsqueue_pop(dsqueue_ptr dsq);


/*----------------------------------------------------------------------
 * Iteration
 *----------------------------------------------------------------------*/
/// Returns an iterator for the first datum in the queue,
/// or NULL if queue is empty.
extern dsqueue_iter_t dsqueue_iter_first(dsqueue_t *dsq);

/// Returns an iterator for the next datum in the queue, or
/// NULL if already at end-of-queue.
extern dsqueue_iter_t dsqueue_iter_next(dsqueue_t *dsq, dsqueue_iter_t dsqi);

/// Returns an iterator for the final datum in the queue,
/// or NULL if queue is empty.
extern dsqueue_iter_t dsqueue_iter_last(dsqueue_t *dsq);

/// Returns an iterator for the previous datum in the queue,
/// or NULL if already at beginning-of-queue.
extern dsqueue_iter_t dsqueue_iter_prev(dsqueue_t *dsq, dsqueue_iter_t dsqi);

/// Returns a true value if p is a valid iterator value, false otherwise.
extern char dsqueue_iter_valid(dsqueue_t *dsq, dsqueue_iter_t dsqi);

/// Get the datum from an iterator.
extern void *dsqueue_iter_data(dsqueue_iter_t dsqi);

/*----------------------------------------------------------------------
 * Utilities
 *----------------------------------------------------------------------*/

/// Allocate and return a new block (try trashbin first)
extern dsqueue_block_t *dsqueue_block_new(dsqueue_t *dsq);

#ifdef DSQUEUE_DEBUG

/// Prepend a block
extern dsqueue_block_t *dsqueue_block_prepend(dsqueue_t *dsq, dsqueue_block_t *e);

/// Append a block
extern dsqueue_block_t *dsqueue_block_append(dsqueue_t *dsq, dsqueue_block_t *e);

#else

#define dsqueue_block_prepend(dsq,e) \
  if (e) { \
    if (dsq->head) dsq->head->prev = e; \
    else dsq->tail = e; \
    e->next = dsq->head; \
    dsq->head = e; \
  }


#define dsqueue_block_append(dsq,e) \
  if (e) { \
    if (dsq->tail) dsq->tail->next = e; \
    else dsq->head = e; \
    e->prev = dsq->tail; \
    dsq->tail = e; \
  }

#endif


/// Shift a block from the front (trashing it)
extern dsqueue_block_t *dsqueue_block_shift(dsqueue_t *dsq);

/// Pop a block from the end (trashing it)
extern dsqueue_block_t *dsqueue_block_pop(dsqueue_t *dsq);

/// Trim empty blocks off the front of the queue.
extern void dsqueue_trim_front(dsqueue_t *dsq);

/// Trim empty blocks off the back of the queue.
extern void dsqueue_trim_back(dsqueue_t *dsq);

#endif /* DSQUEUE_H */
