/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>


static t_class *serial_ms_class;


typedef struct _serial_ms
{
     t_object   x_obj;
     char       x_c[4];
     t_int      x_count;
     t_int      x_posx;
     t_int      x_posy;
     t_outlet *x_out2;
} t_serial_ms;


static void serial_ms_reset( t_serial_ms* x)
{
     x->x_posx=0;
     x->x_posy=0;
     x->x_count = 0;
     outlet_float(x->x_obj.ob_outlet, x->x_posx);
     outlet_float(x->x_out2, x->x_posy);
}

static void serial_ms_init( t_serial_ms* x)
{
     t_atom  cmd[8];

     SETFLOAT(cmd,0.);
     SETSYMBOL(cmd+1,gensym("CLOCAL"));
     SETSYMBOL(cmd+2,gensym("CREAD"));
     SETSYMBOL(cmd+3,gensym("CS7"));
/*     SETSYMBOL(cmd+4,gensym("HUPCL")); */
     outlet_anything(x->x_out2,gensym("setcontrol"),4,cmd);

     SETFLOAT(cmd,0.);
     SETSYMBOL(cmd+1,gensym("IGNBRK"));
     SETSYMBOL(cmd+2,gensym("IGNPAR"));
     outlet_anything(x->x_out2,gensym("setinput"),3,cmd);

     SETFLOAT(cmd,0.);
     SETFLOAT(cmd+1,1.);
     outlet_anything(x->x_out2,gensym("vtime"),2,cmd);

     SETFLOAT(cmd,1200.);
     outlet_anything(x->x_out2,gensym("speed"),1,cmd);
     
     SETSYMBOL(cmd,gensym("*n"));
     outlet_anything(x->x_out2,gensym("send"),1,cmd);


     SETSYMBOL(cmd,gensym("N"));
     outlet_anything(x->x_out2,gensym("send"),1,cmd);
     

}



static void serial_ms_float( t_serial_ms* x,t_floatarg f)
{
     int dx,dy;
     t_atom at[2];

     x->x_c[x->x_count] = (char) f;

     x->x_count = (++x->x_count)%3;
     
     if (x->x_count==2) {
	  dx=      (signed char)(((x->x_c[0] & 0x03) << 6) | 
				 (x->x_c[1] & 0x3F));
	  dy=      (signed char)(((x->x_c[0] & 0x0C) << 4) | 
				 (x->x_c[2] & 0x3F));
	  x->x_posx += dx;
	  x->x_posy += dy;
/*	  post("posx %d, posy %d",x->x_posx,x->x_posy);*/

	  SETFLOAT(at,x->x_posx);
	  SETFLOAT(at+1,x->x_posy);
	  outlet_list(x->x_obj.ob_outlet,&s_list, 2, at);
     }


}


static void *serial_ms_new(t_symbol *s)
{
     t_serial_ms *x = (t_serial_ms *)pd_new(serial_ms_class);
     
     x->x_count = 0;
     x->x_posx = 0;
     x->x_posy = 0;


     outlet_new(&x->x_obj, &s_float);
     x->x_out2 = outlet_new(&x->x_obj, &s_float);
     
     return x;
}


void serial_ms_setup(void)
{
    serial_ms_class = class_new(gensym("serial_ms"), (t_newmethod)serial_ms_new, 
			     NULL,
			     sizeof(t_serial_ms), 0,A_DEFSYM,0);
    class_addfloat(serial_ms_class,serial_ms_float);
    class_addmethod(serial_ms_class,(t_method) serial_ms_reset,gensym("reset"),0);
    class_addmethod(serial_ms_class, (t_method)serial_ms_init, gensym("init"),0);


}


