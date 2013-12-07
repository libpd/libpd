/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemgui written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include "iemgui.h"
#include "g_canvas.h"
#include "g_all_guis.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef MSW
#include <io.h>
#else
#include <unistd.h>
#endif

#define IEM_GUI_IVNT_COLOR_SELECTED        "#0000FF"
#define IEM_GUI_IVNT_COLOR_NORMAL          "#000000"

/* ---------- iem_event   ---------------- */

t_widgetbehavior iem_event_widgetbehavior;
static t_class *iem_event_class;

typedef struct _iem_event
{
  t_iemgui x_gui;
  t_int    x_x;
  t_int    x_y;
  t_int    x_doit;
  t_symbol *x_mouse_shft_alt;
  t_symbol *x_dragg_x_y;
  t_symbol *x_key;
  t_symbol *x_move_x_y;
  t_atom   x_at_out[3];
} t_iem_event;

static void iem_event_key(void *z, t_floatarg fkey);
t_widgetbehavior iem_event_widgetbehavior;
static t_class *iem_event_class;

static void iem_event_draw_new(t_iem_event *x, t_glist *glist)
{
  t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
  t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d  -outline %s -tags %lxBASE\n",
    canvas, xpos, ypos, xpos + x->x_gui.x_w, ypos + x->x_gui.x_h, IEM_GUI_IVNT_COLOR_SELECTED, x);
}
static void iem_event_draw_move(t_iem_event *x, t_glist *glist)
{
  t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
  t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d\n",
    canvas, x, xpos, ypos, xpos + x->x_gui.x_w, ypos + x->x_gui.x_h);
}

static void iem_event_draw_erase(t_iem_event* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
}

static void iem_event_draw_select(t_iem_event *x, t_glist *glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
  {
    t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
    t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
    
    sys_vgui(".x%lx.c create rectangle %d %d %d %d  -outline %s -tags %lxBASE\n",
      canvas, xpos, ypos, xpos + x->x_gui.x_w, ypos + x->x_gui.x_h, IEM_GUI_IVNT_COLOR_SELECTED, x);
  }
  else
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
}

void iem_event_draw(t_iem_event *x, t_glist *glist, int mode)
{
  if(mode == IEM_GUI_DRAW_MODE_MOVE)
    iem_event_draw_move(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_NEW)
    iem_event_draw_new(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_SELECT)
    iem_event_draw_select(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_ERASE)
    iem_event_draw_erase(x, glist);
}

/* ------------------------ cnv widgetbehaviour----------------------------- */

static void iem_event_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
  t_iem_event *x = (t_iem_event *)z;
  
  *xp1 = text_xpix(&x->x_gui.x_obj, glist);
  *yp1 = text_ypix(&x->x_gui.x_obj, glist);
  *xp2 = *xp1 + x->x_gui.x_w;
  *yp2 = *yp1 + x->x_gui.x_h;
}

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void iem_event_save(t_gobj *z, t_binbuf *b)
{
  t_iem_event *x = (t_iem_event *)z;
  t_symbol *srl[3];
  
  srl[0] = x->x_gui.x_snd;
  srl[1] = x->x_gui.x_rcv;
  srl[2] = gensym("empty");
  iemgui_all_sym2dollararg(&x->x_gui, srl);
  
  binbuf_addv(b, "ssiisiiiiss", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    x->x_gui.x_w, x->x_gui.x_h,
    iem_symargstoint(&x->x_gui.x_isa), iem_fstyletoint(&x->x_gui.x_fsf),
    srl[0], srl[1]);
  binbuf_addv(b, ";");
}
#else
static void iem_event_save(t_gobj *z, t_binbuf *b)
{
  t_iem_event *x = (t_iem_event *)z;
  int *ip1, *ip2;
  t_symbol *srl[3];
  
  srl[0] = x->x_gui.x_snd;
  srl[1] = x->x_gui.x_rcv;
  srl[2] = gensym("empty");
  iemgui_all_unique2dollarzero(&x->x_gui, srl);
  iemgui_all_sym2dollararg(&x->x_gui, srl);
  ip1 = (int *)(&x->x_gui.x_isa);
  ip2 = (int *)(&x->x_gui.x_fsf);
  binbuf_addv(b, "ssiisiiiiss", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    x->x_gui.x_w, x->x_gui.x_h,
    (*ip1)&IEM_INIT_ARGS_ALL, (*ip2)&IEM_FSTYLE_FLAGS_ALL,
    srl[0], srl[1]);
  binbuf_addv(b, ";");
}
#endif

static void iem_event_motion(t_iem_event *x, t_floatarg dx, t_floatarg dy)
{
  x->x_x += (t_int)dx;
  x->x_y -= (t_int)dy;
  
  SETFLOAT(x->x_at_out, (t_float)x->x_x);
  SETFLOAT(x->x_at_out+1, (t_float)x->x_y);
  outlet_anything(x->x_gui.x_obj.ob_outlet, x->x_dragg_x_y, 2, x->x_at_out);
  if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
    typedmess(x->x_gui.x_snd->s_thing, x->x_dragg_x_y, 2, x->x_at_out);
}

static void iem_event_key(void *z, t_floatarg fkey)
{
  t_iem_event *x = z;
  char c = (char)fkey;
  
  if(x->x_gui.x_fsf.x_change)
  {
    if((c==0))
    {
      x->x_gui.x_fsf.x_change = 0;
    }
    else if((c=='\n')||(c==13))
    {
      x->x_gui.x_fsf.x_change = 0;
      SETFLOAT(x->x_at_out, 0.0f);
      outlet_anything(x->x_gui.x_obj.ob_outlet, x->x_key, 1, x->x_at_out);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        typedmess(x->x_gui.x_snd->s_thing, x->x_key, 1, x->x_at_out);
    }
    else if((c=='\b')||(c==127))
    {
      SETFLOAT(x->x_at_out, 127.0f);
      outlet_anything(x->x_gui.x_obj.ob_outlet, x->x_key, 1, x->x_at_out);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        typedmess(x->x_gui.x_snd->s_thing, x->x_key, 1, x->x_at_out);
    }
    else
    {
      SETFLOAT(x->x_at_out, fkey);
      outlet_anything(x->x_gui.x_obj.ob_outlet, x->x_key, 1, x->x_at_out);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        typedmess(x->x_gui.x_snd->s_thing, x->x_key, 1, x->x_at_out);
    }
  }
}

static int iem_event_click(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
  t_iem_event* x = (t_iem_event *)z;
  t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
  t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
  
  if(doit != x->x_doit)
  {
    SETFLOAT(x->x_at_out, (t_float)doit);
    SETFLOAT(x->x_at_out+1, (t_float)shift);
    SETFLOAT(x->x_at_out+2, (t_float)(alt?1:0));
    outlet_anything(x->x_gui.x_obj.ob_outlet, x->x_mouse_shft_alt, 3, x->x_at_out);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
      typedmess(x->x_gui.x_snd->s_thing, x->x_mouse_shft_alt, 3, x->x_at_out);
    if(doit)
      x->x_gui.x_fsf.x_change = 1;
    x->x_doit = doit;
  }
  
  x->x_x = xpix - xpos;
  x->x_y = x->x_gui.x_h - (ypix - ypos);
  SETFLOAT(x->x_at_out, (t_float)x->x_x);
  SETFLOAT(x->x_at_out+1, (t_float)x->x_y);
  if(doit)
  {
    glist_grab(x->x_gui.x_glist, &x->x_gui.x_obj.te_g,
      (t_glistmotionfn)iem_event_motion, iem_event_key, (t_float)xpix, (t_float)ypix);
    
    outlet_anything(x->x_gui.x_obj.ob_outlet, x->x_dragg_x_y, 2, x->x_at_out);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
      typedmess(x->x_gui.x_snd->s_thing, x->x_dragg_x_y, 2, x->x_at_out);
  }
  else
  {
    outlet_anything(x->x_gui.x_obj.ob_outlet, x->x_move_x_y, 2, x->x_at_out);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
      typedmess(x->x_gui.x_snd->s_thing, x->x_move_x_y, 2, x->x_at_out);
  }
  return (1);
}

/*static void iem_event_bang(t_iem_event *x)
{
outlet_anything(x->x_gui.x_obj.ob_outlet, atom_getsymbol(x->x_matrix), x->x_n_row*x->x_n_column+2, x->x_matrix+1);
if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
typedmess(x->x_gui.x_snd->s_thing, atom_getsymbol(x->x_matrix), x->x_n_row*x->x_n_column+2, x->x_matrix+1);
}*/

static void iem_event_delta(t_iem_event *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void iem_event_pos(t_iem_event *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}

static void iem_event_size(t_iem_event *x, t_symbol *s, int ac, t_atom *av)
{
  int h, w;
  
  if((ac >= 2)&&IS_A_FLOAT(av, 0) && IS_A_FLOAT(av, 1))
  {
    w = (int)atom_getintarg(0, ac, av);
    if(w < 4)
      w = 4;
    x->x_gui.x_w = w;
    if(ac > 1)
    {
      h = (int)atom_getintarg(1, ac, av);
      if(h < 4)
        h = 4;
      x->x_gui.x_h = h;
    }
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_NEW);
    canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
  }
}

static void iem_event_send(t_iem_event *x, t_symbol *s)
{iemgui_send(x, &x->x_gui, s);}

static void iem_event_receive(t_iem_event *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}


#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void *iem_event_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iem_event *x = (t_iem_event *)pd_new(iem_event_class);
  t_int w=32, h=32;
  
  x->x_gui.x_snd = gensym("empty");
  x->x_gui.x_rcv = gensym("empty");
  x->x_gui.x_lab = gensym("empty");
  x->x_gui.x_fsf.x_font_style = 0;
  if((argc >= 6)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
    &&(IS_A_SYMBOL(argv,4)||IS_A_FLOAT(argv,4))
    &&(IS_A_SYMBOL(argv,5)||IS_A_FLOAT(argv,5)))
  {
    w = (int)atom_getintarg(0, argc, argv);
    h = (int)atom_getintarg(1, argc, argv);
    iem_inttosymargs(&x->x_gui.x_isa, atom_getintarg(2, argc, argv));
    iem_inttofstyle(&x->x_gui.x_fsf, atom_getintarg(3, argc, argv));
    iemgui_new_getnames(&x->x_gui, 4, argv);
  }
  else if((argc >= 2)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1))
  {
    w = (int)atom_getintarg(0, argc, argv);
    h = (int)atom_getintarg(1, argc, argv);
  }
  x->x_gui.x_draw = (t_iemfunptr)iem_event_draw;
  x->x_gui.x_fsf.x_snd_able = 1;
  x->x_gui.x_fsf.x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  if(!strcmp(x->x_gui.x_snd->s_name, "empty"))
    x->x_gui.x_fsf.x_snd_able = 0;
  if(!strcmp(x->x_gui.x_rcv->s_name, "empty"))
    x->x_gui.x_fsf.x_rcv_able = 0;
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  if(w < 4)
    w = 4;
  x->x_gui.x_w = w;
  if(h < 4)
    h = 4;
  x->x_gui.x_h = h;
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  outlet_new(&x->x_gui.x_obj, &s_list);
  x->x_mouse_shft_alt = gensym("mouse_shft_alt");
  x->x_dragg_x_y = gensym("dragg_x_y");
  x->x_key = gensym("key");
  x->x_move_x_y = gensym("move_x_y");
  x->x_x = 0;
  x->x_y = 0;
  x->x_doit = 0;
  return (x);
}
#else
static void *iem_event_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iem_event *x = (t_iem_event *)pd_new(iem_event_class);
  t_int w=32, h=32;
  t_int iinit=0, ifstyle=0;
  t_iem_init_symargs *init=(t_iem_init_symargs *)(&iinit);
  t_iem_fstyle_flags *fstyle=(t_iem_fstyle_flags *)(&ifstyle);
  t_symbol *srl[3];
  char str[144];
  
  x->x_gui.x_snd = gensym("empty");
  x->x_gui.x_rcv = gensym("empty");
  x->x_gui.x_lab = gensym("empty");
  srl[0] = gensym("empty");
  srl[1] = gensym("empty");
  srl[2] = gensym("empty");
  x->x_gui.x_fsf.x_font_style = 0;
  if((argc >= 6)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
    &&(IS_A_SYMBOL(argv,4)||IS_A_FLOAT(argv,4))
    &&(IS_A_SYMBOL(argv,5)||IS_A_FLOAT(argv,5)))
  {
    w = (int)atom_getintarg(0, argc, argv);
    h = (int)atom_getintarg(1, argc, argv);
    iinit = (int)atom_getintarg(2, argc, argv);
    ifstyle = (int)atom_getintarg(3, argc, argv);
    if(IS_A_SYMBOL(argv,4))
      srl[0] = atom_getsymbolarg(4, argc, argv);
    else if(IS_A_FLOAT(argv,4))
    {
      sprintf(str, "%d", (int)atom_getintarg(4, argc, argv));
      srl[0] = gensym(str);
    }
    if(IS_A_SYMBOL(argv,5))
      srl[1] = atom_getsymbolarg(5, argc, argv);
    else if(IS_A_FLOAT(argv,5))
    {
      sprintf(str, "%d", (int)atom_getintarg(5, argc, argv));
      srl[1] = gensym(str);
    }
  }
  else if((argc >= 2)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1))
  {
    w = (int)atom_getintarg(0, argc, argv);
    h = (int)atom_getintarg(1, argc, argv);
  }
  iinit &= IEM_INIT_ARGS_ALL;
  ifstyle &= IEM_FSTYLE_FLAGS_ALL;
  
  x->x_gui.x_draw = (t_iemfunptr)iem_event_draw;
  x->x_gui.x_fsf.x_snd_able = 1;
  x->x_gui.x_fsf.x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  fstyle->x_snd_able = 1;
  fstyle->x_rcv_able = 1;
  if(!strcmp(srl[0]->s_name, "empty"))
    fstyle->x_snd_able = 0;
  if(!strcmp(srl[1]->s_name, "empty"))
    fstyle->x_rcv_able = 0;
  x->x_gui.x_unique_num = 0;
  x->x_gui.x_fsf = *fstyle;
  x->x_gui.x_isa = *init;
  iemgui_first_dollararg2sym(&x->x_gui, srl);
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  if(w < 4)
    w = 4;
  x->x_gui.x_w = w;
  if(h < 4)
    h = 4;
  x->x_gui.x_h = h;
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  outlet_new(&x->x_gui.x_obj, &s_list);
  x->x_mouse_shft_alt = gensym("mouse_shft_alt");
  x->x_dragg_x_y = gensym("dragg_x_y");
  x->x_key = gensym("key");
  x->x_move_x_y = gensym("move_x_y");
  x->x_x = 0;
  x->x_y = 0;
  x->x_doit = 0;
  return (x);
}
#endif

static void iem_event_free(t_iem_event *x)
{
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  gfxstub_deleteforkey(x);
}

void iem_event_setup(void)
{
  iem_event_class = class_new(gensym("ivnt"), (t_newmethod)iem_event_new,
    (t_method)iem_event_free, sizeof(t_iem_event), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)iem_event_new, gensym("iem_event"),
    A_GIMME, 0);
  //class_addbang(iem_event_class,iem_event_bang);
  class_addmethod(iem_event_class, (t_method)iem_event_motion,
    gensym("motion"), A_FLOAT, A_FLOAT, 0);
  class_addmethod(iem_event_class, (t_method)iem_event_size,
    gensym("size"), A_GIMME, 0);
  class_addmethod(iem_event_class, (t_method)iem_event_delta,
    gensym("delta"), A_GIMME, 0);
  class_addmethod(iem_event_class, (t_method)iem_event_pos,
    gensym("pos"), A_GIMME, 0);
  class_addmethod(iem_event_class, (t_method)iem_event_send,
    gensym("send"), A_DEFSYM, 0);
  class_addmethod(iem_event_class, (t_method)iem_event_receive,
    gensym("receive"), A_DEFSYM, 0);
  
  iem_event_widgetbehavior.w_getrectfn =    iem_event_getrect;
  iem_event_widgetbehavior.w_displacefn =   iemgui_displace;
  iem_event_widgetbehavior.w_selectfn =     iemgui_select;
  iem_event_widgetbehavior.w_activatefn =   NULL;
  iem_event_widgetbehavior.w_deletefn =     iemgui_delete;
  iem_event_widgetbehavior.w_visfn =        iemgui_vis;
  iem_event_widgetbehavior.w_clickfn =      iem_event_click;
  
#if((PD_MAJOR_VERSION)&&(PD_MINOR_VERSION < 37))
  iem_event_widgetbehavior.w_propertiesfn = NULL;
  iem_event_widgetbehavior.w_savefn =       iem_event_save;
#else
  class_setsavefn(iem_event_class, iem_event_save);
#endif
  
  class_setwidget(iem_event_class, &iem_event_widgetbehavior);
//  class_sethelpsymbol(iem_event_class, gensym("iemhelp2/help-iem_event"));
}
