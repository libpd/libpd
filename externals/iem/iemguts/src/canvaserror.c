
/******************************************************
 *
 * canvaserror - implementation file
 *
 * copyleft (c) IOhannes m zm-bölnig-A
 *
 *   2007:forum::f-bür::umläute:2007-A
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* 
 * this object provides a way to manipulate the parent-patches arguments (and name!)
 * usage:
 *   + put this object into an abstraction
 *   + put the abstraction in a patch
 *   + send the object a _list_ of arguments
 *    + the next time the patch (wherein the abstraction that holds this object lives)
 *      is saved, it will be saved with the new arguments instead of the old ones!
 *    - example: "list 2 3 4" will save the object as [<absname> 2 3 4]
 *   + you can also change the abstraction name itself by using a selector other than "list"
 *    - example: "bonkers 8 9" will save the object as [bonkers 8 9] regardless of it's original name
 *    - use with care!
 *
 * nice, eh?
 */

#include "m_pd.h"
#include "g_canvas.h"


/* ------------------------- canvaserror ---------------------------- */

static t_class *canvaserror_class;

typedef struct _canvaserror
{
  t_object  x_obj;

  t_canvas  *x_canvas;
} t_canvaserror;


static void canvaserror_any(t_canvaserror *x, t_symbol*s, int argc, t_atom*argv)
{
  t_canvas*c=x->x_canvas;
  t_atom name[1];
  char*bbstring;
  int length;
  

  t_binbuf*bb=binbuf_new();
  SETSYMBOL(name, s);
  binbuf_add(bb, 1, name);
  binbuf_add(bb, argc, argv);
  binbuf_gettext(bb, &bbstring, &length);
  binbuf_free(bb);

  bbstring[length]=0;

  if(!c) {
    pd_error(x, "%s", bbstring);
  } else {
    t_symbol*objectname=c->gl_name;
    pd_error(c, "[%s]: %s", objectname->s_name, bbstring);
  }
  freebytes(bbstring, length);

}

static void canvaserror_free(t_canvaserror *x)
{
}

static void *canvaserror_new(t_floatarg f)
{
  t_canvaserror *x = (t_canvaserror *)pd_new(canvaserror_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);

  int depth=(int)f;
  if(depth<0)depth=0;

  while(depth && canvas) {
    canvas=canvas->gl_owner;
    depth--;
  }

  x->x_canvas = canvas;
  
  return (x);
}

void canvaserror_setup(void)
{
  canvaserror_class = class_new(gensym("canvaserror"), (t_newmethod)canvaserror_new,
                               (t_method)canvaserror_free, sizeof(t_canvaserror), 0, A_DEFFLOAT, 0);
  class_addanything(canvaserror_class, (t_method)canvaserror_any);
}
