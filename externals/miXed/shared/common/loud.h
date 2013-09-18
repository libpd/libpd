/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __LOUD_H__
#define __LOUD_H__

#define LOUD_CLIP  1
#define LOUD_WARN  2

enum { LOUD_ARGOK, LOUD_ARGUNDER, LOUD_ARGOVER, LOUD_ARGTYPE, LOUD_ARGMISSING };

EXTERN_STRUCT _loudcontext;
#define t_loudcontext  struct _loudcontext

int shared_matchignorecase(char *test, char *pattern);

char *loud_ordinal(int n);
void loud_error(t_pd *x, char *fmt, ...);
void loud_errand(t_pd *x, char *fmt, ...);
void loud_syserror(t_pd *x, char *fmt, ...);
void loud_nomethod(t_pd *x, t_symbol *s);
void loud_messarg(t_pd *x, t_symbol *s);
int loud_checkint(t_pd *x, t_float f, int *valuep, t_symbol *mess);
void loud_classarg(t_class *c);
void loud_warning(t_pd *x, char *who, char *fmt, ...);
void loud_notimplemented(t_pd *x, char *name);
int loud_floatarg(t_class *c, int which, int ac, t_atom *av,
		  t_float *vp, t_float minval, t_float maxval,
		  int underaction, int overaction, char *what);

void loudx_error(t_loudcontext *lc, char *fmt, ...);
void loudx_errand(t_loudcontext *lc, char *fmt, ...);
void loudx_nomethod(t_loudcontext *lc, t_symbol *s);
void loudx_messarg(t_loudcontext *lc, t_symbol *s);
void loudx_warning(t_loudcontext *lc, char *fmt, ...);
void loudx_setcontext(t_loudcontext *lc, t_pd *caller, char *callername,
		      t_symbol *s, int ac, t_atom *av);
void loudx_setcaller(t_loudcontext *lc, t_pd *caller, char *callerfmt, ...);
t_symbol *loudx_getselector(t_loudcontext *lc);
t_atom *loudx_getarguments(t_loudcontext *lc, int *acp);
void loudx_freecontext(t_loudcontext *lc);
t_loudcontext *loudx_newcontext(t_pd *caller, char *callername,
				t_symbol *s, int ac, t_atom *av);

void loudbug_post(char *fmt, ...);
void loudbug_startpost(char *fmt, ...);
void loudbug_stringpost(char *s);
void loudbug_endpost(void);
void loudbug_postatom(int ac, t_atom *av);
void loudbug_postbinbuf(t_binbuf *bb);
void loudbug_bug(char *fmt, ...);

#endif
