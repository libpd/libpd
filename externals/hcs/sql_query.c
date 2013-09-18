/*
 * object for generating SQL queries with SQL ? placeholders
 * Written by Hans-Christoph Steiner <hans@eds.org>
 *
 * Copyright (c) 2007 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <m_pd.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#else
#include <stdlib.h>
#endif

#include <string.h>

#define DEBUG(x)
//#define DEBUG(x) x 

#define PLACEHOLDER  '?'

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *sql_query_class;

static t_class *proxy_inlet_class;

typedef struct _proxy_inlet 
{
  t_pd pd;
  void *owner;
  unsigned int id;
} t_proxy_inlet;

typedef struct _sql_query 
{
    t_object            x_obj;

    t_binbuf*           x_query_binbuf;   // binbuf for converting args to string
    
    t_proxy_inlet*      inlets;           // pointer to array of _proxy_inlets
    t_atom*             atoms;            // pointer to array of atoms
    unsigned int        placeholder_count;// number of items in above arrays

    t_outlet*           x_data_outlet;    // for list of data to plug into query
    t_outlet*           x_query_outlet;   // for SQL query
} t_sql_query;
    

/*------------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 */
static void sql_query_set_atom(t_sql_query *x, int atom_num, t_symbol *s, 
                               int argc, t_atom *a);

/*------------------------------------------------------------------------------
 * PROXY INLET FUNCTIONS
 */

static void proxy_inlet_new(t_proxy_inlet *p, t_object *owner, unsigned int id) 
{
	DEBUG(post("proxy_inlet_new"););
	p->pd = proxy_inlet_class;
	p->owner = owner;
    p->id = id;
    inlet_new(owner, &p->pd, 0, 0);
}

static void proxy_inlet_anything(t_proxy_inlet *p, t_symbol *s, int argc, t_atom *argv)
{
	DEBUG(post("proxy_inlet_anything"););
    sql_query_set_atom(p->owner, p->id, s, argc, argv);
}

static void proxy_inlet_setup(void) 
{
	post("proxy_inlet_setup");
	proxy_inlet_class = (t_class *)class_new(gensym("#__PROXY_INLET__"),
                                       0,
                                       0,
                                       sizeof(t_proxy_inlet),
                                       0,
                                       A_GIMME,
                                       0);
	class_addanything(proxy_inlet_class, (t_method)proxy_inlet_anything);
}

/*------------------------------------------------------------------------------
 * STANDARD CLASS FUNCTIONS
 */

static void sql_query_set_atom(t_sql_query *x, int atom_num, t_symbol *s, 
                               int argc, t_atom *a)
{
    DEBUG(post("sql_query_set_atom"););
    if( (s == &s_symbol) || (s == &s_list) || (s == &s_float) )
        x->atoms[atom_num] = *a;
    else
        SETSYMBOL(&x->atoms[atom_num], s);
}

static void sql_query_output(t_sql_query *x)
{
    DEBUG(post("sql_query_output"););
    int natom = binbuf_getnatom(x->x_query_binbuf);
    t_atom *vec = binbuf_getvec(x->x_query_binbuf);
    outlet_anything(x->x_query_outlet, vec[0].a_w.w_symbol, natom - 1, vec + 1);
    outlet_list(x->x_data_outlet, &s_list, x->placeholder_count, x->atoms);
}

static void sql_query_anything(t_sql_query *x, t_symbol *s, int argc, t_atom *argv) 
{
    sql_query_set_atom(x, 0, s, argc, argv);
    sql_query_output(x);
}

static void sql_query_free(t_sql_query *x) 
{
    binbuf_free(x->x_query_binbuf);
}
    
static void *sql_query_new(t_symbol *s, int argc, t_atom *argv) 
{
	DEBUG(post("sql_query_new"););
    unsigned int i;
    int bufsize;
    char *buf;
    char *current = NULL;
	t_sql_query *x = (t_sql_query *)pd_new(sql_query_class);

    x->x_query_binbuf = binbuf_new();
    binbuf_add(x->x_query_binbuf, argc, argv);
    binbuf_gettext(x->x_query_binbuf, &buf, &bufsize);
    buf[bufsize] = 0;

    x->placeholder_count = 0;
    current = strchr(buf, PLACEHOLDER);
    while (current != NULL)
    {
        x->placeholder_count++;
        current = strchr(current + 1, PLACEHOLDER);
    }
    
    x->inlets = getbytes(x->placeholder_count * sizeof(t_proxy_inlet));
    for(i = 1; i < x->placeholder_count; ++i)
        proxy_inlet_new(&x->inlets[i], (t_object *)x, i);
    
    x->atoms = getbytes(x->placeholder_count * sizeof(t_atom));
    for(i = 0; i < x->placeholder_count; ++i)
        SETSYMBOL(&x->atoms[i], &s_);

	x->x_data_outlet = outlet_new(&x->x_obj, 0);
	x->x_query_outlet = outlet_new(&x->x_obj, 0);

	return (x);
}

void sql_query_setup(void) 
{
	sql_query_class = class_new(gensym("sql_query"), 
                                (t_newmethod)sql_query_new, 
                                (t_method)sql_query_free, 
                                sizeof(t_sql_query), 
                                0, 
                                A_GIMME, 
                                0);

	/* add inlet datatype methods */
	class_addbang(sql_query_class, (t_method) sql_query_output);
	class_addanything(sql_query_class, (t_method) sql_query_anything);

    /* set up proxy inlet class */
    proxy_inlet_setup();
}

