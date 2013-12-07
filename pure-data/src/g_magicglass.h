
#ifndef __g_magicglass_h_
#define __g_magicglass_h_

typedef struct _magicGlass
{
    t_object x_obj;
    t_object *x_connectedObj;
    int x_connectedOutno;
    int x_visible;
    char x_string[4096];
    char x_old_string[4096];
    int x_x;
    int x_y;
    t_glist *x_c;
    float x_sigF;
    int x_dspOn;
    int x_viewOn;
    float x_maxSample;
    int x_sampleCount;
    t_clock *x_clearClock;
    t_clock *x_flashClock;
    unsigned int x_maxSize;
    unsigned int x_issignal;
    int x_display_font;
} t_magicGlass;

EXTERN void magicGlass_bind(t_magicGlass *x, t_object *obj, int outno);
EXTERN void magicGlass_unbind(t_magicGlass *x);
EXTERN void magicGlass_bang(t_magicGlass *x);
EXTERN void magicGlass_float(t_magicGlass *x, t_float f);
EXTERN void magicGlass_symbol(t_magicGlass *x, t_symbol *sym);
EXTERN void magicGlass_anything(t_magicGlass *x, t_symbol *sym, int argc, t_atom *argv);
EXTERN void magicGlass_list(t_magicGlass *x, t_symbol *sym, int argc, t_atom *argv);
EXTERN void magicGlass_setCanvas(t_magicGlass *x, t_glist *c);
EXTERN void magicGlass_show(t_magicGlass *x);
EXTERN void magicGlass_hide(t_magicGlass *x);
EXTERN void magicGlass_moveText(t_magicGlass *x, int pX, int pY);
EXTERN int magicGlass_bound(t_magicGlass *x);
EXTERN int magicGlass_isOn(t_magicGlass *x);
EXTERN void magicGlass_setOn(t_magicGlass *x, int i);
EXTERN void magicGlass_setDsp(t_magicGlass *x, int i);
EXTERN void *magicGlass_new(t_glist *c);
EXTERN void magicGlass_free(t_magicGlass *x);
EXTERN void magicGlass_setup(void);

#endif /* __g_magicglass_h_ */
