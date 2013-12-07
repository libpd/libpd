/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include "m_pd.h"
#include "common/loud.h"

typedef struct _spell
{
    t_object  x_ob;
    int       x_minsize;
    int       x_padchar;  /* actually, any nonnegative integer (CHECKED) */
} t_spell;

static t_class *spell_class;

static void spell_fill(t_spell *x, int cnt)
{
    for (; cnt < x->x_minsize; cnt++)
	outlet_float(((t_object *)x)->ob_outlet, x->x_padchar);
}

/* CHECKED: chars are spelled as signed */
static int spell_out(t_spell *x, char *ptr, int flush)
{
    int cnt = 0;
    while (*ptr)
	outlet_float(((t_object *)x)->ob_outlet, *ptr++), cnt++;
    if (flush)
    {
	spell_fill(x, cnt);
	return (0);
    }
    return (cnt);
}

static void spell_bang(t_spell *x)
{
    /* need to somehow override a default bang-to-empty-list conversion... */
    loud_nomethod((t_pd *)x, &s_bang);  /* CHECKED */
}

static void spell_float(t_spell *x, t_float f)
{
    int i;
    if (loud_checkint((t_pd *)x, f, &i, &s_float))  /* CHECKED */
    {
	char buf[16];
	sprintf(buf, "%d", i);  /* CHECKED (negative numbers) */
	spell_out(x, buf, 1);
    }
}

/* CHECKED: 'symbol' selector is not spelled! */
static void spell_symbol(t_spell *x, t_symbol *s)
{
    spell_out(x, s->s_name, 1);
}

static void spell_list(t_spell *x, t_symbol *s, int ac, t_atom *av)
{
    int cnt = 0;
    int addsep = 0;
    while (ac--)
    {
	if (addsep)
	{
	    outlet_float(((t_object *)x)->ob_outlet, x->x_padchar);
	    cnt++;
	}
	else addsep = 1;
	if (av->a_type == A_FLOAT)
	{
	    int i;
	    /* CHECKME */
	    if (loud_checkint((t_pd *)x, av->a_w.w_float, &i, &s_list))
	    {
		char buf[16];
		sprintf(buf, "%d", i);  /* CHECKED (negative numbers) */
		cnt += spell_out(x, buf, 0);
	    }
	    /* CHECKED: floats as empty strings (separator is added) */
	}
	/* CHECKED: symbols as empty strings (separator is added) */
	av++;
    }
    if (cnt)  /* CHECKED: empty list is silently ignored */
	spell_fill(x, cnt);
}

static void spell_anything(t_spell *x, t_symbol *s, int ac, t_atom *av)
{
    int cnt = 0;
    int addsep = 0;
    if (s)
    {
	cnt += spell_out(x, s->s_name, 0);
	addsep = 1;
    }
    while (ac--)
    {
	if (addsep)
	{
	    outlet_float(((t_object *)x)->ob_outlet, x->x_padchar);
	    cnt++;
	}
	else addsep = 1;
	if (av->a_type == A_FLOAT)
	{
	    int i;
	    /* CHECKME */
	    if (loud_checkint((t_pd *)x, av->a_w.w_float, &i, &s_list))
	    {
		char buf[16];
		sprintf(buf, "%d", i);  /* CHECKED (negative numbers) */
		cnt += spell_out(x, buf, 0);
	    }
	    /* CHECKED: floats as empty strings (separator is added) */
	}
	else if (av->a_type == A_SYMBOL && av->a_w.w_symbol)
	    cnt += spell_out(x, av->a_w.w_symbol->s_name, 0);
	av++;
    }
    if (cnt)  /* CHECKED: empty list is silently ignored */
	spell_fill(x, cnt);
}

static void *spell_new(t_floatarg f1, t_floatarg f2)
{
    t_spell *x = (t_spell *)pd_new(spell_class);
    int i2 = (int)f2;  /* CHECKED */
    x->x_minsize = (f1 > 0 ? (int)f1 : 0);
    x->x_padchar = (i2 < 0 ? 0 : (i2 > 0 ? i2 : ' '));  /* CHECKED */
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void spell_setup(void)
{
    spell_class = class_new(gensym("spell"),
			    (t_newmethod)spell_new, 0,
			    sizeof(t_spell), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(spell_class, spell_bang);
    class_addfloat(spell_class, spell_float);
    class_addsymbol(spell_class, spell_symbol);
    class_addlist(spell_class, spell_list);
    class_addanything(spell_class, spell_anything);
}
