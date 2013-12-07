
/*****************************************************
 *
 * receivecanvas - implementation file
 *
 * copyleft (c) 2009, IOhannes m zmölnig
 *
 *   forum::für::umläute
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* 
 * this object provides a way to receive messages to upstream canvases
 * by default it receives messages to the containing canvas, but you can give the "depth" as argument;
 * e.g. [receivecanvas 1] will receive messages to the parent of the containing canvas
 */

/* NOTES:
 *  it would be _really_ nice to get all the messages that are somehow "sent" to a (parent) object,
 *  no matter whether using typedmess() or using sendcanvas()
 *  this would require (however) to overwrite and proxy the classmethods for canvas which is a chore
 *
 *  currently this objects only gets the messages from typedmess()...
 */

#include "m_pd.h"
#include "g_canvas.h"

#include <stdio.h>

static t_class *receivecanvas_class, *receivecanvas_proxy_class;

typedef struct _receivecanvas_proxy
{
  t_object p_obj;
  t_symbol*p_sym;
  t_clock *p_clock;
  struct _receivecanvas*p_parent;
} t_receivecanvas_proxy;

typedef struct _receivecanvas
{
  t_object x_obj;
  t_receivecanvas_proxy*x_proxy;
} t_receivecanvas;

/* ------------------------- receivecanvas proxy ---------------------------- */

static void receivecanvas_anything(t_receivecanvas *x, t_symbol*s, int argc, t_atom*argv);

static void receivecanvas_proxy_anything(t_receivecanvas_proxy *p, t_symbol*s, int argc, t_atom*argv) {
  if(p->p_parent)
    receivecanvas_anything(p->p_parent, s, argc, argv);
}
static void receivecanvas_proxy_free(t_receivecanvas_proxy *p)
{
  if(p->p_sym)
    pd_unbind(&p->p_obj.ob_pd, p->p_sym);
  p->p_sym=NULL;

  clock_free(p->p_clock);
  p->p_clock=NULL;

  p->p_parent=NULL;
  pd_free(&p->p_obj.ob_pd);

  p=NULL;
}
static t_receivecanvas_proxy*receivecanvas_proxy_new(t_receivecanvas *x, t_symbol*s) {
  t_receivecanvas_proxy*p=NULL;

  if(!x) return p;

  p=(t_receivecanvas_proxy*)pd_new(receivecanvas_proxy_class);

  p->p_sym=s;
  if(p->p_sym) {
    pd_bind(&p->p_obj.ob_pd, p->p_sym);
  }
  p->p_parent=x;
  p->p_clock=clock_new(p, (t_method)receivecanvas_proxy_free);

  return p;
}


/* ------------------------- receivecanvas ---------------------------- */
static void receivecanvas_anything(t_receivecanvas *x, t_symbol*s, int argc, t_atom*argv)
{
  outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

static void receivecanvas_free(t_receivecanvas *x)
{
  if(x->x_proxy) {
    x->x_proxy->p_parent = NULL;
    clock_delay(x->x_proxy->p_clock, 0);
  }
}

static void *receivecanvas_new(t_floatarg f)
{
  t_receivecanvas *x = (t_receivecanvas *)pd_new(receivecanvas_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);
  int depth=(int)f;
  if(depth<0)depth=0;

  while(depth && canvas) {
    canvas=canvas->gl_owner;
    depth--;
  }

  x->x_proxy=NULL;

  if(canvas) {
    char buf[MAXPDSTRING];
    snprintf(buf, MAXPDSTRING-1, ".x%lx", (t_int)canvas);
    buf[MAXPDSTRING-1]=0;

    x->x_proxy=receivecanvas_proxy_new(x, gensym(buf));
  }

  outlet_new(&x->x_obj, 0);

  return (x);
}

void receivecanvas_setup(void)
{
  receivecanvas_class = class_new(gensym("receivecanvas"), (t_newmethod)receivecanvas_new,
                               (t_method)receivecanvas_free, sizeof(t_receivecanvas), CLASS_NOINLET, A_DEFFLOAT, 0);

  receivecanvas_proxy_class = class_new(0, 0, 0, sizeof(t_receivecanvas_proxy), CLASS_NOINLET | CLASS_PD, 0);
  class_addanything(receivecanvas_proxy_class, receivecanvas_proxy_anything);
}
