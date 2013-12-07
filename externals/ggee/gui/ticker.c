#include <m_pd.h>
#include "g_canvas.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <stdio.h>
#include "fatom.h"

/* can we use the normal text save function ?? */

static t_class *ticker_class;

static void ticker_save(t_gobj *z, t_binbuf *b)
{

    t_fatom *x = (t_fatom *)z;

    binbuf_addv(b, "ssiiss", gensym("#X"), gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix ,  
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_text,x->x_max,x->x_width);
    binbuf_addv(b, ";");
}

static void ticker_bang(t_fatom* x)
{
  x->x_val = !x->x_val;
  fatom_float(x,x->x_val);
}

static void *ticker_new(t_symbol* t)
{
    t_fatom *x = (t_fatom *)pd_new(ticker_class);
    x->x_type = gensym("checkbutton");
    return fatom_new(x,10,0,0,t);
}


t_widgetbehavior   ticker_widgetbehavior;


void ticker_setup() {
    ticker_class = class_new(gensym("ticker"), (t_newmethod)ticker_new, 0,
				sizeof(t_fatom),0,A_DEFSYMBOL,0);

    class_addbang(ticker_class,ticker_bang);
    fatom_setup_common(ticker_class);
    class_addbang(ticker_class, (t_method)ticker_bang);

    ticker_widgetbehavior.w_getrectfn=  fatom_getrect;
    ticker_widgetbehavior.w_displacefn= fatom_displace;
    ticker_widgetbehavior.w_selectfn=  fatom_select;
    ticker_widgetbehavior.w_activatefn= fatom_activate;
    ticker_widgetbehavior.w_deletefn=   fatom_delete;
    ticker_widgetbehavior.w_visfn=     fatom_vis;
#if PD_MINOR_VERSION < 37
    ticker_widgetbehavior.w_savefn=    ticker_save;
    ticker_widgetbehavior.w_propertiesfn= NULL;
#endif
    ticker_widgetbehavior.w_clickfn=   NULL;


	class_setwidget(ticker_class,&ticker_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(ticker_class,&ticker_save);
#endif
}

