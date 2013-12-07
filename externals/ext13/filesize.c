#include "ext13.h"
#include "m_pd.h"
#include <sys/stat.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* -------------------------- filesize ------------------------------ */
static t_class *filesize_class;

typedef struct _filesize
{
    t_object x_obj;
    t_float x_f;
    t_symbol *x_s;
} t_filesize;


static void *filesize_new(t_symbol *s)
{
    t_filesize *x = (t_filesize *)pd_new(filesize_class);
    x->x_s = s;
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void filesize_bang(t_filesize *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_f);
}

static void filesize_symbol(t_filesize *x, t_symbol *s)
{
   struct stat statbuf;
   int ok=(stat(s->s_name, &statbuf) >= 0);
   if (ok>0) outlet_float(x->x_obj.ob_outlet,x->x_f=statbuf.st_size);
   else post ("filesize:file not found");
}

void filesize_setup(void)
{
    filesize_class = class_new(gensym("filesize"), (t_newmethod)filesize_new, 0,
    	sizeof(t_filesize), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)filesize_new, gensym("fsize"), A_DEFFLOAT, 0);
    class_addbang(filesize_class, filesize_bang);
    class_addsymbol(filesize_class, filesize_symbol);        
}
