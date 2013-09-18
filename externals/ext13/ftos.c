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

/* -------------------------- ftos ------------------------------ */
static t_class *ftos_class;

typedef struct _ftos
{
    t_object x_obj;
    t_symbol *x_s;
    t_symbol *x_fmt;
    t_float  x_f;
} t_ftos;


static void *ftos_new(t_floatarg f)
{
    t_ftos *x = (t_ftos *)pd_new(ftos_class);
    x->x_f=f;
    floatinlet_new(&x->x_obj, &x->x_f);
    outlet_new(&x->x_obj,0);
    x->x_s = gensym("0");
    return (x);
}



static void ftos_bang(t_ftos *x)
{
   outlet_symbol(x->x_obj.ob_outlet,x->x_s); 
}

static void ftos_float(t_ftos *x, t_float f)
{
    char result[MAXPDSTRING];
    char fmt[MAXPDSTRING];                   
    char num[MAXPDSTRING];
/*    if (!f){f=2;}    */
    sprintf(num,"%d",(int)x->x_f);
    strcpy(fmt,"%.");        
    strcat(fmt,num); 
    strcat(fmt,"f");
    x->x_fmt = gensym (fmt);
    sprintf(result,x->x_fmt->s_name,(float)f);
    x->x_s = gensym(result);
    ftos_bang(x);
}



void ftos_setup(void)
{
    ftos_class = class_new(gensym("ftos"), (t_newmethod)ftos_new, 0,
    	sizeof(t_ftos), 0, A_DEFFLOAT, 0);
    class_addbang(ftos_class, ftos_bang);
    class_addfloat(ftos_class, ftos_float);        
/*    class_addfloat(ftos_class; ftos_float);*/
}
