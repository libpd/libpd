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

/* -------------------------- dist_normal ------------------------------ */
/* Generate a normal random variable with mean 0 and standard deviation
   of 1.  To adjust to some other distribution, multiply by the standard
   deviation and add the mean.  Box-Muller method
   note: rand() is a function that returns a uniformly distributed random
   number from 0 to RAND_MAX
*/

static t_class *dist_normal_class;

typedef struct _dist_normal
{
  t_object x_obj;
  t_float x_mn; // mean
  t_float x_dv; // deviation
  t_float x_u1;
  t_float x_u2;

  t_float x_f; // lower limit
  t_float x_g; // upper limit
  unsigned int x_state; // current seed
} t_dist_normal;

static void *dist_normal_new(t_floatarg mn, t_floatarg dv)
{
    t_dist_normal *x = (t_dist_normal *)pd_new(dist_normal_class);
    x->x_mn = (mn) ? mn : 0;
    x->x_dv = (dv) ? dv : 1;
    x->x_u1 = 13;
    x->x_u2 = 1000;
    x->x_f =  0;
    x->x_g =  RAND_MAX;
    //post("cxc/randomix.c: lolim: %f - %f, uplim: %f - %f", x->x_f, f, x->x_g, g);
    x->x_state = makeseed();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    //    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    //outlet_new(&x->x_obj, &s_float);
    outlet_new(&x->x_obj, 0);
    return (x);
}

static double dist_normal_rand(t_dist_normal *x)
{
  int n = x->x_f;
  double nval;
  double m;
  
  m = (double)RAND_MAX * 2;
  
    //    post("cxc/randomix.c: x_state: %d",x->x_state);
  x->x_state = rand_random_fl(x->x_state);
  //nval = ((double)x->x_state / m) * (double)(x->x_g - x->x_f) + (double)x->x_f;
  nval = (double)x->x_state;
  return (nval);
}

static void dist_normal_doit(t_dist_normal *x)
{
  static double V2, fac;
  static int phase = 0;
  double S, Z, U1, U2, V1;

  if (phase)
    Z = V2 * fac;
  else
    {
      do {
/* 	U1 = (double)rand() / RAND_MAX; */
/* 	U2 = (double)rand() / RAND_MAX; */
	U1 = (double)dist_normal_rand(x) / RAND_MAX;
	U2 = (double)dist_normal_rand(x) / RAND_MAX;

	//	post("cxc/randomix.c: test %f %f %f %f", x->x_u1, x->x_u2, U1, U2);
	
	V1 = 2 * U1 - 1;
	V2 = 2 * U2 - 1;
	S = V1 * V1 + V2 * V2;
      } while(S >= 1);
      
      fac = sqrt (-2 * log(S) / S);
      Z = V1 * fac;
    }
  
  phase = 1 - phase;
  
  //return Z;
  outlet_float(x->x_obj.ob_outlet, Z);
}

static void dist_normal_bang(t_dist_normal *x)
{
/*   post("cxc/randomix.c: dist_normal banged"); */
/*   post("cxc/randomix.c: RAND_MAX: %d",RAND_MAX); */
/*   post("cxc/randomix.c: test: %f %f", x->x_u1, x->x_u2); */
  dist_normal_doit(x);
}

void dist_normal_low(t_dist_normal *x, t_floatarg mn)
{
  x->x_mn = mn;
}

void dist_normal_upp(t_dist_normal *x, t_floatarg dv)
{
  x->x_dv = dv;
}

void dist_normal_float(t_dist_normal *x, t_floatarg r)
{
  outlet_float(x->x_obj.ob_outlet, r);
}

void dist_normal_list(t_dist_normal *x, t_symbol *s, int argc, t_atom *argv)
{ 
  outlet_list(x->x_obj.ob_outlet, s, argc, argv);
}

void dist_normal_setup(void)
{
  dist_normal_class = class_new(gensym("dist_normal"), (t_newmethod)dist_normal_new, 0,
			    sizeof(t_dist_normal), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(dist_normal_class, dist_normal_bang);
  class_addmethod(dist_normal_class, (t_method)dist_normal_low, gensym("fl1"), A_FLOAT, 0);
  //  class_addmethod(dist_normal_class, (t_method)dist_normal_upp, gensym("fl2"), A_FLOAT, 0);
  class_addlist    (dist_normal_class, dist_normal_list);

/*   class_addmethod(dist_normal_class, (t_method)dist_normal_seed, */
/* 		  gensym("seed"), A_FLOAT, 0); */
}
