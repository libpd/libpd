/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <math.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ hlshelf ----------------------------- */


#ifndef M_PI
#define M_PI 3.141593f
#endif

#define SRATE 44100.0
#define MAX_GAIN 120.0f

static t_class *hlshelf_class;


typedef struct _hlshelf
{
     t_object x_obj;
     float s_rate;
     float s_gain0;
     float s_gain1;
     float s_gain2;
     float s_ltransfq;
     float s_htransfq;
     float s_lradians;
     float s_hradians;
} t_hlshelf;


int hlshelf_check_stability(t_float fb1,
			    t_float fb2, 
			    t_float ff1,
			    t_float ff2,
			    t_float ff3)
{
    float discriminant = fb1 * fb1 + 4 * fb2;

    if (discriminant < 0) /* imaginary roots -- resonant filter */
    {
    	    /* they're conjugates so we just check that the product
    	    is less than one */
    	if (fb2 >= -1.0f) goto stable;
    }
    else    /* real roots */
    {
    	    /* check that the parabola 1 - fb1 x - fb2 x^2 has a
    	    	vertex between -1 and 1, and that it's nonnegative
    	    	at both ends, which implies both roots are in [1-,1]. */
    	if (fb1 <= 2.0f && fb1 >= -2.0f &&
    	    1.0f - fb1 -fb2 >= 0 && 1.0f + fb1 - fb2 >= 0)
    	    	goto stable;
    }
    return 0;
stable:
    return 1;
}


void hlshelf_check(t_hlshelf *x)
{

     if(x->s_gain0 - x->s_gain1 > MAX_GAIN) {
	  x->s_gain0 = x->s_gain1 + MAX_GAIN; 
	  post("setting gain0 to %f",x->s_gain0);
     }


     if(x->s_gain1 > MAX_GAIN) {
	  x->s_gain1 = MAX_GAIN;
	  post("setting gain1 to %f",x->s_gain1);
     }

     if(x->s_gain2 - x->s_gain1 > MAX_GAIN) {
	  x->s_gain2 = x->s_gain1 + MAX_GAIN; 
	  post("setting gain2 to %f",x->s_gain2);
     }

  /* constrain: 0 <= x->s_ltransfq < x->s_htransfq. */
     x->s_ltransfq = (x->s_ltransfq < x->s_htransfq) ? x->s_ltransfq : x->s_htransfq - 0.5f;
     
     if (x->s_ltransfq < 0) x->s_ltransfq = 0.0f;
     
     x->s_lradians = M_PI * x->s_ltransfq / x->s_rate;
     x->s_hradians= M_PI * (0.5f - (x->s_htransfq / x->s_rate));	

}


void hlshelf_bang(t_hlshelf *x)
{
     t_atom at[6];
     float c0, c1, c2, d0, d1, d2;	/* output coefs */
     float a1, a2, b1, b2, g1, g2;	/* temp coefs */
     double xf;

     hlshelf_check(x);
     
     /* low shelf */
     xf = 0.5 * 0.115129255 * (double)(x->s_gain0 - x->s_gain1); /* ln(10) / 20 = 0.115129255 */
     if(xf < -200.) /* exp(x) -> 0 */
     {
	  a1 = 1.0f;
	  b1 = -1.0f;
	  g1 = 0.0f;
     }
     else
     {
	  double t = tan(x->s_lradians);
	  double e = exp(xf);
	  double r = t / e;
	  double kr = t * e;
	  
	  a1 = (r - 1) / (r + 1);		
	  b1 = (kr - 1) / (kr + 1);
	  g1 = (kr + 1) / (r + 1);
     }
     
     /* high shelf */
     xf = 0.5 * 0.115129255 * (double)(x->s_gain2 - x->s_gain1); /* ln(10) / 20 = 0.115129255 */
     if(xf < -200.) /* exp(x) -> 0 */
     {
	  a2 = -1.0f;
	  b2 = 1.0f;
	  g2 = 0.0f;
     }
     else
     {
	  double t = tan(x->s_hradians);
	  double e = exp(xf);
	  double r = t / e;
	  double kr = t * e;
	  
	  a2 = (1 - r) / (1 + r);
	  b2 = (1 - kr) / (1 + kr);
	  g2 = (1 + kr) / (1 + r);
     }
     
     /* form product */
     c0 = g1 * g2 * (float)(exp((double)(x->s_gain1) * 0.05f * 2.302585093f));  ;
     c1 = a1 + a2;
     c2 = a1 * a2;
     d0 =  1.0f;
     d1 = b1 + b2;
     d2 = b1 * b2;

     if (!hlshelf_check_stability(-c1/d0,-c2/d0,d0/d0,d1/d0,d2/d0)) {
       post("hlshelf: filter unstable -> resetting");
       c0=1.;c1=0.;c2=0.;
       d0=1.;d1=0.;d2=0.;
     }

     SETFLOAT(at,-c1/d0);
     SETFLOAT(at+1,-c2/d0);
     SETFLOAT(at+2,d0/d0);
     SETFLOAT(at+3,d1/d0);
     SETFLOAT(at+4,d2/d0);
     
     outlet_list(x->x_obj.ob_outlet,&s_list,5,at);
}

void hlshelf_float(t_hlshelf *x,t_floatarg f)
{
     x->s_gain0 = f;
     hlshelf_bang(x);
}


static void *hlshelf_new(t_symbol* s,t_int argc, t_atom* at)
{
    t_hlshelf *x = (t_hlshelf *)pd_new(hlshelf_class);
    t_float k0 = atom_getfloat(at);
    t_float k1 = atom_getfloat(at+1);
    t_float k2 = atom_getfloat(at+2);
    t_float f1 = atom_getfloat(at+3);
    t_float f2 = atom_getfloat(at+4);


    f1 = atom_getfloat(at);
    f2 = atom_getfloat(at);

    if ((f1 == 0.0f && f2 == 0.0f) || f1 > f2){ /* all gains = 0db */
	 f1 = 150.0f;	
	 f2 = 5000.0f;
    }

    if (f1 < 0) f1 = 0.0f;
    if (f2 > SRATE) f2 = .5f*SRATE;
 
    x->s_rate = SRATE;		/* srate default  */
    x->s_gain0 = k0;
    x->s_gain1 = k1;
    x->s_gain2 = k2;
    
    x->s_ltransfq = 0.0f;
    x->s_htransfq = SRATE/2;

    x->s_lradians = M_PI * x->s_ltransfq / x->s_rate;
    x->s_hradians= M_PI * (0.5f - (x->s_htransfq / x->s_rate));

    floatinlet_new(&x->x_obj, &x->s_gain1);
    floatinlet_new(&x->x_obj, &x->s_gain2);
    floatinlet_new(&x->x_obj, &x->s_ltransfq);
    floatinlet_new(&x->x_obj, &x->s_htransfq);
    outlet_new(&x->x_obj, &s_list);

    return (x);
}

void hlshelf_setup(void)
{
    hlshelf_class = class_new(gensym("hlshelf"), (t_newmethod)hlshelf_new, 0,
				sizeof(t_hlshelf), 0, A_GIMME, 0);
    class_addbang(hlshelf_class,hlshelf_bang);
    class_addfloat(hlshelf_class,hlshelf_float);
}


