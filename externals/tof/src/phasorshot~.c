#include "m_pd.h"
#include <math.h>

#define UNITBIT32 1572864.  /* 3*2^19; bit 32 has place value 1 */

    /* machine-dependent definitions.  These ifdefs really
    should have been by CPU type and not by operating system! */
#ifdef IRIX
    /* big-endian.  Most significant byte is at low address in memory */
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#define int32 long  /* a data type that has 32 bits */
#endif /* IRIX */

#ifdef MSW
    /* little-endian; most significant byte is at highest address */
#define HIOFFSET 1
#define LOWOFFSET 0
#define int32 long
#endif

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <machine/endian.h>
#endif

#ifdef __linux__
#include <endian.h>
#endif

#if defined(__unix__) || defined(__APPLE__)
#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN)                         
#error No byte order defined                                                    
#endif                                                                          

#if BYTE_ORDER == LITTLE_ENDIAN                                             
#define HIOFFSET 1                                                              
#define LOWOFFSET 0                                                             
#else                                                                           
#define HIOFFSET 0    /* word offset to find MSB */                             
#define LOWOFFSET 1    /* word offset to find LSB */                            
#endif /* __BYTE_ORDER */                                                       
#include <sys/types.h>
#define int32 int32_t
#endif /* __unix__ or __APPLE__*/

union tabfudge
{
    double tf_d;
    int32 tf_i[2];
};

/* -------------------------- phasorshot~ ------------------------------ */
static t_class *phasorshot_class, *scalarphasorshot_class;

#if 1   /* in the style of R. Hoeldrich (ICMC 1995 Banff) */

typedef struct _phasorshot
{
    t_object x_obj;
    double x_phase;
    float x_conv;
    float x_f;      /* scalar frequency */
    float loop;
    t_outlet *x_outlet1;        /* bang out for high thresh */
    t_outlet *x_outlet2;        /* bang out for low thresh */
    t_clock *x_clock;           /* wakeup for message output */
    //int previousState;
    int state;
} t_phasorshot;

 

void phasorshot_tick(t_phasorshot *x)  
{
    if (x->state == 1) {
    outlet_bang(x->x_outlet2);
    } else if ( x->state == 0 ) { 
    outlet_bang(x->x_outlet1);
    }
    //post("bang");
}

 t_int *phasorshot_perform(t_int *w)
{
    t_phasorshot *x = (t_phasorshot *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    double dphase = x->x_phase + UNITBIT32;
    union tabfudge tf;
    int normhipart;

    float conv = x->x_conv;

    tf.tf_d = UNITBIT32;
    normhipart = tf.tf_i[HIOFFSET];
    tf.tf_d = dphase;
    
    //clock_delay(x->x_clock, 0L);

    while (n--)
    {
        // BANG when bounds are reached
         if ( tf.tf_d >= UNITBIT32 + 1 ) {
            tf.tf_d = UNITBIT32 + 1;
            if (x->state != 1) clock_delay(x->x_clock, 0L);
            x->state = 1;
          } else if ( tf.tf_d <= UNITBIT32 ) {
            tf.tf_d = UNITBIT32;
            if (x->state != 0) clock_delay(x->x_clock, 0L);
            x->state = 0;
          } else {
            x->state = -1;
          }
          
        //wrap
        if (x->loop)  tf.tf_i[HIOFFSET] = normhipart; 
    
        dphase += *in++ * conv; //increment
        //dphase = UNITBIT32 + 1; //set to one
        *out++ = tf.tf_d - UNITBIT32;
        tf.tf_d = dphase;
    }
    //if (x->loop) tf.tf_i[HIOFFSET] = normhipart; //wrap
    //wrap
        if (x->loop) { 
          tf.tf_i[HIOFFSET] = normhipart; 
          //x->state = -1;
        } else {
          if ( tf.tf_d > UNITBIT32 + 1 ) {
            tf.tf_d = UNITBIT32 + 1;
          } else if ( tf.tf_d < UNITBIT32 ) {
            tf.tf_d = UNITBIT32;
          }
          
        }
    x->x_phase = tf.tf_d - UNITBIT32;
    return (w+5);
}

 void phasorshot_dsp(t_phasorshot *x, t_signal **sp)
{
    x->x_conv = 1./sp[0]->s_sr;
    dsp_add(phasorshot_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

 void phasorshot_ft1(t_phasorshot *x, t_float f)
{
    if (f < 0) f = 0;
    if (f > 1) f = 1;
    x->x_phase = f;
    if ( f == 1) x->state = 1;
    if ( f == 0) x->state = 0;

}

 void phasorshot_loop(t_phasorshot *x, t_float f)
{
    x->loop = f;
    //if (!f) x->state = -1;
}


void *phasorshot_new(t_symbol *s,int argc,t_atom* argv)
{
    t_phasorshot *x = (t_phasorshot *)pd_new(phasorshot_class);
    
    x->x_f = 0;
    if (argc) x->x_f = atom_getfloat(argv++),argc--;
    
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
    
    x->x_phase = 0;
    x->x_conv = 0;
   
    outlet_new(&x->x_obj, gensym("signal"));
    
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("loop"));
    x->loop = 0;
    
    if (argc) x->loop = atom_getfloat(argv++),argc--;
     
     x->state = 0;
    
    x->x_outlet1 = outlet_new(&x->x_obj, &s_bang);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_bang);
    
    x->x_clock = clock_new(x, (t_method)phasorshot_tick);

    
    return (x);
}


void phasorshot_free(t_phasorshot *x)
{
    clock_free(x->x_clock);
}


 void phasorshot_tilde_setup(void)
{
    phasorshot_class = class_new(gensym("phasorshot~"), (t_newmethod)phasorshot_new, (t_method)phasorshot_free,
        sizeof(t_phasorshot), 0,A_GIMME,0);
    CLASS_MAINSIGNALIN(phasorshot_class, t_phasorshot, x_f);
    class_addmethod(phasorshot_class, (t_method)phasorshot_dsp, gensym("dsp"), 0);
    
    class_addmethod(phasorshot_class, (t_method)phasorshot_ft1,
        gensym("ft1"), A_FLOAT, 0);
    class_addmethod(phasorshot_class, (t_method)phasorshot_loop,
        gensym("loop"), A_FLOAT, 0);
}

#endif  /* Hoeldrich version */
