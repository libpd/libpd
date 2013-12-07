/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_adaptfilt written by Markus Noisternig & Thomas Musil 
noisternig_AT_iem.at; musil_AT_iem.at
(c) Institute of Electronic Music and Acoustics, Graz Austria 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_adaptfilt_class;

static void *iem_adaptfilt_new(void)
{
	t_object *x = (t_object *)pd_new(iem_adaptfilt_class);
    
	return (x);
}

void NLMS_tilde_setup(void);
void NLMSerr_in_tilde_setup(void);
void NLMSCC_tilde_setup(void);
void n_CNLMS_tilde_setup(void);
void n_CLNLMS_tilde_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_adaptfilt_setup(void)
{
  NLMS_tilde_setup();
  NLMSerr_in_tilde_setup();
  NLMSCC_tilde_setup();
  n_CNLMS_tilde_setup();
  n_CLNLMS_tilde_setup();
  	
  	post("----------------------------------------------");
	post("iem_adaptfilt (R-1.02) library loaded!");
	post("(c) Markus Noisternig, Thomas Musil");
	post("    {noisternig, musil}_AT_iem.at");
	post("    IEM Graz, Austria");
	post("----------------------------------------------");
}
