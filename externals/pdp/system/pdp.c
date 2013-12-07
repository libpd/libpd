/*
 *   Pure Data Packet system implementation: setup code
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include "pdp_config.h"
#include "pdp_post.h"

static int initialized = 0;

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

/* module setup declarations (all C-style) */

/* pdp system / internal stuff */
void pdp_debug_setup(void);
void pdp_list_setup(void);
void pdp_pdsym_setup(void);
void pdp_forth_setup(void);
void pdp_forth_def_setup(void);
void pdp_symbol_setup(void);
void pdp_type_setup(void);
void pdp_packet_setup(void);
void pdp_ut_setup(void);
void pdp_queue_setup(void);
void pdp_control_setup(void);
void pdp_image_setup(void);
void pdp_bitmap_setup(void);
void pdp_matrix_setup(void);

/* pdp modules */
void pdp_xv_setup(void);
void pdp_add_setup(void);
void pdp_mul_setup(void);
void pdp_mix_setup(void);
void pdp_randmix_setup(void);
void pdp_qt_setup(void);
void pdp_v4l_setup(void);
void pdp_reg_setup(void);
void pdp_conv_setup(void);
void pdp_bq_setup(void);
void pdp_del_setup(void);
void pdp_snap_setup(void);
void pdp_trigger_setup(void);
void pdp_route_setup(void);
void pdp_noise_setup(void);
void pdp_gain_setup(void);
void pdp_chrot_setup(void);
void pdp_scope_setup(void);
void pdp_scale_setup(void);
void pdp_zoom_setup(void);
void pdp_scan_setup(void);
void pdp_scanxy_setup(void);
void pdp_sdl_setup(void);
void pdp_cheby_setup(void);
void pdp_grey2mask_setup(void);
void pdp_constant_setup(void);
void pdp_logic_setup(void);
void pdp_glx_setup(void);
void pdp_loop_setup(void);
void pdp_description_setup(void);
void pdp_convert_setup(void);
void pdp_stateless_setup(void);
void pdp_mat_mul_setup(void);
void pdp_mat_lu_setup(void);
void pdp_mat_vec_setup(void);
void pdp_plasma_setup(void);
void pdp_cog_setup(void);
void pdp_histo_setup(void);
void pdp_array_setup(void);
void pdp_udp_send_setup(void);
void pdp_udp_receive_setup(void);
void pdp_rawin_setup(void);
void pdp_rawout_setup(void);
void pdp_metro_setup(void);


/* hacks */
void pdp_inspect_setup(void);

/* testing */
void pdp_dpd_test_setup(void);




/* library setup routine */
void pdp_setup(void){
    
    if (initialized) return;

    /* babble */
#ifdef PDP_VERSION
# if PD_MAJOR_VERSION==0 && PD_MINOR_VERSION<43
    pdp_post("PDP: pure data packet version " PDP_VERSION );
# else
    logpost(NULL, 3, "PDP: pure data packet version " PDP_VERSION );
# endif
#else
    pdp_post ("PDP: pure data packet");
#endif


    /* setup pdp system */

    /* kernel */
    pdp_pdsym_setup();
    pdp_debug_setup();
    pdp_symbol_setup();
    pdp_list_setup();
    pdp_type_setup();
    pdp_packet_setup();
    pdp_control_setup();

    /* types */
    pdp_image_setup();
    pdp_bitmap_setup();



#ifdef HAVE_PDP_GSL
    pdp_matrix_setup();
#endif

    pdp_queue_setup();

    /* setup utility toolkit */
    pdp_ut_setup();

    /* setup pdp pd  modules*/
    pdp_add_setup();
    pdp_mul_setup();
    pdp_mix_setup();
    pdp_randmix_setup();
    pdp_reg_setup();
    pdp_conv_setup();
    pdp_bq_setup();
    pdp_del_setup();
    pdp_snap_setup();
    pdp_trigger_setup();
    pdp_route_setup();
    pdp_noise_setup();
    pdp_plasma_setup();
    pdp_gain_setup();
    pdp_chrot_setup();
    pdp_scope_setup();
    pdp_scale_setup();
    pdp_zoom_setup();
    pdp_scan_setup();
    pdp_scanxy_setup();


    pdp_grey2mask_setup();
    pdp_constant_setup();
    pdp_logic_setup();
    pdp_loop_setup();
    pdp_description_setup();
    pdp_convert_setup();
    pdp_stateless_setup();


    pdp_cog_setup();
    pdp_array_setup();
    pdp_rawin_setup();
    pdp_rawout_setup();
    pdp_metro_setup();


    /* experimental stuff */
    pdp_inspect_setup();
    pdp_udp_send_setup();
    pdp_udp_receive_setup();

    /* testing */
    //pdp_dpd_test_setup();

    /* optional stuff */

#ifdef HAVE_PDP_READLINE
    pdp_forthconsole_setup();
#endif

#ifdef HAVE_PDP_GSL
    pdp_histo_setup();
    pdp_cheby_setup();
    pdp_mat_mul_setup();
    pdp_mat_lu_setup();
    pdp_mat_vec_setup();
#endif


#ifdef HAVE_PDP_QT
    pdp_qt_setup();
#endif

#ifdef HAVE_PDP_XV
    pdp_xv_setup();
#endif

#ifdef HAVE_PDP_SDL
    pdp_sdl_setup();
#endif

#ifdef HAVE_PDP_V4L
    pdp_v4l_setup();
#endif

#ifdef HAVE_PDP_GLX
    pdp_glx_setup();
#endif

    initialized++;


}

#ifdef __cplusplus
}
#endif
