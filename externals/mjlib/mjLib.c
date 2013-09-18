#ifdef NT
#include "stdafx.h"
#endif
#include "mjLib.h"
#include "m_pd.h"
#include "things.h"

typedef struct _mjLib
{
     t_object x_obj;
} t_mjLib;

static t_class* mjLib_class;



static void* mjLib_new(t_symbol* s) {
    t_mjLib *x = (t_mjLib *)pd_new( mjLib_class);
    return (x);
}

 void mjLib_setup(void) 
{
    mjLib_class = class_new(gensym("mjLib"), (t_newmethod)mjLib_new, 0,
    	sizeof(t_mjLib), 0, (t_atomtype)0);

      pin_tilde_setup();
	  metroplus_setup();
	  monorhythm_setup();
	  prob_setup();
	  about_setup();
	  synapseA_tilde_setup();
	  n2m_setup();
	  morse_setup();

     post("mjLib by mark williamson");
     post("Contact: mark@junklight.com");
     post("website: http://www.junklight.com");
     post("mjLib: version:  0.1 ");
     post("mjLib: compiled: "__DATE__);
	 post("");
}
