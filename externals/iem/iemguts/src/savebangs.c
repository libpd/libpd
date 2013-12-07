
/******************************************************
 *
 * propertybang - implementation file
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
 * this object outputs a bang when the savfn of the parent abstraction is called
 */

/*
 * LATER sketch: 
 *     monkey-patching the parent canvas so that stores a local function table
 *     then modify this function-table so that the savefn points to us
 *   on call: we call the parents savefunction and output bangs
 */

/* 
 * TODO: how does this behave in sub-patches?
 *      -> BUG: the depth should _really_ refer to the abstraction-depth 
 *              else we get weird duplicates (most likely due to the "$0" trick
 */

#include "iemguts-objlist.h"
#include "g_canvas.h"


/* ------------------------- helper methods for savefunctions ---------------------------- */

typedef struct _savefuns {
  t_class*class;
  t_savefn savefn;

  struct _savefuns *next;
} t_savefuns;

static t_savefuns*s_savefuns=0;


static t_savefn find_savefn(const t_class*class) 
{
  t_savefuns*fun=s_savefuns;
  if(0==s_savefuns || 0==class)
    return 0;
  for(fun=s_savefuns; fun; fun=fun->next) {
    if(class == fun->class) {
      return fun->savefn;
    }
  }

  return 0;
}
static void add_savefn(t_class*class)
{
  if(0!=find_savefn(class)) {
    return;
  } else {
    t_savefuns*sfun=(t_savefuns*)getbytes(sizeof(t_savefuns));
    sfun->class=class;
    sfun->savefn=class_getsavefn(class);
    sfun->next=0;

    if(0==s_savefuns) {
      s_savefuns=sfun;
    } else {
      t_savefuns*sfp=s_savefuns;
      while(sfp->next)
        sfp=sfp->next;
      sfp->next = sfun;      
    }
  }
}

/* ------------------------- savefunctions ---------------------------- */

static void orig_savefn(t_gobj*z, t_binbuf*b)
{
  t_class*class=z->g_pd;
  t_savefn savefn=find_savefn(class);
  if(savefn) {
    savefn(z, b);
  }
}

static void savebangs_bangem(t_iemguts_objlist*objs, int pst);
static void savebangs_savefn(t_gobj*z, t_binbuf*b) {
  /* z is the parent abstraction;
   * we maintain a list of all [savebangs] within such each parent, in order to call all of them 
   */
  t_iemguts_objlist*obj=objectsInCanvas((t_pd*)z);
  savebangs_bangem(obj, 0);
  orig_savefn(z, b);
  savebangs_bangem(obj, 1);
}

/* ------------------------- savebangs ---------------------------- */

static t_class *savebangs_class;

typedef struct _savebangs
{
  t_object  x_obj;
  t_outlet *x_pre, *x_post;
  t_canvas *x_parent;
} t_savebangs;


static void savebangs_bangs(t_savebangs*x, int pst)
{
  if(!pst)
    outlet_bang(x->x_pre);
  else
    outlet_bang(x->x_post);
}

static void savebangs_bangem(t_iemguts_objlist*objs, int pst) {
  while(objs) {
    t_savebangs*x=(t_savebangs*)objs->obj;
    savebangs_bangs(x, pst);
    objs=objs->next;
  }
}

static void savebangs_mysavefn(t_gobj*z, t_binbuf*b) {
  t_savebangs*x=(t_savebangs*)z;
  int doit=(!x->x_parent);
  if(doit)savebangs_bangs(x, 0);
  orig_savefn(z, b);
  if(doit)savebangs_bangs(x, 1);
}

static void *savebangs_new(t_floatarg f)
{
  t_savebangs *x = (t_savebangs *)pd_new(savebangs_class);
  t_glist *glist=(t_glist *)canvas_getcurrent();
  t_canvas *canvas=(t_canvas*)glist_getcanvas(glist);
  t_class *class = 0;

  int depth=(int)f;
  if(depth<0)depth=0;

  if(depth) {
    depth--;
    while(depth && canvas) {
      canvas=canvas->gl_owner;
      depth--;
    }
    
    if(canvas) {
      class=((t_gobj*)canvas)->g_pd;
      add_savefn(class);
      class_setsavefn(class, savebangs_savefn);
      
      x->x_parent=canvas;
    } else {
      x->x_parent=0;
    }

    addObjectToCanvas((t_pd*)canvas, (t_pd*)x);
  }

  x->x_post=outlet_new(&x->x_obj, &s_bang);
  x->x_pre=outlet_new(&x->x_obj, &s_bang);

  return (x);
}
static void savebangs_free(t_savebangs *x)
{
  /* unpatch the parent canvas */
  removeObjectFromCanvases((t_pd*)x);
}

void savebangs_setup(void)
{
  savebangs_class = class_new(gensym("savebangs"), (t_newmethod)savebangs_new,
                              (t_method)savebangs_free, sizeof(t_savebangs), CLASS_NOINLET, A_DEFFLOAT, 0);
  add_savefn(savebangs_class);
  class_setsavefn(savebangs_class, savebangs_mysavefn); 
}
