/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: deque.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: double-ended message queue for pd
 *
 * Copyright (c) 2003-2009 Bryan Jurish.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *=============================================================================*/


#ifndef _M_PD_H
# include <m_pd.h>
# define _M_PD_H
#endif

#include "dsqueue.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define DEQUE_DEBUG 1
//#undef DEQUE_DEBUG

/*--------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------*/
#define DEQUE_DEFAULT_BLOCKSIZE 32

/*=====================================================================
 * pd_deque_elt
 *   + element class
 *=====================================================================*/
typedef struct pd_deque_elt
{
  //t_symbol  *sel;  // message selector
  int       argc;  // number of message arguments
  t_atom   *argv;  // message arguments
} t_pd_deque_elt;

/*--------------------------------------------------------------------
 * pd_deque_elt_new()
 *  + create a new deque element
 *  + arg 'sel' currently ignored
 */
static t_pd_deque_elt *pd_deque_elt_new(t_symbol *sel, int argc, t_atom *argv)
{
  t_pd_deque_elt *elt = (t_pd_deque_elt *)getbytes(sizeof(t_pd_deque_elt));

#ifdef DEQUE_DEBUG
  post("pd_deque_elt_new: got message with argc=%d", argc);
#endif

  //elt->sel  = sel;
  elt->argc = argc;
  if (argc>0) {
    elt->argv = (t_atom *)copybytes(argv, argc*sizeof(t_atom));
  } else {
    elt->argv = NULL;
  }
  return elt;
}

static void pd_deque_elt_free(t_pd_deque_elt *elt)
{
  if (elt->argc > 0) {
    freebytes(elt->argv, elt->argc*sizeof(t_atom));
  }
  elt->argv = NULL;
  freebytes(elt, sizeof(t_pd_deque_elt));
}

/*=====================================================================
 * pd_deque_class
 *=====================================================================*/
static t_class *pd_deque_class;
typedef struct _pd_deque_class
{
  t_object        x_obj;     // black magic
  dsqueue_ptr     x_deque;   // ye olde guts
  t_pd_deque_elt *x_cur;     // current element
  unsigned int    x_size;    // number of stored messages
  t_outlet       *elt_out;   // data outlet
  t_outlet       *eoq_out;   // end-of-queue bang-outlet / other data
} t_pd_deque;


/*=====================================================================
 * pd methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * push_front
 *  + push to the front of the queue
 */
static void pd_deque_push_front(t_pd_deque *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_pd_deque_elt *elt = pd_deque_elt_new(sel,argc,argv);

#ifdef DEQUE_DEBUG
  post("pd_deque_push_back: got sel='%s', argc=%d", sel->s_name, argc);
#endif

  dsqueue_prepend(x->x_deque, elt);
  ++x->x_size;
}

/*--------------------------------------------------------------------
 * push_back
 *  + push to the back of the queue
 */
static void pd_deque_push_back(t_pd_deque *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_pd_deque_elt *elt = pd_deque_elt_new(sel,argc,argv);

#ifdef DEQUE_DEBUG
  post("pd_deque_push_back: got sel='%s', argc=%d", sel->s_name, argc);
#endif

  dsqueue_append(x->x_deque, elt);
  ++x->x_size;
}

/*--------------------------------------------------------------------
 * outlet_cur
 *  + outlets current element to outlet-1, if non-NULL
 *  + otherwise, bangs to outlet-2
 */
static void pd_deque_outlet_cur(t_pd_deque *x)
{
  if (x->x_cur) {
    --x->x_size;
    if (x->x_cur->argc == 1) {
      switch (x->x_cur->argv->a_type) {
      case A_FLOAT:
	outlet_float(x->elt_out, x->x_cur->argv->a_w.w_float);
	return;
      case A_SYMBOL:
	outlet_symbol(x->elt_out, x->x_cur->argv->a_w.w_symbol);
	return;
      case A_POINTER:
	outlet_pointer(x->elt_out, x->x_cur->argv->a_w.w_gpointer);
	return;
      default:
	error("Error: deque: unrecognized atom type '%d' defaults to 'bang'.",
	      x->x_cur->argv->a_type);
	outlet_bang(x->elt_out);
      }
    } else {
      outlet_anything(x->elt_out,
		      atom_getsymbol(x->x_cur->argv),
		      x->x_cur->argc-1,
		      x->x_cur->argv+1
		      );
    }
  } else {
    outlet_bang(x->eoq_out);
  }
}

/*--------------------------------------------------------------------
 * pop_front
 *  + pop from the front of the queue
 */
static void pd_deque_pop_front(t_pd_deque *x)
{
  if (x->x_cur) {
    pd_deque_elt_free(x->x_cur);
  }
  x->x_cur = (t_pd_deque_elt *)dsqueue_shift(x->x_deque);
  pd_deque_outlet_cur(x);
}

/*--------------------------------------------------------------------
 * pop_back
 *  + pop from the back of the queue
 */
static void pd_deque_pop_back(t_pd_deque *x)
{
  if (x->x_cur) {
    pd_deque_elt_free(x->x_cur);
  }
  x->x_cur = (t_pd_deque_elt *)dsqueue_pop(x->x_deque);
  pd_deque_outlet_cur(x);
}


/*--------------------------------------------------------------------
 * clear
 *  + clear the queue
 */
static void pd_deque_clear(t_pd_deque *x)
{
  if (x->x_cur) pd_deque_elt_free(x->x_cur);
  while ( (x->x_cur = (t_pd_deque_elt *)dsqueue_shift(x->x_deque)) )
    pd_deque_elt_free(x->x_cur);
  x->x_size = 0;
}

/*--------------------------------------------------------------------
 * flush
 *  + flush queue contents (front-to-back)
 *  + probably dangerous
 */
static void pd_deque_flush(t_pd_deque *x)
{
  if (x->x_cur) pd_deque_elt_free(x->x_cur);
  while ( (x->x_cur = (t_pd_deque_elt *)dsqueue_shift(x->x_deque)) ) {
    pd_deque_outlet_cur(x);
    pd_deque_elt_free(x->x_cur);
  }
  x->x_size = 0;
}

/*--------------------------------------------------------------------
 * size
 *  + get number of stored messages
 */
static void pd_deque_size(t_pd_deque *x)
{
  t_atom sizeatom;
  SETFLOAT(&sizeatom, x->x_size);
  outlet_anything(x->eoq_out, gensym("size"), 1, &sizeatom);
}

/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
static void *pd_deque_new(t_floatarg f)
{
  t_pd_deque *x;
  x = (t_pd_deque *)pd_new(pd_deque_class);

  /* -- queue -- */
  if (!f) f = DEQUE_DEFAULT_BLOCKSIZE;   // 0: use default value
  if (f < 1) {                           // specified but goofy: complain
    error("deque: bad blocksize %g", f);
    f = DEQUE_DEFAULT_BLOCKSIZE;
  }
  x->x_deque = dsqueue_new((unsigned)f);

  /* -- defaults --- */
  x->x_cur = NULL;
  x->x_size = 0;

  /* --- extra inlet(s) --- */
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("unshift"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("push"));

  /* -- outlets -- */
  x->elt_out = outlet_new(&x->x_obj, &s_anything);
  x->eoq_out = outlet_new(&x->x_obj, &s_bang);

  return (void *)x;
}

static void pd_deque_free(t_pd_deque *x) {
  pd_deque_clear(x);
  dsqueue_destroy(x->x_deque);
}


/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void deque_setup(void) {
  /* banner */
  post("");
  post("deque: double-ended message queue v" PACKAGE_VERSION " by Bryan Jurish");
  post("deque: compiled by " PACKAGE_BUILD_USER " on " PACKAGE_BUILD_DATE);

  /* register class */
  pd_deque_class = class_new(gensym("deque"),              // name 
			     (t_newmethod)pd_deque_new,    // newmethod
			     (t_method)pd_deque_free,      // freemethod
			     sizeof(t_pd_deque),           // size
			     CLASS_DEFAULT,                // flags
			     A_DEFFLOAT,                   // args
			     0);

  /* --- methods: primary inlet --- */
  class_addmethod(pd_deque_class, (t_method)pd_deque_push_front, gensym("unshift"), A_GIMME, 0);
  class_addmethod(pd_deque_class, (t_method)pd_deque_pop_front, gensym("bang"), 0);
  class_addmethod(pd_deque_class, (t_method)pd_deque_pop_front, gensym("shift"), 0);

  class_addmethod(pd_deque_class, (t_method)pd_deque_push_back, gensym("push"), A_GIMME, 0);
  class_addmethod(pd_deque_class, (t_method)pd_deque_pop_back, gensym("pop"), 0);

  class_addmethod(pd_deque_class, (t_method)pd_deque_clear, gensym("clear"), 0);
  class_addmethod(pd_deque_class, (t_method)pd_deque_flush, gensym("flush"), 0);

  class_addmethod(pd_deque_class, (t_method)pd_deque_size, gensym("size"), 0);

  class_addanything(pd_deque_class, (t_method)pd_deque_push_back);   // default: push_back

  /* --- help --- */
  class_sethelpsymbol(pd_deque_class, gensym("deque-help.pd"));
}
