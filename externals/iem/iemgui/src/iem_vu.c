/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemgui written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */

#include "m_pd.h"
#include "iemlib.h"
#include "iemgui.h"
#include "g_canvas.h"
#include "g_all_guis.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef MSW
#include <io.h>
#else
#include <unistd.h>
#endif

/* ------------------------ setup routine ------------------------- */

t_widgetbehavior iem_vu_widgetbehavior;
static t_class *iem_vu_class;

static int iem_vu_db2i[]=
{
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9,10,10,10,10,10,
    11,11,11,11,11,12,12,12,12,12,
    13,13,13,13,14,14,14,14,15,15,
    15,15,16,16,16,16,17,17,17,18,
    18,18,19,19,19,20,20,20,21,21,
    22,22,23,23,24,24,25,26,27,28,
    29,30,31,32,33,33,34,34,35,35,
    36,36,37,37,37,38,38,38,39,39,
    39,39,39,39,40,40
};

static int iem_vu_col[]=
{
  0,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    15,15,15,15,15,15,15,15,15,15,14,14,13,13,13,13,13,13,13,13,13,13,13,19,19,19
};

typedef struct _iem_vu
{
  t_iemgui  x_gui;
  int       x_peak;
  int       x_rms;
  t_float     x_fp;
  t_float     x_fr;
  short     x_scale;
  short     x_old_width;
  int       x_scale_w;
  int       x_scale_h;
  void      *x_out_rms;
  void      *x_out_peak;
  char      x_scale_gif[750];
  char      x_bkgd_gif_bord[990];
  char      x_bkgd_gif_cent[990];
} t_iem_vu;

static int iem_vu_clip_width(int w)
{
  if(w < 8)
    w = 8;
  w &= 0xffffffc;
  return(w);
}

static void iem_vu_update_rms(t_iem_vu *x, t_glist *glist)
{
  if(glist_isvisible(glist))
  {
    int ypos1=text_ypix(&x->x_gui.x_obj, glist)-1;
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    
    sys_vgui(".x%lx.c coords %lxRCOVER %d %d %d %d\n",
      glist_getcanvas(glist), x, xpos, ypos1, xpos+x->x_gui.x_w-1,
      ypos1 + 3*(IEM_VU_STEPS-x->x_rms)+1);
  }
}

static void iem_vu_update_peak(t_iem_vu *x, t_glist *glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(glist_isvisible(glist))
  {
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    
    if(x->x_peak)
    {
      int i=iem_vu_col[x->x_peak];
      int j=ypos + 3*(IEM_VU_STEPS+1-x->x_peak) - 1;
      
      sys_vgui(".x%lx.c coords %lxPLED %d %d %d %d\n", canvas, x,
        xpos, j, xpos+x->x_gui.x_w, j);
      sys_vgui(".x%lx.c itemconfigure %lxPLED -fill #%6.6x\n", canvas, x, my_iemgui_color_hex[i]);
    }
    else
    {
      int mid=xpos+x->x_gui.x_w/2;
      
      sys_vgui(".x%lx.c itemconfigure %lxPLED -fill #%6.6x\n", canvas, x, x->x_gui.x_bcol);
      sys_vgui(".x%lx.c coords %lxPLED %d %d %d %d\n",
        canvas, x, mid, ypos+20, mid, ypos+20);
    }
  }
}

static void iem_vu_cpy(char *src, char *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static void iem_vu_change_bkgd_col(t_iem_vu *x, t_glist *glist, int drw_new)
{
  int i, j;
  char pix_bkgd_col[10];
  char pix_led_col[10];
  char *cvec;
  
  sprintf(pix_bkgd_col, "#%6.6x ", x->x_gui.x_bcol);
  cvec = x->x_bkgd_gif_bord;
  for(i=0; i<122; i++)
  {
    iem_vu_cpy(pix_bkgd_col, cvec, 8);
    cvec += 8;
  }
  iem_vu_cpy(pix_bkgd_col, cvec, 8);
  cvec[7] = 0;
  
  cvec = x->x_bkgd_gif_cent;
  iem_vu_cpy(pix_bkgd_col, cvec, 8);
  cvec += 8;
  iem_vu_cpy(pix_bkgd_col, cvec, 8);
  cvec += 8;
  for(i=IEM_VU_STEPS; i>=1; i--)
  {
    j = iem_vu_col[i];
    sprintf(pix_led_col, "#%6.6x ", my_iemgui_color_hex[j]);
    iem_vu_cpy(pix_led_col, cvec, 8);
    cvec += 8;
    iem_vu_cpy(pix_led_col, cvec, 8);
    cvec += 8;
    iem_vu_cpy(pix_bkgd_col, cvec, 8);
    cvec += 8;
  }
  iem_vu_cpy(pix_bkgd_col, cvec, 8);
  cvec[7] = 0;
  
  if(glist_isvisible(glist) || drw_new)
  {
    sys_vgui("%lxBKGDIMAGE_PROTO put {%s} -to 0 0\n", x, x->x_bkgd_gif_bord);
    sys_vgui("%lxBKGDIMAGE_PROTO put {%s} -to 1 0\n", x, x->x_bkgd_gif_cent);
    sys_vgui("%lxBKGDIMAGE_PROTO put {%s} -to 2 0\n", x, x->x_bkgd_gif_cent);
    sys_vgui("%lxBKGDIMAGE_PROTO put {%s} -to 3 0\n", x, x->x_bkgd_gif_bord);
  }
}

static void iem_vu_draw_new(t_iem_vu *x, t_glist *glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  int mid=xpos+x->x_gui.x_w/2;
  int zoom=x->x_gui.x_w/4;
  
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxBASE\n",
    canvas, xpos-1, ypos-2, xpos+x->x_gui.x_w, ypos+x->x_gui.x_h+2, x);
  
  sys_vgui("image create photo %lxBKGDIMAGE_PROTO -format gif -width %d -height %d\n",
    x, 5, 123);
  sys_vgui("%lxBKGDIMAGE_PROTO blank\n", x);
  iem_vu_change_bkgd_col(x, glist, 1);
  
  sys_vgui("image create photo %lxBKGDIMAGE -format gif -width %d -height %d\n",
    x, x->x_gui.x_w, x->x_gui.x_h+3);
  sys_vgui("%lxBKGDIMAGE blank\n", x);
  sys_vgui("%lxBKGDIMAGE copy %lxBKGDIMAGE_PROTO -zoom %d 1\n", x, x, zoom);
  sys_vgui(".x%lx.c create image %d %d -image %lxBKGDIMAGE -tags %lxBKGDPHOTO\n",
    canvas, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2, x, x);
  
  sys_vgui("image create photo %lxSCALEIMAGE -format gif -width %d -height %d\n",
    x, x->x_scale_w, x->x_scale_h);
  sys_vgui("%lxSCALEIMAGE blank\n", x);
  if(x->x_scale)
  {
    sys_vgui(".x%lx.c create image %d %d -image %lxSCALEIMAGE -tags %lxSCALEPHOTO\n",
      canvas, xpos+x->x_gui.x_w+x->x_scale_w/2+2, ypos+x->x_gui.x_h/2+2, x, x);
    my_iemgui_change_scale_col(x->x_scale_gif, x->x_gui.x_lcol);
    sys_vgui("%lxSCALEIMAGE configure -data {%s}\n", x, x->x_scale_gif);
  }
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxRCOVER\n",
    canvas, xpos, ypos-1, xpos+x->x_gui.x_w-1,
    ypos + 3*(IEM_VU_STEPS-x->x_rms), x->x_gui.x_bcol, x->x_gui.x_bcol, x);
  
  if(x->x_peak)
  {
    int i=iem_vu_col[x->x_peak];
    int j=ypos + 3*(IEM_VU_STEPS+1-x->x_peak) - 1;
    
    sys_vgui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxPLED\n",
    canvas, xpos, j, xpos+x->x_gui.x_w, j, 2, my_iemgui_color_hex[i], x);
  }
  else
    sys_vgui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxPLED\n",
      canvas, mid, ypos+10, mid, ypos+10, 2, x->x_gui.x_bcol, x);

  sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w \
    -font {%s %d bold} -fill #%6.6x -tags %lxLABEL\n",
    canvas, xpos+x->x_gui.x_ldx, ypos+x->x_gui.x_ldy,
    strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"",
    x->x_gui.x_font, x->x_gui.x_fontsize, x->x_gui.x_lcol, x);
  if(!x->x_gui.x_fsf.x_snd_able)
  {
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
      canvas, xpos-1, ypos + x->x_gui.x_h+1,
      xpos + IOWIDTH-1, ypos + x->x_gui.x_h+2, x, 0);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
      canvas, xpos+x->x_gui.x_w-IOWIDTH, ypos + x->x_gui.x_h+1,
      xpos+x->x_gui.x_w, ypos + x->x_gui.x_h+2, x, 1);
  }
  if(!x->x_gui.x_fsf.x_rcv_able)
  {
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
      canvas, xpos-1, ypos-2, xpos + IOWIDTH-1, ypos-1, x, 0);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
      canvas, xpos+x->x_gui.x_w-IOWIDTH, ypos-2,
      xpos+x->x_gui.x_w, ypos-1, x, 1);
  }
}

static void iem_vu_draw_move(t_iem_vu *x, t_glist *glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  
  sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d\n",
    canvas, x, xpos-1, ypos-2, xpos+x->x_gui.x_w,ypos+x->x_gui.x_h+2);
  sys_vgui(".x%lx.c coords %lxBKGDPHOTO %d %d\n",
    canvas, x, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2);
  if(x->x_scale)
  {
    sys_vgui(".x%lx.c coords %lxSCALEPHOTO %d %d\n",
      canvas, x, xpos+x->x_gui.x_w+x->x_scale_w/2+2, ypos+x->x_gui.x_h/2+2);
  }
  iem_vu_update_peak(x, glist);
  iem_vu_update_rms(x, glist);
  sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
    canvas, x, xpos+x->x_gui.x_ldx, ypos+x->x_gui.x_ldy);
  if(!x->x_gui.x_fsf.x_snd_able)
  {
    sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
      canvas, x, 0, xpos-1, ypos + x->x_gui.x_h+1,
      xpos + IOWIDTH-1, ypos + x->x_gui.x_h+2);
    sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
      canvas, x, 1,xpos+x->x_gui.x_w-IOWIDTH, ypos + x->x_gui.x_h+1,
      xpos+x->x_gui.x_w, ypos + x->x_gui.x_h+2);
  }
  if(!x->x_gui.x_fsf.x_rcv_able)
  {
    sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
      canvas, x, 0, xpos-1, ypos-2,
      xpos + IOWIDTH-1, ypos-1);
    sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
      canvas, x, 1, xpos+x->x_gui.x_w-IOWIDTH, ypos-2,
      xpos+x->x_gui.x_w, ypos-1);
  }
}

static void iem_vu_draw_erase(t_iem_vu* x,t_glist* glist)
{
  int i;
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
  sys_vgui(".x%lx.c delete %lxBKGDPHOTO\n", canvas, x);
  sys_vgui("image delete %lxBKGDIMAGE\n", x);
  sys_vgui("image delete %lxBKGDIMAGE_PROTO\n", x);
  if(x->x_scale)
    sys_vgui(".x%lx.c delete %lxSCALEPHOTO\n", canvas, x);
  sys_vgui("image delete %lxSCALEIMAGE\n", x);
  
  sys_vgui(".x%lx.c delete %lxPLED\n", canvas, x);
  sys_vgui(".x%lx.c delete %lxRCOVER\n", canvas, x);
  sys_vgui(".x%lx.c delete %lxLABEL\n", canvas, x);
  if(!x->x_gui.x_fsf.x_snd_able)
  {
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 1);
  }
  if(!x->x_gui.x_fsf.x_rcv_able)
  {
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 1);
  }
}

static void iem_vu_draw_config(t_iem_vu* x, t_glist* glist)
{
  int i, zoom = x->x_gui.x_w / 4;
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(glist_isvisible(glist))
  {
    iem_vu_change_bkgd_col(x, glist, 0);
    if(x->x_gui.x_w != x->x_old_width)
    {
      x->x_old_width = x->x_gui.x_w;
      sys_vgui("%lxBKGDIMAGE blank\n", x);
      sys_vgui("%lxBKGDIMAGE configure -width %d -height %d\n",
        x, x->x_gui.x_w, x->x_gui.x_h+3);
    }
    sys_vgui("%lxBKGDIMAGE copy %lxBKGDIMAGE_PROTO -zoom %d 1\n", x, x, zoom);
    
    my_iemgui_change_scale_col(x->x_scale_gif, x->x_gui.x_lcol);
    sys_vgui("%lxSCALEIMAGE configure -data {%s}\n", x, x->x_scale_gif);
    
    sys_vgui(".x%lx.c itemconfigure %lxLABEL -font {%s %d bold} -fill #%6.6x -text {%s} \n",
      canvas, x, x->x_gui.x_font, x->x_gui.x_fontsize,
      x->x_gui.x_fsf.x_selected?IEM_GUI_COLOR_SELECTED:x->x_gui.x_lcol,
      strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"");
    
    sys_vgui(".x%lx.c itemconfigure %lxRCOVER -fill #%6.6x -outline #%6.6x\n",
      canvas, x, x->x_gui.x_bcol, x->x_gui.x_bcol);
    sys_vgui(".x%lx.c itemconfigure %lxPLED -width %d\n", canvas, x, 2);
  }
}

static void iem_vu_draw_io(t_iem_vu* x, t_glist* glist, int old_snd_rcv_flags)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  if((old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && !x->x_gui.x_fsf.x_rcv_able)
  {
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
      canvas, xpos-1, ypos-2, xpos + IOWIDTH-1, ypos-1, x, 0);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
      canvas, xpos+x->x_gui.x_w-IOWIDTH, ypos-2,
      xpos+x->x_gui.x_w, ypos-1, x, 1);
  }
  if(!(old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && x->x_gui.x_fsf.x_rcv_able)
  {
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 1);
  }
}

static void iem_vu_draw_select(t_iem_vu* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
  {
    sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_GUI_COLOR_SELECTED);
    if(x->x_scale)
    {
      my_iemgui_change_scale_col(x->x_scale_gif, IEM_GUI_COLOR_SELECTED);
      sys_vgui("%lxSCALEIMAGE configure -data {%s}\n", x, x->x_scale_gif);
    }
    sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_GUI_COLOR_SELECTED);
  }
  else
  {
    sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_GUI_COLOR_NORMAL);
    if(x->x_scale)
    {
      my_iemgui_change_scale_col(x->x_scale_gif, x->x_gui.x_lcol);
      sys_vgui("%lxSCALEIMAGE configure -data {%s}\n", x, x->x_scale_gif);
    }
    sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, x->x_gui.x_lcol);
  }
}

void iem_vu_draw(t_iem_vu *x, t_glist *glist, int mode)
{
  if(mode == IEM_GUI_DRAW_MODE_MOVE)
    iem_vu_draw_move(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_NEW)
    iem_vu_draw_new(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_SELECT)
    iem_vu_draw_select(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_ERASE)
    iem_vu_draw_erase(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_CONFIG)
    iem_vu_draw_config(x, glist);
  else if(mode >= IEM_GUI_DRAW_MODE_IO)
    iem_vu_draw_io(x, glist, mode - IEM_GUI_DRAW_MODE_IO);
}

/* ------------------------ vu widgetbehaviour----------------------------- */


static void iem_vu_getrect(t_gobj *z, t_glist *glist,
                           int *xp1, int *yp1, int *xp2, int *yp2)
{
  t_iem_vu* x = (t_iem_vu*)z;
  
  *xp1 = text_xpix(&x->x_gui.x_obj, glist) - 1;
  *yp1 = text_ypix(&x->x_gui.x_obj, glist) - 2;
  *xp2 = *xp1 + x->x_gui.x_w + 2;
  *yp2 = *yp1 + x->x_gui.x_h + 4;
}

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void iem_vu_save(t_gobj *z, t_binbuf *b)
{
  t_iem_vu *x = (t_iem_vu *)z;
  int bflcol[3];
  t_symbol *srl[3];
  
  iemgui_save(&x->x_gui, srl, bflcol);
  binbuf_addv(b, "ssiisiissiiiiiiii", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    /*x->x_gui.x_w+1*/ x->x_gui.x_w, 120,
    srl[1], srl[2],
    x->x_gui.x_ldx, x->x_gui.x_ldy,
    iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
    bflcol[0], bflcol[2], x->x_scale, iem_symargstoint(&x->x_gui.x_isa));
  binbuf_addv(b, ";");
}
#else
static void iem_vu_save(t_gobj *z, t_binbuf *b)
{
  t_iem_vu *x = (t_iem_vu *)z;
  int bflcol[3], *ip1, *ip2;
  t_symbol *srl[3];
  
  iemgui_save(&x->x_gui, srl, bflcol);
  ip1 = (int *)(&x->x_gui.x_isa);
  ip2 = (int *)(&x->x_gui.x_fsf);
  binbuf_addv(b, "ssiisiissiiiiiiii", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    /*x->x_gui.x_w+1*/ x->x_gui.x_w, 120,
    srl[1], srl[2],
    x->x_gui.x_ldx, x->x_gui.x_ldy,
    (*ip2)&IEM_FSTYLE_FLAGS_ALL, x->x_gui.x_fontsize,
    bflcol[0], bflcol[2], x->x_scale, (*ip1)&IEM_INIT_ARGS_ALL);
  binbuf_addv(b, ";");
}
#endif

static void iem_vu_scale(t_iem_vu *x, t_floatarg fscale)
{
  int i, scale = (int)fscale;
  
  if(scale != 0)
    scale = 1;
  if(x->x_scale && !scale)
  {
    x->x_scale = scale;
    if(glist_isvisible(x->x_gui.x_glist))
    {
      t_canvas *canvas=glist_getcanvas(x->x_gui.x_glist);
      
      sys_vgui(".x%lx.c delete %lxSCALEPHOTO\n", canvas, x);
    }
  }
  if(!x->x_scale && scale)
  {
    x->x_scale = scale;
    if(glist_isvisible(x->x_gui.x_glist))
    {
      t_canvas *canvas=glist_getcanvas(x->x_gui.x_glist);
      int xpos=text_xpix(&x->x_gui.x_obj, x->x_gui.x_glist);
      int ypos=text_ypix(&x->x_gui.x_obj, x->x_gui.x_glist);
      
      sys_vgui(".x%lx.c create image %d %d -image %lxSCALEIMAGE -tags %lxSCALEPHOTO\n",
        canvas, xpos+x->x_gui.x_w+x->x_scale_w/2+3, ypos+x->x_gui.x_h/2+2, x, x);
      my_iemgui_change_scale_col(x->x_scale_gif, x->x_gui.x_lcol);
      sys_vgui("%lxSCALEIMAGE configure -data {%s}\n", x, x->x_scale_gif);
    }
  }
}

static void iem_vu_properties(t_gobj *z, t_glist *owner)
{
  t_iem_vu *x = (t_iem_vu *)z;
  char buf[800];
  t_symbol *srl[3];
  
  iemgui_properties(&x->x_gui, srl);
  sprintf(buf, "pdtk_iemgui_dialog %%s IEM_VU-METER \
    --------dimensions(pix)(pix):-------- %d %d width: %d %d height: \
    empty 0.0 empty 0.0 empty %d \
    %d no_scale scale %d %d empty %d \
    %s %s \
    %s %d %d \
    %d %d \
    %d %d %d\n",
    x->x_gui.x_w, IEM_GUI_MINSIZE, 120, 80,
    0,/*no_schedule*/
    x->x_scale, -1, -1, -1,/*no linlog, no init, no multi*/
    "nosndno", srl[1]->s_name,/*no send*/
    srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
    x->x_gui.x_fsf.x_font_style, x->x_gui.x_fontsize,
    0xffffff & x->x_gui.x_bcol, -1/*no front-color*/, 0xffffff & x->x_gui.x_lcol);
  gfxstub_new(&x->x_gui.x_obj.ob_pd, x, buf);
}

void iem_vu_dialog(t_iem_vu *x, t_symbol *s, int argc, t_atom *argv)
{
  t_symbol *srl[3];
  int w = (int)atom_getintarg(0, argc, argv);
  int scale = (int)atom_getintarg(4, argc, argv);
  int sr_flags;
  
  srl[0] = gensym("empty");
  sr_flags = iemgui_dialog(&x->x_gui, srl, argc, argv);
  //  post("srl-flag = %lx", sr_flags);
  x->x_gui.x_fsf.x_snd_able = 0;
  x->x_gui.x_isa.x_loadinit = 0;
  x->x_gui.x_w = iem_vu_clip_width(w+1);
  x->x_gui.x_h = 120;
  if(scale != 0)
    scale = 1;
  iem_vu_scale(x, (t_float)scale);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_MOVE);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_CONFIG);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_IO + sr_flags);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
}

static void iem_vu_size(t_iem_vu *x, t_symbol *s, int ac, t_atom *av)
{
  x->x_gui.x_w = iem_vu_clip_width((int)atom_getintarg(0, ac, av)+1);
  x->x_gui.x_h = 120;
  if(glist_isvisible(x->x_gui.x_glist))
  {
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_MOVE);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_CONFIG);
    canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
  }
}

static void iem_vu_delta(t_iem_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void iem_vu_pos(t_iem_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}

static void iem_vu_color(t_iem_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_color((void *)x, &x->x_gui, s, ac, av);}

static void iem_vu_receive(t_iem_vu *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}

static void iem_vu_label(t_iem_vu *x, t_symbol *s)
{iemgui_label((void *)x, &x->x_gui, s);}

static void iem_vu_label_pos(t_iem_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void iem_vu_label_font(t_iem_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_label_font((void *)x, &x->x_gui, s, ac, av);}

static void iem_vu_float(t_iem_vu *x, t_floatarg rms)
{
  int i;
  
  if(rms <= IEM_VU_MINDB)
    x->x_rms = 0;
  else if(rms >= IEM_VU_MAXDB)
    x->x_rms = IEM_VU_STEPS;
  else
  {
    int ii = (int)(2.0*(rms + IEM_VU_OFFSET));
    x->x_rms = iem_vu_db2i[ii];
  }
  i = (int)(100.0*rms + 10000.5);
  rms = 0.01*(t_float)(i - 10000);
  x->x_fr = rms;
  outlet_float(x->x_out_rms, rms);
  iem_vu_update_rms(x, x->x_gui.x_glist);
}

static void iem_vu_ft1(t_iem_vu *x, t_floatarg peak)
{
  int i;
  
  if(peak <= IEM_VU_MINDB)
    x->x_peak = 0;
  else if(peak >= IEM_VU_MAXDB)
    x->x_peak = IEM_VU_STEPS;
  else
  {
    int ii = (int)(2.0*(peak + IEM_VU_OFFSET));
    x->x_peak = iem_vu_db2i[ii];
  }
  i = (int)(100.0*peak + 10000.5);
  peak = 0.01*(t_float)(i - 10000);
  x->x_fp = peak;
  outlet_float(x->x_out_peak, peak);
  iem_vu_update_peak(x, x->x_gui.x_glist);
}

static void iem_vu_set(t_iem_vu *x, t_symbol *s, int ac, t_atom *av)
{
  t_float rms=-100.0f, peak=-100.0f;
  int i;
  
  if( (ac >= 2) && IS_A_FLOAT(av,0) && IS_A_FLOAT(av,1) )
  {
    rms = (t_float)atom_getfloatarg(0, ac, av);
    peak = (t_float)atom_getfloatarg(1, ac, av);
  }
  else if( (ac == 1) && IS_A_FLOAT(av,0) )
  {
    rms = (t_float)atom_getfloatarg(0, ac, av);
    peak = rms;
  }
  if(rms <= IEM_VU_MINDB)
    x->x_rms = 0;
  else if(rms >= IEM_VU_MAXDB)
    x->x_rms = IEM_VU_STEPS;
  else
  {
    int ii = (int)(2.0*(rms + IEM_VU_OFFSET));
    x->x_rms = iem_vu_db2i[ii];
  }
  i = (int)(100.0*rms + 10000.5);
  rms = 0.01*(t_float)(i - 10000);
  x->x_fr = rms;
  
  if(peak <= IEM_VU_MINDB)
    x->x_peak = 0;
  else if(peak >= IEM_VU_MAXDB)
    x->x_peak = IEM_VU_STEPS;
  else
  {
    int ii = (int)(2.0*(peak + IEM_VU_OFFSET));
    x->x_peak = iem_vu_db2i[ii];
  }
  i = (int)(100.0*peak + 10000.5);
  peak = 0.01*(t_float)(i - 10000);
  x->x_fp = peak;
  
  iem_vu_update_rms(x, x->x_gui.x_glist);
  iem_vu_update_peak(x, x->x_gui.x_glist);
}

static void iem_vu_bang(t_iem_vu *x)
{
  outlet_float(x->x_out_peak, x->x_fp);
  outlet_float(x->x_out_rms, x->x_fr);
  iem_vu_update_rms(x, x->x_gui.x_glist);
  iem_vu_update_peak(x, x->x_gui.x_glist);
}

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void *iem_vu_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iem_vu *x = (t_iem_vu *)pd_new(iem_vu_class);
  int bflcol[]={-66577, -1, -1};
  int w=IEM_GUI_DEFAULTSIZE;
  int ldx=-1, ldy=-8, f=0, fs=8, scale=1;
  int ftbreak=IEM_BNG_DEFAULTBREAKFLASHTIME, fthold=IEM_BNG_DEFAULTHOLDFLASHTIME;
  char str[144];
  
  iem_inttosymargs(&x->x_gui.x_isa, 0);
  iem_inttofstyle(&x->x_gui.x_fsf, 0);
  
  if((argc >= 11)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&(IS_A_SYMBOL(argv,2)||IS_A_FLOAT(argv,2))
    &&(IS_A_SYMBOL(argv,3)||IS_A_FLOAT(argv,3))
    &&IS_A_FLOAT(argv,4)&&IS_A_FLOAT(argv,5)
    &&IS_A_FLOAT(argv,6)&&IS_A_FLOAT(argv,7)
    &&IS_A_FLOAT(argv,8)&&IS_A_FLOAT(argv,9)&&IS_A_FLOAT(argv,10))
  {
    w = (int)atom_getintarg(0, argc, argv);
    iemgui_new_getnames(&x->x_gui, 1, argv);
    ldx = (int)atom_getintarg(4, argc, argv);
    ldy = (int)atom_getintarg(5, argc, argv);
    iem_inttofstyle(&x->x_gui.x_fsf, atom_getintarg(6, argc, argv));
    fs = (int)atom_getintarg(7, argc, argv);
    bflcol[0] = (int)atom_getintarg(8, argc, argv);
    bflcol[2] = (int)atom_getintarg(9, argc, argv);
    scale = (int)atom_getintarg(10, argc, argv);
  }
  else iemgui_new_getnames(&x->x_gui, 1, 0);
  if((argc == 12)&&IS_A_FLOAT(argv,11))
    iem_inttosymargs(&x->x_gui.x_isa, atom_getintarg(11, argc, argv));
  x->x_gui.x_draw = (t_iemfunptr)iem_vu_draw;
  
  x->x_gui.x_fsf.x_snd_able = 0;
  x->x_gui.x_fsf.x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  if (!strcmp(x->x_gui.x_rcv->s_name, "empty"))
    x->x_gui.x_fsf.x_rcv_able = 0;
  if (x->x_gui.x_fsf.x_font_style == 1)
    strcpy(x->x_gui.x_font, "helvetica");
  else if(x->x_gui.x_fsf.x_font_style == 2)
    strcpy(x->x_gui.x_font, "times");
  else { x->x_gui.x_fsf.x_font_style = 0;
  strcpy(x->x_gui.x_font, "courier"); }
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  x->x_gui.x_ldx = ldx;
  x->x_gui.x_ldy = ldy;
  
  if(fs < 4)
    fs = 4;
  x->x_gui.x_fontsize = fs;
  //  x->x_gui.x_w = iem_vu_clip_width(w)-1;
  x->x_gui.x_w = iem_vu_clip_width(w+1);
  x->x_old_width = x->x_gui.x_w;
  x->x_gui.x_h = 120;
  
  iemgui_all_colfromload(&x->x_gui, bflcol);
  if(scale != 0)
    scale = 1;
  x->x_scale = scale;
  x->x_peak = 0;
  x->x_rms = 0;
  x->x_fp = -101.0;
  x->x_fr = -101.0;
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  inlet_new(&x->x_gui.x_obj, &x->x_gui.x_obj.ob_pd, &s_float, gensym("ft1"));
  x->x_out_rms = outlet_new(&x->x_gui.x_obj, &s_float);
  x->x_out_peak = outlet_new(&x->x_gui.x_obj, &s_float);
  x->x_scale_w = 14;
  x->x_scale_h = 129;
  strcpy(x->x_scale_gif, my_iemgui_black_vscale_gif);
  x->x_gui.x_fsf.x_selected = 0;
  return (x);
}
#else
static void *iem_vu_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iem_vu *x = (t_iem_vu *)pd_new(iem_vu_class);
  int bflcol[]={-66577, -1, -1};
  t_symbol *srl[3];
  int w=IEM_GUI_DEFAULTSIZE;
  int ldx=-1, ldy=-8, f=0, fs=8, scale=1;
  int iinit=0, ifstyle=0;
  int ftbreak=IEM_BNG_DEFAULTBREAKFLASHTIME, fthold=IEM_BNG_DEFAULTHOLDFLASHTIME;
  t_iem_init_symargs *init=(t_iem_init_symargs *)(&iinit);
  t_iem_fstyle_flags *fstyle=(t_iem_fstyle_flags *)(&ifstyle);
  char str[144];
  
  srl[0] = gensym("empty");
  srl[1] = gensym("empty");
  srl[2] = gensym("empty");
  
  if((argc >= 11)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&(IS_A_SYMBOL(argv,2)||IS_A_FLOAT(argv,2))
    &&(IS_A_SYMBOL(argv,3)||IS_A_FLOAT(argv,3))
    &&IS_A_FLOAT(argv,4)&&IS_A_FLOAT(argv,5)
    &&IS_A_FLOAT(argv,6)&&IS_A_FLOAT(argv,7)
    &&IS_A_FLOAT(argv,8)&&IS_A_FLOAT(argv,9)&&IS_A_FLOAT(argv,10))
  {
    w = (int)atom_getintarg(0, argc, argv);
    if(IS_A_SYMBOL(argv,2))
      srl[1] = atom_getsymbolarg(2, argc, argv);
    else if(IS_A_FLOAT(argv,2))
    {
      sprintf(str, "%d", (int)atom_getintarg(2, argc, argv));
      srl[1] = gensym(str);
    }
    if(IS_A_SYMBOL(argv,3))
      srl[2] = atom_getsymbolarg(3, argc, argv);
    else if(IS_A_FLOAT(argv,3))
    {
      sprintf(str, "%d", (int)atom_getintarg(3, argc, argv));
      srl[2] = gensym(str);
    }
    ldx = (int)atom_getintarg(4, argc, argv);
    ldy = (int)atom_getintarg(5, argc, argv);
    ifstyle = (int)atom_getintarg(6, argc, argv);
    fs = (int)atom_getintarg(7, argc, argv);
    bflcol[0] = (int)atom_getintarg(8, argc, argv);
    bflcol[2] = (int)atom_getintarg(9, argc, argv);
    scale = (int)atom_getintarg(10, argc, argv);
  }
  if((argc == 12)&&IS_A_FLOAT(argv,11))
    iinit = (int)atom_getintarg(11, argc, argv);
  x->x_gui.x_draw = (t_iemfunptr)iem_vu_draw;
  iinit &= IEM_INIT_ARGS_ALL;
  ifstyle &= IEM_FSTYLE_FLAGS_ALL;
  
  fstyle->x_snd_able = 0;
  fstyle->x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  x->x_gui.x_isa = *init;
  if(!strcmp(srl[1]->s_name, "empty")) fstyle->x_rcv_able = 0;
  x->x_gui.x_unique_num = 0;
  if(fstyle->x_font_style == 1)
    strcpy(x->x_gui.x_font, "helvetica");
  else if(fstyle->x_font_style == 2)
    strcpy(x->x_gui.x_font, "times");
  else
  { 
    fstyle->x_font_style = 0;
    strcpy(x->x_gui.x_font, "courier");
  }
  x->x_gui.x_fsf = *fstyle;
  iemgui_first_dollararg2sym(&x->x_gui, srl);
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, srl[1]);
  x->x_gui.x_snd = srl[0];
  x->x_gui.x_rcv = srl[1];
  x->x_gui.x_lab = srl[2];
  x->x_gui.x_ldx = ldx;
  x->x_gui.x_ldy = ldy;
  
  if(fs < 4)
    fs = 4;
  x->x_gui.x_fontsize = fs;
  //  x->x_gui.x_w = iem_vu_clip_width(w)-1;
  x->x_gui.x_w = iem_vu_clip_width(w+1);
  x->x_old_width = x->x_gui.x_w;
  x->x_gui.x_h = 120;
  
  iemgui_all_colfromload(&x->x_gui, bflcol);
  if(scale != 0)
    scale = 1;
  x->x_scale = scale;
  x->x_peak = 0;
  x->x_rms = 0;
  x->x_fp = -101.0;
  x->x_fr = -101.0;
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  inlet_new(&x->x_gui.x_obj, &x->x_gui.x_obj.ob_pd, &s_float, gensym("ft1"));
  x->x_out_rms = outlet_new(&x->x_gui.x_obj, &s_float);
  x->x_out_peak = outlet_new(&x->x_gui.x_obj, &s_float);
  x->x_scale_w = 14;
  x->x_scale_h = 129;
  strcpy(x->x_scale_gif, my_iemgui_black_vscale_gif);
  x->x_gui.x_fsf.x_selected = 0;
  return (x);
}
#endif

static void iem_vu_free(t_iem_vu *x)
{
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  gfxstub_deleteforkey(x);
}

void iem_vu_setup(void)
{
  iem_vu_class = class_new(gensym("iem_vu"), (t_newmethod)iem_vu_new, (t_method)iem_vu_free,
    sizeof(t_iem_vu), 0, A_GIMME, 0);
  class_addbang(iem_vu_class, iem_vu_bang);
  class_addfloat(iem_vu_class, iem_vu_float);
  class_addmethod(iem_vu_class, (t_method)iem_vu_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_set, gensym("set"), A_GIMME, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_dialog, gensym("dialog"), A_GIMME, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_size, gensym("size"), A_GIMME, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_scale, gensym("scale"), A_DEFFLOAT, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_delta, gensym("delta"), A_GIMME, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_pos, gensym("pos"), A_GIMME, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_color, gensym("color"), A_GIMME, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_receive, gensym("receive"), A_DEFSYM, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_label, gensym("label"), A_DEFSYM, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_label_pos, gensym("label_pos"), A_GIMME, 0);
  class_addmethod(iem_vu_class, (t_method)iem_vu_label_font, gensym("label_font"), A_GIMME, 0);
  iem_vu_widgetbehavior.w_getrectfn = iem_vu_getrect;
  iem_vu_widgetbehavior.w_displacefn =  iemgui_displace;
  iem_vu_widgetbehavior.w_selectfn =   iemgui_select;
  iem_vu_widgetbehavior.w_activatefn =  NULL;
  iem_vu_widgetbehavior.w_deletefn =   iemgui_delete;
  iem_vu_widgetbehavior.w_visfn =   iemgui_vis;
  iem_vu_widgetbehavior.w_clickfn =   NULL;
  
#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
  class_setsavefn(iem_vu_class, iem_vu_save);
  class_setpropertiesfn(iem_vu_class, iem_vu_properties);
#else
  iem_vu_widgetbehavior.w_propertiesfn = iem_vu_properties;
  iem_vu_widgetbehavior.w_savefn =    iem_vu_save;
#endif
  
  class_setwidget(iem_vu_class, &iem_vu_widgetbehavior);
//  class_sethelpsymbol(iem_vu_class, gensym("iemhelp2/help-iem_vu"));
}
