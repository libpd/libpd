#ifndef VERSION
#define VERSION "0.25"
#endif

#include <m_pd.h>


#ifndef __DATE__ 
#define __DATE__ "without using a gnu compiler"
#endif

typedef struct _pdogg
{
     t_object x_obj;
} t_pdogg;

static t_class* pdogg_class;

// tilde objects
void oggamp_tilde_setup();
void oggcast_tilde_setup();
void oggread_tilde_setup();
void oggwrite_tilde_setup();

static void* pdogg_new(t_symbol* s) {
    t_pdogg *x = (t_pdogg *)pd_new(pdogg_class);
    return (x);
}

void pdogg_setup(void) 
{
    pdogg_class = class_new(gensym("pdogg"), (t_newmethod)pdogg_new, 0,
    	sizeof(t_pdogg), 0,0);

     oggamp_tilde_setup();
     oggcast_tilde_setup();
     oggread_tilde_setup();
     oggwrite_tilde_setup();

     post("\n       pdogg :: Ogg Vorbis library for pure-data");
     post("       written by Olaf Matthes <olaf.matthes@gmx.de>");
     post("       version: "VERSION);
     post("       compiled: "__DATE__", using Ogg Vorbis library 1.0");
     post("       home: http://www.akustische-kunst.org/puredata/pdogg/");
     post("       including: oggamp~0.2f, oggcast~0.2h, oggread~0.2c, oggwrite~0.1c\n");
}
