/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "g_canvas.h"
#include "iemlib.h"


/* -------------- parentdollarzero --------------- */
/* -- receives the $0 value of the parent canvas --*/

static t_class *parentdollarzero_class;

typedef struct _parentdollarzero
{
  t_object     x_obj;
  t_symbol     *s_parent_unique;
  unsigned int x_is_there_a_parent;
} t_parentdollarzero;

static void parentdollarzero_bang(t_parentdollarzero *x)
{
  if(x->x_is_there_a_parent)
    outlet_symbol(x->x_obj.ob_outlet, x->s_parent_unique);
}

static void *parentdollarzero_new(void)
{
  t_parentdollarzero *x = (t_parentdollarzero *)pd_new(parentdollarzero_class);
  t_glist *glist = (t_glist *)canvas_getcurrent();
  t_canvas *this_canvas = glist_getcanvas(glist);

  x->x_is_there_a_parent = (unsigned int)(0!=this_canvas->gl_owner);

  if(x->x_is_there_a_parent)
    x->s_parent_unique = canvas_realizedollar((t_canvas *)this_canvas->gl_owner, gensym("$0"));
  else
    x->s_parent_unique = gensym("");
  outlet_new(&x->x_obj, &s_symbol);
  return (x);
}

void parentdollarzero_setup(void)
{
  parentdollarzero_class = class_new(gensym("parentdollarzero"), (t_newmethod)parentdollarzero_new,
           0, sizeof(t_parentdollarzero), 0, 0);
  class_addcreator((t_newmethod)parentdollarzero_new, gensym("parent$0"), 0);
  class_addbang(parentdollarzero_class, (t_method)parentdollarzero_bang);
//  class_sethelpsymbol(parentdollarzero_class, gensym("iemhelp/help-parentdollarzero"));
}
