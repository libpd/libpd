
/******************************************************
 *
 * canvasname - implementation file
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


/* ------------------------- canvasname ---------------------------- */

static t_class *canvasname_class;

typedef struct _canvasname
{
  t_object  x_obj;

  t_canvas  *x_canvas;
} t_canvasname;


static void canvasname_bang(t_canvasname *x)
{
  t_canvas*c=x->x_canvas;
  t_binbuf*b=0;
  t_atom name[1];

  if(!x->x_canvas) return;
  b=x->x_canvas->gl_obj.te_binbuf;
  if(b) {
    /* get the binbufs atomlist */
    t_atom*ap=binbuf_getvec(b);
    t_symbol*s=atom_getsymbol(ap);
    outlet_symbol(x->x_obj.ob_outlet, s);

    return;
  } else {
    //post("empty binbuf for %x", x->x_canvas);
  }
}

static void canvasname_symbol(t_canvasname *x, t_symbol*s)
{
  t_canvas*c=x->x_canvas;
  t_binbuf*b=0;
  t_atom name[1];

  if(!x->x_canvas) return;
  b=x->x_canvas->gl_obj.te_binbuf;

  if(b) {
    /* get the binbufs atomlist */
    t_atom*ap=binbuf_getvec(b);

    SETSYMBOL(ap, s);
    return;
  }
}

static void canvasname_free(t_canvasname *x)
{
}

static void *canvasname_new(t_floatarg f)
{
  t_canvasname *x = (t_canvasname *)pd_new(canvasname_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);

  int depth=(int)f;
  if(depth<0)depth=0;

  while(depth && canvas) {
    canvas=canvas->gl_owner;
    depth--;
  }

  x->x_canvas = canvas;

  outlet_new(&x->x_obj, &s_symbol);
  
  return (x);
}

void canvasname_setup(void)
{
  canvasname_class = class_new(gensym("canvasname"), (t_newmethod)canvasname_new,
                               (t_method)canvasname_free, sizeof(t_canvasname), 0, A_DEFFLOAT, 0);
  class_addsymbol(canvasname_class, (t_method)canvasname_symbol);
  class_addbang  (canvasname_class, (t_method)canvasname_bang);
}
