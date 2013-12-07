
/******************************************************
 *
 * classtest - implementation file
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
 * e.g. [classtest 1] will send messages to the parent of the containing canvas
 */

#include "m_pd.h"
#include "g_canvas.h"

int glist_getindex(t_glist *x, t_gobj *y);

/* ------------------------- classtest ---------------------------- */

static t_class *classtest_class;

typedef struct _classtest
{
  t_object  x_obj;
  t_outlet *x_out;
} t_classtest;

static void classtest_symbol(t_classtest *x, t_symbol*s)
{
  t_float result=0.;
  if(!pd_objectmaker) {
    pd_error(x, "[classtest]: couldn't find pd_objectmaker!");
    return;
  }
  if(0!=zgetfn(&pd_objectmaker, s))
    result=1.;

  outlet_float(x->x_out, result);



}

static void classtest_free(t_classtest *x)
{
  outlet_free(x->x_out);
}

static void *classtest_new(t_floatarg f)
{
  t_classtest *x = (t_classtest *)pd_new(classtest_class);

  x->x_out = outlet_new(&x->x_obj, &s_float);
  return (x);
}

void classtest_setup(void)
{
  classtest_class = class_new(gensym("classtest"), (t_newmethod)classtest_new,
                               (t_method)classtest_free, sizeof(t_classtest), 0, 
                              0);
  class_addsymbol(classtest_class, (t_method)classtest_symbol);
}
