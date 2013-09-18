/* --------------------- SMLib ----------------------------- */
/*

  Signal processing for Mapping
	objects:

  float stream
		- pid controller?
		- deltas (generate difference vector between current values and values of the past)
  vector 
		- vquant (quantizer with hysteresis)
		- vv/
		- vv>
		- vv<
		- s2v (stream to vector, incl ola)
		- v2s (vector to stream, incl ola)

  vector math
		- vreverse

		- delread
		- delwrite
		- upsample
		- fir
		- autorescale
*/	

#include "defines.h"


// in alphabetical order
extern void bp_setup();
extern void decimator_setup();
extern void	deltas_setup();
extern void	hip_setup();
extern void hist_setup();
extern void itov_setup();
extern void lavg_setup();
extern void lhist_setup();
extern void lhisti_setup();
extern void linspace_setup();
extern void lmax_setup();
extern void lmin_setup();
extern void lrange_setup();
extern void lstd_setup();
extern void prevl_setup();
extern void threshold_setup();
extern void vabs_setup();
extern void vclip_setup();
extern void vcog_setup();
extern void vdbtorms_setup();
extern void vdelta_setup();
extern void vfmod_setup();
extern void vftom_setup();
extern void vlavg_setup();
extern void vlmax_setup();
extern void vlmin_setup();
extern void vlrange_setup();
extern void vmax_setup();
extern void vmin_setup();
extern void vmtof_setup();
extern void vpow_setup();
extern void vrms_setup();
extern void vrmstodb_setup();
extern void vstd_setup();
extern void vsum_setup();
extern void vthreshold_setup();
extern void vvconv_setup();
extern void vvminus_setup();
extern void vvplus_setup();

static t_class *SMLib_class;

typedef struct _lstd
{
    t_object x_obj;
} t_SMLib;

static void SMLib_help(t_SMLib *x)
{
	/*
	*/
	post("");
	post("");
	post("    ..........................................................");
	post("    . SMLib                                                  .");
	post("    .   Signal processing for Mapping                        .");
	post("    .     v0.12 24/11/2002                                   .");
	post("    ..........................................................");
	post("    . processing stream of floats (context) (float output)   .");
	post("    .                                                        .");
	post("    .   lavg        leaky average                            .");
	post("    .   lmax        leaky maximum                            .");
	post("    .   lmin        leaky minimum                            .");
	post("    .   lrange      leaky range                              .");
	post("    u   lstd        leaky standard deviation                 .");
	post("    u   decimator   passes 1 in n input values               .");
	post("    .   threshold   detection with hysteresis                .");
	post("    .   hip         first order high-pass filter             .");
	post("    .   bp          second order (resonant) high-pass filter .");
	post("    ..........................................................");
	post("    . analyzing stream of floats (vector output)             .");
	post("    .                                                        .");
	post("    .   hist        histogram                                .");
	post("    .   lhist       leaky histogram, clips samples           .");
	post("    .   lhisti      leaky histogram, ignore samples          .");
	post("    .                 outside bins                           .");
	post("    .   itov        bin index to value (for the histograms   .");
	post("    .   prevl       previous floats in a list                .");
	post("    .   deltas      difference between last float and        .");
	post("    .                 previous floats                        .");
//	post("    o   filterbank  lineairly spaced set of bandpass filters .");
	post("    ..........................................................");
	post("    . immediate vector analysis (float output)               .");
	post("    .                                                        .");
	post("    .   vsum        sum of vector elements                   .");
	post("    .   vcog        center of gravity                        .");
	post("    .   vmax        maximum and its location                 .");
	post("    .   vmin        minimum and its location                 .");
	post("    .   vrms        root mean square                         .");
	post("    .   vstd        standard deviation                       .");
	post("    ..........................................................");
	post("    . vector processors (vector output)                      .");
	post("    .                                                        .");
	post("    .   vv+         vector addition                          .");
	post("    .   vv-         vector substraction                      .");
	post("    .   vvconv      vector convolution                       .");
	post("    .   vclip       clip elements                            .");
	post("    .   vfmod       floating point modulo                    .");
	post("    .   vpow        power                                    .");
	post("    .   vthreshold  detections with hysteresises             .");
	post("    .                                                        .");
	post("    . unit conversions on vectors                            .");
	post("    .                                                        .");
	post("    .   vftom       frequency to midi                        .");
	post("    .   vmtof       midi to frequency                        .");
	post("    .   vdbtorms    dB to rms                                .");
	post("    .   vrmstodb    rms to dB                                .");
	post("    ..........................................................");
	post("    . vector synthesis (vector output)                       .");
	post("    .                                                        .");
	post("    .   linspace    linearly spaced vector                   .");
//	post("    o   logspace    logarithmically spaced vector            .");
//	post("    o   rand        uniformly distributed random vector      .");
//	post("    o   randn       normally distributed random vector       .");
	post("    ..........................................................");
	post("    . vector stream processing (vector output) (context)     .");
	post("    .                                                        .");
	post("    .   vlavg       leaky averages                           .");
	post("    .   vlmax       leaky maxima                             .");
	post("    .   vlmin       leaky minima                             .");
	post("    .   vlrange     leaky ranges                             .");
	post("    .   vdelta      difference between successive vectors    .");
	post("    ..........................................................");
//	post("    .              o    = future additions                   .");
//	post("    .              e    = experimental                       .");
	post("    .              u    = undocumented                       .");
	post("    .              j#|@ = johannes.taelman@rug.ac.be         .");
	post("    ..........................................................");
	post("");
	post("");
}

static void *SMLib_new()
{
	t_SMLib *x=(t_SMLib *)pd_new(SMLib_class);
	return (void *)x;
}

#ifdef WIN32
__declspec(dllexport) void __cdecl SMLib_setup( void)
#else
void SMLib_setup( void)
#endif
{
	// dummy object for help-system
    SMLib_class = class_new(gensym("SMLib"),
    	(t_newmethod)SMLib_new, 0,
		sizeof(t_SMLib), 
		CLASS_DEFAULT,
	    0);
    class_addbang(SMLib_class, (t_method)SMLib_help);
	class_addmethod(SMLib_class, (t_method)SMLib_help, gensym("help"),0);
		
	// real objects in alphabetical order
	bp_setup();
	decimator_setup();
	deltas_setup();
	hip_setup();
	hist_setup();
	itov_setup();
	lavg_setup();
	lhist_setup();
	lhisti_setup();
	linspace_setup();
	lmax_setup();
	lmin_setup();
	lrange_setup();
	lstd_setup();
	prevl_setup();
	threshold_setup();
	vabs_setup();
	vclip_setup();
	vcog_setup();
	vdbtorms_setup();
	vdelta_setup();
	vfmod_setup();
	vftom_setup();
	vlavg_setup();
	vlmax_setup();
	vlmin_setup();
	vlrange_setup();
	vmax_setup();
	vmin_setup();
	vmtof_setup();
	vpow_setup();
	vrms_setup();
	vrmstodb_setup();
	vstd_setup();
	vsum_setup();
	vthreshold_setup();
	vvconv_setup();
	vvminus_setup();
	vvplus_setup();

	post("");
	post("    ..........................................................");
	post("    .   SMLib                                                .");
	post("    .    Signal processing for Mapping                       .");
	post("    .     v0.12 24/11/2002                                   .");
	post("    ..........................................................");
	post("");

}
