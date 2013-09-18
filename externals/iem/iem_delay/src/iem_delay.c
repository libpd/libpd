/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_delay written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_delay_class;

static void *iem_delay_new(void)
{
	t_object *x = (t_object *)pd_new(iem_delay_class);
    
	return (x);
}

void n_delay1p_line_tilde_setup(void);
void n_delay2p_line_tilde_setup(void);
void nz_tilde_setup(void);
void block_delay_tilde_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_delay_setup(void)
{
	n_delay1p_line_tilde_setup();
	n_delay2p_line_tilde_setup();
	nz_tilde_setup();
	block_delay_tilde_setup();

    post("iem_delay (R-1.17) library loaded!   (c) Thomas Musil 11.2006");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
