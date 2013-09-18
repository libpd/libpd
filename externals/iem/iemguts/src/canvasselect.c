
/******************************************************
 *
 * canvasselect - implementation file
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
 * this object allows to select other objects on the canvas
 * it also allows to query the selection of the canvas
 * 
 * it also adds a "select" message to the canvas
 */

#include "m_pd.h"

#include "g_canvas.h"

/* ------------------------- canvasselect ---------------------------- */

static t_class *canvasselect_class;

typedef struct _canvasselect
{
  t_object  x_obj;

  t_canvas*x_canvas;
} t_canvasselect;

static void canvasselect_bang(t_canvasselect *x)
{
  /* get the selection of the canvas */
  t_glist*glist=x->x_canvas;
  t_gobj*obj=NULL;
  int index=0;

  if(NULL==glist) {
    return;
  }
  t_atom selected_index;
  int nselected=0;
  
  for(obj=glist->gl_list; obj; obj=obj->g_next, index++) {
    if(glist_isselected(glist, obj)) {
      // post("selected: %d", index);
      nselected++;
    }
  }
  int n=0;
  index=0;
  t_atom *atombuf;
  
  atombuf = (t_atom *)getbytes(sizeof(t_atom)*nselected);
  
  for(obj=glist->gl_list; obj; obj=obj->g_next, index++) {
    if(glist_isselected(glist, obj)) {
      SETFLOAT(&atombuf[n], index);
      n++;
    }
  }
  
  outlet_list(x->x_obj.ob_outlet, &s_list, nselected, atombuf);
}


static int canvasselect_doselect(t_glist*glist, int index)
{
  t_gobj*obj=NULL;
  int i=index;
  if(NULL==glist) {
    return -1;
  }
  if(i<0) {
    return -1;
  }

  obj=glist->gl_list;

  while(i-- && obj) {
    obj=obj->g_next;
  }

  if(obj) {
    if(!glist_isselected(glist, obj))
       glist_select(glist, obj);
  }
  else {
    return -1;
  }

  return index;
}

static void canvasselect_select(t_canvasselect*x, t_floatarg f)
{
  t_glist*glist=x->x_canvas;
  int i=f;
  if(canvasselect_doselect(x->x_canvas, i)<0) {
    pd_error(x, "invalid selection %d", i);
  }
}

static void canvasselect_selectall(t_canvasselect*x)
{
  t_glist*glist=x->x_canvas;
  if(glist)glist_selectall(glist);
}

static void canvasselect_select_cb(t_canvasselect*x, t_symbol*s, int argc, t_atom*argv)
{
  if(argc) {
    while(argc--){
      canvasselect_select(x, atom_getint(argv++));
    }
  } else {
    canvasselect_selectall(x);
  }
}



static int canvasselect_dodeselect(t_glist*glist, int index)
{
  t_gobj*obj=NULL;
  int i=index;
  if(NULL==glist) {
    return -1;
  }
  if(i<0) {
    return -1;
  }

  obj=glist->gl_list;

  while(i-- && obj) {
    obj=obj->g_next;
  }

  if(obj) {
    if(glist_isselected(glist, obj)) {
      glist_deselect(glist, obj);
    }  else  return index;
  } else {
    return -1;
  }

  return index;
}

static void canvasselect_deselect(t_canvasselect*x, t_floatarg f)
{
  t_glist*glist=x->x_canvas;
  int i=f;
  if(canvasselect_dodeselect(x->x_canvas, i)<0) {
    pd_error(x, "invalid deselection %d", i);
  }
}

static void canvasselect_deselectall(t_canvasselect*x)
{
  if(NULL==x->x_canvas) {
    return;
  }

  glist_noselect((t_glist*)x->x_canvas);
}


static void canvasselect_deselect_cb(t_canvasselect*x, t_symbol*s, int argc, t_atom*argv)
{
  if(argc) {
    while(argc--){
      canvasselect_deselect(x, atom_getint(argv++));
    }
  } else {
    canvasselect_deselectall(x);
  }
}

static void *canvasselect_new(t_floatarg f)
{
  t_canvasselect *x = (t_canvasselect *)pd_new(canvasselect_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);
  int depth=(int)f;
  
  if(depth<0)depth=0;

  while(depth && canvas) {
    canvas=canvas->gl_owner;
    depth--;
  }

  x->x_canvas = canvas;
  
  outlet_new(&x->x_obj, 0);
  return (x);
}

static void canvasselect_free(t_canvasselect*x)
{

}

static void canvas_select_cb(t_canvas*x, t_float f)
{
  canvasselect_doselect(x, f);
}
static void canvas_selectall_cb(t_canvas*x)
{
  t_glist*glist=x;
  if(glist)glist_selectall(glist);
}

static void canvas_deselect_cb(t_canvas*x, t_float f)
{
  canvasselect_dodeselect(x, f);
}
static void canvas_deselectall_cb(t_canvas*x)
{
  t_glist*glist=x;
  if(glist)glist_noselect(glist);
}
static void register_methods(void)
{
  if(NULL==canvas_class)return;
  /* check whether we already have this function: zgetfn(canvasclass, gensym("select")) */
  if(NULL==zgetfn(&canvas_class, gensym("select"))) class_addmethod(canvas_class, (t_method)canvas_select_cb, gensym("select"), A_FLOAT, 0);
  if(NULL==zgetfn(&canvas_class, gensym("selectall"))) class_addmethod(canvas_class, (t_method)canvas_selectall_cb, gensym("selectall"), 0);
  if(NULL==zgetfn(&canvas_class, gensym("deselect"))) class_addmethod(canvas_class, (t_method)canvas_deselect_cb, gensym("deselect"), A_FLOAT, 0);
  if(NULL==zgetfn(&canvas_class, gensym("deselectall"))) class_addmethod(canvas_class, (t_method)canvas_deselectall_cb, gensym("deselectall"), 0);
}


void canvasselect_setup(void)
{
  canvasselect_class = class_new(gensym("canvasselect"), 
                                 (t_newmethod)canvasselect_new, (t_method)canvasselect_free, 
                                 sizeof(t_canvasselect), 0,
                                 A_DEFFLOAT, 0);
  class_addbang(canvasselect_class, (t_method)canvasselect_bang);
  class_addmethod(canvasselect_class, (t_method)canvasselect_select_cb, gensym("select"), A_GIMME, 0);
  class_addmethod(canvasselect_class, (t_method)canvasselect_deselect_cb, gensym("deselect"), A_GIMME, 0);

  register_methods();
}
