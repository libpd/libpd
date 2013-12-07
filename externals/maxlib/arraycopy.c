/* ------------------------- arraycopy  --------------------------------------- */
/*                                                                              */
/* Copy data from one array to another .                                        */
/* Written by Olaf Matthes <olaf.matthes@gmx.de>                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include <stdlib.h>

static char *version = "arraycopy v0.2.1, written by Olaf Matthes <olaf.matthes@gmx.de>";

typedef struct arraycopy
{
	t_object x_obj;		
	t_symbol *x_destarray;
	t_symbol *x_sourcearray;
	t_garray *x_destbuf;
	t_garray *x_sourcebuf;
	t_int    x_start;
	t_int    x_end;
	t_int    x_pos;
	short    x_print;
} t_arraycopy;

	/* choose the destination array to copy to */
static void arraycopy_setdestarray(t_arraycopy *x, t_symbol *s)
{
	t_garray *b;
	
	if ((b = (t_garray *)pd_findbyclass(s, garray_class)))
	{
		// post("arraycopy: destination array set to \"%s\"", s->s_name);
		x->x_destbuf = b;
	} else {
		post("arraycopy: no array \"%s\" (error %d)", s->s_name, b);
		x->x_destbuf = 0;
	}
}

static void arraycopy_setdest(t_arraycopy *x, t_symbol *s)
{
		x->x_destarray = s;
		arraycopy_setdestarray(x, x->x_destarray);
}

	/* choose the source array to copy from */
static void arraycopy_setsourcearray(t_arraycopy *x, t_symbol *s)
{
	t_garray *b;
	
	if ((b = (t_garray *)pd_findbyclass(s, garray_class)))
	{
		// post("arraycopy: source array set to \"%s\"", s->s_name);
		x->x_sourcebuf = b;
	} else {
		post("arraycopy: no array \"%s\" (error %d)", s->s_name, b);
		x->x_sourcebuf = 0;
	}
}

	/* this is the routine that actually does the copying */
	/* get's called directly when we get a 'bang' */
static void arraycopy_docopy(t_arraycopy *x)
{
  /* use new 64-bit compatible array API if available */
#if (defined PD_MAJOR_VERSION && defined PD_MINOR_VERSION) && (PD_MAJOR_VERSION > 0 || PD_MINOR_VERSION >= 41)
# define arraynumber_t t_word
# define array_getarray garray_getfloatwords
# define array_get(pointer, index) (pointer[index].w_float)
# define array_set(pointer, index, value) ((pointer[index].w_float)=value)
#else
# define arraynumber_t t_float
# define array_getarray garray_getfloatarray
# define array_get(pointer, index) (pointer[index])
# define array_set(pointer, index, value) ((pointer[index])=value)
#endif

	t_garray *b;		/* make local copy of array */
	arraynumber_t *tab;  /* the content itself */
	int sourcesize, destsize;
	t_int i;
	t_garray *A;
	arraynumber_t *vec;

	if(!x->x_destarray)
	{
		post("arraycopy: no destination array specified");
		return;
	}
	if(!x->x_sourcearray)
	{
		post("arraycopy: no source array specified");
		return;
	}

	A = x->x_destbuf;

	if ((b = (t_garray *)pd_findbyclass(x->x_sourcearray, garray_class)))
	{
		// post("arraycopy: source array set to \"%s\"", x->x_sourcearray->s_name);
	} else {
		post("arraycopy: no array \"%s\" (error %d)", x->x_sourcearray->s_name, b);
		return;
	}

		// read from our array
	if (!array_getarray(b, &sourcesize, &tab))
	{
		pd_error(x, "arraycopy: couldn't read from source array '%s'!",
                 x->x_sourcearray->s_name);
		return;
	}

	if (!(A = (t_garray *)pd_findbyclass(x->x_destarray, garray_class)))
		error("arraycopy: %s: no such array", x->x_destarray->s_name);
	else if (!array_getarray(A, &destsize, &vec))
		error("arraycopy: %s: bad template ", x->x_destarray->s_name);
	else
	{
		if(x->x_start > sourcesize)
		{
			pd_error(x, "arraycopy: start point %i out of range for source '%s'",
                     (int)x->x_start, x->x_sourcearray->s_name);
			return;
		}
		if(x->x_start > destsize)
		{
			pd_error(x, "arraycopy: start point %i out of range for destination '%s'",
                     (int)x->x_start, x->x_destarray->s_name);
			return;
		}
		if(x->x_end)	// end point is specified
		{
            if(x->x_end > sourcesize)
            {
                logpost(x, 2, "arraycopy: end point %i out of range for source '%s', using %i",
                        (int)x->x_end, x->x_sourcearray->s_name, sourcesize);
                x->x_end = sourcesize;
            }
            if(x->x_end > destsize)
            {
                logpost(x, 2, "arraycopy: end point %i out of range for destination '%s', using %i",
                        (int)x->x_end, x->x_destarray->s_name, destsize);
                x->x_end = destsize;
			}
		}
        else
            x->x_end = (sourcesize < destsize ? sourcesize : destsize);

		if(x->x_pos)
			vec += x->x_pos;

		for(i = x->x_start; i < x->x_end; i++)
		{
            array_set(vec, 0, array_get(tab, i));
			vec++;
		}
		garray_redraw(A);
		if(x->x_print)post("arraycopy: copied %d values from array \"%s\" to array \"%s\"", 
					        x->x_end-x->x_start, x->x_sourcearray->s_name, x->x_destarray->s_name);
	}
}

static void arraycopy_list(t_arraycopy *x, t_symbol *s, int argc, t_atom *argv)
{
	if(argc > 1) {
		x->x_sourcearray = atom_getsymbolarg(0, argc, argv);
		x->x_destarray   = atom_getsymbolarg(1, argc, argv);
	}
}

static void arraycopy_source(t_arraycopy *x, t_symbol *s)
{
	x->x_sourcearray = s;
	x->x_start = x->x_end = x->x_pos = 0;
	arraycopy_docopy(x);
}

static void arraycopy_print(t_arraycopy *x, t_floatarg f)
{
	if(f)
		x->x_print = 1;
	else
		x->x_print = 0;
}

static void arraycopy_copy(t_arraycopy *x, t_symbol *s, int argc, t_atom *argv)
{
	if(argc == 1)		// source array name supplied
	{
		x->x_sourcearray = atom_getsymbolarg(0, argc, argv);
		x->x_start = x->x_end = x->x_pos = 0;
	}
	else if(argc == 2)	// array name and start point supplied
	{
		x->x_sourcearray = atom_getsymbolarg(0, argc, argv);
		x->x_start = atom_getfloatarg(1, argc, argv);
		x->x_end = x->x_pos = 0;
	}
	else if(argc == 3)	// arrayname and start & end point supplied
	{
		x->x_sourcearray = atom_getsymbolarg(0, argc, argv);
		x->x_start = atom_getfloatarg(1, argc, argv);
		if(argv[2].a_type == A_FLOAT)	// real position
		{
			x->x_end = atom_getfloatarg(2, argc, argv);
		}
		else	// offset given
		{
			t_symbol *offset = atom_getsymbolarg(2, argc, argv);
			x->x_end = (t_int)atoi(offset->s_name) + x->x_start;
		}
		x->x_pos = 0;
	}
	else if(argc == 4)	// as above & dest. array
	{
		x->x_sourcearray = atom_getsymbolarg(0, argc, argv);
		x->x_start = atom_getfloatarg(1, argc, argv);
		if(argv[2].a_type == A_FLOAT)	// real position
		{
			x->x_end = atom_getfloatarg(2, argc, argv);
		}
		else	// offset given
		{
			t_symbol *offset = atom_getsymbolarg(2, argc, argv);
			x->x_end = (t_int)atoi(offset->s_name) + x->x_start;
		}
		x->x_destarray = atom_getsymbolarg(3, argc, argv);
		arraycopy_setdestarray(x, x->x_destarray);
		x->x_pos = 0;
	}
	else if(argc == 5)	// as above & dest. array & pos. in dest.
	{
		x->x_sourcearray = atom_getsymbolarg(0, argc, argv);
		x->x_start = atom_getfloatarg(1, argc, argv);
		if(argv[2].a_type == A_FLOAT)	// real position
		{
			x->x_end = atom_getfloatarg(2, argc, argv);
		}
		else	// offset given
		{
			t_symbol *offset = atom_getsymbolarg(2, argc, argv);
			x->x_end = (t_int)atoi(offset->s_name) + x->x_start;
		}
		x->x_destarray = atom_getsymbolarg(3, argc, argv);
		arraycopy_setdestarray(x, x->x_destarray);
		x->x_pos = atom_getfloatarg(4, argc, argv);
	}
	else post("arraycopy: copy: wrong number of arguments");
	
	arraycopy_docopy(x);
}

static t_class *arraycopy_class;

static void *arraycopy_new(t_symbol *s, int argc, t_atom *argv)
{
    t_arraycopy *x = (t_arraycopy *)pd_new(arraycopy_class);

	if (argc > 0) {
		x->x_destarray = atom_getsymbolarg(0, argc, argv);
		arraycopy_setdestarray(x, x->x_destarray);
	}
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym("dest"));
	x->x_start = x->x_end = x->x_pos = x->x_print = 0;
	return (x);
}

#ifndef MAXLIB
void arraycopy_setup(void)
{
	/* the object's class: */
    arraycopy_class = class_new(gensym("arraycopy"), (t_newmethod)arraycopy_new,
    	0, sizeof(t_arraycopy), 0, A_GIMME, 0);
#else
void maxlib_arraycopy_setup(void)
{
	/* the object's class: */
    arraycopy_class = class_new(gensym("maxlib_arraycopy"), (t_newmethod)arraycopy_new,
    	0, sizeof(t_arraycopy), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)arraycopy_new, gensym("arraycopy"), A_GIMME, 0);
#endif
	class_addmethod(arraycopy_class, (t_method)arraycopy_copy, gensym("copy"), A_GIMME, 0);
	class_addmethod(arraycopy_class, (t_method)arraycopy_print, gensym("print"), A_FLOAT, 0);
	class_addmethod(arraycopy_class, (t_method)arraycopy_setdest, gensym("dest"), A_SYMBOL, 0);
    class_addsymbol(arraycopy_class, arraycopy_source);
	class_addbang(arraycopy_class, arraycopy_docopy);
	// class_addlist(arraycopy_class, arraycopy_list);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
    class_sethelpsymbol(arraycopy_class, gensym("maxlib/arraycopy-help.pd"));
#endif
}
