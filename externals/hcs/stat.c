/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting file type (dir, link, exe, etc) using a filename       */
/* Written by Hans-Christoph Steiner <hans@eds.org>                         */
/*                                                                           */
/* Copyright (c) 2006 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <m_pd.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#else
#include <stdlib.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

static char *version = "$Revision: 1.5 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *stat_class;

#ifdef _WIN32
typedef struct _stat_win {
#else
typedef struct _stat {
#endif /* _WIN32 */
	t_object            x_obj;
	t_symbol            *x_filename;
/* output */
	t_atom              *output; // holder for a list of atoms to be outputted
	t_int               output_count;  // number of atoms in in x->output 
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_stat;



/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

/* add one new atom to the list to be outputted */
static void add_atom_to_output(t_stat *x, t_atom *new_atom)
{
    t_atom *new_atom_list;

	new_atom_list = (t_atom *)getbytes((x->output_count + 1) * sizeof(t_atom));
	memcpy(new_atom_list, x->output, x->output_count * sizeof(t_atom));
	freebytes(x->output, x->output_count * sizeof(t_atom));
	x->output = new_atom_list;
	memcpy(x->output + x->output_count, new_atom, sizeof(t_atom));
	++(x->output_count);
}

/*
static void add_symbol_to_output(t_stat *x, t_symbol *s)
{
	t_atom *temp_atom = getbytes(sizeof(t_atom));
	SETSYMBOL(temp_atom, s); 
	add_atom_to_output(x,temp_atom);
	freebytes(temp_atom,sizeof(t_atom));
}
*/
		
static void add_float_to_output(t_stat *x, t_float f)
{
	t_atom *temp_atom = getbytes(sizeof(t_atom));
	SETFLOAT(temp_atom, f);
	add_atom_to_output(x,temp_atom);
	freebytes(temp_atom,sizeof(t_atom));
}

static void reset_output(t_stat *x)
{
	if(x->output)
	{
		freebytes(x->output, x->output_count * sizeof(t_atom));
		x->output = NULL;
		x->output_count = 0;
	}
}

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void stat_output_error(t_stat *x)
{
	DEBUG(post("stat_output_error"););
	t_atom output_atoms[2];
	switch(errno)
	{
	case EACCES:
		error("[stat]: access denied: %s", x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("access_denied"));
		break;
	case EIO:
		error("[stat]: An error occured while reading %s", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("io_error"));
		break;
#ifndef _WIN32
	case ELOOP:
		error("[stat]: A loop exists in symbolic links in %s", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("symlink_loop"));
		break;
#endif
	case ENAMETOOLONG:
		error("[stat]: The filename %s is too long", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("name_too_long"));
		break;
	case ENOENT:
		error("[stat]: %s does not exist", x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("does_not_exist"));
		break;
	case ENOTDIR:
		error("[stat]: A component of %s is not a existing folder", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("not_folder"));
		break;
#ifndef _WIN32
	case EOVERFLOW:
		error("[stat]: %s caused overflow in stat struct", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("internal_overflow"));
		break;
#endif
	case EFAULT:
		error("[stat]: fault in stat struct (%s)", x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("internal_fault"));
		break;
	case EINVAL:
		error("[stat]: invalid argument to stat() (%s)", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("invalid"));
		break;
	default:
		error("[stat]: unknown error %d: %s", errno, x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("unknown"));
	}
	SETSYMBOL(output_atoms + 1, x->x_filename);
	outlet_anything(x->x_status_outlet, gensym("error"), 2, output_atoms);
}

static void stat_output(t_stat* x)
{
	DEBUG(post("stat_output"););
#ifdef _WIN32
	struct _stat stat_buffer;
#else
	struct stat stat_buffer;
#endif
	int result;

#ifdef _WIN32
	result = _stat(x->x_filename->s_name, &stat_buffer);
#else
	result = stat(x->x_filename->s_name, &stat_buffer);
#endif /* _WIN32 */
	if(result != 0)
	{
		stat_output_error(x);
	}
	else
	{
		reset_output(x);
		add_float_to_output(x, (t_float) stat_buffer.st_mode);
		add_float_to_output(x, (t_float) stat_buffer.st_nlink);
		add_float_to_output(x, (t_float) stat_buffer.st_uid);
		add_float_to_output(x, (t_float) stat_buffer.st_gid);
		add_float_to_output(x, (t_float) stat_buffer.st_rdev);
		add_float_to_output(x, (t_float) stat_buffer.st_size);
#ifdef _WIN32
		add_float_to_output(x, (t_float) 0);
		add_float_to_output(x, (t_float) 0);
#else
		add_float_to_output(x, (t_float) stat_buffer.st_blocks);
		add_float_to_output(x, (t_float) stat_buffer.st_blksize);
#endif
		/* 86400 seconds == 24 hours == 1 day */
#if defined(_POSIX_C_SOURCE) || defined(_WIN32)
		add_float_to_output(x, (t_float) (stat_buffer.st_atime / 86400));
		add_float_to_output(x, (t_float) (stat_buffer.st_atime % 86400));
		add_float_to_output(x, (t_float) (stat_buffer.st_mtime / 86400));
		add_float_to_output(x, (t_float) (stat_buffer.st_mtime % 86400));
		add_float_to_output(x, (t_float) (stat_buffer.st_ctime / 86400));
		add_float_to_output(x, (t_float) (stat_buffer.st_ctime % 86400));
#else
		add_float_to_output(x, 
				 (t_float) (stat_buffer.st_atimespec.tv_sec / 86400));
		add_float_to_output(x, 
				 (t_float) (stat_buffer.st_atimespec.tv_sec % 86400));
		add_float_to_output(x, 
				 (t_float) (stat_buffer.st_mtimespec.tv_sec / 86400));
		add_float_to_output(x, 
				 (t_float) (stat_buffer.st_mtimespec.tv_sec % 86400));
		add_float_to_output(x, 
				 (t_float) (stat_buffer.st_ctimespec.tv_sec / 86400));
		add_float_to_output(x, 
				 (t_float) (stat_buffer.st_ctimespec.tv_sec % 86400));
#endif /* _POSIX_C_SOURCE */
		outlet_anything(x->x_data_outlet,x->x_filename,
						x->output_count,x->output);
	}
}


static void stat_set(t_stat* x, t_symbol *s) 
{
	DEBUG(post("stat_set"););
#ifdef _WIN32
	char string_buffer[MAX_PATH];
	ExpandEnvironmentStrings(s->s_name, string_buffer, MAX_PATH);
	x->x_filename = gensym(string_buffer);
#else
	x->x_filename = s;
#endif	
}


static void stat_symbol(t_stat *x, t_symbol *s) 
{
   stat_set(x,s);
   stat_output(x);
}


static void *stat_new(t_symbol *s) 
{
	DEBUG(post("stat_new"););

	t_stat *x = (t_stat *)pd_new(stat_class);

    symbolinlet_new(&x->x_obj, &x->x_filename);
	x->x_data_outlet = outlet_new(&x->x_obj, 0);
	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	/* set to the value from the object argument, if that exists */
	if (s != &s_)
	{
		x->x_filename = s;
	}
	else
	{
		x->x_filename = canvas_getcurrentdir();
		post("setting pattern to default: %s",x->x_filename->s_name);
	}

	return (x);
}

void stat_setup(void) 
{
	DEBUG(post("stat_setup"););
	stat_class = class_new(gensym("stat"), 
								  (t_newmethod)stat_new, 
								  0,
								  sizeof(t_stat), 
								  0, 
								  A_DEFSYM, 
								  0);
	/* add inlet datatype methods */
	class_addbang(stat_class,(t_method) stat_output);
	class_addsymbol(stat_class,(t_method) stat_symbol);
	
	/* add inlet message methods */
	class_addmethod(stat_class,(t_method) stat_set,gensym("set"), 
					A_DEFSYM, 0);
    logpost(NULL, 4, "[stat] %s",version);  
    logpost(NULL, 4, "\twritten by Hans-Christoph Steiner <hans@eds.org>");
    logpost(NULL, 4, "\tcompiled on "__DATE__" at "__TIME__ " ");
}

