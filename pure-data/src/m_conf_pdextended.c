/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* changes by Thomas Musil IEM KUG Graz Austria 2001 */
/* all changes are labeled with      iemlib      */

#include "m_pd.h"

void g_array_setup(void);
void g_canvas_setup(void);
void g_guiconnect_setup(void);
void g_io_setup(void);
void g_scalar_setup(void);
void g_template_setup(void);
void g_text_setup(void);
void g_traversal_setup(void);
void m_pd_setup(void);
void x_acoustics_setup(void);
void x_connective_setup(void);
void x_arithmetic_setup(void);
void d_arithmetic_setup(void);
void d_array_setup(void);
void d_ctl_setup(void);
void d_filter_setup(void);
void d_global_setup(void);
void d_math_setup(void);
void d_osc_setup(void);
void d_soundfile_setup(void);
void d_ugen_setup(void);
void gfxstub_setup(void);
void e_midi_setup(void);
/* kludge until there is a declare API for externals, hans@eds.org */
void import_setup(void);
void path_setup(void);
void initbang_setup(void);
void closebang_setup(void);
void magicGlass_setup(void);

void conf_init(void)
{
    g_array_setup();
    g_canvas_setup();
    g_guiconnect_setup();
    g_io_setup();
    g_scalar_setup();
    g_template_setup();
    g_text_setup();
    g_traversal_setup();
    m_pd_setup();
    x_acoustics_setup();
    x_connective_setup();
    x_arithmetic_setup();
    d_arithmetic_setup();
    d_array_setup();
    d_ctl_setup();
    d_delay_setup();
    d_filter_setup();
    d_global_setup();
    d_math_setup();
    d_osc_setup();
    d_soundfile_setup();
    d_ugen_setup();
    gfxstub_setup();
    e_midi_setup();
    import_setup();
    path_setup();
    print_setup();
    loadbang_setup();
    initbang_setup();
    closebang_setup();
    magicGlass_setup();
}

