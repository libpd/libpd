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
# include <io.h>
#else
# include <unistd.h>
#endif

/* ------------------------ setup routine ------------------------- */

t_widgetbehavior vfad_scale_widgetbehavior;
static t_class *vfad_scale_class;

typedef struct _vfad_scale
{
  t_iemgui  x_gui;
  char      x_gif[720];
} t_vfad_scale;

static void vfad_scale_draw_new(t_vfad_scale *x, t_glist *glist)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui("image create photo %xPHOTOIMAGE -format gif -data {%s} -width %d -height %d\n",
    x, x->x_gif, x->x_gui.x_w, x->x_gui.x_h);
  sys_vgui(".x%x.c create image %d %d -image %xPHOTOIMAGE -tags %xPHOTO\n",
    canvas, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2, x, x);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%x.c create rectangle %d %d %d %d -outline #%6.6x -tags %xBASE\n",
    canvas, xpos, ypos,
    xpos + x->x_gui.x_w, ypos + x->x_gui.x_h,
    IEM_GUI_COLOR_SELECTED, x);
}

static void vfad_scale_draw_move(t_vfad_scale *x, t_glist *glist)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  sys_vgui(".x%x.c coords %xPHOTO %d %d\n",
    canvas, x, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2);
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%x.c coords %xBASE %d %d %d %d\n",
    canvas, x, xpos, ypos,
    xpos + x->x_gui.x_w, ypos + x->x_gui.x_h);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
}

static void vfad_scale_draw_erase(t_vfad_scale* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%x.c delete %xBASE\n", canvas, x);
  sys_vgui("image delete %xPHOTOIMAGE\n", x);
  sys_vgui(".x%x.c delete %xPHOTO\n", canvas, x);
}

static void vfad_scale_draw_select(t_vfad_scale* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
  {
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    
    sys_vgui(".x%x.c create rectangle %d %d %d %d -outline #%6.6x -tags %xBASE\n",
      canvas, xpos, ypos, xpos + x->x_gui.x_w,
      ypos + x->x_gui.x_h, IEM_GUI_COLOR_SELECTED, x);
  }
  else
    sys_vgui(".x%x.c delete %xBASE\n", canvas, x);
}

static void vfad_scale_draw(t_vfad_scale *x, t_glist *glist, int mode)
{
  if(mode == IEM_GUI_DRAW_MODE_MOVE)
    vfad_scale_draw_move(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_NEW)
    vfad_scale_draw_new(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_SELECT)
    vfad_scale_draw_select(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_ERASE)
    vfad_scale_draw_erase(x, glist);
}

/* ------------------------ cnv widgetbehaviour----------------------------- */

static void vfad_scale_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
  t_vfad_scale *x = (t_vfad_scale *)z;
  
  *xp1 = text_xpix(&x->x_gui.x_obj, glist);
  *yp1 = text_ypix(&x->x_gui.x_obj, glist);
  *xp2 = *xp1 + x->x_gui.x_w;
  *yp2 = *yp1 + x->x_gui.x_h;
}

static void vfad_scale_color(t_vfad_scale *x, t_symbol *s, int argc, t_atom *argv)
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
      sys_vgui("%xPHOTOIMAGE configure -data {%s}\n", x, x->x_gif);
  }
}

static void vfad_scale_save(t_gobj *z, t_binbuf *b)
{
  t_vfad_scale *x = (t_vfad_scale *)z;
  
  binbuf_addv(b, "ssiisi", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    -1 - (((0xfc0000 & x->x_gui.x_lcol) >> 6)|
    ((0xfc00 & x->x_gui.x_lcol) >> 4)|((0xfc & x->x_gui.x_lcol) >> 2)));
  binbuf_addv(b, ";");
}

static void *vfad_scale_new(t_symbol *s, int argc, t_atom *argv)
{
  t_vfad_scale *x = (t_vfad_scale *)pd_new(vfad_scale_class);
  
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
  x->x_gui.x_draw = (t_iemfunptr)vfad_scale_draw;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  x->x_gui.x_w = 12;
  x->x_gui.x_h = 126;
  strcpy(x->x_gif, my_iemgui_black_vscale_gif);
  my_iemgui_change_scale_col(x->x_gif, x->x_gui.x_lcol);
  x->x_gui.x_fsf.x_selected = 0;
  return(x);
}

static void vfad_scale_ff(t_vfad_scale *x)
{
  gfxstub_deleteforkey(x);
}

void vfad_scale_setup(void)
{
  vfad_scale_class = class_new(gensym("vfad_scale"), (t_newmethod)vfad_scale_new,
    (t_method)vfad_scale_ff, sizeof(t_vfad_scale), 0, A_GIMME, 0);
  class_addmethod(vfad_scale_class, (t_method)vfad_scale_color, gensym("color"), A_GIMME, 0);
  vfad_scale_widgetbehavior.w_getrectfn = vfad_scale_getrect;
  vfad_scale_widgetbehavior.w_displacefn = iemgui_displace;
  vfad_scale_widgetbehavior.w_selectfn = iemgui_select;
  vfad_scale_widgetbehavior.w_activatefn = NULL;
  vfad_scale_widgetbehavior.w_deletefn = iemgui_delete;
  vfad_scale_widgetbehavior.w_visfn = iemgui_vis;
  vfad_scale_widgetbehavior.w_clickfn = NULL;
  
#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
  class_setsavefn(vfad_scale_class, vfad_scale_save);
#else
  vfad_scale_widgetbehavior.w_propertiesfn = NULL;
  vfad_scale_widgetbehavior.w_savefn = vfad_scale_save;
#endif
  
  class_setwidget(vfad_scale_class, &vfad_scale_widgetbehavior);
//  class_sethelpsymbol(vfad_scale_class, gensym("iemhelp2/help-vfad_scale"));
}
