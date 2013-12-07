/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __FORKY_H__
#define __FORKY_H__

#ifdef PD_MINOR_VERSION
#define FORKY_VERSION  PD_MINOR_VERSION
#elif defined(PD_VERSION)
#define FORKY_VERSION  36
#else
#define FORKY_VERSION  35
#endif

#if FORKY_VERSION >= 37
#define FORKY_WIDGETPADDING
#else
#warning You are entering a back-compatibility mode.  Delete this warning from forky.h to proceed.
#define FORKY_WIDGETPADDING  0,0
#endif

typedef void (*t_forkysavefn)(t_gobj *x, t_binbuf *bb);
typedef void (*t_forkypropertiesfn)(t_gobj *x, t_glist *gl);

t_pd *forky_newobject(t_symbol *s, int ac, t_atom *av);
void forky_setsavefn(t_class *c, t_forkysavefn fn);
void forky_setpropertiesfn(t_class *c, t_forkypropertiesfn fn);
int forky_hasfeeders(t_object *x, t_glist *glist, int inno, t_symbol *outsym);
t_int forky_getbitmask(int ac, t_atom *av);

#endif
