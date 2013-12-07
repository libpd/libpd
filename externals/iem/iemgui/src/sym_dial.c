/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemgui written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2007 */

#include "m_pd.h"
#include "iemlib.h"
#include "iemgui.h"
#include "g_canvas.h"
#include "g_all_guis.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef MSW
#include <io.h>
#else
#include <unistd.h>
#endif

#define IEM_SDL_FLASH_TIME 200

t_widgetbehavior sym_dial_widgetbehavior;
static t_class *sym_dial_class;

typedef struct _sym_dial
{
  t_iemgui x_gui;
  t_clock  *x_clock;
  void     *x_outlet;
  int      x_index;
  t_symbol **x_syms;
  int      x_ac;
  int      x_max_ac;
  int      x_symwidth;
  int      x_snd_flt0_sym1;
  
} t_sym_dial;

/* widget helper functions */

static void sym_dial_draw_swap(t_sym_dial *x, t_glist *glist, int swap)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n",
    canvas, x,
    swap?x->x_gui.x_fcol:x->x_gui.x_bcol);
  sys_vgui(".x%lx.c itemconfigure %lxSYMBOL -fill #%6.6x\n",
    canvas, x,
    swap?x->x_gui.x_bcol:x->x_gui.x_fcol);
}

static void sym_dial_tick(t_sym_dial *x)
{
  sym_dial_draw_swap(x, x->x_gui.x_glist, 0);
}

static void sym_dial_inc(t_sym_dial *x)
{
  x->x_index = (x->x_index + 1) % x->x_ac;
}

void sym_dial_calc_fontwidth(t_sym_dial *x)
{
  int w, f=31;
  
  if(x->x_gui.x_fsf.x_font_style == 1)
    f = 27;
  else if(x->x_gui.x_fsf.x_font_style == 2)
    f = 25;
  
  w = x->x_gui.x_fontsize * f * x->x_gui.x_w;
  w /= 36;
  x->x_symwidth = w + (x->x_gui.x_h / 2) + 4;
}

static void sym_dial_draw_update(t_sym_dial *x, t_glist *glist)
{
  if (glist_isvisible(glist))
  {
    char string[200];
    int l;
    
    strcpy(string, x->x_syms[x->x_index]->s_name);
    l = strlen(string);
    if(l > x->x_gui.x_w)
    {
      string[x->x_gui.x_w-1] = '~';
      string[x->x_gui.x_w] = 0;
    }
    sys_vgui(".x%lx.c itemconfigure %lxSYMBOL -fill #%6.6x -text {%s} \n",
      glist_getcanvas(glist), x,
      x->x_gui.x_fsf.x_selected?IEM_GUI_COLOR_SELECTED:x->x_gui.x_fcol,
      string);
  }
}

static void sym_dial_draw_new(t_sym_dial *x, t_glist *glist)
{
  int half=x->x_gui.x_h/2, d=x->x_gui.x_h/34;
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  char string[200];
  int l;
  
  strcpy(string, x->x_syms[x->x_index]->s_name);
  l = strlen(string);
  if(l > x->x_gui.x_w)
  {
    string[x->x_gui.x_w] = '~';
    string[x->x_gui.x_w+1] = 0;
  }
  
  sys_vgui(".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d %d %d -outline #%6.6x -fill #%6.6x -tags %lxBASE\n",
    canvas, xpos-1, ypos,
    xpos + x->x_symwidth-4, ypos,
    xpos + x->x_symwidth, ypos+4,
    xpos + x->x_symwidth, ypos + x->x_gui.x_h,
    xpos-1, ypos + x->x_gui.x_h,
    xpos+half-1, ypos+half,
    IEM_GUI_COLOR_NORMAL, x->x_gui.x_bcol, x);
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w \
      -font {%s %d bold} -fill #%6.6x -tags %lxLABEL\n",
      canvas, xpos+x->x_gui.x_ldx, ypos+x->x_gui.x_ldy,
      strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"",
      x->x_gui.x_font, x->x_gui.x_fontsize, x->x_gui.x_lcol, x);
      sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w \
        -font {%s %d bold} -fill #%6.6x -tags %lxSYMBOL\n",
        canvas, xpos+half+2, ypos+half+d,
        string, x->x_gui.x_font, x->x_gui.x_fontsize, x->x_gui.x_fcol, x);
      if(!x->x_gui.x_fsf.x_snd_able)
      {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
          canvas,
          xpos, ypos + x->x_gui.x_h-1,
          xpos+IOWIDTH, ypos + x->x_gui.x_h,
          x, 0);
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
          canvas,
          xpos+x->x_symwidth-IOWIDTH, ypos + x->x_gui.x_h-1,
          xpos+x->x_symwidth, ypos + x->x_gui.x_h, x, 1);
      }
      if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos, ypos, xpos+IOWIDTH, ypos+1, x, 0);
}

static void sym_dial_draw_move(t_sym_dial *x, t_glist *glist)
{
  int half = x->x_gui.x_h/2, d=x->x_gui.x_h/34;
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d %d %d %d %d %d %d %d %d\n",
    canvas, x, xpos-1, ypos,
    xpos + x->x_symwidth-4, ypos,
    xpos + x->x_symwidth, ypos+4,
    xpos + x->x_symwidth, ypos + x->x_gui.x_h,
    xpos-1, ypos + x->x_gui.x_h,
    xpos+half-1, ypos+half);
  sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
    canvas, x, xpos+x->x_gui.x_ldx, ypos+x->x_gui.x_ldy);
  sys_vgui(".x%lx.c coords %lxSYMBOL %d %d\n",
    canvas, x, xpos+half+2, ypos+half+d);
  if(!x->x_gui.x_fsf.x_snd_able)
  {
    sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
      canvas, x, 0,
      xpos, ypos + x->x_gui.x_h-1,
      xpos+IOWIDTH, ypos + x->x_gui.x_h);
    sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
      canvas, x, 1,
      xpos+x->x_symwidth-IOWIDTH, ypos + x->x_gui.x_h-1,
      xpos+x->x_symwidth, ypos + x->x_gui.x_h);
  }
  if(!x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
    canvas, x, 0,
    xpos, ypos,
    xpos+IOWIDTH, ypos+1);
}

static void sym_dial_draw_erase(t_sym_dial* x,t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
  sys_vgui(".x%lx.c delete %lxLABEL\n", canvas, x);
  sys_vgui(".x%lx.c delete %lxSYMBOL\n", canvas, x);
  if(!x->x_gui.x_fsf.x_snd_able)
  {
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 1);
  }
  if(!x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void sym_dial_draw_config(t_sym_dial* x,t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui(".x%lx.c itemconfigure %lxLABEL -font {%s %d bold} -fill #%6.6x -text {%s} \n",
    canvas, x, x->x_gui.x_font, x->x_gui.x_fontsize,
    x->x_gui.x_fsf.x_selected?IEM_GUI_COLOR_SELECTED:x->x_gui.x_lcol,
    strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"");
  sys_vgui(".x%lx.c itemconfigure %lxSYMBOL -font {%s %d bold} -fill #%6.6x \n",
    canvas, x, x->x_gui.x_font, x->x_gui.x_fontsize,
    x->x_gui.x_fsf.x_selected?IEM_GUI_COLOR_SELECTED:x->x_gui.x_fcol);
  sys_vgui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas,
    x, x->x_gui.x_bcol);
}

static void sym_dial_draw_io(t_sym_dial* x,t_glist* glist, int old_snd_rcv_flags)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  if((old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && !x->x_gui.x_fsf.x_snd_able)
  {
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
      canvas,
      xpos, ypos + x->x_gui.x_h-1,
      xpos+IOWIDTH, ypos + x->x_gui.x_h,
      x, 0);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
      canvas,
      xpos+x->x_symwidth-IOWIDTH, ypos + x->x_gui.x_h-1,
      xpos+x->x_symwidth, ypos + x->x_gui.x_h,
      x, 1);
  }
  if(!(old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && x->x_gui.x_fsf.x_snd_able)
  {
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 1);
  }
  if((old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && !x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
    canvas,
    xpos, ypos,
    xpos+IOWIDTH, ypos+1,
    x, 0);
  if(!(old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void sym_dial_draw_select(t_sym_dial *x, t_glist *glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
  {
    //  pd_bind(&x->x_gui.x_obj.ob_pd, iemgui_key_sym2);
    sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_GUI_COLOR_SELECTED);
    sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_GUI_COLOR_SELECTED);
    sys_vgui(".x%lx.c itemconfigure %lxSYMBOL -fill #%6.6x\n", canvas, x, IEM_GUI_COLOR_SELECTED);
  }
  else
  {
    //  pd_unbind(&x->x_gui.x_obj.ob_pd, iemgui_key_sym2);
    sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_GUI_COLOR_NORMAL);
    sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, x->x_gui.x_lcol);
    sys_vgui(".x%lx.c itemconfigure %lxSYMBOL -fill #%6.6x\n", canvas, x, x->x_gui.x_fcol);
  }
}

void sym_dial_draw(t_sym_dial *x, t_glist *glist, int mode)
{
  if(mode == IEM_GUI_DRAW_MODE_UPDATE)
    sym_dial_draw_update(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_MOVE)
    sym_dial_draw_move(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_NEW)
    sym_dial_draw_new(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_SELECT)
    sym_dial_draw_select(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_ERASE)
    sym_dial_draw_erase(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_CONFIG)
    sym_dial_draw_config(x, glist);
  else if(mode >= IEM_GUI_DRAW_MODE_IO)
    sym_dial_draw_io(x, glist, mode - IEM_GUI_DRAW_MODE_IO);
}

/* ------------------------ vsl widgetbehaviour----------------------------- */

static void sym_dial_getrect(t_gobj *z, t_glist *glist,
                             int *xp1, int *yp1, int *xp2, int *yp2)
{
  t_sym_dial* x = (t_sym_dial*)z;
  
  *xp1 = text_xpix(&x->x_gui.x_obj, glist);
  *yp1 = text_ypix(&x->x_gui.x_obj, glist);
  *xp2 = *xp1 + x->x_symwidth;
  *yp2 = *yp1 + x->x_gui.x_h;
}

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void sym_dial_save(t_gobj *z, t_binbuf *b)
{
  t_sym_dial *x = (t_sym_dial *)z;
  int bflcol[3];
  t_symbol *srl[3];
  int i;
  
  iemgui_save(&x->x_gui, srl, bflcol);
  binbuf_addv(b, "ssiisiiiisssiiiiiiiii", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    gensym("sdl"), x->x_gui.x_w, x->x_gui.x_h,
    iem_symargstoint(&x->x_gui.x_isa), x->x_snd_flt0_sym1,
    srl[0], srl[1], srl[2],
    x->x_gui.x_ldx, x->x_gui.x_ldy,
    iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
    bflcol[0], bflcol[1], bflcol[2],
    x->x_index, x->x_ac);
  for(i=0; i<x->x_ac; i++)  /*16 + ac syms*/
  {
    binbuf_addv(b, "s", x->x_syms[i]);
  }
  binbuf_addv(b, ";");
}
#else
static void sym_dial_save(t_gobj *z, t_binbuf *b)
{
  t_sym_dial *x = (t_sym_dial *)z;
  int bflcol[3], *ip1, *ip2;
  t_symbol *srl[3];
  int i;
  
  iemgui_save(&x->x_gui, srl, bflcol);
  ip1 = (int *)(&x->x_gui.x_isa);
  ip2 = (int *)(&x->x_gui.x_fsf);
  binbuf_addv(b, "ssiisiiiisssiiiiiiiii", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    gensym("sdl"), x->x_gui.x_w, x->x_gui.x_h,
    (*ip1)&IEM_INIT_ARGS_ALL, x->x_snd_flt0_sym1,
    srl[0], srl[1], srl[2],
    x->x_gui.x_ldx, x->x_gui.x_ldy,
    (*ip2)&IEM_FSTYLE_FLAGS_ALL, x->x_gui.x_fontsize,
    bflcol[0], bflcol[1], bflcol[2],
    x->x_index, x->x_ac);
  for(i=0; i<x->x_ac; i++)  /*16 + ac syms*/
  {
    binbuf_addv(b, "s", x->x_syms[i]);
  }
  binbuf_addv(b, ";");
}
#endif

static void sym_dial_properties(t_gobj *z, t_glist *owner)
{
  t_sym_dial *x = (t_sym_dial *)z;
  char buf[800];
  t_symbol *srl[3];
  
  iemgui_properties(&x->x_gui, srl);
  sprintf(buf, "pdtk_iemgui_dialog %%s SYM_DIAL \
    -------dimensions(digits)(pix):------- %d %d width: %d %d height: \
    empty 0 empty 0 empty 0 \
    %d snd_flt snd_sym %d %d empty -1 \
    %s %s \
    %s %d %d \
    %d %d \
    %d %d %d\n",
    x->x_gui.x_w, 1, x->x_gui.x_h, 8,
    /*no_schedule*/
    x->x_snd_flt0_sym1, x->x_gui.x_isa.x_loadinit, -1,/*no multi, but iem-characteristic*/
    srl[0]->s_name, srl[1]->s_name,
    srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
    x->x_gui.x_fsf.x_font_style, x->x_gui.x_fontsize,
    0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol, 0xffffff & x->x_gui.x_lcol);
  gfxstub_new(&x->x_gui.x_obj.ob_pd, x, buf);
}

static void sym_dial_out(t_sym_dial *x)
{
  outlet_symbol(x->x_outlet, x->x_syms[x->x_index]);
  outlet_float(x->x_gui.x_obj.ob_outlet, x->x_index);
  if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
  {
    if(x->x_snd_flt0_sym1)
      pd_symbol(x->x_gui.x_snd->s_thing, x->x_syms[x->x_index]);
    else
      pd_float(x->x_gui.x_snd->s_thing, x->x_index);
  }
}

static void sym_dial_bang(t_sym_dial *x)
{
  sym_dial_inc(x);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  sym_dial_out(x);
}

static void sym_dial_dialog(t_sym_dial *x, t_symbol *s, int argc, t_atom *argv)
{
  t_symbol *srl[3];
  int w = (int)atom_getintarg(0, argc, argv);
  int h = (int)atom_getintarg(1, argc, argv);
  int snd_fs = (int)atom_getintarg(4, argc, argv);
  int sr_flags;
  
  if(snd_fs != 0) snd_fs = 1;
  x->x_snd_flt0_sym1 = snd_fs;
  sr_flags = iemgui_dialog(&x->x_gui, srl, argc, argv);
  if(w < 1)
    w = 1;
  x->x_gui.x_w = w;
  if(h < 8)
    h = 8;
  x->x_gui.x_h = h;
  sym_dial_calc_fontwidth(x);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_IO + sr_flags);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_CONFIG);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_MOVE);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
}

static void sym_dial_click(t_sym_dial *x, t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
  sym_dial_inc(x);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  sym_dial_draw_swap(x, x->x_gui.x_glist, 1);
  clock_delay(x->x_clock, IEM_SDL_FLASH_TIME);
  sym_dial_out(x);
}

static int sym_dial_newclick(t_gobj *z, struct _glist *glist,
                             int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
  if(doit)
  {
    sym_dial_click((t_sym_dial *)z, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift, 0, (t_floatarg)alt);
  }
  return (1);
}

static void sym_dial_set(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac > 0)
  {
    int i=0;
    
    if(IS_A_FLOAT(av, 0))
    {
      i=(int)atom_getintarg(0, ac, av);
      if(i < 0)
        i = 0;
      else if(i >= x->x_ac)
        i = x->x_ac - 1;
      x->x_index = i;
    }
    else if(IS_A_SYMBOL(av, 0))
    {
      t_symbol *sy=atom_getsymbolarg(0, ac, av);
      for(i=0; i<x->x_ac; i++)
      {
        if(x->x_syms[i] == sy)
          break;
      }
      x->x_index = i;
    }
  }
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

static void sym_dial_float(t_sym_dial *x, t_floatarg f)
{
  int i=(int)f;
  
  if(i < 0)
    i = 0;
  else if(i >= x->x_ac)
    i = x->x_ac - 1;
  x->x_index = i;
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  if(x->x_gui.x_fsf.x_put_in2out)
    sym_dial_out(x);
}

static void sym_dial_symbol(t_sym_dial *x, t_symbol *s)
{
  int i;
  
  for(i=0; i<x->x_ac; i++)
  {
    if(x->x_syms[i] == s)
      break;
  }
  x->x_index = i;
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  if(x->x_gui.x_fsf.x_put_in2out)
    sym_dial_out(x);
}

static void sym_dial_size(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{
  int h, w;
  
  w = (int)atom_getintarg(0, ac, av);
  if(w < 1)
    w = 1;
  x->x_gui.x_w = w;
  if(ac > 1)
  {
    h = (int)atom_getintarg(1, ac, av);
    if(h < 8)
      h = 8;
    x->x_gui.x_h = h;
  }
  sym_dial_calc_fontwidth(x);
  iemgui_size((void *)x, &x->x_gui);
}

static void sym_dial_delta(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void sym_dial_pos(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}

static void sym_dial_color(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_color((void *)x, &x->x_gui, s, ac, av);}

static void sym_dial_send(t_sym_dial *x, t_symbol *s)
{iemgui_send(x, &x->x_gui, s);}

static void sym_dial_receive(t_sym_dial *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}

static void sym_dial_label(t_sym_dial *x, t_symbol *s)
{iemgui_label((void *)x, &x->x_gui, s);}

static void sym_dial_label_pos(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void sym_dial_label_font(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{
  int f = (int)atom_getintarg(1, ac, av);
  
  if(f < 4)
    f = 4;
  x->x_gui.x_fontsize = f;
  f = (int)atom_getintarg(0, ac, av);
  if((f < 0) || (f > 2))
    f = 0;
  x->x_gui.x_fsf.x_font_style = f;
  sym_dial_calc_fontwidth(x);
  iemgui_label_font((void *)x, &x->x_gui, s, ac, av);
}

static void sym_dial_send_sym(t_sym_dial *x)
{
  x->x_snd_flt0_sym1 = 1;
}

static void sym_dial_send_flt(t_sym_dial *x)
{
  x->x_snd_flt0_sym1 = 0;
}

static void sym_dial_init(t_sym_dial *x, t_floatarg f)
{
  x->x_gui.x_isa.x_loadinit = (f==0.0)?0:1;
}

static void sym_dial_loadbang(t_sym_dial *x)
{
  if(!sys_noloadbang && x->x_gui.x_isa.x_loadinit)
  {
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    sym_dial_bang(x);
  }
}

static void sym_dial_set_item_name(t_sym_dial *x, t_symbol *name, t_float findex)
{
  int i = (int)findex;
  
  if(i < 0)
    i = 0;
  else if(i >= x->x_max_ac)
  {
    x->x_syms = (t_symbol **)t_resizebytes(x->x_syms, x->x_max_ac * sizeof(t_symbol *),
      x->x_max_ac * (2*sizeof(t_symbol *)));
    x->x_max_ac *= 2;
  }
  if(i >= x->x_ac)
  {
    t_symbol *default_sym=gensym("no_entry");
    int j;
    
    for(j=x->x_ac; j<i; j++)
      x->x_syms[j] = default_sym;
    x->x_ac++;
  }
  x->x_syms[i] = gensym(name->s_name);
  if(i == x->x_index)
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

static void sym_dial_add(t_sym_dial *x, t_float findex, t_symbol *name)
{
  int i = (int)findex;
  
  if(i < 0)
    i = 0;
  else if(i >= x->x_max_ac)
  {
    x->x_syms = (t_symbol **)t_resizebytes(x->x_syms, x->x_max_ac * sizeof(t_symbol *),
      x->x_max_ac * (2*sizeof(t_symbol *)));
    x->x_max_ac *= 2;
  }
  if(i >= x->x_ac)
  {
    t_symbol *default_sym=gensym("no_entry");
    int j;
    
    for(j=x->x_ac; j<i; j++)
      x->x_syms[j] = default_sym;
    x->x_ac++;
  }
  x->x_syms[i] = gensym(name->s_name);
  if(i == x->x_index)
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

/*static void sym_dial_list(t_sym_dial *x, t_symbol *s, int ac, t_atom *av)
{
int l=iemgui_list((void *)x, &x->x_gui, s, ac, av);

  if(l < 0)
  {
  if((ac==2)&&(IS_A_FLOAT(av,0))&&(IS_A_SYMBOL(av,1)))
  {
  sym_dial_float(x, atom_getfloatarg(0, ac, av));
  }
  }
  if(l > 0)
  {
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_MOVE);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
  }
}*/

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void *sym_dial_new(t_symbol *s, int argc, t_atom *argv)
{
  t_sym_dial *x = (t_sym_dial *)pd_new(sym_dial_class);
  int bflcol[]={-262144, -1, -1};
  int w=6, h=14, ac=0, i, j;
  int snd_fs=0, f=0, ldx=59, ldy=7;
  int fs=9, iindex=0;
  char str[144];
  
  iem_inttosymargs(&x->x_gui.x_isa, 0);
  iem_inttofstyle(&x->x_gui.x_fsf, 0);
  
  if((argc >= 16)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
    &&(IS_A_SYMBOL(argv,4)||IS_A_FLOAT(argv,4))
    &&(IS_A_SYMBOL(argv,5)||IS_A_FLOAT(argv,5))
    &&(IS_A_SYMBOL(argv,6)||IS_A_FLOAT(argv,6))
    &&IS_A_FLOAT(argv,7)&&IS_A_FLOAT(argv,8)
    &&IS_A_FLOAT(argv,9)&&IS_A_FLOAT(argv,10)
    &&IS_A_FLOAT(argv,11)&&IS_A_FLOAT(argv,12)&&IS_A_FLOAT(argv,13)
    &&IS_A_FLOAT(argv,14)&&IS_A_FLOAT(argv,15))
  {
    w = (int)atom_getintarg(0, argc, argv);
    h = (int)atom_getintarg(1, argc, argv);
    iem_inttosymargs(&x->x_gui.x_isa, atom_getintarg(2, argc, argv));
    snd_fs = (int)atom_getintarg(3, argc, argv);
    iemgui_new_getnames(&x->x_gui, 4, argv);
    ldx = (int)atom_getintarg(7, argc, argv);
    ldy = (int)atom_getintarg(8, argc, argv);
    iem_inttofstyle(&x->x_gui.x_fsf, atom_getintarg(9, argc, argv));
    fs = (int)atom_getintarg(10, argc, argv);
    bflcol[0] = (int)atom_getintarg(11, argc, argv);
    bflcol[1] = (int)atom_getintarg(12, argc, argv);
    bflcol[2] = (int)atom_getintarg(13, argc, argv);
    iindex = atom_getintarg(14, argc, argv);
    ac = (int)atom_getintarg(15, argc, argv);
    if((ac+16) == argc)
    {
      x->x_ac = ac;
      x->x_max_ac = ac;
      x->x_syms = (t_symbol **)getbytes(x->x_max_ac * sizeof(t_symbol *));
      for(i=0, j=16; i<ac; i++, j++)
      {
        if(IS_A_SYMBOL(argv, j))
          x->x_syms[i] = atom_getsymbolarg(j, argc, argv);
        else if(IS_A_FLOAT(argv, j))
        {
          sprintf(str, "%d", (int)atom_getintarg(j, argc, argv));
          x->x_syms[i] = gensym(str);
        }
      }
    }
  }
  else
  {
    iemgui_new_getnames(&x->x_gui, 1, 0);
    x->x_ac = 1;
    x->x_max_ac = 10;
    x->x_syms = (t_symbol **)getbytes(x->x_max_ac * sizeof(t_symbol *));
    x->x_syms[0] = gensym("sdl");
    iindex = 0;
  }
  
  x->x_gui.x_draw = (t_iemfunptr)sym_dial_draw;
  x->x_gui.x_fsf.x_snd_able = 1;
  x->x_gui.x_fsf.x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  if(x->x_gui.x_isa.x_loadinit)
    x->x_index = iindex;
  else
    x->x_index = 0;
  if(snd_fs != 0)
    snd_fs = 1;
  x->x_snd_flt0_sym1 = snd_fs;
  
  if(!strcmp(x->x_gui.x_snd->s_name, "empty"))
    x->x_gui.x_fsf.x_snd_able = 0;
  if(!strcmp(x->x_gui.x_rcv->s_name, "empty"))
    x->x_gui.x_fsf.x_rcv_able = 0;
  if(x->x_gui.x_fsf.x_font_style == 1)
    strcpy(x->x_gui.x_font, "helvetica");
  else if(x->x_gui.x_fsf.x_font_style == 2)
    strcpy(x->x_gui.x_font, "times");
  else
  {
    x->x_gui.x_fsf.x_font_style = 0;
    strcpy(x->x_gui.x_font, "courier");
  }
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  x->x_gui.x_ldx = ldx;
  x->x_gui.x_ldy = ldy;
  if(fs < 4)
    fs = 4;
  x->x_gui.x_fontsize = fs;
  if(w < 1)
    w = 1;
  x->x_gui.x_w = w;
  if(h < 8)
    h = 8;
  x->x_gui.x_h = h;
  sym_dial_calc_fontwidth(x);
  iemgui_all_colfromload(&x->x_gui, bflcol);
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  x->x_clock = clock_new(x, (t_method)sym_dial_tick);
  outlet_new(&x->x_gui.x_obj, &s_float);
  x->x_outlet = outlet_new(&x->x_gui.x_obj, &s_symbol);
  return (x);
}
#else
static void *sym_dial_new(t_symbol *s, int argc, t_atom *argv)
{
  t_sym_dial *x = (t_sym_dial *)pd_new(sym_dial_class);
  int bflcol[]={-262144, -1, -1};
  t_symbol *srl[3];
  int w=6, h=14, ac=0, i, j;
  int snd_fs=0, f=0, ldx=59, ldy=7;
  int fs=9, iinit=0, ifstyle=0, iindex=0;
  t_iem_init_symargs *init=(t_iem_init_symargs *)(&iinit);
  t_iem_fstyle_flags *fstyle=(t_iem_fstyle_flags *)(&ifstyle);
  char str[144];
  
  srl[0] = gensym("empty");
  srl[1] = gensym("empty");
  srl[2] = gensym("empty");
  
  if((argc >= 16)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
    &&(IS_A_SYMBOL(argv,4)||IS_A_FLOAT(argv,4))
    &&(IS_A_SYMBOL(argv,5)||IS_A_FLOAT(argv,5))
    &&(IS_A_SYMBOL(argv,6)||IS_A_FLOAT(argv,6))
    &&IS_A_FLOAT(argv,7)&&IS_A_FLOAT(argv,8)
    &&IS_A_FLOAT(argv,9)&&IS_A_FLOAT(argv,10)
    &&IS_A_FLOAT(argv,11)&&IS_A_FLOAT(argv,12)&&IS_A_FLOAT(argv,13)
    &&IS_A_FLOAT(argv,14)&&IS_A_FLOAT(argv,15))
  {
    w = (int)atom_getintarg(0, argc, argv);
    h = (int)atom_getintarg(1, argc, argv);
    iinit = (int)atom_getintarg(2, argc, argv);
    snd_fs = (int)atom_getintarg(3, argc, argv);
    srl[0] = atom_getsymbolarg(4, argc, argv);
    srl[1] = atom_getsymbolarg(5, argc, argv);
    srl[2] = atom_getsymbolarg(6, argc, argv);
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
    if(IS_A_SYMBOL(argv,6))
      srl[2] = atom_getsymbolarg(6, argc, argv);
    else if(IS_A_FLOAT(argv,6))
    {
      sprintf(str, "%d", (int)atom_getintarg(6, argc, argv));
      srl[2] = gensym(str);
    }
    ldx = (int)atom_getintarg(7, argc, argv);
    ldy = (int)atom_getintarg(8, argc, argv);
    ifstyle = (int)atom_getintarg(9, argc, argv);
    fs = (int)atom_getintarg(10, argc, argv);
    bflcol[0] = (int)atom_getintarg(11, argc, argv);
    bflcol[1] = (int)atom_getintarg(12, argc, argv);
    bflcol[2] = (int)atom_getintarg(13, argc, argv);
    iindex = atom_getintarg(14, argc, argv);
    ac = (int)atom_getintarg(15, argc, argv);
    if((ac+16) == argc)
    {
      x->x_ac = ac;
      x->x_max_ac = ac;
      x->x_syms = (t_symbol **)getbytes(x->x_max_ac * sizeof(t_symbol *));
      for(i=0, j=16; i<ac; i++, j++)
      {
        if(IS_A_SYMBOL(argv, j))
          x->x_syms[i] = atom_getsymbolarg(j, argc, argv);
        else if(IS_A_FLOAT(argv, j))
        {
          sprintf(str, "%d", (int)atom_getintarg(j, argc, argv));
          x->x_syms[i] = gensym(str);
        }
      }
    }
  }
  else
  {
    x->x_ac = 1;
    x->x_max_ac = 10;
    x->x_syms = (t_symbol **)getbytes(x->x_max_ac * sizeof(t_symbol *));
    x->x_syms[0] = gensym("sdl");
    iindex = 0;
  }
  
  x->x_gui.x_draw = (t_iemfunptr)sym_dial_draw;
  iinit &= IEM_INIT_ARGS_ALL;
  ifstyle &= IEM_FSTYLE_FLAGS_ALL;
  fstyle->x_snd_able = 1;
  fstyle->x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  x->x_gui.x_isa = *init;
  if(x->x_gui.x_isa.x_loadinit)
    x->x_index = iindex;
  else
    x->x_index = 0;
  if(snd_fs != 0) snd_fs = 1;
  x->x_snd_flt0_sym1 = snd_fs;
  if(!strcmp(srl[0]->s_name, "empty")) fstyle->x_snd_able = 0;
  if(!strcmp(srl[1]->s_name, "empty")) fstyle->x_rcv_able = 0;
  x->x_gui.x_unique_num = 0;
  if(fstyle->x_font_style == 1) strcpy(x->x_gui.x_font, "helvetica");
  else if(fstyle->x_font_style == 2) strcpy(x->x_gui.x_font, "times");
  else { fstyle->x_font_style = 0;
  strcpy(x->x_gui.x_font, "courier"); }
  x->x_gui.x_fsf = *fstyle;
  iemgui_first_dollararg2sym(&x->x_gui, srl);
  if(x->x_gui.x_fsf.x_rcv_able) pd_bind(&x->x_gui.x_obj.ob_pd, srl[1]);
  x->x_gui.x_snd = srl[0];
  x->x_gui.x_rcv = srl[1];
  x->x_gui.x_lab = srl[2];
  x->x_gui.x_ldx = ldx;
  x->x_gui.x_ldy = ldy;
  if(fs < 4)
    fs = 4;
  x->x_gui.x_fontsize = fs;
  if(w < 1)
    w = 1;
  x->x_gui.x_w = w;
  if(h < 8)
    h = 8;
  x->x_gui.x_h = h;
  sym_dial_calc_fontwidth(x);
  iemgui_all_colfromload(&x->x_gui, bflcol);
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  x->x_clock = clock_new(x, (t_method)sym_dial_tick);
  outlet_new(&x->x_gui.x_obj, &s_float);
  x->x_outlet = outlet_new(&x->x_gui.x_obj, &s_symbol);
  return (x);
}
#endif

static void sym_dial_free(t_sym_dial *x)
{
  //    if(x->x_gui.x_fsf.x_selected)
  //  pd_unbind(&x->x_gui.x_obj.ob_pd, iemgui_key_sym2);
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  freebytes(x->x_syms, x->x_max_ac * sizeof(t_symbol *));
  clock_free(x->x_clock);
}

void sym_dial_setup(void)
{
  sym_dial_class = class_new(gensym("sdl"), (t_newmethod)sym_dial_new,
    (t_method)sym_dial_free, sizeof(t_sym_dial), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)sym_dial_new, gensym("sym_dial"), A_GIMME, 0);
  class_addbang(sym_dial_class,sym_dial_bang);
  class_addfloat(sym_dial_class,sym_dial_float);
  //    class_addlist(sym_dial_class, sym_dial_list);
  class_addsymbol(sym_dial_class, sym_dial_symbol);
  class_addmethod(sym_dial_class, (t_method)sym_dial_click, gensym("click"),
    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_dialog, gensym("dialog"),
    A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_loadbang, gensym("loadbang"), 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_set, gensym("set"), A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_size, gensym("size"), A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_delta, gensym("delta"), A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_pos, gensym("pos"), A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_color, gensym("color"), A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_send, gensym("send"), A_DEFSYM, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_receive, gensym("receive"), A_DEFSYM, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_label, gensym("label"), A_DEFSYM, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_label_pos, gensym("label_pos"), A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_label_font, gensym("label_font"), A_GIMME, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_send_flt, gensym("send_flt"), 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_send_sym, gensym("send_sym"), 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_init, gensym("init"), A_FLOAT, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_set_item_name, gensym("set_item_name"), A_SYMBOL, A_FLOAT, 0);
  class_addmethod(sym_dial_class, (t_method)sym_dial_add, gensym("add"), A_FLOAT, A_SYMBOL, 0);
  //    if(!iemgui_key_sym2)
  //    iemgui_key_sym2 = gensym("#keyname");
  sym_dial_widgetbehavior.w_getrectfn =    sym_dial_getrect;
  sym_dial_widgetbehavior.w_displacefn =   iemgui_displace;
  sym_dial_widgetbehavior.w_selectfn =     iemgui_select;
  sym_dial_widgetbehavior.w_activatefn =   NULL;
  sym_dial_widgetbehavior.w_deletefn =     iemgui_delete;
  sym_dial_widgetbehavior.w_visfn =        iemgui_vis;
  sym_dial_widgetbehavior.w_clickfn =      sym_dial_newclick;
  
#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
  class_setsavefn(sym_dial_class, sym_dial_save);
  class_setpropertiesfn(sym_dial_class, sym_dial_properties);
#else
  sym_dial_widgetbehavior.w_savefn =       sym_dial_save;
  sym_dial_widgetbehavior.w_propertiesfn = sym_dial_properties;
#endif
  
  class_setwidget(sym_dial_class, &sym_dial_widgetbehavior);
//  class_sethelpsymbol(sym_dial_class, gensym("iemhelp2/help-sym_dial"));
}
