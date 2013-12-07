/*
 *
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

#include <stdlib.h>
#include <errno.h>
#include "squeue.h"

/*----------------------------------------------------------------------
 * Creation / Deletion
 *----------------------------------------------------------------------*/
squeue_ptr squeue_new(unsigned int size) {
  squeue_ptr sq = (squeue_ptr)malloc(sizeof(squeue_t));
  if (!sq || !size) {
    errno = ENOMEM;
    return NULL;
  }
  sq->data = (void **)malloc(size*sizeof(void *));
  if (!sq->data) {
    errno = ENOMEM;
    return NULL;
  }
# ifdef SQUEUE_DEBUG
  memset(sq->data,0,size*sizeof(void *));
# endif
  sq->size = size-1;
  sq->head = NULL;
  sq->tail = NULL;
  return sq;
}

void squeue_destroy(squeue_ptr sq) {
  squeue_clear(sq);
  free(sq->data);
  free(sq);
}

/*----------------------------------------------------------------------
 * Predicates
 *----------------------------------------------------------------------*/
#ifdef SQUEUE_DEBUG

char squeue_empty(squeue_ptr sq) {
  return (!sq->head);
}

char squeue_full(squeue_ptr sq) {
  return (sq->head && sq->head == squeue_next(sq,sq->tail));
}

#endif


/*----------------------------------------------------------------------
 * utilities
 *----------------------------------------------------------------------*/
#ifdef SQUEUE_DEBUG

void **squeue_prev(squeue_t *sq, void **p) {
  //return (p && p > sq->data ? p-1 : p+sq->size);
  return (p && p > sq->data ? p-1 : sq->data+sq->size);
}

void **squeue_next(squeue_t *sq, void **p) {
  return (p && p < sq->data+sq->size ? p+1 : sq->data);
}

#endif

/*----------------------------------------------------------------------
 * Manipulation
 *----------------------------------------------------------------------*/

#ifdef SQUEUE_DEBUG

void squeue_clear(squeue_ptr sq) {
  sq->head = NULL;
  sq->tail = NULL;
# ifdef SQUEUE_DEBUG
  memset(sq->data,0,sq->size*sizeof(void *));
# endif
}

#endif

void **squeue_prepend(squeue_ptr sq, void *data) {
  sq->head = squeue_prev(sq,sq->head);
  if (!sq->tail) {
    // -- empty-queue
    sq->tail = sq->head;
  } else if (sq->head == sq->tail) {
    // -- xrun (full queue)
    sq->head = squeue_next(sq,sq->head);
    return NULL;
    // alternative: push the overwritten older pointer along a notch
    //sq->tail = squeue_prev(sq,sq->tail);
  }
  // -- set the data
  *(sq->head) = data;
  return sq->head;
}

void **squeue_append(squeue_ptr sq, void *data) {
  sq->tail = squeue_next(sq,sq->tail);
  if (!sq->head) {
    // -- empty-queue check
    sq->head = sq->tail;
  } else if (sq->tail == sq->head) {
    // -- xrun (full queue)
    sq->tail = squeue_prev(sq,sq->tail);
    return NULL;
    // alternative: push the overwritten older pointer along a notch
    //sq->head = squeue_next(sq,sq->head);
  }
  // -- set the data
  *(sq->tail) = data;
  return sq->tail;
}


/*----------------------------------------------------------------------
 * Access
 *----------------------------------------------------------------------*/

void *squeue_shift(squeue_ptr sq) {
  if (squeue_empty(sq)) return NULL;
  else {
    void *data = *(sq->head);
    // -- empty queue check
    if (sq->head == sq->tail) {
      sq->head = NULL;
      sq->tail = NULL;
    } else {
      sq->head = squeue_next(sq, sq->head);
    }
    return data;
  }
}

void *squeue_pop(squeue_ptr sq) {
  if (squeue_empty(sq)) return NULL;
  else {
    void *data = *(sq->tail);
    if (sq->head == sq->tail) {
      sq->head = NULL;
      sq->tail = NULL;
    } else {
      sq->tail = squeue_prev(sq, sq->tail);
    }
    return data;
  }
}

#ifdef SQUEUE_DEBUG

// Returns the first datum in the queue, or NULL if queue is empty.
void *squeue_peek_head(squeue_ptr sq) {
  return sq->head ? *(sq->head) : NULL;
}

// Returns the final datum in the queue, or NULL if queue is empty.
void *squeue_peek_tail(squeue_ptr sq) {
  return sq->tail ? *(sq->tail) : NULL;
}

#endif

/*----------------------------------------------------------------------
 * Iteration
 *----------------------------------------------------------------------*/
#ifdef SQUEUE_DEBUG

// Returns a pointer-pointer to the first datum in the queue,
// or NULL if queue is empty.
squeue_iter_t squeue_iter_first(squeue_t *sq) { return sq->head; }

// Returns a pointer-pointer to the final datum in the queue,
// or NULL if queue is empty.
squeue_iter_t squeue_iter_last(squeue_t *sq) { return sq->tail; }



// Returns a pointer to the next datum in the queue, or
// NULL if queue is empty.
squeue_iter_t squeue_iter_next(squeue_t *sq, squeue_iter_t p) {
  return p == sq->tail ? NULL : squeue_next(sq,p);
}

// Returns a pointer-pointer to the final datum in the queue,
// or NULL if queue is empty.
extern squeue_iter_t squeue_iter_prev(squeue_t *sq, squeue_iter_t p) {
  return p == sq->head ? NULL : squeue_prev(sq,p);
}


// Returns a true value if p is a valid iterator value, false otherwise.
// Warning: this is not even as paranoid as it ought to be!
extern char squeue_iter_valid(squeue_t *sq, squeue_iter_t p) {
  return (p && p >= sq->data && p <= sq->data+sq->size);
}

/// Get the datum from an iterator.
void *squeue_iter_data(squeue_iter_t p) { return *p; }


#endif
