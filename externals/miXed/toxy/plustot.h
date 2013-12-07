/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __PLUSTOT_H__
#define __PLUSTOT_H__

#define PD_EXTERN  EXTERN
#undef EXTERN
#include <tcl.h>
#undef EXTERN
#define EXTERN  PD_EXTERN
#undef PD_EXTERN

#ifdef KRZYSZCZ
#define PLUSTOT_DEBUG
#endif

EXTERN_STRUCT _plustin;
#define t_plustin  struct _plustin
EXTERN_STRUCT _plustob;
#define t_plustob  struct _plustob
EXTERN_STRUCT _plusvar;
#define t_plusvar  struct _plusvar

EXTERN_STRUCT _plusstring;
#define t_plusstring  struct _plusstring

EXTERN_STRUCT _pluswidget;
#define t_pluswidget  struct _pluswidget

typedef struct _plusobject
{
    t_object       po_ob;
    t_glist       *po_glist;
    t_pluswidget  *po_widget;
    int            po_ninlets;
    int            po_noutlets;
} t_plusobject;

t_symbol *totps_plustot;
t_symbol *plusps_tot;
t_symbol *plusps_Ti;
t_symbol *plusps_To;
t_symbol *plusps_Tv;

#define PLUSTOB_MAKEIT  ((Tcl_Obj *)-1)

typedef int (*t_plusifsharedfn)(t_plusbob *, Tcl_Obj *);

enum { PLUSTIN_GLIST_THIS, PLUSTIN_GLIST_ANY, PLUSTIN_GLIST_UP };

void plusloud_tclerror(t_pd *caller, Tcl_Interp *interp, char *msg);

t_plustin *plustin_create(t_plustype *tp, t_plusbob *parent, t_symbol *id);
Tcl_Interp *plustin_getinterp(t_plustin *tin);
t_symbol *plustin_glistid(t_glist *gl);
t_plustin *plustin_glistfind(t_glist *gl, int mode);
t_plustin *plustin_glistprovide(t_glist *gl, int mode, int create);
t_symbol *plustin_getglistname(t_plustin *tin);

t_plustob *plustob_create(t_plustype *tp, t_plustin *tin, Tcl_Obj *ob);
t_plustob *plustob_new(t_plustin *tin, Tcl_Obj *ob);
void plustob_setifshared(t_plustob *tob, t_plusifsharedfn ifsharedfn);
int plustob_isshared(t_plustob *tob);
Tcl_Obj *plustob_getvalue(t_plustob *tob);

t_plustin *plustag_tobtin(t_symbol *s, t_pd *caller);
Tcl_Obj *plustag_tobvalue(t_symbol *s, t_pd *caller);
Tcl_Obj *plusatom_tobvalue(t_atom *ap, t_pd *caller);

int plustob_clear(t_plustob *tob);
Tcl_Obj *plustob_set(t_plustob *tob, t_plustin *tin, Tcl_Obj *ob);
Tcl_Obj *plustob_setfloat(t_plustob *tob, t_float f);
Tcl_Obj *plustob_setsymbol(t_plustob *tob, t_symbol *s);
Tcl_Obj *plustob_setlist(t_plustob *tob, int ac, t_atom *av);
Tcl_Obj *plustob_setbinbuf(t_plustob *tob, t_binbuf *bb);
Tcl_Obj *plustob_grabresult(t_plustob *tob);
Tcl_Obj *plustob_evalob(t_plustob *tob, Tcl_Obj *ob);

t_plusvar *plusvar_create(t_plustype *tp, t_plustin *tin, Tcl_Obj *ob,
			  char *name, char *index);
t_plusvar *plusvar_new(char *name, char *index, t_plustin *tin);
Tcl_Obj *plusvar_push(t_plusvar *var);
Tcl_Obj *plusvar_pull(t_plusvar *var);
void plusvar_clear(t_plusvar *var, int doit);
Tcl_Obj *plusvar_set(t_plusvar *var, Tcl_Obj *ob, int doit);
Tcl_Obj *plusvar_setfloat(t_plusvar *var, t_float f, int doit);
Tcl_Obj *plusvar_setsymbol(t_plusvar *var, t_symbol *s, int doit);
Tcl_Obj *plusvar_setlist(t_plusvar *var, int ac, t_atom *av, int doit);

void plusstring_preserve(t_plusstring *ps);
void plusstring_release(t_plusstring *ps);
char *plusstring_get(t_plusstring *ps, int *lenp);

void plusobject_free(t_plusobject *po);
t_plusobject *plusobject_new(t_class *c, t_symbol *s, int ac, t_atom *av,
			     t_plusstring *ps);
t_inlet *plusinlet_new(t_plusobject *po, t_pd *dest,
		       t_symbol *s1, t_symbol *s2);
t_outlet *plusoutlet_new(t_plusobject *po, t_symbol *s);
void plusclass_inherit(t_class *c, t_symbol *s);

void plustot_ar_setup(void);
void plustot_env_setup(void);
void plustot_in_setup(void);
void plustot_var_setup(void);
void plustot_out_setup(void);
void plustot_qlist_setup(void);
void plustot_print_setup(void);

void *plustot_ar_new(t_symbol *s, int ac, t_atom *av);
void *plustot_env_new(t_symbol *s, int ac, t_atom *av);
void *plustot_in_new(t_symbol *s, int ac, t_atom *av);
void *plustot_var_new(t_symbol *s, int ac, t_atom *av);
void *plustot_out_new(t_symbol *s, int ac, t_atom *av);
void *plustot_qlist_new(t_symbol *s, int ac, t_atom *av);
void *plustot_print_new(t_symbol *s, int ac, t_atom *av);

#endif
