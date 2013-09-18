#include "ext13.h"
#include "m_pd.h"
#include "m_imp.h"
/*
#ifndef PD_MAJOR_VERSION
#include "s_stuff.h"
#else 
#include "m_imp.h"
#endif*/

#include <sys/stat.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* -------------------------- openpatch ------------------------------ */
static t_class *openpatch_class;

typedef struct _openpatch
{
    t_object x_obj;
    t_symbol *x_s;
    t_symbol *x_path;
} t_openpatch;


static void *openpatch_new(t_symbol *s)
{
    t_openpatch *x = (t_openpatch *)pd_new(openpatch_class);
    x->x_s = s;
    x->x_path=gensym("./");
    symbolinlet_new(&x->x_obj, &x->x_path);
    return (x);
}



static void openpatch_symbol(t_openpatch *x, t_symbol *s)
{
      char *lastslash;
      char path[MAXPDSTRING], filename[MAXPDSTRING];
      x->x_s = s;
      lastslash=strrchr (s->s_name,'/');
      if (lastslash){
        strncpy (path,s->s_name,lastslash-s->s_name+1);
        path[lastslash-s->s_name]=0;
        strcpy (filename,lastslash+1);
        filename[lastslash-s->s_name+1]=0;
      }
      else {
        strcpy (filename,s->s_name);
        strcpy (path,x->x_path->s_name);
      }  
    post ("path:%s , name:%s",path,filename);
    glob_evalfile(0,gensym(filename),gensym(path));
}


static void openpatch_bang(t_openpatch *x)
{
  if (x->x_s){openpatch_symbol (x,x->x_s);}
}

void openpatch_setup(void)
{
    openpatch_class = class_new(gensym("openpatch"), (t_newmethod)openpatch_new, 0,
    	sizeof(t_openpatch), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)openpatch_new, gensym("opa"), A_DEFFLOAT, 0);
    class_addbang(openpatch_class, openpatch_bang);
    class_addsymbol(openpatch_class, openpatch_symbol);        
}
