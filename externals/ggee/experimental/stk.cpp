#include <m_pd.h>
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* STK includes */

#include "stk.h"

/* ------------------------ stk ----------------------------- */

static t_class *stk_class;

typedef struct _stk
{
     t_object x_obj;
     Instrmnt *instrument;
     t_float  x_velo;
} t_stk;


char* stk_instruments[255];

void stk_print(t_stk* x) {
     int i=0;

     while (strncmp(stk_instruments[i],"LastInst",8)) {
	  post("%s",stk_instruments[i]);
	  i++;
     }
}


#define DI(name)   if ((stk_instruments[i++] = #name) && !strcmp(s->s_name, #name )) \
{ x->instrument = new name;}

#define DI2(name,y)   if ((stk_instruments[i++] = #name) && !strcmp(s->s_name, #name )) \
{ x->instrument = new name(y);}


static void stk_set_instrument(t_stk* x,t_symbol* s) 
{
  int i = 0;

  x->instrument = NULL;
  DI2(Clarinet,10.0);
  DI2(BlowHole,10.0);
  DI2(Saxofony,10.0);
  DI2(Flute,10.0);
  DI2(Brass,10.0);
  DI(BlowBotl);
  DI2(Bowed,10.0);
  DI2(Plucked,5.0);
  DI2(StifKarp,5.0);
  DI2(Sitar,5.0);
  DI2(Mandolin,5.0);

  DI(Rhodey);
  DI(Wurley);
  DI(TubeBell);
  DI(HevyMetl);
  DI(PercFlut);
  DI(BeeThree);
  DI(FMVoices);

  DI(VoicForm);
  DI(Moog);
  DI(Simple);
  DI(Drummer);
  DI(BandedWG);
  DI(Shakers);
  DI(ModalBar);
  //  DI2(Mesh2D,10, 10);
  DI(Resonate);
  DI(Whistle);

  if (!x->instrument)
       error("No such instrument %s",s->s_name);

  stk_instruments[i] = "LastInst";
}


static void stk_bang(t_stk *x)
{
     post("bang");
}


static t_int *stk_perform(t_int *w)
{
     t_stk* x = (t_stk*)(w[1]);
     t_float *out = (t_float *)(w[2]);
     int n = (int)(w[3]);

     while (n--) {
	  *out++ = x->instrument->tick();
     }


     return (w+4);
}


static void stk_dsp(t_stk *x, t_signal **sp)
{
     dsp_add(stk_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


static void stk_noteOn(t_stk *x,t_float f) 
{
     x->instrument->noteOn(f,x->x_velo);
}


static void stk_noteOff(t_stk *x) 
{
     x->instrument->noteOff(0);
}


static void stk_control(t_stk *x,t_floatarg f1,t_floatarg f2) 
{
     x->instrument->controlChange((int)f1,f2);
}

static void stk_control1(t_stk *x,t_floatarg f1) 
{
     x->instrument->controlChange(1,f1);
}

static void stk_control2(t_stk *x,t_floatarg f1) 
{
     x->instrument->controlChange(2,f1);
}

static void stk_control3(t_stk *x,t_floatarg f1) 
{
     x->instrument->controlChange(3,f1);
}

static void stk_control4(t_stk *x,t_floatarg f1) 
{
     x->instrument->controlChange(4,f1);
}

static void stk_aftertouch(t_stk *x,t_floatarg f1) 
{
     x->instrument->controlChange(128,f1);
}

static void stk_pitchbend(t_stk *x,t_floatarg f1) 
{
     x->instrument->setFrequency(f1);
}

static void stk_float(t_stk* x,t_floatarg f)
{
     if (f > 20) stk_noteOn(x,f);
     else stk_noteOff(x); 
     
}


static void *stk_new(t_symbol* s)
{
    t_stk *x = (t_stk *)pd_new(stk_class);

    stk_set_instrument(x,s); 

    if (x->instrument == NULL) {
      post("stk: %s no such instrument",s->s_name);
      return NULL;
    }
    floatinlet_new(&x->x_obj, &x->x_velo);
    x->x_velo = 0.1;

    outlet_new(&x->x_obj, gensym("signal"));

    return (x);
}



extern "C" {
     void stk_setup(void)
     {
	  stk_class = class_new(gensym("stk"), (t_newmethod)stk_new, 0,
				sizeof(t_stk), 0,A_DEFSYM,A_NULL);
	  class_addmethod(stk_class, nullfn, gensym("signal"), A_NULL);
	  class_addmethod(stk_class, (t_method) stk_dsp, gensym("dsp"), A_NULL);
	  
	  class_addbang(stk_class,stk_bang);
	  class_addfloat(stk_class,stk_float);
	  class_addmethod(stk_class,(t_method) stk_control,gensym("control"),A_DEFFLOAT,A_DEFFLOAT,A_NULL);
	  class_addmethod(stk_class,(t_method) stk_control1,gensym("control1"),A_DEFFLOAT,A_NULL);
	  class_addmethod(stk_class,(t_method) stk_control2,gensym("control2"),A_DEFFLOAT,A_NULL);
	  class_addmethod(stk_class,(t_method) stk_control3,gensym("control3"),A_DEFFLOAT,A_NULL);
	  class_addmethod(stk_class,(t_method) stk_control4,gensym("control4"),A_DEFFLOAT,A_NULL);
	  class_addmethod(stk_class,(t_method) stk_aftertouch,gensym("aftertouch"),A_DEFFLOAT,A_NULL);
	  class_addmethod(stk_class,(t_method) stk_pitchbend,gensym("pitchbend"),A_DEFFLOAT,A_NULL);
	  class_addmethod(stk_class,(t_method) stk_print,gensym("print"),A_NULL);
	  
     }
}
