/*
 *   Pure Data Packet header file: memory allocation
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

#ifndef _PDP_MEM_H_
#define _PDP_MEM_H_

#include <pthread.h>

/* a wrapper around malloc and free to keep track of pdp's memory usage */
void *pdp_alloc(int size);
void pdp_dealloc(void *stuff);


/* fast allocator object (for lists and atoms) */
typedef struct _pdp_fastalloc
{
  unsigned int atom_size;
  unsigned int block_elements;
  pthread_mutex_t mut;
  struct _fastalloc *freelist;
  
} t_pdp_fastalloc;

void *pdp_fastalloc_new_atom(t_pdp_fastalloc *x);
void pdp_fastalloc_save_atom(t_pdp_fastalloc *x, void *atom);
t_pdp_fastalloc *pdp_fastalloc_new(unsigned int size);

#endif
