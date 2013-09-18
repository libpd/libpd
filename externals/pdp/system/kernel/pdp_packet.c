/*
 *   Pure Data Packet system implementation: Packet Manager
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



#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "pdp_post.h"
#include "pdp_packet.h"
#include "pdp_mem.h"
#include "pdp_list.h"
#include "pdp_type.h"
#include "pdp_debug.h"


#define D if(0)

/* packet implementation. contains class and packet (instance) handling 

   some notes on packet operations.
   copy ro/rw and unregister are relatively straightforward
   packet creation can be done in 2 ways in this interface:
     create + reuse
   however, these methods should only be called by specific factory
   methods, so the user should only create packets using pdp_factory_newpacket

   reuse or create is thus the responsability of the factory methods for
   each packet type (class) implementation


*/


/* NOTE:
   the packet pool methods are called within the pool locks. this probably
   needs to change, because it will cause deadlocks for container packets (fobs) */


/* new implementation: probably just a minor adjustment: add the reuse fifo attached
   to type desc symbol name
   need to check and possibly eliminate hacks for non-pure packets

   pdp_packet_new:
      LOCK
      1. check reuse fifo
      2. empty -> create packet+return (search array)
      3. element -> check if type is correct, yes->pop+return, no->goto 1.
      UNLOCK
      4. wakeup

   pdp_packet_mark_unused
      
      1. check refcount. if > 1 dec  + exit
      2. if 1 put packet to sleep 
      3. dec refcount
      4. add to reuse fifo (no fifo -> create)

   pdp_packet_delete: analogous to mark_unused
   pdp_packet_copy_ro/rw: analogous to new

*/      


/* the pool */
#define PDP_INITIAL_POOL_SIZE 64
static int pdp_pool_size;
static t_pdp** pdp_pool;

/* mutex: protects the pool and reuse lists attached to symbols */
static pthread_mutex_t pdp_pool_mutex;
#define LOCK   pthread_mutex_lock   (&pdp_pool_mutex)
#define UNLOCK pthread_mutex_unlock (&pdp_pool_mutex)

/* the list of classes */
static t_pdp_list *class_list;

/* debug */
void
pdp_packet_print_debug(int packet)
{
    t_pdp *h = pdp_packet_header(packet);
    pdp_post("debug info for packet %d", packet);
    if (!h){
	pdp_post("invalid packet");
    }
    else{
	pdp_post ("\ttype: %d", h->type);
	pdp_post ("\tdesc: %s", h->desc ? h->desc->s_name : "unknown");
	pdp_post ("\tsize: %d", h->size);
	pdp_post ("\tflags: %x", h->flags);
	pdp_post ("\tusers: %d", h->users);
	pdp_post ("\tclass: %x", h->theclass);
    }
}



/* setup methods */

void 
pdp_packet_setup(void)
{

    pdp_pool_size = PDP_INITIAL_POOL_SIZE;
    pdp_pool = (t_pdp **)pdp_alloc(PDP_INITIAL_POOL_SIZE * sizeof(t_pdp *));
    bzero(pdp_pool, pdp_pool_size * sizeof(t_pdp *));
    class_list = pdp_list_new(0);
    pthread_mutex_init(&pdp_pool_mutex, NULL);
}

/* class methods */
t_pdp_class *pdp_class_new(t_pdp_symbol *type, t_pdp_factory_method create){
    t_pdp_class *c = (t_pdp_class *)pdp_alloc(sizeof(t_pdp_class));
    memset(c, 0, sizeof(t_pdp_class));
    c->create = create;
    c->type = type; // set type
    pdp_list_add(class_list, a_pointer, (t_pdp_word)((void *)c));
    return c;
}

/* the packet factory */
int pdp_factory_newpacket(t_pdp_symbol *type)
{
    int p;
    t_pdp_class *c;
    t_pdp_atom *a = class_list->first;

    /* try to reuse first 
       THINK: should this be the responsability of the type specific constructors,
       or should a packet allways be reusable (solution: depends on what the cleanup method returns??)
     */
    p = pdp_packet_reuse(type);
    if (-1 != p) return p;


    /* call class constructor */
    while(a){
        D pdp_post("new: %s", type->s_name);
	c = (t_pdp_class *)(a->w.w_pointer);
	if (c->type && pdp_type_description_match(type, c->type)){
	    //pdp_post("method %x, type %s", c->create, type->s_name);
	    return (c->create) ? (*c->create)(type) : -1;
	}
	a = a->next;
    }
    return -1;
}

static void
_pdp_pool_expand_nolock(void){
    int i;

    /* double the size */
    int new_pool_size = pdp_pool_size << 1;
    t_pdp **new_pool = (t_pdp **)pdp_alloc(new_pool_size * sizeof(t_pdp *));
    bzero(new_pool, new_pool_size * sizeof(t_pdp *));
    memcpy(new_pool, pdp_pool, pdp_pool_size * sizeof(t_pdp *));
    pdp_dealloc(pdp_pool);
    pdp_pool = new_pool;
    pdp_pool_size = new_pool_size;
}




/* private _pdp_packet methods */

/* packets can only be created and destroyed using these 2 methods */
/* it updates the mem usage and total packet count */

static void
_pdp_packet_dealloc_nolock(t_pdp *p)
{
    /* free memory */
    pdp_dealloc (p);
}

static t_pdp*
_pdp_packet_alloc_nolock(unsigned int datatype, unsigned int datasize)
{
    unsigned int totalsize = datasize + PDP_HEADER_SIZE;
    t_pdp *p = (t_pdp *)pdp_alloc(totalsize);
    if (p){
	memset(p, 0, PDP_HEADER_SIZE); //initialize header to 0
	p->type = datatype;
	p->size = totalsize;
	p->users = 1;
    }
    return p;
}


/* create a new packet and expand pool if necessary */
static int
_pdp_packet_create_nolock(unsigned int datatype, unsigned int datasize)
{
    int p = 0;
    while(1){
	for (; p < pdp_pool_size; p++){
	    if (!pdp_pool[p]){
		/* found slot to store packet*/
		t_pdp *header = _pdp_packet_alloc_nolock(datatype, datasize);
		if (!header) return -1; // error allocating packet
		pdp_pool[p] = header;
		return p;
	    }
	}
	/* no slot found, expand pool */
	_pdp_pool_expand_nolock();
    }
}


void 
pdp_packet_destroy(void)
{
    int i = 0;
    /* dealloc all the data in object stack */
    pdp_post("DEBUG: pdp_packet_destroy: clearing object pool.");
    while ((i < pdp_pool_size) && (pdp_pool[i])) _pdp_packet_dealloc_nolock(pdp_pool[i++]);
}








/* public pool operations: have to be thread safe so each entry point
   locks the mutex */


/* create a new packet.
   this should only be used by type specific factory methods, and only if the
   reuse method fails, since it will always create a new packet */
int 
pdp_packet_create(unsigned int datatype, unsigned int datasize /*without header*/)
{
    int packet;
    LOCK;
    packet = _pdp_packet_create_nolock(datatype, datasize);
    UNLOCK;
    return packet;
}


/* return a new packet.
   it tries to reuse a packet based on
    1. matching data size
    2. abscence of destructor (which SHOULD mean there are no enclosed references)

    it obviously can't use the reuse fifo tagged to a symbolic type description

    ALWAYS USE pdp_packet_reuse BEFORE calling pdp_packet_new if possible
    use both ONLY IN CONSTRUCTORS !!!

    use pdp_packet_factory to create packets as a "user"

    this is a summary of all internal packet creation mechanisms:

     -> pdp_packet_reuse, which uses symbolic type descriptions, and should work for all packet types
        it returns an initialized container (meta = correct, data = garbage)

     -> pdp_packet_new, which only works for non-pure packets, and reuses packets based on data type
        it returns a pure packet (meta + data = garbage)

     -> pdp_packet_create, like pdp_packet_new, only it always creates a new packet



*/

/* NEW DOES NOT USE THE REUSE FIFO !!!! 
   this is a true and genuine mess:
   the reuse fifo can grow indefinitely with garbage elements if it's never used,
   while it points to stale packets.. backdoor access = BAD.

   if i recall, this is mainly a compatibility issue..

*/

int 
pdp_packet_new(unsigned int datatype, unsigned int datasize)
{
    t_pdp *header;
    int packet;
    LOCK;
    for (packet = 0; packet < pdp_pool_size; packet++){
	header = pdp_pool[packet];
	/* check data size */
	if (header 
	    && header->users == 0 // must be unused
	    && header->size == datasize + PDP_HEADER_SIZE // must be same size
	    && !(header->theclass && header->theclass->cleanup)){ // must be pure packet (no destructor)

	    /* ok, got one. initialize */
	    memset(header, 0, PDP_HEADER_SIZE);
	    header->users = 1;
	    header->type = datatype;
	    header->size = datasize + PDP_HEADER_SIZE;

	    UNLOCK; //EXIT1
	    return packet;
	}
    }

    /* no usable non-pure packet found, create a new one */

    UNLOCK; //EXIT2
    return pdp_packet_create(datatype, datasize);



}


/* internal method to add a packet to a packet type
   description symbol's unused packet fifo */
void
_pdp_packet_save_nolock(int packet)
{



    t_pdp *header = pdp_packet_header(packet);
    t_pdp_symbol *s;
    PDP_ASSERT(header);
    PDP_ASSERT(header->users == 0);
    PDP_ASSERT(header->desc);
    s = header->desc;
    if (!s->s_reusefifo) s->s_reusefifo = pdp_list_new(0);


    /* big o hack: since pdp_packet_new can reap packets behind our back,
       we won't add a packet if it's already in here */

    if (1) {
      t_pdp_atom *a = s->s_reusefifo->first;
      while (a){
	if (a->w.w_packet == packet) goto found;
	a = a->next;
      }
    }

    pdp_list_add(s->s_reusefifo, a_packet, (t_pdp_word)packet);
    
 found:
      


    if (PDP_DEBUG){
      int el = s->s_reusefifo->elements;
      int maxel = 100;
      if (el > maxel) pdp_post("WARNING: %s reuse fifo has %d elements.", s->s_name, el);
    }
}

/* this will revive a packet matching a certain type description
   no wildcards are allowed */
int
pdp_packet_reuse(t_pdp_symbol *type_description)
{
    int packet = -1;
    t_pdp *header = 0;
    t_pdp_list *l = 0;
    LOCK;
    if (!type_description || !(l = type_description->s_reusefifo)) goto exit;
    while(l->elements){
	packet = pdp_list_pop(l).w_packet;
	header = pdp_packet_header(packet);

	/* check if reuse fifo is consistent (packet unused + correct type)
	   packet could be deleted and replaced with another one, or
	   revived without the index updated (it's a "hint cache") */

	if (header->users == 0){
	    /* check if type matches */
	    if (pdp_type_description_match(header->desc, type_description)){
		header->users++; // revive
		goto exit;
	    }
	    /* if not, add the packet to the correct reuse fifo */
	    else{
		_pdp_packet_save_nolock(packet);
	    }
	}

	/* remove dangling refs */
	header = 0;
	packet = -1;
    }
    
  exit:
    UNLOCK;
    if (header && header->theclass && header->theclass->wakeup){
	header->theclass->wakeup(header); // revive if necessary
    }
    return packet;
}

/* find all unused packets in pool, marked as used (to protect from other reapers)
   and return them as a list. non-pure packets are not revived */





/* this returns a copy of a packet for read only access. 
   (increases refcount of the packet -> packet will become readonly if it was
   writable, i.e. had rc=1 */

int
pdp_packet_copy_ro(int handle)
{
    t_pdp* header;

    if (header = pdp_packet_header(handle)){
	PDP_ASSERT(header->users); // consistency check
	LOCK;
	header->users++;           // increment reference count
	UNLOCK;
    }
    else handle = -1;
    return handle;
}

/* clone a packet: create a new packet with the same
   type as the source packet */

int
pdp_packet_clone_rw(int handle)
{
    t_pdp* header;
    int new_handle = -1;


    if (header = pdp_packet_header(handle)){
	/* consistency checks */
	PDP_ASSERT(header->users);
	PDP_ASSERT(header->desc);

	/* first try to reuse old packet */
	new_handle = pdp_packet_reuse(header->desc);

	/* if this failed, create a new one using the central packet factory method */
	if (-1 == new_handle) new_handle = pdp_factory_newpacket(header->desc);

	/* if the factory method failed cline it manually */
	if (-1 == new_handle) {
	    t_pdp *new_header;
	    //pdp_post("WARNING: pdp_clone_rw: working around non-implemented factory method.");
	    new_handle = pdp_packet_new(header->type, header->size - PDP_HEADER_SIZE);
	    new_header = pdp_packet_header(new_handle);
	    if (new_header){
		memcpy(new_header, header, PDP_HEADER_SIZE);
	    }
	}
    }

    return new_handle;
}

/* return a copy of a packet (clone + copy data) */
int
pdp_packet_copy_rw(int handle)
{
    t_pdp *header, *new_header;
    int new_handle = -1;

    if (!(header = pdp_packet_header(handle))) return -1;

    /* check if we are allowed to copy */
    if (header->flags & PDP_FLAG_DONOTCOPY) return -1;

    /* get target packet */
    new_handle = pdp_packet_clone_rw(handle);
    if (-1 == new_handle) return -1;
    new_header = pdp_packet_header(new_handle);

    /* if there is a copy method, use that one */
    if (header->theclass && header->theclass->copy){
	header->theclass->copy(header, new_header);
    }

    /* otherwize copy the data verbatim */
    else {
	memcpy(pdp_packet_data(new_handle),
	       pdp_packet_data(handle),
	       pdp_packet_data_size(handle));
    }

    return new_handle;
    
}


/* decrement refcount */
void pdp_packet_mark_unused(int handle)
{
    t_pdp *header;
    if (!(header = pdp_packet_header(handle))) return;
    
    PDP_ASSERT(header->users); // consistency check

    LOCK;

    /* just decrement refcount */
    if (header->users > 1){
	header->users--;
    }

    /* put packet to sleep if refcount 1->0 */
    else {
	if (header->theclass && header->theclass->sleep){
	    /* call sleep method (if any) outside of lock
	       while the packet is still alive, so it won't be
	       acclaimed by another thread */
	    UNLOCK;
	    header->theclass->sleep(header); 
	    LOCK;
	}
	/* clear refcount & save in fifo for later use */
	header->users = 0;
	if (header->desc) // sleep could have destructed packet..
	    _pdp_packet_save_nolock(handle);
    }

    UNLOCK;
}



/* delete a packet. rc needs to be == 1 */
void pdp_packet_delete(int handle)
{
    t_pdp *header;
    header = pdp_packet_header(handle);
    PDP_ASSERT(header);
    PDP_ASSERT(header->users == 1); // consistency check

    LOCK;
    
    if (header->theclass && header->theclass->cleanup){
	/* call cleanup method (if any) outside of lock
	   while the packet is still alive, so it won't be
	   acclaimed by another thread */
	UNLOCK;
	header->theclass->cleanup(header); 
	LOCK;
    }

    /* delete the packet */
    pdp_pool[handle] = 0;
    _pdp_packet_dealloc_nolock(header);
    

    UNLOCK;
}







/* public data access methods */

t_pdp*
pdp_packet_header(int handle)
{
    if ((handle >= 0) && (handle < pdp_pool_size)) return pdp_pool[handle];
    else return 0;
}

void*
pdp_packet_subheader(int handle)
{
    t_pdp* header = pdp_packet_header(handle);
    if (!header) return 0;
    return (void *)(&header->info.raw);
}

void*
pdp_packet_data(int handle)
{
    t_pdp *h;
    if ((handle >= 0) && (handle < pdp_pool_size)) 
	{
	    h = pdp_pool[handle];
	    if (!h) return 0;
	    return (char *)(h) + PDP_HEADER_SIZE;
	}
    else return 0;
}

int
pdp_packet_data_size(int handle)
{
    t_pdp *h;
    if ((handle >= 0) && (handle < pdp_pool_size)) 
	{
	    h = pdp_pool[handle];
	    if (!h) return 0;
	    return h->size - PDP_HEADER_SIZE;
	}
    else return 0;    
}




int pdp_packet_writable(int packet) /* returns true if packet is writable */
{
    t_pdp *h = pdp_packet_header(packet);
    if (!h) return 0;
    return (h->users == 1);
}

void pdp_packet_replace_with_writable(int *packet) /* replaces a packet with a writable copy */
{
    int new_p;
    if (!pdp_packet_writable(*packet)){
	new_p = pdp_packet_copy_rw(*packet);
	pdp_packet_mark_unused(*packet);
	*packet = new_p;
    }
	
}

/* pool stuff */

int
pdp_pool_collect_garbage(void)
{
    pdp_post("ERROR: garbage collector not implemented");
    return 0;
}

void
pdp_pool_set_max_mem_usage(int max)
{
    pdp_post("ERROR: mem limit not implemented");
}






#ifdef __cplusplus
}
#endif
