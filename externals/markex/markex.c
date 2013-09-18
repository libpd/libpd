/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.

 MarkEx, Copyright (c) 1997-1999 Mark Danks  */

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"

static t_class *MarkEx_class;

static void *MarkEx_new(void)
{
  t_object *x = (t_object *)pd_new(MarkEx_class);
  return (x);
}

void abs_tilde_setup(void);
void alternate_setup(void);
void average_setup(void);
void counter_setup(void);
void hsv2rgb_setup(void);
void invert_setup(void);
void multiselect_setup(void);
void oneshot_setup(void);
void randomF_setup(void);
void reson_tilde_setup(void);
void rgb2hsv_setup(void);
void strcat_setup(void);
void tripleLine_setup(void);
void tripleRand_setup(void);
void vectorabs_setup(void);
void vectorpack_setup(void);
void setup_vector0x2a(void);
void setup_vector0x2f(void);
void setup_vector0x2d(void);
void setup_vector0x2b(void);


/* ------------------------ setup routine ------------------------- */

void MarkEx_setup(void)
{
  MarkEx_class = class_new(gensym("MarkEx"), MarkEx_new, 0,
                           sizeof(t_object), CLASS_NOINLET, 0);

  abs_tilde_setup();
  alternate_setup();
  average_setup();
  counter_setup();
  hsv2rgb_setup();
  invert_setup();
  multiselect_setup();
  oneshot_setup();
  randomF_setup();
  reson_tilde_setup();
  rgb2hsv_setup();
  strcat_setup();
  tripleLine_setup();
  tripleRand_setup();
  vectorabs_setup();
  vectorpack_setup();
  setup_vector0x2a();
  setup_vector0x2f();
  setup_vector0x2d();
  setup_vector0x2b();
  post("MarkEx loaded!   (c) 1997-1999 Mark Danks ");
}
