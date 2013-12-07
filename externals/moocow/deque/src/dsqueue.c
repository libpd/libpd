/*
 * File: dsqueue.c
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
#include <stdlib.h>
#include <errno.h>

#include "dsqueue.h"
#include "squeue.h"

#include "mooPdUtils.h"

/*----------------------------------------------------------------------
 * Creation / Deletion
 *----------------------------------------------------------------------*/
dsqueue_ptr dsqueue_new(unsigned blocksize) {
  dsqueue_ptr dsq = (dsqueue_ptr)malloc(sizeof(dsqueue_t));
  if (!dsq) {
    errno = ENOMEM;
    return NULL;
  }
  dsq->blocksize = blocksize;
  dsq->trash = NULL;
  dsq->head = NULL;
  dsq->tail = NULL;
  return dsq;
}

void dsqueue_destroy(dsqueue_ptr dsq) {
  dsqueue_block_t *dsqe, *dsqe_next;
  dsqueue_clear(dsq);
  for (dsqe = dsq->head; dsqe != NULL; dsqe = dsqe_next) {
    squeue_destroy(dsqe->sq);
    dsqe_next = dsqe->next;
    free(dsqe);
  }
  for (dsqe = dsq->trash; dsqe != NULL; dsqe = dsqe_next) {
    squeue_destroy(dsqe->sq);
    dsqe_next = dsqe->next;
    free(dsqe);
  }
  free(dsq);
}

/*----------------------------------------------------------------------
 * Predicates
 *----------------------------------------------------------------------*/
char dsqueue_is_empty(dsqueue_ptr dsq) {
  while (dsq->head && squeue_empty(dsq->head->sq)) {
    dsqueue_block_t *dsqe = dsqueue_block_shift(dsq);
    dsqe->next = dsq->trash;
    if (dsq->trash) dsq->trash->prev = dsqe;
    dsq->trash = dsqe;
  }
  return dsq->head ? 0 : 1;
}

/*----------------------------------------------------------------------
 * Manipulation
 *----------------------------------------------------------------------*/

void dsqueue_clear(dsqueue_ptr dsq) {
  if (dsq->head && dsq->tail) {
    dsq->tail->next = dsq->trash;
    if (dsq->trash) dsq->trash->prev = dsq->tail;
    dsq->trash = dsq->head;
  }
  dsq->head = NULL;
  dsq->tail = NULL;
}

dsqueue_block_t *dsqueue_prepend(dsqueue_ptr dsq, void *data) {
  dsqueue_block_t *dsqe = dsq->head;
  while (!dsqe || squeue_full(dsqe->sq)) {
    dsqe = dsqueue_block_new(dsq);
    if (!dsqe) {
      errno = ENOMEM;
      return NULL;
    }
    dsqueue_block_prepend(dsq,dsqe);
  }
  squeue_prepend(dsqe->sq,data);
  return dsqe;
}

dsqueue_block_t *dsqueue_append(dsqueue_ptr dsq, void *data) {
  dsqueue_block_t *dsqe = dsq->tail;
  while (!dsqe || squeue_full(dsqe->sq)) {
    dsqe = dsqueue_block_new(dsq);
    if (!dsqe) {
      errno = ENOMEM;
      return NULL;
    }
    dsqueue_block_append(dsq,dsqe);
  }
  squeue_append(dsqe->sq,data);
  return dsqe;
}



/*----------------------------------------------------------------------
 * Access
 *----------------------------------------------------------------------*/

void *dsqueue_shift(dsqueue_ptr dsq) {
  dsqueue_trim_front(dsq);
  return dsq->head ? squeue_shift(dsq->head->sq) : NULL;
}

void *dsqueue_pop(dsqueue_ptr dsq) {
  dsqueue_trim_back(dsq);
  return dsq->tail ? squeue_pop(dsq->tail->sq) : NULL;
}


/*----------------------------------------------------------------------
 * Iterators
 *----------------------------------------------------------------------*/

// Returns an iterator for the first datum in the queue,
// or NULL if queue is empty.
dsqueue_iter_t dsqueue_iter_first(dsqueue_t *dsq) {
  dsqueue_iter_t dsqi = {NULL,NULL};
  dsqueue_trim_front(dsq);
  if (dsq->head) {
    dsqi.block = dsq->head;
    dsqi.sqi = squeue_iter_first(dsqi.block->sq);
  }
  return dsqi;
}

/// Returns an iterator for the final datum in the queue,
/// or NULL if queue is empty.
dsqueue_iter_t dsqueue_iter_last(dsqueue_t *dsq) {
  dsqueue_iter_t dsqi = {NULL,NULL};
  dsqueue_trim_back(dsq);
  if (dsq->tail) {
    dsqi.block = dsq->tail;
    dsqi.sqi = squeue_iter_last(dsqi.block->sq);
  }
  return dsqi;
}


/// Returns an iterator for the next datum in the queue, or
/// NULL if already at end-of-queue.
dsqueue_iter_t dsqueue_iter_next(MOO_UNUSED dsqueue_t *dsq, dsqueue_iter_t dsqi) {
  while (dsqi.block) {
    dsqi.sqi = squeue_iter_next(dsqi.block->sq, dsqi.sqi);
    if (dsqi.sqi) break;
    dsqi.block = dsqi.block->next;
  }
  return dsqi;
}

/// Returns an iterator for the previous datum in the queue,
/// or NULL if already at beginning-of-queue.
dsqueue_iter_t dsqueue_iter_prev(MOO_UNUSED dsqueue_t *dsq, dsqueue_iter_t dsqi) {
  while (dsqi.block) {
    dsqi.sqi = squeue_iter_prev(dsqi.block->sq, dsqi.sqi);
    if (dsqi.sqi) break;
    dsqi.block = dsqi.block->prev;
  }
  return dsqi;
}

/// Returns a true value if p is a valid iterator value, false otherwise.
char dsqueue_iter_valid(MOO_UNUSED dsqueue_t *dsq, dsqueue_iter_t dsqi) {
  return (dsqi.block && dsqi.sqi && 
	  squeue_iter_valid(dsqi.block->sq, dsqi.sqi));
}

/// Get the datum from an iterator.
void *dsqueue_iter_data(dsqueue_iter_t dsqi) {
  return (dsqi.block && dsqi.sqi ? squeue_iter_data(dsqi.sqi) : NULL);
}



/*----------------------------------------------------------------------
 * Utilities
 *----------------------------------------------------------------------*/

// Allocate and return a new block.
dsqueue_block_t *dsqueue_block_new(dsqueue_t *dsq) {
  // -- first try trashbin
  dsqueue_block_t *dsqe = dsq->trash;
  if (dsqe) {
    dsq->trash = dsqe->next;
    if (dsq->trash) dsq->trash->prev = NULL;
  } else {
    dsqe = (dsqueue_block_t *)malloc(sizeof(dsqueue_block_t));
    if (!dsqe) {
      errno = ENOMEM;
      return NULL;
    }
    dsqe->sq = squeue_new(dsq->blocksize);
    if (!dsqe->sq) {
      errno = ENOMEM;
      free(dsqe);
      return NULL;
    }
  }
  // -- initialize
  dsqe->prev = NULL;
  dsqe->next = NULL;
  return dsqe;
}


#ifdef DSQUEUE_DEBUG

dsqueue_block_t *dsqueue_block_prepend(dsqueue_t *dsq, dsqueue_block_t *e) {
  if (!e) return NULL;
  else if (dsq->head) dsq->head->prev = e;
  else dsq->tail = e;
  e->next = dsq->head;
  dsq->head = e;
  return dsq->head;
}

dsqueue_block_t *dsqueue_block_append(dsqueue_t *dsq, dsqueue_block_t *e) {
  if (!e) return NULL;
  else if (dsq->tail) dsq->tail->next = e;
  else dsq->head = e;
  e->prev = dsq->tail;
  dsq->tail = e;
  return dsq->tail;
}

#endif


dsqueue_block_t *dsqueue_block_shift(dsqueue_t *dsq) {
  dsqueue_block_t *dsqe = dsq->head;
  if (!dsqe) return dsqe;
  dsq->head = dsqe->next;
  if (dsq->head) {
    dsq->head->prev = NULL;
  } else {
    dsq->tail = NULL;
  }
  dsqe->next = NULL;
  return dsqe;
}
dsqueue_block_t *dsqueue_block_pop(dsqueue_t *dsq) {
  dsqueue_block_t *dsqe = dsq->tail;
  if (!dsqe) return dsqe;
  dsq->tail = dsqe->prev;
  if (dsq->tail) {
    dsq->tail->next = NULL;
  } else {
    dsq->head = NULL;
  }
  dsqe->prev = NULL;
  return dsqe;
}


void dsqueue_trim_front(dsqueue_t *dsq) {
  while (dsq->head && squeue_empty(dsq->head->sq)) {
    dsqueue_block_t *dsqe = dsqueue_block_shift(dsq);
    if (dsq->trash) dsq->trash->prev = dsqe;
    dsqe->next = dsq->trash;
    dsq->trash = dsqe;
  }
}

void dsqueue_trim_back(dsqueue_t *dsq) {
  while (dsq->tail && squeue_empty(dsq->tail->sq)) {
    dsqueue_block_t *dsqe = dsqueue_block_pop(dsq);
    if (dsq->trash) dsq->trash->prev = dsqe;
    dsqe->next = dsq->trash;
    dsq->trash = dsqe;
  }
}
