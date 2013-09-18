/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */


#include "m_pd.h"
#include "iemlib.h"

static t_class *iemlib1_class;

static void *iemlib1_new(void)
{
  t_object *x = (t_object *)pd_new(iemlib1_class);
  
  return (x);
}

void biquad_freq_resp_setup(void);
void db2v_setup(void);
void f2note_setup(void);
void filter_tilde_setup(void);
void FIR_tilde_setup(void);
void forpp_setup(void);
void gate_setup(void);
void hml_shelf_tilde_setup(void);
void iem_cot4_tilde_setup(void);
void iem_delay_tilde_setup(void);
void iem_pow4_tilde_setup(void);
void iem_sqrt4_tilde_setup(void);
void lp1_t_tilde_setup(void);
void mov_avrg_kern_tilde_setup(void);
void para_bp2_tilde_setup(void);
void peakenv_tilde_setup(void);
void peakenv_hold_tilde_setup(void);
void prvu_tilde_setup(void);
void pvu_tilde_setup(void);
void rvu_tilde_setup(void);
void sin_phase_tilde_setup(void);
void soundfile_info_setup(void);
void split_setup(void);
void v2db_setup(void);
void vcf_filter_tilde_setup(void);

/* ------------------------ setup routine ------------------------- */

void iemlib1_setup(void)
{
  iemlib1_class = class_new(gensym("iemlib1"), iemlib1_new, 0,
    sizeof(t_object), CLASS_NOINLET, 0);
  
  biquad_freq_resp_setup();
  db2v_setup();
  f2note_setup();
  filter_tilde_setup();
  FIR_tilde_setup();
  forpp_setup();
  gate_setup();
  hml_shelf_tilde_setup();
  iem_cot4_tilde_setup();
  iem_delay_tilde_setup();
  iem_pow4_tilde_setup();
  iem_sqrt4_tilde_setup();
  lp1_t_tilde_setup();
  mov_avrg_kern_tilde_setup();
  para_bp2_tilde_setup();
  peakenv_tilde_setup();
  peakenv_hold_tilde_setup();
  prvu_tilde_setup();
  pvu_tilde_setup();
  rvu_tilde_setup();
  sin_phase_tilde_setup();
  soundfile_info_setup();
  split_setup();
  v2db_setup();
  vcf_filter_tilde_setup();
  
	post("iemlib1 (R-1.17) library loaded!   (c) Thomas Musil 11.2008");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
