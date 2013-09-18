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

static int makeseed(void)
{
    static unsigned int random1_nextseed = 1489853723;
    random1_nextseed = random1_nextseed * 435898247 + 938284287;
    return (random1_nextseed & 0x7fffffff);
}

/* -------------------------- random_tw ------------------------------ */
/* Combination of three tausworth generators. Has parameters for two different generators. Fast and excellent. */
/* Combination of 3 tausworth generators -- assumes 32-bit integers */

static t_class *random_tw_class;

typedef struct _random_tw
{
  t_object x_obj;
  t_float x_f; // lower limit
  t_float x_g; // upper limit

  t_int x_s1;
  t_int x_s2;
  t_int x_s3;

  t_int x_mask1;
  t_int x_mask2;
  t_int x_mask3;

  t_int x_shft1;
  t_int x_shft2;
  t_int x_shft3;
  t_int x_k1;
  t_int x_k2;
  t_int x_k3;

  t_int x_q1;
  t_int x_q2;
  t_int x_q3;
  t_int x_p1;
  t_int x_p2;
  t_int x_p3;

  unsigned int x_state; // current seed
} t_random_tw;

void random_tw_rand_seed (t_random_tw *x, unsigned int a, unsigned int b, unsigned int c)
{
    static unsigned int zf = 4294967295U;

    x->x_shft1 = x->x_k1 - x->x_p1;
    x->x_shft2=x->x_k2-x->x_p2;
    x->x_shft3=x->x_k3-x->x_p3;
    x->x_mask1 = zf << (32-x->x_k1);
    x->x_mask2 = zf << (32-x->x_k2);
    x->x_mask3 = zf << (32-x->x_k3);
    if (a > (unsigned int)(1<<x->x_shft1)) x->x_s1 = a;
    if (b > (unsigned int)(1<<x->x_shft2)) x->x_s2 = b;
    if (c > (unsigned int)(1<<x->x_shft3)) x->x_s3 = c;
    //    rand();
}

static void *random_tw_new(t_floatarg f, t_floatarg g)
{
  t_random_tw *x = (t_random_tw *)pd_new(random_tw_class);

  x->x_f = (f) ? f : 0;
  x->x_g = (g) ? g : RAND_MAX;
  //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
  x->x_state = makeseed();
  
  x->x_s1=390451501;
  x->x_s2=613566701;
  x->x_s3=858993401;
  
  x->x_k1=31;
  x->x_k2=29;
  x->x_k3=28;
  
  x->x_q1=13;
  x->x_q2=2;
  x->x_q3=3;
  x->x_p1=12;
  x->x_p2=4;
  x->x_p3=17;
  
/*   x->x_q1=3; */
/*   x->x_q2=2; */
/*   x->x_q3=13; */
/*   x->x_p1=20; */
/*   x->x_p2=16; */
/*   x->x_p3=7; */
  
  random_tw_rand_seed(x, makeseed(),makeseed(),makeseed());
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
  outlet_new(&x->x_obj, &s_float);
  return (x);
}


static void random_tw_bang(t_random_tw *x)
{
  unsigned int b;
  double nval;
  static unsigned int zf = 4294967295U;
  
  b  = ((x->x_s1 << x->x_q1)^x->x_s1) >> x->x_shft1;
  x->x_s1 = ((x->x_s1 & x->x_mask1) << x->x_p1) ^ b;
  b  = ((x->x_s2 << x->x_q2) ^ x->x_s2) >> x->x_shft2;
  x->x_s2 = ((x->x_s2 & x->x_mask2) << x->x_p2) ^ b;
  b  = ((x->x_s3 << x->x_q3) ^ x->x_s3) >> x->x_shft3;
  x->x_s3 = ((x->x_s3 & x->x_mask3) << x->x_p3) ^ b;
  nval = (((double)(x->x_s1 ^ x->x_s2 ^ x->x_s3) / (double)(zf) + 0.5) * (double)(x->x_g - x->x_f) + (double)x->x_f);
  
  //nval = ((x->x_state / (double)m) * (double)(x->x_g - x->x_f) + (double)x->x_f);
  //post("cxc/randomix.c: current rand: %f", nval);
  outlet_float(x->x_obj.ob_outlet, nval);
}

void random_tw_low(t_random_tw *x, t_floatarg f)
{
  if(f >= x->x_g) {
    post("cxc/randomix.c: lower larger than upper, setting to upper - %f = %f",
	 SMALLEST_RANGE,
	 x->x_g - SMALLEST_RANGE);
    x->x_f = x->x_g - SMALLEST_RANGE;
  } else x->x_f = f;
}

void random_tw_upp(t_random_tw *x, t_floatarg f)
{
  if(f <= x->x_f) {
    post("cxc/randomix.c: upper smaller than lower, setting to lower + %f = %f",
	 SMALLEST_RANGE,
	 x->x_f + SMALLEST_RANGE);
    x->x_g = x->x_f + SMALLEST_RANGE;
  } else x->x_g = f;
}

static void random_tw_seed(t_random_tw *x, float f, float glob)
{
  //x->x_state = f;
  // questionable .. dont quite get how this one's seeded ..
  random_tw_rand_seed(x, (int)f, (int)(f*0.455777), (int)f);
}

static void random_tw_help(t_random_tw *x)
{
  post("RAND_MAX: %d",RAND_MAX);
  post("range: %f - %f", x->x_f, x->x_g);
}

void random_tw_setup(void)
{
  random_tw_class = class_new(gensym("random_tw"), (t_newmethod)random_tw_new, 0,
			    sizeof(t_random_tw), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(random_tw_class, random_tw_bang);
  class_addmethod(random_tw_class, (t_method)random_tw_low, gensym("fl1"), A_FLOAT, 0);
  class_addmethod(random_tw_class, (t_method)random_tw_upp, gensym("fl2"), A_FLOAT, 0);

  class_addmethod(random_tw_class, (t_method)random_tw_seed,
		  gensym("seed"), A_FLOAT, 0);
  class_addmethod(random_tw_class, (t_method)random_tw_help,
		  gensym("help"), 0, 0);
}
