/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __FITTER_H__
#define __FITTER_H__

typedef void (*t_fitterstate_callback)(void);

void fitter_setup(t_class *owner, t_fitterstate_callback callback);
void fitter_drop(t_class *owner);
t_float *fitter_getfloat(t_symbol *s);
t_symbol *fitter_getsymbol(t_symbol *s);
void fitter_setmode(t_symbol *s);
t_symbol *fitter_getmode(void);
void fittermax_set(void);
int fittermax_get(void);
void fittermax_warning(t_class *c, char *fmt, ...);
void fittermax_rangewarning(t_class *c, int maxmax, char *what);

#endif
