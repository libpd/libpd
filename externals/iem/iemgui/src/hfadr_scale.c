/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemgui written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include "iemgui.h"
#include "g_canvas.h"
#include "g_all_guis.h"
#include <string.h>

#ifdef MSW
#include <io.h>
#else
#include <unistd.h>
#endif

/* ------------------------ setup routine ------------------------- */

t_widgetbehavior hfadr_scale_widgetbehavior;
static t_class *hfadr_scale_class;

typedef struct _hfadr_scale
{
  t_iemgui  x_gui;
  char      x_gif[870];
} t_hfadr_scale;

static void hfadr_scale_draw_new(t_hfadr_scale *x, t_glist *glist)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui("image create photo %lxPHOTOIMAGE -format gif -data {%s} -width %d -height %d\n",
    x, x->x_gif, x->x_gui.x_w, x->x_gui.x_h);
  sys_vgui(".x%lx.c create image %d %d -image %lxPHOTOIMAGE -tags %lxPHOTO\n",
    canvas, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2-1, x, x);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline #%6.6x -tags %lxBASE\n",
    canvas, xpos-1, ypos+1,
    xpos + x->x_gui.x_w-1, ypos + x->x_gui.x_h,
    IEM_GUI_COLOR_SELECTED, x);
}

static void hfadr_scale_draw_move(t_hfadr_scale *x, t_glist *glist)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui(".x%lx.c coords %lxPHOTO %d %d\n",
    canvas, x, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2-1);
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d\n",
    canvas, x, xpos-1, ypos+1,
    xpos + x->x_gui.x_w-1, ypos + x->x_gui.x_h);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
}

static void hfadr_scale_draw_erase(t_hfadr_scale* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
  sys_vgui("image delete %lxPHOTOIMAGE\n", x);
  sys_vgui(".x%lx.c delete %lxPHOTO\n", canvas, x);
}

static void hfadr_scale_draw_select(t_hfadr_scale* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
  {
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline #%6.6x -tags %lxBASE\n",
      canvas, xpos-1, ypos+1, xpos + x->x_gui.x_w-1,
      ypos + x->x_gui.x_h, IEM_GUI_COLOR_SELECTED, x);
  }
  else
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
}

static void hfadr_scale_draw(t_hfadr_scale *x, t_glist *glist, int mode)
{
  if(mode == IEM_GUI_DRAW_MODE_MOVE)
    hfadr_scale_draw_move(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_NEW)
    hfadr_scale_draw_new(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_SELECT)
    hfadr_scale_draw_select(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_ERASE)
    hfadr_scale_draw_erase(x, glist);
}

/* ------------------------ cnv widgetbehaviour----------------------------- */

static void hfadr_scale_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
  t_hfadr_scale *x = (t_hfadr_scale *)z;
  
  *xp1 = text_xpix(&x->x_gui.x_obj, glist)-1;
  *yp1 = text_ypix(&x->x_gui.x_obj, glist)+1;
  *xp2 = *xp1 + x->x_gui.x_w;
  *yp2 = *yp1 + x->x_gui.x_h-1;
}

static void hfadr_scale_color(t_hfadr_scale *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 1)&&IS_A_FLOAT(argv,0))
  {
    int j, i = (int)atom_getintarg(0, argc, argv);
    
    if(i >= 0)
    {
      j = iemgui_modulo_color(i);
      x->x_gui.x_lcol = my_iemgui_color_hex[j];
    }
    else
      x->x_gui.x_lcol = (-1 - i) & 0xffffff;
    my_iemgui_change_scale_col(x->x_gif, x->x_gui.x_lcol);
    if(glist_isvisible(x->x_gui.x_glist))
      sys_vgui("%lxPHOTOIMAGE configure -data {%s}\n", x, x->x_gif);
  }
}

static void hfadr_scale_save(t_gobj *z, t_binbuf *b)
{
  t_hfadr_scale *x = (t_hfadr_scale *)z;
  
  binbuf_addv(b, "ssiisi", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    -1 - (((0xfc0000 & x->x_gui.x_lcol) >> 6)|
    ((0xfc00 & x->x_gui.x_lcol) >> 4)|((0xfc & x->x_gui.x_lcol) >> 2)));
  binbuf_addv(b, ";");
}

static void *hfadr_scale_new(t_symbol *s, int argc, t_atom *argv)
{
  t_hfadr_scale *x = (t_hfadr_scale *)pd_new(hfadr_scale_class);
  
  if((argc >= 1)&&IS_A_FLOAT(argv,0))
  {
    int j, i = (int)atom_getintarg(0, argc, argv);
    
    if(i >= 0)
    {
      j = iemgui_modulo_color(i);
      x->x_gui.x_lcol = my_iemgui_color_hex[j];
    }
    else
    {
      j = -1 - i;
      x->x_gui.x_lcol = ((j & 0x3f000) << 6)|((j & 0xfc0) << 4)|((j & 0x3f) << 2);
    }
  }
  else
    x->x_gui.x_lcol = 0;
  x->x_gui.x_draw = (t_iemfunptr)hfadr_scale_draw;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  x->x_gui.x_w = 126;
  x->x_gui.x_h = 21;
  strcpy(x->x_gif, my_iemgui_black_hrscale_gif);
  my_iemgui_change_scale_col(x->x_gif, x->x_gui.x_lcol);
  x->x_gui.x_fsf.x_selected = 0;
  return(x);
}

static void hfadr_scale_ff(t_hfadr_scale *x)
{
  gfxstub_deleteforkey(x);
}

void hfadr_scale_setup(void)
{
  hfadr_scale_class = class_new(gensym("hfadr_scale"), (t_newmethod)hfadr_scale_new,
    (t_method)hfadr_scale_ff, sizeof(t_hfadr_scale), 0, A_GIMME, 0);
  class_addmethod(hfadr_scale_class, (t_method)hfadr_scale_color, gensym("color"), A_GIMME, 0);
  hfadr_scale_widgetbehavior.w_getrectfn = hfadr_scale_getrect;
  hfadr_scale_widgetbehavior.w_displacefn = iemgui_displace;
  hfadr_scale_widgetbehavior.w_selectfn = iemgui_select;
  hfadr_scale_widgetbehavior.w_activatefn = NULL;
  hfadr_scale_widgetbehavior.w_deletefn = iemgui_delete;
  hfadr_scale_widgetbehavior.w_visfn = iemgui_vis;
  hfadr_scale_widgetbehavior.w_clickfn = NULL;
  
#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
  class_setsavefn(hfadr_scale_class, hfadr_scale_save);
#else
  hfadr_scale_widgetbehavior.w_propertiesfn = NULL;
  hfadr_scale_widgetbehavior.w_savefn = hfadr_scale_save;
#endif
  
  class_setwidget(hfadr_scale_class, &hfadr_scale_widgetbehavior);
//  class_sethelpsymbol(hfadr_scale_class, gensym("iemhelp2/help-hfadr_scale"));
}
