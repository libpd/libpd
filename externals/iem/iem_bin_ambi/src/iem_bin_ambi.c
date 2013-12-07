/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_bin_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */


#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_bin_ambi_class;

static void *iem_bin_ambi_new(void)
{
	t_object *x = (t_object *)pd_new(iem_bin_ambi_class);
    
	return (x);
}

void bin_ambi_calc_HRTF_setup(void);
void bin_ambi_reduced_decode_setup(void);
void bin_ambi_reduced_decode2_setup(void);
void bin_ambi_reduced_decode_fft_setup(void);
void bin_ambi_reduced_decode_fir_setup(void);
void bin_ambi_reduced_decode_fft2_setup(void);
void bin_ambi_reduced_decode_fir2_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_bin_ambi_setup(void)
{
	bin_ambi_calc_HRTF_setup();
	bin_ambi_reduced_decode_setup();
	bin_ambi_reduced_decode2_setup();
	bin_ambi_reduced_decode_fft_setup();
	bin_ambi_reduced_decode_fir_setup();
	bin_ambi_reduced_decode_fft2_setup();
	bin_ambi_reduced_decode_fir2_setup();

    post("iem_bin_ambi (R-1.18) library loaded!   (c) Thomas Musil 01.2009");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
