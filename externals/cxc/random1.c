/*
  (c) 2002:cxc@web.fm
  randomix: various PRNG's
  code taken from: http://remus.rutgers.edu/%7Erhoads/Code/code.html
  let's check it out
 */

#include <m_pd.h>
#include <stdlib.h>
#include <math.h>

#define SMALLEST_RANGE .0001

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

static int makeseed(void);
static int rand_random1(int);

static int makeseed(void)
{
    static unsigned int random1_nextseed = 1489853723;
    random1_nextseed = random1_nextseed * 435898247 + 938284287;
    return (random1_nextseed & 0x7fffffff);
}

/* -------------------------- random1 ------------------------------ */
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */
static int rand_random1(seed)
{
  int state;
  
  //unsigned int a = 1588635695, m = 4294967291U, q = 2, r = 1117695901;
  //  unsigned int a = 1588635695, m = (RAND_MAX * 2), q = 2, r = 1117695901;
  
  /* static unsigned int a = 1223106847, m = 4294967291U, q = 3, r = 625646750;*/
  /* static unsigned int a = 279470273, m = 4294967291U, q = 15, r = 102913196;*/
  //static unsigned int a = 1583458089, m = 2147483647, q = 1, r = 564025558;
  static unsigned int a = 784588716, m = 2147483647, q = 2, r = 578306215;
  /* static unsigned int a = 16807, m = 2147483647, q = 127773, r = 2836;      */
  /* static unsigned int a = 950706376, m = 2147483647, q = 2, r = 246070895;  */
  
  //state = (seed) ? seed : makeseed();
  state = seed;
  state = a*(state % q) - r*(state / q);
  return (state);
}

/* -------------------------- random1 ------------------------------ */
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */

static t_class *random1_class;

typedef struct _random1
{
  t_object x_obj;
  t_float x_f; // lower limit
  t_float x_g; // upper limit
  unsigned int x_state; // current seed
} t_random1;

static void *random1_new(t_floatarg f, t_floatarg g)
{
    t_random1 *x = (t_random1 *)pd_new(random1_class);
    x->x_f = (f) ? f : 0;
    x->x_g = (g) ? g : RAND_MAX;
    //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
    x->x_state = makeseed();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

/* -------------------------- random_fl ------------------------------ */
/* An improved (faster) implementation of the Linear Congruential Generator. Has parameters for 6 separate */
/* linear congruence formulas. These formulas are different than those above because the previous formulas won't work */
/* correctly in this implementation. Also, this method only works if your floating point mantissa has at least 53 bits. */

/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */
/* works if your floating pt. mantissa has at least 53 bits.
   faster than other versions */
static void random1_bang(t_random1 *x)
{
    int n = x->x_f;
    double nval;
    unsigned int m;
    // this seems weird?
    m = (RAND_MAX * 2);
    //    unsigned int a = 1588635695, m = 4294967291U, q = 2, r = 1117695901;

    //    post("cxc/randomix.c: x_state: %d",x->x_state);
    x->x_state = rand_random1(x->x_state);
    nval = ((double)x->x_state / (double)m) * (double)(x->x_g - x->x_f) + (double)x->x_f;
    //    post("cxc/randomix.c: lolim: %f, uplim: %f", x->x_f, x->x_g);
    //    post("cxc/randomix.c: nval: %f",nval);
    outlet_float(x->x_obj.ob_outlet, nval);
}

void random1_low(t_random1 *x, t_floatarg f)
{
  if(f >= x->x_g) {
    post("cxc/randomix.c: lower larger than upper, setting to upper - %f = %f",
	 SMALLEST_RANGE,
	 x->x_g - SMALLEST_RANGE);
    x->x_f = x->x_g - SMALLEST_RANGE;
  } else x->x_f = f;
}

void random1_upp(t_random1 *x, t_floatarg f)
{
  if(f <= x->x_f) {
    post("cxc/randomix.c: upper smaller than lower, setting to lower + %f = %f",
	 SMALLEST_RANGE,
	 x->x_f + SMALLEST_RANGE);
    x->x_g = x->x_f + SMALLEST_RANGE;
  } else x->x_g = f;
}

static void random1_seed(t_random1 *x, float f, float glob)
{
    x->x_state = f;
}

void random1_setup(void)
{
  random1_class = class_new(gensym("random1"), (t_newmethod)random1_new, 0,
			    sizeof(t_random1), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(random1_class, random1_bang);
  class_addmethod(random1_class, (t_method)random1_low, gensym("fl1"), A_FLOAT, 0);
  class_addmethod(random1_class, (t_method)random1_upp, gensym("fl2"), A_FLOAT, 0);

  class_addmethod(random1_class, (t_method)random1_seed,
		  gensym("seed"), A_FLOAT, 0);
}

