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

/* -------------------------- random1 ------------------------------ */
/* linear congruential generator.  Generator x[n+1] = a * x[n] mod m */
/* signal version */

static t_class *random1_tilde_class;

typedef struct _random1_tilde
{
  t_object x_obj;
  unsigned int x_state; // current seed
} t_random1_tilde;

static void *random1_tilde_new()
{
    t_random1_tilde *x = (t_random1_tilde *)pd_new(random1_tilde_class);
    x->x_state = makeseed();
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static void random1_tilde_seed(t_random1_tilde *x, float f, float glob)
{
    x->x_state = f;
}

static t_int *random1_tilde_perform(t_int *w)
{
  t_random1_tilde *x = (t_random1_tilde *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);

  while (n--) {
    // generate new noise sample
    x->x_state = rand_random1(x->x_state);
    *out++ = (t_float)x->x_state / RAND_MAX - 1;
  }

  return (w+4);
}

static void random1_tilde_dsp(t_random1_tilde *x, t_signal **sp)
{
  dsp_add(random1_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

void random1_tilde_setup(void)
{
  random1_tilde_class = class_new(gensym("random1~"),
				  (t_newmethod)random1_tilde_new,
				  0, sizeof(t_random1_tilde), 0, 0);
  class_addmethod(random1_tilde_class, (t_method)random1_tilde_seed,gensym("seed"), A_FLOAT, 0);
  class_addmethod(random1_tilde_class, (t_method)random1_tilde_dsp, gensym("dsp"), 0);
}
