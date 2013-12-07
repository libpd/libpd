/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>

/* -------------------------- rtout -------------------------- */

void sys_putmidimess(int portno, int a, int b, int c);

static t_class *rtout_class;



typedef struct _rtout
{
    t_object x_obj;
    t_float x_rt;
    t_float x_channel;
} t_rtout;

static void *rtout_new(t_floatarg channel)
{
    t_rtout *x = (t_rtout *)pd_new(rtout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    return (x);
}

static void rtout_float(t_rtout *x, t_float f)
{
    int binchan = (int) x->x_channel - 1;
    sys_putmidimess((binchan>>4),(int) f,0,0);
}

void rtout_setup(void)
{
    rtout_class = class_new(gensym("rtout"), (t_newmethod)rtout_new, 0,
    	sizeof(t_rtout), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(rtout_class, rtout_float);
}

