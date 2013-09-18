/*
 *   Pure Data Packet system implementation: Packet Manager Interface
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

/*

    This file contains the pdp packet manager interface specification.

    It is an implementation of the "Object Pool" pattern with lazy instantiation
    and lazy destruction.

    The pool is a growable array. It can only grow larger. Packets are represented
    by an integer, which is an index in this array.

    The standard "pure" packets (the ones which use a flat memory buffer) have recovery
    for resource depletion (main memory). If an out of memory condition is met
    on allocation of a new package, the garbage collector kicks in and frees unused
    packets until the out of memory condition is solved. Since an out of memory
    condition can be fatal for other parts of the program, pdp also supports a
    memory limit, to ensure some kind of safety margin.

    The "not so pure" packets should resolve resource conflicts in their own factory method,
    since the constructor is responsible for allocating external resources. The standard
    way to do this is to allocate a packet, free it's resources and allocate a new packet
    until the resource allocation succeeds. Especially for these kinds of packets, the
    pdp pool supports an explicit reuse method. This returns a valid packet if it can reuse 
    one (based on the high level type description).

    Packets that don't have memory managing methods defined in the packet class
    (Standard packets) are treated as a header concatenated with a flat memory buffer, and
    can be copied and cloned without problems. So, if a packet contains pointers to other
    data or code, it can't be a pure packet.

    The interface to the packet manager contains the following managing methods:

    * pdp_packet_create: create a new packet
    * pdp_packet_mark_unused: release a packet
    * pdp_packet_copy_ro: register a packet for read only use
    * pdp_packet_copy_rw: register a packet for read/write use (this creates a copy if necessary)
    * pdp_packet_clone_rw: create a new packet using a template, but don't copy the data

    ( some types have shortcut methods pdp_packet_new_ which create/reuse a packet
    unless you are writing a factory method for a packet, you should NEVER call a _create method )

    And two methods for raw data access

    * pdp_packet_header: retreive the header of the packet
    * pdp_packet_data: retreive the data buffer of the packet (only for static packets)

    All the methods declared in this header are supposed to be thread safe, so you
    can call them from the pd and pdp thread.

*/

#ifndef PDP_PACKET_H
#define PDP_PACKET_H

#include "pdp_symbol.h"
#include "pdp_types.h"

// this is legacy stuff: images are basic types
#include "pdp_image.h"
#include "pdp_bitmap.h"


#define PDP_HEADER_SIZE 256



/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif



typedef int (*t_pdp_factory_method)(t_pdp_symbol *); //returns bool success

/* packet class header */
typedef struct _pdp_class
{
    /* packet manips: non-pure data packets (using external resources) must define these */
    t_pdp_packet_method1 wakeup;     /* called before returning a reused packet (rc:0->1) */
    t_pdp_packet_method2 copy;       /* copy data from source packet to destination packet */
    t_pdp_packet_method1 cleanup;    /* free packet's resources (destructor) */
    t_pdp_packet_method1 sleep;      /* mark_unused notify: called when refcount reaches zero */
    t_pdp_symbol *type;              /* type template for packet class */
    t_pdp_factory_method create;     /* the constructor: create a packet with uninitialized data */
}t_pdp_class;


/* TODO:
   implement garbage collection for fobs. 
   (fobs are forth object dictionaries, but for the gc these count as lists)
*/

#define PDP_GC_COLOUR_GREY  0      /* 0 == default: object is reachable */
#define PDP_GC_COLOUR_WHITE 1
#define PDP_GC_COLOUR_BLACK 2


/* packet object header */
struct _pdp
{
    /* meta info */
    unsigned int type;             /* main datatype of this object */
    t_pdp_symbol *desc;            /* high level type description (sort of a mime type) */
    unsigned int size;             /* datasize including header */
    unsigned int flags;            /* packet flags */

    /* reference count */
    unsigned int users;            /* nb users of this object, readonly if > 1 */

    /* class object */
    t_pdp_class *theclass;         /* if zero, the packet is a pure packet (just data, no member functions) */

    u32 pad[10];                   /* LATER: reserve bytes to provide compatibility with future extensions */

    union                          /* each packet type has a unique subheader */
    {
	t_raw    raw;              /* raw subheader (for extensions unkown to pdp core system) */
	struct _image  image;      /* (nonstandard internal) 16 bit signed planar bitmap image format */
	struct _bitmap bitmap;     /* (standard) bitmap image (fourcc coded) */
	//t_ca     ca;             /* cellular automaton state data */
	//t_ascii  ascii;          /* ascii packet */
    } info;

};


/* pdp data packet type id */
#define PDP_IMAGE                 1  /* 16bit signed planar scanline encoded image packet */
//RESERVED: #define PDP_CA        2  /* 1bit toroidial shifted scanline encoded cellular automaton */
//RESERVED: #define PDP_ASCII     3  /* ascii packet */
//RESERVED: #define PDP_TEXTURE   4  /* opengl texture object */
//RESERVED: #define PDP_3DCONTEXT 5  /* opengl render context */
#define PDP_BITMAP                6  /* 8bit image packet (fourcc coded??) */
//RESERVED: #define PDP_MATRIX    7  /* floating point/double matrix/vector packet (from gsl) */
#define PDP_FOB                   8  /* small c->forth object wrapper */

/* PACKET FLAGS */
#define PDP_FLAG_DONOTCOPY (1<<0)   /* don't copy the packet on register_rw, instead return an invalid packet */



/* class methods */
t_pdp_class *pdp_class_new(t_pdp_symbol *type, t_pdp_factory_method create);

#if 0
void pdp_class_addmethod(t_pdp_class *c, t_pdp_symbol *name, t_pdp_attribute_method method,
			 struct _pdp_list *in_spec, struct _pdp_list *out_spec);
#endif

/* packet factory method + registration */
int pdp_factory_newpacket(t_pdp_symbol *type);

#if 0
/* send a message to a packet (packet polymorphy) 
   this returns NULL on failure, or a return list
   the return list should be freed by the caller */

int pdp_packet_op(t_pdp_symbol *operation, struct _pdp_list *stack);
#endif

/* debug */
void pdp_packet_print_debug(int packet);


/* hard coded packet methods */
int pdp_packet_copy_ro(int handle); /* get a read only copy */
int pdp_packet_copy_rw(int handle); /* get a read/write copy */
int pdp_packet_clone_rw(int handle); /* get an empty read/write packet of the same type (only copy header) */
void pdp_packet_mark_unused(int handle); /* indicate that you're done with the packet */
void pdp_packet_delete(int packet); /* like mark_unused, but really delete when refcount == 0 */

t_pdp* pdp_packet_header(int handle);    /* get packet header */
void*  pdp_packet_subheader(int handle); /* get packet subheader */
void*  pdp_packet_data(int handle);      /* get packet raw data */
int    pdp_packet_data_size(int handle); /* get packet raw data size */

int pdp_packet_compat(int packet0, int packet1);
int pdp_packet_reuse(t_pdp_symbol *description);
int pdp_packet_create(unsigned int datatype, unsigned int datasize); /* create a new packet, don't reuse */

int pdp_packet_writable(int packet); /* returns true if packet is writable */
void pdp_packet_replace_with_writable(int *packet); /* replaces a packet with a writable copy */
//void pdp_packet_mark_unused_atomic(int *handle); /* mark unused + set reference to -1 (for thread synchro) */


/* pool stuff */
int pdp_pool_collect_garbage(void); /* free all unused packets */
void pdp_pool_set_max_mem_usage(int max); /* set max mem usage */



#ifdef __cplusplus
}
#endif

#endif 
