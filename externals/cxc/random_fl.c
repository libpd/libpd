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
static int rand_random_fl(int);

static int makeseed(void)
{
    static unsigned int random1_nextseed = 1489853723;
    random1_nextseed = random1_nextseed * 435898247 + 938284287;
    return (random1_nextseed & 0x7fffffff);
}

static int rand_random_fl(seed) {
  int q;
  double state;

  /* The following parameters are recommended settings based on research
     uncomment the one you want. */
  
  double a = 1389796, m = RAND_MAX;  
  /* static double a = 950975,  m = 2147483647;  */
  /* static double a = 3467255, m = 21474836472; */
  /* static double a = 657618,  m = 4294967291;  */
  /* static double a = 93167,   m = 4294967291;  */
  /* static double a = 1345659, m = 4294967291;  */

  state = seed;
  state *= a;
  q = state / m;
  state -= q*m;
  return state;
}

/* -------------------------- random_fl ------------------------------ */
/* An improved (faster) implementation of the Linear Congruential Generator. Has parameters for 6 separate */
/* linear congruence formulas. These formulas are different than those above because the previous formulas won't work */
/* correctly in this implementation. Also, this method only works if your floating point mantissa has at least 53 bits. */

/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */
/* works if your floating pt. mantissa has at least 53 bits.
   faster than other versions */

static t_class *random_fl_class;

typedef struct _random_fl
{
  t_object x_obj;
  t_float x_f; // lower limit
  t_float x_g; // upper limit
  unsigned int x_state; // current seed
} t_random_fl;

static void *random_fl_new(t_floatarg f, t_floatarg g)
{
    t_random_fl *x = (t_random_fl *)pd_new(random_fl_class);
    x->x_f = (f) ? f : 0;
    x->x_g = (g) ? g : RAND_MAX;
    //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
    x->x_state = makeseed();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void random_fl_bang(t_random_fl *x)
{
  int n = x->x_f;
  int q;
  double nval;
  double m;
  
  m = RAND_MAX;
  x->x_state = rand_random_fl(x->x_state);
  nval = ((x->x_state / m) * (double)(x->x_g - x->x_f) + (double)x->x_f);
  outlet_float(x->x_obj.ob_outlet, nval);
}

void random_fl_low(t_random_fl *x, t_floatarg f)
{
  if(f >= x->x_g) {
    post("cxc/randomix.c: lower larger than upper, setting to upper - %f = %f",
	 SMALLEST_RANGE,
	 x->x_g - SMALLEST_RANGE);
    x->x_f = x->x_g - SMALLEST_RANGE;
  } else x->x_f = f;
}

void random_fl_upp(t_random_fl *x, t_floatarg f)
{
  if(f <= x->x_f) {
    post("cxc/randomix.c: upper smaller than lower, setting to lower + %f = %f",
	 SMALLEST_RANGE,
	 x->x_f + SMALLEST_RANGE);
    x->x_g = x->x_f + SMALLEST_RANGE;
  } else x->x_g = f;
}

static void random_fl_seed(t_random_fl *x, float f, float glob)
{
    x->x_state = f;
}

void random_fl_setup(void)
{
  random_fl_class = class_new(gensym("random_fl"), (t_newmethod)random_fl_new, 0,
			    sizeof(t_random_fl), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(random_fl_class, random_fl_bang);
  class_addmethod(random_fl_class, (t_method)random_fl_low, gensym("fl1"), A_FLOAT, 0);
  class_addmethod(random_fl_class, (t_method)random_fl_upp, gensym("fl2"), A_FLOAT, 0);

  class_addmethod(random_fl_class, (t_method)random_fl_seed,
		  gensym("seed"), A_FLOAT, 0);
}
