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

/* ------------------------------- random_fl signal version -------------------- */

static t_class *random_fl_tilde_class;

typedef struct _random_fl_tilde
{
  t_object x_obj;
  unsigned int x_state; // current seed
} t_random_fl_tilde;

static void *random_fl_tilde_new()
{
    t_random_fl_tilde *x = (t_random_fl_tilde *)pd_new(random_fl_tilde_class);
    x->x_state = makeseed();
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static void random_fl_tilde_seed(t_random_fl_tilde *x, float f, float glob)
{
  x->x_state = f;
}

static t_int *random_fl_tilde_perform(t_int *w)
{
  t_random_fl_tilde *x = (t_random_fl_tilde *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);

  while (n--) {
    // generate new noise sample
    x->x_state = rand_random_fl(x->x_state);
    *out++ = (t_float)x->x_state / RAND_MAX * 2 - 1;
  }

  return (w+4);
}

static void random_fl_tilde_dsp(t_random_fl_tilde *x, t_signal **sp)
{
  dsp_add(random_fl_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

void random_fl_tilde_setup(void)
{
  random_fl_tilde_class = class_new(gensym("random_fl~"),
				    (t_newmethod)random_fl_tilde_new, 0,
				    sizeof(t_random_fl_tilde), 0, 0);
  class_addmethod(random_fl_tilde_class, (t_method)random_fl_tilde_seed,
		  gensym("seed"), A_FLOAT, 0);
  class_addmethod(random_fl_tilde_class, (t_method)random_fl_tilde_dsp, gensym("dsp"), 0);
}
