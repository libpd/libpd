
/******************************************************
 *
 * sendcanvas - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   2007:forum::für::umläute:2007
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* 
 * this object provides a way to send messages to upstream canvases
 * by default it sends messages to the containing canvas, but you can give the
 * "depth" as argument;
 * e.g. [sendcanvas 1] will send messages to the parent of the containing canvas
 */

#include "m_pd.h"
#include "g_canvas.h"

int glist_getindex(t_glist *x, t_gobj *y);

/* ------------------------- sendcanvas ---------------------------- */

static t_class *sendcanvas_class;

typedef struct _sendcanvas
{
  t_object  x_obj;
  t_pd  *x_pd;
} t_sendcanvas;

static void sendcanvas_anything(t_sendcanvas *x, t_symbol*s, int argc, t_atom*argv)
{
  if(0==x->x_pd)
    return;

  typedmess(x->x_pd, s, argc, argv);
}

static void sendcanvas_free(t_sendcanvas *x)
{
  x->x_pd=0;
}

static void *sendcanvas_new(t_floatarg f)
{
  t_sendcanvas *x = (t_sendcanvas *)pd_new(sendcanvas_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);
  int depth=(int)f;
  if(depth<0)depth=0;

  while(depth && canvas) {
    canvas=canvas->gl_owner;
    depth--;
  }

  x->x_pd = (t_pd*)canvas;
  return (x);
}

void sendcanvas_setup(void)
{
  sendcanvas_class = class_new(gensym("sendcanvas"), (t_newmethod)sendcanvas_new,
                               (t_method)sendcanvas_free, sizeof(t_sendcanvas), 0, A_DEFFLOAT, 0);
  class_addanything(sendcanvas_class, (t_method)sendcanvas_anything);
}
