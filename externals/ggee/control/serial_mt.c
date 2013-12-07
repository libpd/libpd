/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>


static t_class *serial_mt_class;


typedef struct _serial_mt
{
     t_object   x_obj;
     char       x_c[4];
     t_int      x_count;
     t_int      x_posx;
     t_int      x_posy;
     t_outlet *x_out2;
} t_serial_mt;


static void serial_mt_reset( t_serial_mt* x)
{
     x->x_posx=0;
     x->x_posy=0;
     x->x_count = 0;
}

static void serial_mt_float( t_serial_mt* x,t_floatarg f)
{
     int dx,dy;

     x->x_c[x->x_count] = (char) f;

     x->x_count = (++x->x_count)%3;
     
     if (x->x_count==2) {
	  dx=      (signed char)(((x->x_c[0] & 0x03) << 6) | 
				 (x->x_c[1] & 0x3F));
	  dy=      (signed char)(((x->x_c[0] & 0x0C) << 4) | 
				 (x->x_c[2] & 0x3F));
	  x->x_posx += dx;
	  x->x_posy += dy;
/*	  post("posx %d, posy %d",x->x_posx,x->x_posy); */
	  outlet_float(x->x_obj.ob_outlet, x->x_posx);
	  outlet_float(x->x_out2, x->x_posy);
     }


}


static void *serial_mt_new(t_symbol *s)
{
     t_serial_mt *x = (t_serial_mt *)pd_new(serial_mt_class);
     
     x->x_count = 0;
     x->x_posx = 0;
     x->x_posy = 0;


     outlet_new(&x->x_obj, &s_float);
     x->x_out2 = outlet_new(&x->x_obj, &s_float);
     
     return x;
}


void serial_mt_setup(void)
{
    serial_mt_class = class_new(gensym("serial_mt"), (t_newmethod)serial_mt_new, 
			     NULL,
			     sizeof(t_serial_mt), 0,A_DEFSYM,0);
    class_addfloat(serial_mt_class,serial_mt_float);
    class_addmethod(serial_mt_class,(t_method) serial_mt_reset,gensym("reset"),0);
}


