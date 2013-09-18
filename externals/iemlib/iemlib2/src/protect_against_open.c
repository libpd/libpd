/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include "g_canvas.h"
#include "g_all_guis.h"
#include <string.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

/* ---------------------- protect_against_open ----------------------- */
/* -- if you putting this object into a subpatch or an abstraction, -- */
/* ------------ you cannot open again this subpatch ------------------ */

static t_widgetbehavior protect_against_open_widgetbehavior;
static t_class *protect_against_open_class;

typedef struct _protect_against_open
{
  t_object x_obj;
  t_symbol *x_sym;
  t_pd     *x_owner;
  void     *x_clock;
} t_protect_against_open;

/* ------------------------ cnv widgetbehaviour----------------------------- */

static void protect_against_open_tick(t_protect_against_open *x)
{
  t_symbol *sym = gensym("vis");
  t_atom at[1];

  SETFLOAT(at, 0.0);
  typedmess(x->x_sym->s_thing, sym, 1, at);
  clock_unset(x->x_clock);
}

static void protect_against_open_vis(t_gobj *z, t_glist *glist, int vis)
{
  t_protect_against_open *x = (t_protect_against_open *)z;

  if(vis)
    clock_delay(x->x_clock, 5);
}

static void *protect_against_open_new(t_symbol *s, int ac, t_atom *av)
{
  t_protect_against_open *x = (t_protect_against_open *)pd_new(protect_against_open_class);
  t_glist *glist = (t_glist *)canvas_getcurrent();
  t_canvas *this_canvas = glist_getcanvas(glist);
  t_symbol *s_unique;
  char str[100];

  x->x_owner = (t_pd *)glist;
  s_unique = canvas_realizedollar(glist_getcanvas(glist), gensym("$0"));
  strcpy(str, s_unique->s_name);
  strcat(str, "-quabla");
  x->x_sym = gensym(str);
  if(*x->x_sym->s_name)
    pd_bind(x->x_owner, x->x_sym);
  x->x_clock = clock_new(x, (t_method)protect_against_open_tick);
  return(x);
}

static void protect_against_open_ff(t_protect_against_open *x)
{
  if(*x->x_sym->s_name)
    pd_unbind(x->x_owner, x->x_sym);
  clock_free(x->x_clock);
}

void protect_against_open_setup(void)
{
  protect_against_open_class = class_new(gensym("protect_against_open"), (t_newmethod)protect_against_open_new,
        (t_method)protect_against_open_ff, sizeof(t_protect_against_open), 0, A_GIMME, 0);

  protect_against_open_widgetbehavior.w_getrectfn = NULL;
  protect_against_open_widgetbehavior.w_displacefn = NULL;
  protect_against_open_widgetbehavior.w_selectfn = NULL;
  protect_against_open_widgetbehavior.w_activatefn = NULL;
  protect_against_open_widgetbehavior.w_deletefn = NULL;
  protect_against_open_widgetbehavior.w_visfn = protect_against_open_vis;
  protect_against_open_widgetbehavior.w_clickfn = NULL;
#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)

#else
  protect_against_open_widgetbehavior.w_propertiesfn = NULL;
  protect_against_open_widgetbehavior.w_savefn =    NULL;
#endif
  class_setwidget(protect_against_open_class, &protect_against_open_widgetbehavior);
}
