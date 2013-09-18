/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <string.h>

#define DEBUG(x) 
/*#define DEBUG(x) x*/

static t_class *serial_bird_class;


#define BIRD_DATA_START 0x80

#define BIRDCMD_MODE_POS 86
#define BIRD_BYTES_POS 6

#define BIRDCMD_MODE_POSANG 89
#define BIRD_BYTES_POSANG 12

#define BIRDCMD_MODE_POSMAT 90
#define BIRD_BYTES_POSMAT 24

#define BIRDCMD_MODE_POSQUAT 93
#define BIRD_BYTES_POSQUAT 14

#define BIRDCMD_MODE_QUAT 92
#define BIRD_BYTES_QUAT 8

#define BIRDCMD_STREAM 64
#define BIRDCMD_POINT 66

#define BIRD_GETDATA(x,y) ((float)((short)(y<<9 | x<<2)))

#define MAXBUFFER   32



typedef struct _serial_bird
{
     t_object     x_obj;
     char         x_c[MAXBUFFER];
     t_int        x_dataformat;
     t_int        x_maxcount;
     t_int        x_count;
     t_float      x_posx;
     t_float      x_posy;
     t_float      x_posz;
     t_outlet    *x_out2;
} t_serial_bird;


static void serial_bird_reset( t_serial_bird* x)
{
     x->x_posx=0;
     x->x_posy=0;
     x->x_count = 0;
     outlet_float(x->x_obj.ob_outlet, x->x_posx);
     outlet_float(x->x_out2, x->x_posy);
}

static void serial_bird_float( t_serial_bird* x,t_floatarg f)
{
     unsigned char c = (unsigned char) f;
     t_atom  at[BIRD_BYTES_POSMAT];
     t_int ac = 0;

     if (c&BIRD_DATA_START) {
	  x->x_count=0;
	  x->x_c[x->x_count] = c & 0x7f;
     }
     else
	  x->x_c[x->x_count] = c;

     DEBUG(post("data %d in = %x, start = %d",x->x_count,c,c&BIRD_DATA_START);)
     
     if (x->x_count == x->x_maxcount-1) {
	  switch (x->x_dataformat) {
	  case BIRDCMD_MODE_POS:
	       ac = 3;
	       SETFLOAT(&at[0], 0.25*BIRD_GETDATA(x->x_c[0],x->x_c[1]));
	       SETFLOAT(&at[1], 0.25*BIRD_GETDATA(x->x_c[2],x->x_c[3]));
	       SETFLOAT(&at[2], 0.25*BIRD_GETDATA(x->x_c[4],x->x_c[5]));
	       break;
	  case BIRDCMD_MODE_POSANG:
	       ac = 6;
	       SETFLOAT(&at[0], 0.25*BIRD_GETDATA(x->x_c[0],x->x_c[1]));
	       SETFLOAT(&at[1], 0.25*BIRD_GETDATA(x->x_c[2],x->x_c[3]));
	       SETFLOAT(&at[2], 0.25*BIRD_GETDATA(x->x_c[4],x->x_c[5]));
	       SETFLOAT(&at[3], BIRD_GETDATA(x->x_c[6],x->x_c[7]));
	       SETFLOAT(&at[4], BIRD_GETDATA(x->x_c[8],x->x_c[9]));
	       SETFLOAT(&at[5], BIRD_GETDATA(x->x_c[10],x->x_c[11]));
	       break;
	  case BIRDCMD_MODE_POSMAT:
	       ac = 12;
	       SETFLOAT(&at[0], BIRD_GETDATA(x->x_c[0],x->x_c[1]));
	       SETFLOAT(&at[1], BIRD_GETDATA(x->x_c[2],x->x_c[3]));
	       SETFLOAT(&at[2], BIRD_GETDATA(x->x_c[4],x->x_c[5]));
	       SETFLOAT(&at[3], BIRD_GETDATA(x->x_c[6],x->x_c[7]));
	       SETFLOAT(&at[4], BIRD_GETDATA(x->x_c[8],x->x_c[9]));
	       SETFLOAT(&at[5], BIRD_GETDATA(x->x_c[10],x->x_c[11]));
	       SETFLOAT(&at[6], BIRD_GETDATA(x->x_c[12],x->x_c[13]));
	       SETFLOAT(&at[7], BIRD_GETDATA(x->x_c[14],x->x_c[15]));
	       SETFLOAT(&at[8], BIRD_GETDATA(x->x_c[16],x->x_c[17]));
	       SETFLOAT(&at[9], BIRD_GETDATA(x->x_c[18],x->x_c[19]));
	       SETFLOAT(&at[10], BIRD_GETDATA(x->x_c[20],x->x_c[21]));
	       SETFLOAT(&at[11], BIRD_GETDATA(x->x_c[22],x->x_c[23]));
	       break;
	  case BIRDCMD_MODE_POSQUAT:
	       ac = 7;
	       SETFLOAT(&at[0], BIRD_GETDATA(x->x_c[0],x->x_c[1]));
	       SETFLOAT(&at[1], BIRD_GETDATA(x->x_c[2],x->x_c[3]));
	       SETFLOAT(&at[2], BIRD_GETDATA(x->x_c[4],x->x_c[5]));
	       SETFLOAT(&at[3], BIRD_GETDATA(x->x_c[6],x->x_c[7]));
	       SETFLOAT(&at[4], BIRD_GETDATA(x->x_c[8],x->x_c[9]));
	       SETFLOAT(&at[5], BIRD_GETDATA(x->x_c[10],x->x_c[11]));
	       SETFLOAT(&at[6], BIRD_GETDATA(x->x_c[12],x->x_c[13]));
	       break;
	  case BIRDCMD_MODE_QUAT:
	       ac = 4;
	       SETFLOAT(&at[0], BIRD_GETDATA(x->x_c[0],x->x_c[1]));
	       SETFLOAT(&at[1], BIRD_GETDATA(x->x_c[2],x->x_c[3]));
	       SETFLOAT(&at[2], BIRD_GETDATA(x->x_c[4],x->x_c[5]));
	       SETFLOAT(&at[3], BIRD_GETDATA(x->x_c[6],x->x_c[7]));
	       break;
	  }

/*	  post("posx %d, posy %d",x->x_posx,x->x_posy);*/
	  outlet_list(x->x_obj.ob_outlet,&s_list, ac, at);
     }

     x->x_count = (++x->x_count)%(x->x_maxcount);
}

static void serial_bird_poll( t_serial_bird* x) 
{
     post("poll");
     /*     outlet_float(x->x_out2,(float)x->x_dataformat);*/
     outlet_float(x->x_out2,(float)BIRDCMD_POINT);
}

static void serial_bird_mode( t_serial_bird* x,t_symbol* s)
{
     post("mode");
     /*     outlet_float(x->x_out2,(float)x->x_dataformat);*/


     if (!strcmp(s->s_name,"position")) {
       x->x_dataformat = BIRDCMD_MODE_POS;
       x->x_maxcount = BIRD_BYTES_POS;
     }

     if (!strcmp(s->s_name,"positionangle")) {
       x->x_dataformat = BIRDCMD_MODE_POSANG;
       x->x_maxcount = BIRD_BYTES_POSANG;
     }

     if (!strcmp(s->s_name,"positionmatrix")) {
       x->x_dataformat = BIRDCMD_MODE_POSMAT;
       x->x_maxcount = BIRD_BYTES_POSMAT;
     }

     if (!strcmp(s->s_name,"positionquat")) {
       x->x_dataformat = BIRDCMD_MODE_POSQUAT;
       x->x_maxcount = BIRD_BYTES_POSQUAT;
     }

     if (!strcmp(s->s_name,"quaternion")) {
       x->x_dataformat = BIRDCMD_MODE_QUAT;
       x->x_maxcount = BIRD_BYTES_QUAT;
     }

     outlet_float(x->x_out2,(float)x->x_dataformat);
} 

static void serial_bird_init( t_serial_bird* x) 
{
  t_atom  cmd[8];

  SETFLOAT(cmd,14400.);
  outlet_anything(x->x_out2,gensym("speed"),1,cmd);


  SETFLOAT(cmd,0.);
  SETSYMBOL(cmd+1,gensym("CLOCAL"));
  SETSYMBOL(cmd+2,gensym("CREAD"));
  SETSYMBOL(cmd+3,gensym("CS8"));
  outlet_anything(x->x_out2,gensym("setcontrol"),4,cmd);

  SETFLOAT(cmd,0.);
  SETSYMBOL(cmd+1,gensym("IXOFF"));
  outlet_anything(x->x_out2,gensym("setinput"),2,cmd);

  SETFLOAT(cmd,0.);
  outlet_anything(x->x_out2,gensym("setlocal"),1,cmd);

  SETFLOAT(cmd,0.);
  SETFLOAT(cmd+1,20.);
  outlet_anything(x->x_out2,gensym("vtime"),2,cmd);


  SETSYMBOL(cmd,gensym("RTS"));
  SETFLOAT(cmd+1,0.);
  outlet_anything(x->x_out2,gensym("setlines"),2,cmd);

  SETSYMBOL(cmd,gensym("DTR"));
  SETFLOAT(cmd+1,1.);
  outlet_anything(x->x_out2,gensym("setlines"),2,cmd);


  /* start the polling on the serial device immediately */

  outlet_anything(x->x_out2,gensym("start"),0,cmd);

}

static void serial_bird_start( t_serial_bird* x) 
{
     post("start");
     /*     outlet_float(x->x_out2,(float)x->x_dataformat);*/
     outlet_float(x->x_out2,(float)BIRDCMD_STREAM);
}


static void serial_bird_stop( t_serial_bird* x)
{
     post("stop");
     outlet_float(x->x_out2,(float)BIRDCMD_POINT);
}

static void *serial_bird_new(t_symbol *s)
{
     t_serial_bird *x = (t_serial_bird *)pd_new(serial_bird_class);
     
     x->x_count = 0;
     x->x_posx = 0;
     x->x_posy = 0;
     x->x_dataformat = BIRDCMD_MODE_POSANG;
     x->x_maxcount = BIRD_BYTES_POSANG;


     outlet_new(&x->x_obj, &s_float);
     x->x_out2 = outlet_new(&x->x_obj, &s_float);
     
     return x;
}


void serial_bird_setup(void)
{
    serial_bird_class = class_new(gensym("serial_bird"), (t_newmethod)serial_bird_new, 
			     NULL,
			     sizeof(t_serial_bird), 0,A_DEFSYM,0);
    class_addfloat(serial_bird_class,serial_bird_float);
    class_addmethod(serial_bird_class,(t_method) serial_bird_reset,gensym("reset"),0);
    class_addmethod(serial_bird_class,(t_method) serial_bird_init,gensym("init"),0);


    class_addmethod(serial_bird_class,(t_method) serial_bird_start,gensym("start"),0);
    class_addmethod(serial_bird_class,(t_method) serial_bird_stop,gensym("stop"),0);
    class_addmethod(serial_bird_class,(t_method) serial_bird_poll,gensym("poll"),0);
    class_addmethod(serial_bird_class,(t_method) serial_bird_mode,gensym("mode"),A_SYMBOL,NULL);
}


