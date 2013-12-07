/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __PLUSBOB_H__
#define __PLUSBOB_H__

EXTERN_STRUCT _plustype;
#define t_plustype  struct _plustype
EXTERN_STRUCT _plusbob;
#define t_plusbob  struct _plusbob
EXTERN_STRUCT _plusenv;
#define t_plusenv  struct _plusenv

/* LATER move to plusbob.c */
struct _plusbob
{
    t_symbol    *bob_stub;  /* points back to stub = symbol, pointer-to-here */
    t_plustype  *bob_type;  /* our type */
    t_plustype  *bob_root;  /* our base type directly derived from t_plusbob */
    t_pd        *bob_owner;
    int          bob_refcount;
    int          bob_dorefcount;
    t_plusbob   *bob_children;  /* empty, unless we are a parent */
    /* each bob has exactly one parent, unless being a special, `orphan' case */
    t_plusbob   *bob_parent;
    t_plusbob   *bob_prev;      /* younger brother */
    t_plusbob   *bob_next;      /* older sister */
};

struct _plusenv
{
    t_plusbob  env_bob;
    t_symbol  *env_id;  /* LATER use local symbol namespace */
};

#define PLUSBOB_OWNER  ((t_pd *)-1)

typedef void (*t_plustypefn)(void *);

t_plusbob *plustag_isvalid(t_symbol *tag, t_pd *caller);
t_plusbob *plustag_validtype(t_symbol *tag, t_symbol *tname, t_pd *caller);
t_plusbob *plustag_validroot(t_symbol *tag, t_symbol *rname, t_pd *caller);
t_symbol *plustag_typename(t_symbol *tag, int validate, t_pd *caller);
t_symbol *plustag_rootname(t_symbol *tag, int validate, t_pd *caller);

t_plustype *plustype_new(t_plustype *base, t_symbol *name, size_t sz,
			 t_plustypefn deletefn,
			 t_plustypefn preservefn, t_plustypefn releasefn,
			 t_plustypefn attachfn);

t_plusbob *plusbob_create(t_plustype *tp, t_plusbob *parent);
void plusbob_preserve(t_plusbob *bob);
void plusbob_release(t_plusbob *bob);
t_plusbob *plusbob_getparent(t_plusbob *bob);
void plusbob_attach(t_plusbob *bob, t_plusbob *newparent);
t_plusbob *plusbob_getnext(t_plusbob *bob);
t_plusbob *plusbob_getchildren(t_plusbob *bob);
void plusbob_detachchildren(t_plusbob *bob, t_plusbob *newparent);
void plusbob_detachownedchildren(t_plusbob *bob, t_plusbob *newparent,
				 t_pd *owner);
void plusbob_setowner(t_plusbob *bob, t_pd *owner);
t_pd *plusbob_getowner(t_plusbob *bob);
void outlet_plusbob(t_outlet *o, t_plusbob *bob);

t_plusenv *plusenv_create(t_plustype *tp, t_plusbob *parent, t_symbol *id);
t_plusenv *plusenv_find(t_symbol *id, t_plusenv *defenv);
t_symbol *plusenv_getid(t_plusenv *env);
t_plusbob *plusenv_getparent(t_plustype *tp);
t_plustype *plusenv_setup(void);

#endif
