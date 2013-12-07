/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __SIC_H__
#define __SIC_H__

typedef struct _sic
{
    t_object  s_ob;
    t_float   s_f;
    int       s_disabled;
} t_sic;

#define SIC_FLOATTOSIGNAL   ((void *)0)
#define SIC_NOMAINSIGNALIN  ((void *)-1)

#define SIC_NCOSTABLES  16  /* this is oscbank~'s max, LATER rethink */

t_inlet *sic_inlet(t_sic *x, int ix, t_float df, int ax, int ac, t_atom *av);
t_inlet *sic_newinlet(t_sic *x, t_float f);
t_float *sic_makecostable(int *sizep);
void sic_setup(t_class *c, void *dspfn, void *floatfn);

#endif
