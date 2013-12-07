/* iemnet
 *
 * data handling code
 *  - wrappers for data "chunks"
 *  - queues
 *
 *  copyright (c) 2010 IOhannes m zmölnig, IEM
 */

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

#define DEBUGLEVEL 8

#include "iemnet.h"
#include "iemnet_data.h"

#include <stdlib.h>

#include <string.h>
#include <stdio.h>

#include <sys/types.h>

#include <pthread.h>



#define INBUFSIZE 65536L /* was 4096: size of receiving data buffer */


/* data handling */

t_iemnet_floatlist*iemnet__floatlist_init(t_iemnet_floatlist*cl) {
  unsigned int i;
  if(NULL==cl)return NULL;
  for(i=0; i<cl->size; i++)
    SETFLOAT((cl->argv+i), 0.f);

  return cl;
}

void iemnet__floatlist_destroy(t_iemnet_floatlist*cl) {
  if(NULL==cl)return;
  if(cl->argv) free(cl->argv);
  cl->argv=NULL;
  cl->argc=0;
  cl->size=0;

  free(cl);
}

t_iemnet_floatlist*iemnet__floatlist_create(unsigned int size) {
  t_iemnet_floatlist*result=(t_iemnet_floatlist*)malloc(sizeof(t_iemnet_floatlist));
  if(NULL==result)return NULL;

  result->argv = (t_atom*)malloc(size*sizeof(t_atom));
  if(NULL==result->argv) {
    iemnet__floatlist_destroy(result);
    return NULL;
  }

  result->argc=size;
  result->size=size;

  result=iemnet__floatlist_init(result);

  return result;
}

t_iemnet_floatlist*iemnet__floatlist_resize(t_iemnet_floatlist*cl, unsigned int size) {
  t_atom*tmp;
  if (NULL==cl) {
    return iemnet__floatlist_create(size);
  }

  if(size<=cl->size) {
    cl->argc=size;
    return cl;
  }

  tmp=(t_atom*)malloc(size*sizeof(t_atom));
  if(NULL==tmp) return NULL;

  free(cl->argv);

  cl->argv=tmp;
  cl->argc=cl->size=size;

  cl=iemnet__floatlist_init(cl);

  return cl;
}

void iemnet__chunk_destroy(t_iemnet_chunk*c) {
  if(NULL==c)return;

  if(c->data)free(c->data);

  c->data=NULL;
  c->size=0;

  free(c);
}

t_iemnet_chunk* iemnet__chunk_create_empty(int size) {
  t_iemnet_chunk*result=(t_iemnet_chunk*)malloc(sizeof(t_iemnet_chunk));
  if(result) {
    result->size=size;
    result->data=(unsigned char*)malloc(sizeof(unsigned char)*size); 

    if(NULL == result->data) {
      result->size=0;
      iemnet__chunk_destroy(result);
      return NULL;
    }

    memset(result->data, 0, result->size);

    result->addr=0L;
    result->port=0;

  }
  return result;
}

t_iemnet_chunk* iemnet__chunk_create_data(int size, unsigned char*data) {
  t_iemnet_chunk*result=iemnet__chunk_create_empty(size);
  if(result) {
    memcpy(result->data, data, result->size);
  }
  return result;
}

t_iemnet_chunk* iemnet__chunk_create_dataaddr(int size, 
					      unsigned char*data,
					      struct sockaddr_in*addr) {
  t_iemnet_chunk*result=iemnet__chunk_create_data(size, data);
  if(result && addr) {
    result->addr = ntohl(addr->sin_addr.s_addr);
    result->port = ntohs(addr->sin_port);
  }
  return result;
}

t_iemnet_chunk* iemnet__chunk_create_list(int argc, t_atom*argv) {
  int i;
  t_iemnet_chunk*result=iemnet__chunk_create_empty(argc);
  if(NULL==result)return NULL;

  for(i=0; i<argc; i++) {
    unsigned char c = atom_getint(argv);
    result->data[i]=c;
    argv++;
  }

  return result;
}

t_iemnet_chunk*iemnet__chunk_create_chunk(t_iemnet_chunk*c) {
  t_iemnet_chunk*result=NULL;
  if(NULL==c)return NULL;
  result=iemnet__chunk_create_data(c->size, c->data);
  result->addr=c->addr;
  result->port=c->port;

  return result;
}


t_iemnet_floatlist*iemnet__chunk2list(t_iemnet_chunk*c, t_iemnet_floatlist*dest) {
  unsigned int i;
  if(NULL==c)return NULL;
  dest=iemnet__floatlist_resize(dest, c->size);
  if(NULL==dest)return NULL;

  for(i=0; i<c->size; i++) {
    dest->argv[i].a_w.w_float = c->data[i];
  }

  return dest;
}


/* queue handling */

/*
 *   using code found at http://newsgroups.derkeiler.com/Archive/Comp/comp.programming.threads/2008-02/msg00502.html
 */


#ifdef t_iemnet_queue
# undef t_iemnet_queue
#endif
typedef struct _node {
  struct _node* next;
  t_iemnet_chunk*data;
} t_node;

struct _iemnet_queue {
  t_node* head; /* = 0 */
  t_node* tail; /* = 0 */
  pthread_mutex_t mtx;
  pthread_cond_t cond;

  int done; // in cleanup state
  int size;

  pthread_mutex_t usedmtx;
  pthread_cond_t usedcond;
  int used; // use counter, so queue_finish can wait for blocking accesses to finish
};

static void queue_use_increment(t_iemnet_queue* _this) {
  pthread_mutex_lock(&_this->usedmtx);
  _this->used++;
  pthread_mutex_unlock(&_this->usedmtx);
}
static void queue_use_decrement(t_iemnet_queue* _this) {
  pthread_mutex_lock(&_this->usedmtx);
  _this->used--;
  pthread_cond_signal(&_this->usedcond);
  pthread_mutex_unlock(&_this->usedmtx);
}

/* push a  chunk into the queue
 * this will return the current queue size
 */
int queue_push(
	       t_iemnet_queue* const _this,
	       t_iemnet_chunk* const data
	       ) {
  t_node* tail;
  t_node* n=NULL;
  int size=-1;
  if(NULL == _this)return size;

  pthread_mutex_lock(&_this->mtx);
  size=_this->size;
  pthread_mutex_unlock(&_this->mtx);

  if(NULL == data) return size;

  n=(t_node*)malloc(sizeof(t_node));

  n->next = 0;
  n->data = data;

  pthread_mutex_lock(&_this->mtx);
  if (! (tail = _this->tail)) {
    _this->head = n;
  } else {
    tail->next = n;
  }
  _this->tail = n;

  _this->size+=data->size;
  size=_this->size;

  // added new chunk, so tell waiting threads that they can pop the data
  pthread_cond_signal(&_this->cond);
  pthread_mutex_unlock(&_this->mtx);

  return size;
}


/* pop a chunk from the queue
 * if the queue is empty, this will block until 
 *    something has been pushed
 *   OR the queue is "done" (in which case NULL is returned)
 */
t_iemnet_chunk* queue_pop_block(
				t_iemnet_queue* const _this
				) {

  t_node* head=0;
  t_iemnet_chunk*data=0;
  if(NULL == _this)return NULL;

  queue_use_increment(_this);
  pthread_mutex_lock(&_this->mtx);

  /* if the queue is empty, wait */
  if(NULL == _this->head) {
    pthread_cond_wait(&_this->cond, &_this->mtx);
    /* somebody signaled us, that we should do some work
     * either the queue has been filled, or we are done...
     */
    if(_this->done) {
      pthread_mutex_unlock(&_this->mtx);
      queue_use_decrement(_this);
      return NULL;
    }
  }
  /* save the head, below we gonna work on this */
  head = _this->head;

  /* update _this */
  if (! (_this->head = head->next)) {
    _this->tail = 0;
  }
  if(head && head->data) {
    _this->size-=head->data->size;
  }

  pthread_mutex_unlock(&_this->mtx);

  if(head) {
    data=head->data;
    free(head);
    head=NULL;
  }
  queue_use_decrement(_this);
  return data;
}
/* pop a chunk from the queue
 * if the queue is empty, this will immediately return NULL
 * (note that despite of the name this does block for synchronization) 
 */
t_iemnet_chunk* queue_pop_noblock(
				  t_iemnet_queue* const _this
				  ) {
  t_node* head=0;
  t_iemnet_chunk*data=0;
  if(NULL == _this)return NULL;

  queue_use_increment(_this);
  pthread_mutex_lock(&_this->mtx);
  if (! (head = _this->head)) {
    // empty head
    pthread_mutex_unlock(&_this->mtx);
    queue_use_decrement(_this);
    return NULL;
  }

  if (! (_this->head = head->next)) {
    _this->tail = 0;
  }
  if(head && head->data) {
    _this->size-=head->data->size;
  }

  pthread_mutex_unlock(&_this->mtx);

  if(head) {
    data=head->data;
    free(head);
    head=NULL;
  }
  queue_use_decrement(_this);
  return data;
}

t_iemnet_chunk* queue_pop(t_iemnet_queue* const _this) {
  return queue_pop_block(_this);
}

int queue_getsize(t_iemnet_queue* const _this) {
  int size=-1;
  if(_this) {
    pthread_mutex_lock(&_this->mtx);
    size=_this->size;
    pthread_mutex_unlock(&_this->mtx);
  }
  return size;
}
void queue_finish(t_iemnet_queue* q) {
  DEBUG("queue_finish: %x", q);
  if(NULL==q) return;

  pthread_mutex_lock(&q->mtx);
  q->done=1;
  DEBUG("queue signaling: %x", q);
  pthread_cond_signal(&q->cond);
  DEBUG("queue signaled: %x", q);
  pthread_mutex_unlock(&q->mtx);

  /* wait until queue is no longer used */
  pthread_mutex_lock(&q->usedmtx);
  while(q->used) pthread_cond_wait(&q->usedcond, &q->usedmtx);
  pthread_mutex_unlock(&q->usedmtx);

  DEBUG("queue_finished: %x", q);
}

void queue_destroy(t_iemnet_queue* q) {
  t_iemnet_chunk*c=NULL;
  if(NULL==q) 
    return;
  DEBUG("queue destroy %x", q);

  queue_finish(q);

  /* remove all the chunks from the queue */
  while(NULL!=(c=queue_pop_noblock(q))) {
    iemnet__chunk_destroy(c);
  }

  q->head=NULL;
  q->tail=NULL;

  pthread_mutex_destroy(&q->mtx);
  pthread_cond_destroy(&q->cond);

  pthread_mutex_destroy(&q->usedmtx);
  pthread_cond_destroy(&q->usedcond);

  free(q);
  q=NULL;
  DEBUG("queue destroyed %x", q);
}

t_iemnet_queue* queue_create(void) {
  static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

  t_iemnet_queue*q=(t_iemnet_queue*)malloc(sizeof(t_iemnet_queue));
  DEBUG("queue create %x", q);
  if(NULL==q)return NULL;

  q->head = NULL;
  q->tail = NULL;

  memcpy(&q->cond, &cond, sizeof(pthread_cond_t));
  memcpy(&q->mtx , &mtx, sizeof(pthread_mutex_t));

  memcpy(&q->usedcond, &cond, sizeof(pthread_cond_t));
  memcpy(&q->usedmtx , &mtx, sizeof(pthread_mutex_t));

  q->done = 0;
  q->size = 0;
  q->used = 0;
  DEBUG("queue created %x", q);
  return q;
}
