/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <string.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ stripdir ----------------------------- */

static t_class *stripdir_class;


typedef struct _stripdir
{
     t_object x_obj;
} t_stripdir;


void stripdir_symbol(t_stripdir *x,t_symbol* s)
{
     int len = strlen(s->s_name);

     while (len--)
	  if (*(s->s_name + len) == '/') {
	       outlet_symbol(x->x_obj.ob_outlet,gensym(s->s_name + len + 1));
	       break;
	  }

}

static void *stripdir_new()
{
    t_stripdir *x = (t_stripdir *)pd_new(stripdir_class);
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void stripdir_setup(void)
{
    stripdir_class = class_new(gensym("stripdir"), (t_newmethod)stripdir_new, 0,
				sizeof(t_stripdir), 0,0);
    class_addsymbol(stripdir_class,stripdir_symbol);
}


