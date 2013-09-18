/*
  wacom graphire on serial port only...
*/

#include <m_pd.h>
#include "s_stuff.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BUFSIZE 256
#define HEADER_BIT	0x80
#define ZAXIS_SIGN_BIT	0x40
#define ZAXIS_BIT    	0x04
#define ZAXIS_BITS    	0x3f
#define POINTER_BIT     0x20
#define PROXIMITY_BIT   0x40
#define BUTTON_FLAG	0x08
#define BUTTONS_BITS	0x78
#define TILT_SIGN_BIT	0x40
#define TILT_BITS	0x3f

/* defines to discriminate second side button and the eraser */
#define ERASER_PROX	4
#define OTHER_PROX	1

#define Threshold 1000
unsigned char data[7];

typedef struct _wac
{
    t_object t_ob;
    t_outlet *axis_out;
    t_outlet *button_out;
    t_symbol *file;
    int fd;
    int count;
    int oldbuttons;
    unsigned char data[BUFSIZE];
} t_wac;

t_class *wac_class;

void wac_setup(void);
static void wac_read(t_wac *x,int fd);
static void wac_process(t_wac *x);

static void wac_open(t_wac *x)
{
    if(x->fd>=0) return;

    x->fd = open (x->file->s_name, O_RDONLY | O_NONBLOCK);
    if(x->fd<0)
    {
        post("open (%s, O_RDONLY | O_NONBLOCK)",x->file->s_name);
        perror("open");
        return;
    }
    sys_addpollfn(x->fd,(t_fdpollfn)wac_read,(void *)x);
}

static void wac_close(t_wac *x)
{
    if(x->fd<0) return;

    sys_rmpollfn(x->fd);
    close(x->fd);
    x->fd=-1;
}

static void wac_float(t_wac *x,t_floatarg connect)
{
    if(connect!=0) wac_open(x);
    else wac_close(x);
}

static void *wac_new(t_symbol *file)
{
    t_wac *x = (t_wac *)pd_new(wac_class);

    //if(file->s_name)
    if(file!=&s_)
        x->file=file;
    else x->file=gensym("/dev/ttyS0");

    post("wac_new file=%s",x->file->s_name);
    x->axis_out = outlet_new(&x->t_ob, &s_list);
    x->button_out = outlet_new(&x->t_ob, &s_list);

    x->fd=-1;

    return (void *)x;
}

void wac_setup(void)
{
    wac_class = class_new(gensym("wac"),(t_newmethod)wac_new,
                          (t_method)wac_close, sizeof(t_wac), 0, A_DEFSYM, 0);
    class_addfloat(wac_class, wac_float);


}


static void wac_read(t_wac *x,int fd)
{
    int len,i=0;
    unsigned char b;
    unsigned char buffer[BUFSIZE];

    while((len=read(fd,buffer,BUFSIZE))> -1)
    {

        for(i=0; i<len; i++)
        {
            if(buffer[i]&128) x->count=0;
            x->data[x->count++]=buffer[i];
            if(x->count==7) wac_process(x);
        }
    }
}

static void wac_process(t_wac *X)
{
    int			is_stylus = 1, is_button, is_proximity, wheel=0;
    int			x, y, z, buttons, tx = 0, ty = 0;
    unsigned char *data=X->data;
    t_atom ats[3];

    is_stylus = (data[0] & POINTER_BIT);

    if(!is_stylus) return;

    x = (((data[0] & 0x3) << 14) +
         (data[1] << 7) +
         data[2]);
    y = (((data[3] & 0x3) << 14) +
         (data[4] << 7) +
         data[5]);


    z = ((data[6] & ZAXIS_BITS) * 2) +
        ((data[3] & ZAXIS_BIT) >> 2);

    //z = z*4 + ((data[0] & ZAXIS_BIT) >> 1);

    if (!(data[6] & ZAXIS_SIGN_BIT))
    {
        z += 128;
    }

    is_proximity = (data[0] & PROXIMITY_BIT);

    buttons = ((data[3] & 0x38) >> 3);
    /*if (is_stylus) {
        buttons = ((data[3] & 0x30) >> 3) |
    	(z >= Threshold ? 1 : 0);
    }
    else {
        buttons = (data[3] & 0x38) >> 3;

        wheel = (data[6] & 0x30) >> 4;

        if (data[6] & 0x40) {
    	wheel = -wheel;
        }
    }*/
    //is_button = (buttons != 0);
    if(buttons!=X->oldbuttons)
    {
        X->oldbuttons=buttons;

        SETFLOAT(&ats[0],buttons&1);
        SETFLOAT(&ats[1],(buttons&2)!=0);
        SETFLOAT(&ats[2],(buttons&4)!=0);
        outlet_list(X->button_out,0,3,ats);
    }
    SETFLOAT(&ats[0],x/5103.0);
    SETFLOAT(&ats[1],y/3711.0);
    SETFLOAT(&ats[2],z/256.0);
    outlet_list(X->axis_out,0,3,ats);
}
/* Format of 7 bytes data packet for Wacom Tablets
Byte 1
bit 7  Sync bit always 1
bit 6  Pointing device detected
bit 5  Cursor = 0 / Stylus = 1
bit 4  Reserved
bit 3  1 if a button on the pointing device has been pressed
bit 2  Reserved
bit 1  X15
bit 0  X14

Byte 2
bit 7  Always 0
bits 6-0 = X13 - X7

Byte 3
bit 7  Always 0
bits 6-0 = X6 - X0

Byte 4
bit 7  Always 0
bit 6  B3
bit 5  B2
bit 4  B1
bit 3  B0
bit 2  P0
bit 1  Y15
bit 0  Y14

Byte 5
bit 7  Always 0
bits 6-0 = Y13 - Y7

Byte 6
bit 7  Always 0
bits 6-0 = Y6 - Y0

Byte 7
bit 7 Always 0
bit 6  Sign of pressure data
bit 5  P6
bit 4  P5
bit 3  P4
bit 2  P3
bit 1  P2
bit 0  P1

byte 8 and 9 are optional and present only
in tilt mode.

Byte 8
bit 7 Always 0
bit 6 Sign of tilt X
bit 5  Xt6
bit 4  Xt5
bit 3  Xt4
bit 2  Xt3
bit 1  Xt2
bit 0  Xt1

Byte 9
bit 7 Always 0
bit 6 Sign of tilt Y
bit 5  Yt6
bit 4  Yt5
bit 3  Yt4
bit 2  Yt3
bit 1  Yt2
bit 0  Yt1

*/

