
#ifndef VERSION
#define VERSION "0.05"
#endif

#ifndef __DATE__ 
#define __DATE__ "without using a gnu compiler"
#endif

#include <m_pd.h>
#include "cxc.h"

typedef struct _cxc
{
     t_object x_obj;
} t_cxc;

static t_class* cxc_class;

//void binshift_setup();
void ascwave_setup();
void ascseq_setup();
void ixprint_setup();
//void mixer_setup();
//void makesymbol_setup();
void bfilt_setup();
void bfilt2_setup();
/* void sendOSC_setup(); */
/* void dumpOSC_setup(); */
/* void routeOSC_setup(); */
//void testy_tilde_setup();
//void garlic_tilde_setup();
void cxc_counter_setup();
void reson_tilde_setup();
//void serialize_setup();
void cxc_prepend_setup();
void cxc_split_setup();
void utime_setup();
// RNG's
void random1_setup();
void random1_tilde_setup(); // signal version
void random_fl_setup();
void random_fl_tilde_setup();
void random_icg_setup();
void random_icg_tilde_setup();
void random_tw_setup();
void dist_normal_setup();

void ENV_setup();
void proc_setup();

void cxmean_setup();
void cxavgdev_setup();
void cxstddev_setup();

void mean_tilde_setup();

void delta_tilde_setup();

static void* cxc_new(t_symbol* s) {
    t_cxc *x = (t_cxc *)pd_new(cxc_class);
    return (x);
}

void cxc_setup(void) 
{
  cxc_class = class_new(gensym("cxc"), (t_newmethod)cxc_new, 0,
			  sizeof(t_cxc), 0,0);
  ixprint_setup();
//  binshift_setup();
  ascwave_setup();
  ascseq_setup();
  //  mixer_setup();
  //  makesymbol_setup();
  bfilt_setup();
  bfilt2_setup();
/*   sendOSC_setup(); */
/*   dumpOSC_setup(); */
/*   routeOSC_setup(); */
  cxc_counter_setup();
  reson_tilde_setup();
  //serialize_setup();
  cxc_prepend_setup();
  cxc_split_setup();
  utime_setup();
  // RNG's
  random1_setup();
  random1_tilde_setup();
  random_fl_setup();
  random_fl_tilde_setup();
  random_icg_setup();
  random_icg_tilde_setup();
  random_tw_setup();
  dist_normal_setup();
  
  ENV_setup();
  proc_setup();

  cxmean_setup();
  cxavgdev_setup();
  cxstddev_setup();

  mean_tilde_setup();

  delta_tilde_setup();
 
  post("c  : jdl@xdv.org ========================================");
  post(" x : ver: "VERSION" ============================================");
  post("  c: compiled: "__DATE__" ==================================");
   // post("\\");
   // L;
}
