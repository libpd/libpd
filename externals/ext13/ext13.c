#ifndef VERSION
#define VERSION "0.17"
#endif

#include "m_pd.h"

#ifndef __DATE__ 
#define __DATE__ "05/22/01"
#endif

typedef struct _ext13
{
     t_object x_obj;
} t_ext13;

static t_class* ext13_class;

void kalashnikov_setup();
void filesize_setup();
void openpatch_setup();
void sigsend13_setup();
void sigcatch13_setup();
void sigthrow13_setup();
void sigreceive13_setup();
void strippath_setup();
void streamin13_setup();
void streamout13_setup();
void pipewrite_tilde_setup();
void piperead_tilde_setup();
void wavinfo_setup();
void ftos_setup();
void mandelbrot_setup();
void scramble_tile_setup();
void promiscous_tilde_setup();
void ossmixer_setup();
void send13_setup();
void receive13_setup();
void cdplayer_setup();
void sfwrite13_setup();

static void* ext13_new(t_symbol* s) {
    t_ext13 *x = (t_ext13 *)pd_new(ext13_class);
    return (x);
}

void ext13_setup(void) 
{
    ext13_class = class_new(gensym("ext13"), (t_newmethod)ext13_new, 0,
    	sizeof(t_ext13), 0,0);

     kalashnikov_setup();
     filesize_setup();
     openpatch_setup();
     sigsend13_setup();
     sigcatch13_setup();
     sigthrow13_setup();
     sigreceive13_setup();
     strippath_setup();
     streamin13_setup();
     streamout13_setup();
     piperead_tilde_setup();
     pipewrite_tilde_setup();
     wavinfo_setup();
     ftos_setup();
     mandelbrot_tilde_setup();
     scramble_tilde_setup();
     promiscous_tilde_setup();
     ossmixer_setup();
     send13_setup();
     receive13_setup();
     cdplayer_setup();
     sfwrite13_setup();

     post("EXT13 by dieb13@klingt.org");
     post("EXT13: version: "VERSION);
     post("EXT13: compiled: "__DATE__);
}
