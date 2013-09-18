#include "m_pd.h"
#include <math.h>

#include <string.h>
#include <stdio.h>

/* ----------------------- hex2dec --------------------- */

static t_class *hex2dec_class;

typedef struct _hex2dec
{
    t_object x_obj;
    t_symbol *x_format;
} t_hex2dec;

static void *hex2dec_new(t_symbol *s)
{
    t_hex2dec *x = (t_hex2dec *)pd_new(hex2dec_class);
    if (!s->s_name) s = gensym("file.%d");
    outlet_new(&x->x_obj, &s_symbol);
    x->x_format = s;
    return (x);
}

static void hex2dec_float(t_hex2dec *x, t_floatarg f)
{
    char buf[MAXPDSTRING];
    sprintf(buf, x->x_format->s_name, (int)f);
    outlet_symbol(x->x_obj.ob_outlet, gensym(buf));
}

static void hex2dec_symbol(t_hex2dec *x, t_symbol *s)
{
    char buf[MAXPDSTRING];
    sprintf(buf, x->x_format->s_name, s->s_name);
    outlet_symbol(x->x_obj.ob_outlet, gensym(buf));
}

void hex2dec_setup(void)
{
    hex2dec_class = class_new(gensym("hex2dec"),
    (t_newmethod)hex2dec_new, 0,
    	sizeof(t_hex2dec), 0, A_DEFSYM, 0);
    class_addfloat(hex2dec_class, hex2dec_float);
    class_addsymbol(hex2dec_class, hex2dec_symbol);
}
