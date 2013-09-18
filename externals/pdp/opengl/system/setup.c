#include "pdp_opengl.h"

/* 3dp overview:

 - texture packets (gl)
 - drawable packets (glX windows and pbufs)

 the 3dp system connects to a display server and creates a common context
 this can be a pbuf context (if supported, glx >= 1.3) or a normal glX context
 textures are standard opengl
 drawable packets are wrappers around glx drawables (windows or pbufs)
 they share the central display connection and rendering context

*/


#ifdef __cplusplus
extern "C"
{
#endif

/* opengl lib kernel setup */
void pdp_opengl_system_setup(void);

/* packet type setup */
void pdp_3Dcontext_glx_setup(void); /* glx specific part of the 3D context packet */
void pdp_3Dcontext_common_setup(void); /* common part of the 3D context packet */
void pdp_texture_setup(void); /* texture packet */


/* module setup */
void pdp_3d_windowcontext_setup(void);
void pdp_3d_draw_setup(void);
void pdp_3d_view_setup(void);
void pdp_3d_light_setup(void);
void pdp_3d_color_setup(void);
void pdp_3d_push_setup(void);
void pdp_3d_snap_setup(void);
void pdp_3d_dlist_setup(void);
void pdp_3d_drawmesh_setup(void);
void pdp_3d_for_setup(void);
void pdp_3d_state_setup(void);
void pdp_3d_subcontext_setup(void);


    //#define D(x) { pdp_post_n( #x ".." ); x; pdp_post("done"); }
#define D(x) x


void pdp_setup(void);
void pdp_opengl_setup(void)
{
    int i;

    pdp_setup();

    post("PDP: opengl extensions version " PDP_VERSION);

    /* setup system */
    D(pdp_opengl_system_setup());

    /* setup packet types */
    D(pdp_3Dcontext_glx_setup());
    D(pdp_3Dcontext_common_setup());
    D(pdp_texture_setup());


    /* setup modules */
    D(pdp_3d_windowcontext_setup());
    D(pdp_3d_draw_setup());
    D(pdp_3d_view_setup());
    D(pdp_3d_push_setup());
    D(pdp_3d_light_setup());
    D(pdp_3d_dlist_setup());
    D(pdp_3d_color_setup());
    D(pdp_3d_snap_setup());
    D(pdp_3d_drawmesh_setup());
    D(pdp_3d_for_setup());
    D(pdp_3d_state_setup());
    D(pdp_3d_subcontext_setup());


}


#ifdef __cplusplus
}
#endif
