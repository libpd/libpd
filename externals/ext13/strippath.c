#include "ext13.h"
#include "m_pd.h"
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

/* -------------------------- strippath ------------------------------ */
static t_class *strippath_class;

typedef struct _strippath
{
    t_object x_obj;
    t_symbol *x_s;
    t_symbol *x_path;
} t_strippath;


static void *strippath_new(t_symbol *s)
{
    t_strippath *x = (t_strippath *)pd_new(strippath_class);
    x->x_s = s;
    x->x_path=gensym("./");
    symbolinlet_new(&x->x_obj, &x->x_path);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}



static void strippath_symbol(t_strippath *x, t_symbol *s)
{
      char *lastslash;
      char path[MAXPDSTRING], filename[MAXPDSTRING];
      x->x_s = s;
      lastslash=strrchr (s->s_name,'/');
      if (lastslash){
        strncpy (path,s->s_name,lastslash-s->s_name+1);
        path[lastslash-s->s_name]=0;
        strcpy (filename,lastslash+1);
//        filename[lastslash-s->s_name+1]=0;
      }
      else {
        strcpy (filename,s->s_name);
        strcpy (path,x->x_path->s_name);
      }  
      outlet_symbol(x->x_obj.ob_outlet,gensym(filename));
//      post ("path:%s , name:%s",path,filename);
}


static void strippath_bang(t_strippath *x)
{
  if (x->x_s){strippath_symbol (x,x->x_s);}
}

void strippath_setup(void)
{
    strippath_class = class_new(gensym("strippath"), (t_newmethod)strippath_new, 0,
    	sizeof(t_strippath), 0, A_DEFFLOAT, 0);
    class_addbang(strippath_class, strippath_bang);
    class_addsymbol(strippath_class, strippath_symbol);        
}
