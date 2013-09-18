/*
 *   Pure Data Packet system file: memory allocation
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "pdp_mem.h"
#include "pdp_debug.h"

// defined here so we don't need to rebuild when changing it (deps for headers are broken)
#define PDP_FASTALLOC_BLOCK_ELEMENTS 4096
//#define PDP_FASTALLOC_BLOCK_ELEMENTS 1
#define D if (0)

/* malloc wrapper that calls garbage collector */
void *pdp_alloc(int size)
{
    void *ptr = malloc(size);
    
    PDP_ASSERT(ptr);

    D fprintf(stderr, "alloc %p %d\n", ptr, size);

    return ptr;

    //TODO: REPAIR THIS
    //post ("malloc failed in a pdp module: running garbage collector.");
    //pdp_pool_collect_garbage();
    //return malloc(size);
}


void pdp_dealloc(void *stuff)
{
    D fprintf(stderr, "dealloc %p\n", stuff);
    free (stuff);
}


/* fast atom allocation object
   well, this is not too fast yet, but will be later
   when it suports linux futexes or atomic operations */

//#include <pthread.h>

/* private linked list struct */
typedef struct _fastalloc
{
  struct _fastalloc * next;
} t_fastalloc;




static void _pdp_fastalloc_lock(t_pdp_fastalloc *x){pthread_mutex_lock(&x->mut);}
static void _pdp_fastalloc_unlock(t_pdp_fastalloc *x){pthread_mutex_unlock(&x->mut);}

static void _pdp_fastalloc_refill_freelist(t_pdp_fastalloc *x)
{
    t_fastalloc *atom;
    unsigned int i;

    PDP_ASSERT(x->freelist == 0);

    /* get a new block 
       there is no means of freeing the data afterwards,
       this is a fast implementation with the tradeoff of data
       fragmentation "memory leaks".. */

    x->freelist = pdp_alloc(x->block_elements * x->atom_size);

    /* link all atoms together */
    atom = x->freelist;
    for (i=0; i<x->block_elements-1; i++){
      atom->next = (t_fastalloc *)(((char *)atom) + x->atom_size);
      atom = atom->next;
    }
    atom->next = 0;
    
}

#define USE_FASTALLOC 1

#if USE_FASTALLOC

void *pdp_fastalloc_new_atom(t_pdp_fastalloc *x)
{
  t_fastalloc *atom;

  _pdp_fastalloc_lock(x);

  /* get an atom from the freelist
     or refill it and try again */
  while (!(atom = x->freelist)){
    _pdp_fastalloc_refill_freelist(x);
  }

  /* delete the element from the freelist */
  x->freelist = x->freelist->next;
  atom->next = 0;

  _pdp_fastalloc_unlock(x);

  return (void *)atom;
	 
}
void pdp_fastalloc_save_atom(t_pdp_fastalloc *x, void *atom)
{
  _pdp_fastalloc_lock(x);
  ((t_fastalloc *)atom)->next = x->freelist;
  x->freelist = (t_fastalloc *)atom;
  _pdp_fastalloc_unlock(x);
}

t_pdp_fastalloc *pdp_fastalloc_new(unsigned int size)
{
  t_pdp_fastalloc *x = pdp_alloc(sizeof(*x));
  if (size < sizeof(t_fastalloc)) size = sizeof(t_fastalloc);
  x->freelist = 0;
  x->atom_size = size;
  x->block_elements = PDP_FASTALLOC_BLOCK_ELEMENTS;
  pthread_mutex_init(&x->mut, NULL);
  return x;
}

#else

void *pdp_fastalloc_new_atom(t_pdp_fastalloc *x)             {return pdp_alloc(12);}
void pdp_fastalloc_save_atom(t_pdp_fastalloc *x, void *atom) {pdp_dealloc(atom);}
t_pdp_fastalloc *pdp_fastalloc_new(unsigned int size)        {return 0;}

#endif
