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

#define IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT  10
#define IEM_GUI_NBXM_DRAW_MODE_UPDATE_ROW      11
#define IEM_GUI_NBXM_DRAW_MODE_UPDATE_COLUMN   12
#define IEM_GUI_NBXM_DRAW_MODE_UPDATE_MATRIX   13

#define IEM_GUI_NBXM_COLOR_SELECTED        "#0000FF"
#define IEM_GUI_NBXM_COLOR_NORMAL          "#000000"
#define IEM_GUI_NBXM_COLOR_EDITED          "#FF0000"

/* ---------- numberbox_matrix  a matrix of number-boxes ---------------- 
passive (like set) input messages :
matrix r c e11 e12 e13 e14 e21 ..... e63 e64
row i ei1 ei2 ei3 ei4
col j e1j e2j e3j e4j e5j e6j
element ir jc e
list ir jc e
float e ... for all elements = e

  active messages:
  bang   .... outputs the whole matrix-message
*/

t_widgetbehavior numberbox_matrix_widgetbehavior;
static t_class *numberbox_matrix_class;

typedef struct _numberbox_matrix
{
  t_iemgui x_gui;
  t_clock  *x_clock_reset;
  t_clock  *x_clock_wait;
  t_int    x_n_column;
  t_int    x_n_row;
  t_int    x_sel_column;
  t_int    x_sel_row;
  t_int    x_update_mode;
  t_int    x_resid;
  double   x_val;
  double   x_min;
  double   x_max;
  double   x_k;
  char     x_buf[IEMGUI_MAX_NUM_LEN];
  t_int    x_numwidth;
  t_symbol *x_front_color;
  t_atom   *x_matrix;
  t_atom   x_at_out[3];
} t_numberbox_matrix;

static void numberbox_matrix_key(void *z, t_floatarg fkey);
t_widgetbehavior numberbox_matrix_widgetbehavior;
static t_class *numberbox_matrix_class;

static void numberbox_matrix_tick_reset(t_numberbox_matrix *x)
{
  if(x->x_gui.x_fsf.x_change)
  {
    x->x_gui.x_fsf.x_change = 0;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, x->x_update_mode);
  }
}

static void numberbox_matrix_tick_wait(t_numberbox_matrix *x)
{
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, x->x_update_mode);
}

void numberbox_matrix_clip_element(t_numberbox_matrix *x, t_atom *av)
{
  t_float f=atom_getfloat(av);
  
  if(f < x->x_min)
    f = x->x_min;
  if(f > x->x_max)
    f = x->x_max;
  
  SETFLOAT(av, f);
}

void numberbox_matrix_calc_fontwidth(t_numberbox_matrix *x)
{
  int w, f=31;
  
  if(x->x_gui.x_fsf.x_font_style == 1)
    f = 27;
  else if(x->x_gui.x_fsf.x_font_style == 2)
    f = 25;
  
  w = x->x_gui.x_fontsize * f * x->x_gui.x_w;
  w /= 36;
  x->x_numwidth = w + 4;
}

void numberbox_matrix_ftoa(t_numberbox_matrix *x, t_float g, char *buf)
{
  double f=(double)g;
  int bufsize, is_exp=0, i, idecimal;
  
  sprintf(buf, "%g", f);
  bufsize = strlen(buf);
  if(bufsize >= 5)/* if it is in exponential mode */
  {
    i = bufsize - 4;
    if((buf[i] == 'e') || (buf[i] == 'E'))
      is_exp = 1;
  }
  if(bufsize > x->x_gui.x_w)/* if to reduce */
  {
    if(is_exp)
    {
      if(x->x_gui.x_w <= 5)
      {
        buf[0] = (f < 0.0 ? '-' : '+');
        buf[1] = 0;
      }
      i = bufsize - 4;
      for(idecimal=0; idecimal < i; idecimal++)
        if(buf[idecimal] == '.')
          break;
        if(idecimal > (x->x_gui.x_w - 4))
        {
          buf[0] = (f < 0.0 ? '-' : '+');
          buf[1] = 0;
        }
        else
        {
          int new_exp_index=x->x_gui.x_w-4, old_exp_index=bufsize-4;
          
          for(i=0; i < 4; i++, new_exp_index++, old_exp_index++)
            buf[new_exp_index] = buf[old_exp_index];
          buf[x->x_gui.x_w] = 0;
        }
        
    }
    else
    {
      for(idecimal=0; idecimal < bufsize; idecimal++)
        if(buf[idecimal] == '.')
          break;
        if(idecimal > x->x_gui.x_w)
        {
          buf[0] = (f < 0.0 ? '-' : '+');
          buf[1] = 0;
        }
        else
          buf[x->x_gui.x_w] = 0;
    }
  }
}

static void numberbox_matrix_output_element(t_numberbox_matrix *x, t_atom *av)
{
  
  if(x->x_n_row == 1)
  {
    if(x->x_n_column == 1) //nr=1, nc=1
    {
      t_float f = atom_getfloat(av);
      
      outlet_float(x->x_gui.x_obj.ob_outlet, f);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, f);
    }
    else // nr=1, nc>1 : duo of jc, element
    {
      SETFLOAT(x->x_at_out, (t_float)(x->x_sel_column+1));
      x->x_at_out[1] = *av;
      outlet_list(x->x_gui.x_obj.ob_outlet, &s_list, 2, x->x_at_out);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_list(x->x_gui.x_snd->s_thing, &s_list, 2, x->x_at_out);
    }
  }
  else
  {
    if(x->x_n_column == 1) // nr>1, nc=1 : duo of ir, element
    {
      SETFLOAT(x->x_at_out, (t_float)(x->x_sel_row+1));
      x->x_at_out[1] = *av;
      outlet_list(x->x_gui.x_obj.ob_outlet, &s_list, 2, x->x_at_out);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_list(x->x_gui.x_snd->s_thing, &s_list, 2, x->x_at_out);
    }
    else // nr>1, nc>1 : trio of ir, jc, element
    {
      SETFLOAT(x->x_at_out, (t_float)(x->x_sel_row+1));
      SETFLOAT(x->x_at_out+1, (t_float)(x->x_sel_column+1));
      x->x_at_out[2] = *av;
      outlet_list(x->x_gui.x_obj.ob_outlet, &s_list, 3, x->x_at_out);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_list(x->x_gui.x_snd->s_thing, &s_list, 3, x->x_at_out);
    }
  }
}

static void numberbox_matrix_draw_update(t_numberbox_matrix *x, t_glist *glist)
{
  if(glist_isvisible(glist))
  {
    if(x->x_update_mode == IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT)
    {
      t_int ir=x->x_sel_row;
      t_int jc=x->x_sel_column;
      t_int k;
      
      if(x->x_gui.x_fsf.x_change)
      {
        if(x->x_buf[0])
        {
          char *cp=x->x_buf;
          int sl = strlen(x->x_buf);
          
          x->x_buf[sl] = '>';
          x->x_buf[sl+1] = 0;
          if(sl >= x->x_gui.x_w)
            cp += sl - x->x_gui.x_w + 1;
          sys_vgui(
            ".x%lx.c itemconfigure %lxNUMBER_%d_%d -fill %s -text {%s} \n",
            glist_getcanvas(glist), x, ir, jc, IEM_GUI_NBXM_COLOR_EDITED, cp);
          x->x_buf[sl] = 0;
        }
        else
        {
          k = x->x_sel_row;
          k *= x->x_n_column;
          k += x->x_sel_column + 3;
          numberbox_matrix_ftoa(x, atom_getfloat(x->x_matrix+k), x->x_buf);
          sys_vgui(".x%lx.c itemconfigure %lxNUMBER_%d_%d  -fill %s -text {%s} \n",
            glist_getcanvas(glist), x, ir, jc, IEM_GUI_NBXM_COLOR_EDITED, x->x_buf);
          x->x_buf[0] = 0;
        }
      }
      else
      {
        k = x->x_sel_row;
        k *= x->x_n_column;
        k += x->x_sel_column + 3;
        numberbox_matrix_ftoa(x, atom_getfloat(x->x_matrix+k), x->x_buf);
        sys_vgui(".x%lx.c itemconfigure %lxNUMBER_%d_%d -fill %s -text {%s} \n",
          glist_getcanvas(glist), x, ir, jc, x->x_front_color->s_name, x->x_buf);
        x->x_buf[0] = 0;
      }
    }
    else if(x->x_update_mode == IEM_GUI_NBXM_DRAW_MODE_UPDATE_MATRIX)
    {
      t_int i, r=x->x_n_row;
      t_int j, c=x->x_n_column;
      t_int k = 3;
      
      for(i=0; i<r; i++)
      {
        for(j=0; j<c; j++)
        {
          x->x_val = atom_getfloat(x->x_matrix+k);
          k++;
          numberbox_matrix_ftoa(x, x->x_val, x->x_buf);
          sys_vgui(".x%lx.c itemconfigure %lxNUMBER_%d_%d -fill %s -text {%s} \n",
            glist_getcanvas(glist), x, i, j, x->x_front_color->s_name, x->x_buf);
        }
      }
    }
    else if(x->x_update_mode == IEM_GUI_NBXM_DRAW_MODE_UPDATE_ROW)
    {
      t_int ir=x->x_sel_row;
      t_int j, c=x->x_n_column;
      t_int k = 3+ir*c;
      
      for(j=0; j<c; j++)
      {
        x->x_val = atom_getfloat(x->x_matrix+k);
        k++;
        numberbox_matrix_ftoa(x, x->x_val, x->x_buf);
        sys_vgui(".x%lx.c itemconfigure %lxNUMBER_%d_%d -fill %s -text {%s} \n",
          glist_getcanvas(glist), x, ir, j, x->x_front_color->s_name, x->x_buf);
      }
    }
    else if(x->x_update_mode == IEM_GUI_NBXM_DRAW_MODE_UPDATE_COLUMN)
    {
      t_int i, r=x->x_n_row;
      t_int c=x->x_n_column;
      t_int jc=x->x_sel_column;
      t_int k = 3+jc;
      
      for(i=0; i<r; i++)
      {
        x->x_val = atom_getfloat(x->x_matrix+k);
        k += c;
        numberbox_matrix_ftoa(x, x->x_val, x->x_buf);
        sys_vgui(".x%lx.c itemconfigure %lxNUMBER_%d_%d -fill %s -text {%s} \n",
          glist_getcanvas(glist), x, i, jc, x->x_front_color->s_name, x->x_buf);
      }
    }
  }
}

static void numberbox_matrix_draw_new(t_numberbox_matrix *x, t_glist *glist)
{
  t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
  t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_int i, j, k, r, c;
  t_canvas *canvas=glist_getcanvas(glist);
  t_int h=x->x_gui.x_h, w=x->x_numwidth;
  t_int hh=h/2, hw=w/2;
  t_int xx, yy;
  
  r = x->x_n_row;
  c = x->x_n_column;
  for(i=0; i<c; i+=2)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxCOLUMNS_%d\n",
    canvas, xpos + x->x_numwidth*i, ypos, xpos + x->x_numwidth*(i+1), ypos + x->x_gui.x_h*r, x, i);
  for(i=0; i<r; i+=2)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxROWS_%d\n",
    canvas, xpos, ypos + x->x_gui.x_h*i, xpos + x->x_numwidth*c, ypos + x->x_gui.x_h*(i+1), x, i);
  k = 3;
  yy = ypos + hh;
  for(i=0; i<r; i++)
  {
    xx = xpos + hw;
    for(j=0; j<c; j++)
    {
      x->x_val = atom_getfloat(x->x_matrix+k);
      k++;
      numberbox_matrix_ftoa(x, x->x_val, x->x_buf);
      sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor c \
        -font {%s %d bold} -fill %s -tags %lxNUMBER_%d_%d\n", canvas, xx, yy,
        x->x_buf, x->x_gui.x_font, x->x_gui.x_fontsize, x->x_front_color->s_name, x, i, j);
      xx += w;
    }
    yy += h;
  }
  
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxFRAME\n",
    canvas, xpos, ypos, xpos + x->x_numwidth*c, ypos + x->x_gui.x_h*r, x);
  if(!x->x_gui.x_fsf.x_snd_able)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
    canvas,
    xpos, ypos + x->x_gui.x_h*r-1,
    xpos+IOWIDTH, ypos + x->x_gui.x_h*r,
    x, 0);
  if(!x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
    canvas,
    xpos, ypos,
    xpos+IOWIDTH, ypos+1,
    x, 0);
}
static void numberbox_matrix_draw_move(t_numberbox_matrix *x, t_glist *glist)
{
  t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
  t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_int i, j, r, c;
  t_canvas *canvas=glist_getcanvas(glist);
  t_int h=x->x_gui.x_h, w=x->x_numwidth;
  t_int hh=h/2, hw=w/2;
  t_int xx, yy;
  
  r = x->x_n_row;
  c = x->x_n_column;
  
  for(i=0; i<c; i+=2)
    sys_vgui(".x%lx.c coords %lxCOLUMNS_%d %d %d %d %d\n",
    canvas, x, i, xpos + x->x_numwidth*i, ypos, xpos + x->x_numwidth*(i+1), ypos + x->x_gui.x_h*r);
  for(i=0; i<r; i+=2)
    sys_vgui(".x%lx.c coords %lxROWS_%d %d %d %d %d\n",
    canvas, x, i, xpos, ypos + x->x_gui.x_h*i, xpos + x->x_numwidth*c, ypos + x->x_gui.x_h*(i+1));
  
  yy = ypos + hh;
  for(i=0; i<r; i++)
  {
    xx = xpos + hw;
    for(j=0; j<c; j++)
    {
      sys_vgui(".x%lx.c coords %lxNUMBER_%d_%d %d %d\n",
        canvas, x, i, j, xx, yy);
      xx += w;
    }
    yy += h;
  }
  
  sys_vgui(".x%lx.c coords %lxFRAME %d %d %d %d\n",
    canvas, x, xpos, ypos, xpos + x->x_numwidth*c, ypos + x->x_gui.x_h*r);
  if(!x->x_gui.x_fsf.x_snd_able)
    sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
    canvas, x, 0,
    xpos, ypos + x->x_gui.x_h*r-1,
    xpos+IOWIDTH, ypos + x->x_gui.x_h*r);
  if(!x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
    canvas, x, 0,
    xpos, ypos,
    xpos+IOWIDTH, ypos+1);
}

static void numberbox_matrix_draw_erase(t_numberbox_matrix* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  t_int i, j, r, c;
  
  r = x->x_n_row;
  c = x->x_n_column;
  for(i=0; i<c; i+=2)
    sys_vgui(".x%lx.c delete %lxCOLUMNS_%d\n", canvas, x, i);
  for(i=0; i<r; i+=2)
    sys_vgui(".x%lx.c delete %lxROWS_%d\n", canvas, x, i);
  
  for(i=0; i<r; i++)
  {
    for(j=0; j<c; j++)
    {
      sys_vgui(".x%lx.c delete %lxNUMBER_%d_%d\n", canvas, x, i, j);
    }
  }
  
  sys_vgui(".x%lx.c delete %lxFRAME\n", canvas, x);
  if(!x->x_gui.x_fsf.x_snd_able)
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
  if(!x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void numberbox_matrix_draw_select(t_numberbox_matrix *x, t_glist *glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
  {
  /*if(x->x_gui.x_fsf.x_change)
  {
  x->x_gui.x_fsf.x_change = 0;
  clock_unset(x->x_clock_reset);
  x->x_buf[0] = 0;
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  }*/
    sys_vgui(".x%lx.c itemconfigure %lxFRAME -outline %s\n",
      canvas, x, IEM_GUI_NBXM_COLOR_SELECTED);
  }
  else
  {
    sys_vgui(".x%lx.c itemconfigure %lxFRAME -outline %s\n",
      canvas, x, IEM_GUI_NBXM_COLOR_NORMAL);
  }
}

static void numberbox_matrix_draw_io(t_numberbox_matrix* x,t_glist* glist, int old_snd_rcv_flags)
{
  t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
  t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  t_int r=x->x_n_row;
  
  if((old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && !x->x_gui.x_fsf.x_snd_able)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
    canvas,
    xpos, ypos + x->x_gui.x_h*r-1,
    xpos+IOWIDTH, ypos + x->x_gui.x_h*r,
    x, 0);
  if(!(old_snd_rcv_flags & IEM_GUI_OLD_SND_FLAG) && x->x_gui.x_fsf.x_snd_able)
    sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
  if((old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && !x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
    canvas,
    xpos, ypos,
    xpos+IOWIDTH, ypos+1,
    x, 0);
  if(!(old_snd_rcv_flags & IEM_GUI_OLD_RCV_FLAG) && x->x_gui.x_fsf.x_rcv_able)
    sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void numberbox_matrix_draw(t_numberbox_matrix *x, t_glist *glist, int mode)
{
  if(mode == IEM_GUI_DRAW_MODE_UPDATE)
    numberbox_matrix_draw_update(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_MOVE)
    numberbox_matrix_draw_move(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_NEW)
    numberbox_matrix_draw_new(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_SELECT)
    numberbox_matrix_draw_select(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_ERASE)
    numberbox_matrix_draw_erase(x, glist);
  else if(mode >= IEM_GUI_DRAW_MODE_IO)
    numberbox_matrix_draw_io(x, glist, mode - IEM_GUI_DRAW_MODE_IO);
}

/* ------------------------ cnv widgetbehaviour----------------------------- */

static void numberbox_matrix_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
  t_numberbox_matrix *x = (t_numberbox_matrix *)z;
  
  *xp1 = text_xpix(&x->x_gui.x_obj, glist);
  *yp1 = text_ypix(&x->x_gui.x_obj, glist);
  *xp2 = *xp1 + x->x_numwidth*x->x_n_column;
  *yp2 = *yp1 + x->x_gui.x_h*x->x_n_row;
}

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void numberbox_matrix_save(t_gobj *z, t_binbuf *b)
{
  t_numberbox_matrix *x = (t_numberbox_matrix *)z;
  t_symbol *srl[3];
  
  srl[0] = x->x_gui.x_snd;
  srl[1] = x->x_gui.x_rcv;
  srl[2] = gensym("empty");
  iemgui_all_sym2dollararg(&x->x_gui, srl);
  
  binbuf_addv(b, "ssiisiiiiffisssii", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    x->x_n_row, x->x_n_column, x->x_gui.x_w, x->x_gui.x_h,
    x->x_min, x->x_max, iem_symargstoint(&x->x_gui.x_isa),
    srl[0], srl[1], x->x_front_color,
    iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize);
  binbuf_addv(b, ";");
}
#else
static void numberbox_matrix_save(t_gobj *z, t_binbuf *b)
{
  t_numberbox_matrix *x = (t_numberbox_matrix *)z;
  int *ip1, *ip2;
  t_symbol *srl[3];
  
  srl[0] = x->x_gui.x_snd;
  srl[1] = x->x_gui.x_rcv;
  srl[2] = gensym("empty");
  iemgui_all_unique2dollarzero(&x->x_gui, srl);
  iemgui_all_sym2dollararg(&x->x_gui, srl);
  ip1 = (int *)(&x->x_gui.x_isa);
  ip2 = (int *)(&x->x_gui.x_fsf);
  binbuf_addv(b, "ssiisiiiiffisssii", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    x->x_n_row, x->x_n_column, x->x_gui.x_w, x->x_gui.x_h,
    x->x_min, x->x_max, (*ip1)&IEM_INIT_ARGS_ALL,
    srl[0], srl[1], x->x_front_color,
    (*ip2)&IEM_FSTYLE_FLAGS_ALL, x->x_gui.x_fontsize);
  binbuf_addv(b, ";");
}
#endif

/*static void numberbox_matrix_properties(t_gobj *z, t_glist *owner)
{
t_numberbox_matrix *x = (t_numberbox_matrix *)z;
char buf[800];
t_symbol *srl[3];

  iemgui_properties(&x->x_gui, srl);
  if(x->x_gui.x_fsf.x_change)
  {
  x->x_gui.x_fsf.x_change = 0;
  clock_unset(x->x_clock_reset);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  
    }
    sprintf(buf, "pdtk_iemgui_dialog %%s MATRIXBOX \
    -------dimensions(digits)(pix):------- %d %d width: %d %d height: \
    -----------output-range:----------- %g min: %g max: %d \
    %d lin log %d %d log-height: %d \
    %s %s \
    %s %d %d \
    %d %d \
    %d %d %d\n",
    x->x_gui.x_w, 1, x->x_gui.x_h, 8,
    x->x_min, x->x_max, 0,
    -1, -1, -1, -1,
    srl[0]->s_name, srl[1]->s_name,
    "no_label", 0, 0,
    x->x_gui.x_fsf.x_font_style, x->x_gui.x_fontsize,
    -1, 0xffffff & x->x_gui.x_fcol, -1);
    gfxstub_new(&x->x_gui.x_obj.ob_pd, x, buf);
}*/

/*static void numberbox_matrix_dialog(t_numberbox_matrix *x, t_symbol *s, int argc,
t_atom *argv)
{
t_symbol *srl[3];
int w = (int)atom_getintarg(0, argc, argv);
int h = (int)atom_getintarg(1, argc, argv);
double min = (double)atom_getfloatarg(2, argc, argv);
double max = (double)atom_getfloatarg(3, argc, argv);
int lilo = (int)atom_getintarg(4, argc, argv);
int log_height = (int)atom_getintarg(6, argc, argv);
int sr_flags;
t_int i, r=x->x_n_row;
t_int j, c=x->x_n_column;
t_int k = 3;

  if(lilo != 0) lilo = 1;
  x->x_lin0_log1 = lilo;
  sr_flags = iemgui_dialog(&x->x_gui, srl, argc, argv);
  if(w < 1)
  w = 1;
  x->x_gui.x_w = w;
  if(h < 8)
  h = 8;
  x->x_gui.x_h = h;
  if(log_height < 10)
  log_height = 10;
  x->x_log_height = log_height;
  numberbox_matrix_calc_fontwidth(x);
  for(i=0; i<r; i++)
  {
  for(j=0; j<c; j++)
  {
  numberbox_matrix_clip(x, x->x_matrix+k);
  k++;
  }
  }
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_NEW);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
}*/

static void numberbox_matrix_motion(t_numberbox_matrix *x, t_floatarg dx, t_floatarg dy)
{
  double k2=1.0;
  t_int k, res;
  t_float f;
  
  k = x->x_sel_row;
  k *= x->x_n_column;
  k += x->x_sel_column + 3;
  f = atom_getfloat(x->x_matrix+k);
  x->x_resid += (t_int)dy;
  res = x->x_resid / 2;
  dy = (t_floatarg)res;
  x->x_resid = x->x_resid - res*2;
  if(x->x_gui.x_fsf.x_finemoved)
    k2 = 0.01;
  f -= k2*dy;
  
  SETFLOAT(x->x_matrix+k, f);
  numberbox_matrix_clip_element(x, x->x_matrix+k);
  x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT;
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  numberbox_matrix_output_element(x, x->x_matrix+k);
  clock_unset(x->x_clock_reset);
}

static void numberbox_matrix_key(void *z, t_floatarg fkey)
{
  t_numberbox_matrix *x = z;
  char c=fkey;
  char buf[3];
  buf[1] = 0;
  
  if(c == 0)
  {
    x->x_gui.x_fsf.x_change = 0;
    clock_unset(x->x_clock_reset);
    x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    return;
  }
  if(((c>='0')&&(c<='9'))||(c=='.')||(c=='-')||
    (c=='e')||(c=='+')||(c=='E'))
  {
    if(strlen(x->x_buf) < (IEMGUI_MAX_NUM_LEN-2))
    {
      buf[0] = c;
      strcat(x->x_buf, buf);
      x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT;
      (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    }
  }
  else if((c=='\b')||(c==127))
  {
    int sl=strlen(x->x_buf)-1;
    
    if(sl < 0)
      sl = 0;
    x->x_buf[sl] = 0;
    x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  }
  else if((c=='\n')||(c==13))
  {
    t_int k=3+x->x_sel_column+x->x_sel_row*x->x_n_column;
    
    x->x_val = atof(x->x_buf);
    SETFLOAT(x->x_matrix+k, x->x_val);
    x->x_buf[0] = 0;
    x->x_gui.x_fsf.x_change = 0;
    clock_unset(x->x_clock_reset);
    
    numberbox_matrix_clip_element(x, x->x_matrix+k);
    numberbox_matrix_output_element(x, x->x_matrix+k);
    x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  }
  clock_delay(x->x_clock_reset, 3000);
}

static void numberbox_matrix_click(t_numberbox_matrix *x, t_floatarg xpos, t_floatarg ypos,
                                   t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
  glist_grab(x->x_gui.x_glist, &x->x_gui.x_obj.te_g,
    (t_glistmotionfn)numberbox_matrix_motion, numberbox_matrix_key, xpos, ypos);
}

static int numberbox_matrix_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
  t_numberbox_matrix* x = (t_numberbox_matrix *)z;
  
  if(doit)
  {
    t_int xpos=text_xpix(&x->x_gui.x_obj, glist);
    t_int ypos=text_ypix(&x->x_gui.x_obj, glist);
    //t_int r=x->x_n_row, c=x->x_n_column;
    t_int w=x->x_numwidth, h=x->x_gui.x_h;
    
    numberbox_matrix_click( x, (t_floatarg)xpix, (t_floatarg)ypix,
      (t_floatarg)shift, 0, (t_floatarg)alt);
    if(shift)
      x->x_gui.x_fsf.x_finemoved = 1;
    else
      x->x_gui.x_fsf.x_finemoved = 0;
    x->x_sel_column = (xpix - xpos) / w;
    x->x_sel_row = (ypix - ypos) / h;
    
    //post("row %d, col %d", x->x_sel_row, x->x_sel_column);
    if(!x->x_gui.x_fsf.x_change)
    {
      clock_delay(x->x_clock_wait, 50);
      x->x_gui.x_fsf.x_change = 1;
      clock_delay(x->x_clock_reset, 3000);
      
      x->x_buf[0] = 0;
    }
    else
    {
      x->x_gui.x_fsf.x_change = 0;
      clock_unset(x->x_clock_reset);
      x->x_buf[0] = 0;
      x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT;
      (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
    }
  }
  return (1);
}

static void numberbox_matrix_bang(t_numberbox_matrix *x)
{
  outlet_anything(x->x_gui.x_obj.ob_outlet, atom_getsymbol(x->x_matrix), x->x_n_row*x->x_n_column+2, x->x_matrix+1);
  if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
    typedmess(x->x_gui.x_snd->s_thing, atom_getsymbol(x->x_matrix), x->x_n_row*x->x_n_column+2, x->x_matrix+1);
}

static void numberbox_matrix_float(t_numberbox_matrix *x, t_floatarg f)
{
  t_int i, j, k, r, c;
  
  r = x->x_n_row;
  c = x->x_n_column;
  k = 3;
  for(i=0; i<r; i++)
  {
    for(j=0; j<c; j++)
    {
      SETFLOAT(x->x_matrix+k, f);
      k++;
    }
  }
  x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_MATRIX;
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

static void numberbox_matrix_list(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  t_int ir, jc;
  
  if((ac >= 3)&&IS_A_FLOAT(av,0)&&IS_A_FLOAT(av,1)&&IS_A_FLOAT(av,2))
  {
    ir = atom_getintarg(0, ac, av);
    jc = atom_getintarg(1, ac, av);
    if(ir < 1)
      ir = 1;
    if(ir > x->x_n_row)
      ir = x->x_n_row;
    if(jc < 1)
      jc = 1;
    if(jc > x->x_n_column)
      jc = x->x_n_column;
    ir--;
    jc--;
    x->x_sel_row = ir;
    x->x_sel_column = jc;
    ir *= x->x_n_column;
    ir += jc + 3;
    x->x_matrix[ir] = av[2];
    x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ELEMENT;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  }
}

static void numberbox_matrix_element(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  numberbox_matrix_list(x, s, ac, av);
}

static void numberbox_matrix_row(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  t_int i, ir, c=x->x_n_column;
  
  if(ac <= c){
    pd_error(x, "numberbox_matrix: bad row (too short)");
    return;
  }
  for(i=0; i<=c; i++)
  {
    if(!IS_A_FLOAT(av, i)){
      pd_error(x, "numberbox_matrix: bad row (not all numbers)");
      return;
    }
  }
  ir = atom_getintarg(0, ac, av);
  i=ir;
  if(ir < 1)
    ir = 1;
  if(ir > x->x_n_row)
    ir = x->x_n_row;
  if(i!=ir)post("numberbox_matrix: index %d out of range (using %d)", i, ir);
  
  ir--;
  x->x_sel_row = ir;
  ir *= c;
  ir += 3;
  for(i=1; i<=c; i++)
    x->x_matrix[ir++] = av[i];
  
  x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_ROW;
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

static void numberbox_matrix_col(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  t_int i, ic, r=x->x_n_row, c=x->x_n_column;
  
  if(ac <= r){
    pd_error(x, "numberbox_matrix: bad column (too short)");
    return;
  }
  for(i=0; i<=r; i++)
  {
    if(!IS_A_FLOAT(av, i)){
      pd_error(x, "numberbox_matrix: bad row (not all numbers)");
      return;
    }
  }
  
  ic = atom_getintarg(0, ac, av);
  i=ic;
  if(ic < 1)
    ic = 1;
  if(ic > c)
    ic = c;
  if(i!=ic)post("numberbox_matrix: index %d out of range (using %d)", i, ic);
  
  ic--;
  x->x_sel_column = ic;
  ic += 3;
  for(i=1; i<=r; i++)
  {
    x->x_matrix[ic] = av[i];
    ic += c;
  }
  
  x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_COLUMN;
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

static void numberbox_matrix_matrix(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  t_int i, k, c=0, r=0, C=0;
  if(ac<3){
    pd_error(x, "numberbox_matrix: invalid input matrix!");
    return;
  }
  r=atom_getintarg(0, ac, av);
  c=atom_getintarg(1, ac, av);
  C=c;
  
  if(ac != ((c*r)+2))
  {
    pd_error(x, "numberbox_matrix: invalid input matrix (incosistent #elements)!");
    return;
  }
  for(i=0; i<ac; i++)
  {
    if(!IS_A_FLOAT(av, i)){
      pd_error(x, "numberbox_matrix: invalid input matrix (not all floats)!");
      return;
    }
  }
  
  if(r != x->x_n_row)
    post("numberbox_matrix: #rows do not match %d!=%d", r, x->x_n_row);
  if(c != x->x_n_column)
    post("numberbox_matrix: #columns do not match %d!=%d", c, x->x_n_column);
  
  if(r>x->x_n_row)
    r=x->x_n_row;
  if(c>x->x_n_column)
    c=x->x_n_column;
  
  for(i=0; i<r; i++)
    for(k=0; k<c; k++){
      int out_index=i*(x->x_n_column)+k;
      int in_index =i*(C)+k;
      x->x_matrix[3+out_index]=av[2+in_index];
    }
    
    x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_MATRIX;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
}

static void numberbox_matrix_dim(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  int r=0, c=0;
  
  if(ac > 0)
  {
    r = (int)atom_getintarg(0, ac, av);
  }
  if(r < 1)
    r = 1;
  
  if(ac > 1)
  {
    c = (int)atom_getintarg(1, ac, av);
  }
  if(c < 1)
    c = 1;
  
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
  if(x->x_matrix)
  {
    if((r*c) != (x->x_n_row*x->x_n_column))
      x->x_matrix = (t_atom *)resizebytes(x->x_matrix, (x->x_n_row*x->x_n_column+3)*sizeof(t_atom), (r*c+3)*sizeof(t_atom));
  }
  x->x_n_row = r;
  x->x_n_column = c;
  SETFLOAT(x->x_matrix+1, x->x_n_row);
  SETFLOAT(x->x_matrix+2, x->x_n_column);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_NEW);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
}

static void numberbox_matrix_delta(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void numberbox_matrix_pos(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}

static void numberbox_matrix_size(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  int h, w;
  
  if((ac >= 2)&&IS_A_FLOAT(av, 0) && IS_A_FLOAT(av, 1))
  {
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
    numberbox_matrix_calc_fontwidth(x);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_NEW);
    canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
  }
}

static void numberbox_matrix_font(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  t_int f;
  
  if((ac >= 2)&&IS_A_FLOAT(av, 0) && IS_A_FLOAT(av, 1))
  {
    f = atom_getintarg(1, ac, av);
    if(f < 4)
      f = 4;
    x->x_gui.x_fontsize = f;
    f = (int)atom_getintarg(0, ac, av);
    if((f < 0) || (f > 2))
      f = 0;
    x->x_gui.x_fsf.x_font_style = f;
    numberbox_matrix_calc_fontwidth(x);
    f = (int)atom_getintarg(0, ac, av);
    if(f == 1)
      strcpy(x->x_gui.x_font, "helvetica");
    else if(f == 2)
      strcpy(x->x_gui.x_font, "times");
    else
    {
      f = 0;
      strcpy(x->x_gui.x_font, "courier");
    }
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_NEW);
  }
}

static void numberbox_matrix_range(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  t_int i, j, k, r, c;
  t_float f;
  
  r = x->x_n_row;
  c = x->x_n_column;
  if((ac >= 2)&&IS_A_FLOAT(av, 0) && IS_A_FLOAT(av, 1))
  {
    x->x_min = (double)atom_getfloatarg(0, ac, av);
    x->x_max = (double)atom_getfloatarg(1, ac, av);
    if(x->x_min > x->x_max)
    {
      x->x_max = (double)atom_getfloatarg(0, ac, av);
      x->x_min = (double)atom_getfloatarg(1, ac, av);
    }
    x->x_k = 1.0;
    k = 3;
    for(i=0; i<r; i++)
    {
      for(j=0; j<c; j++)
      {
        f = atom_getfloat(x->x_matrix+k);
        if(f > x->x_max)
          f = x->x_max;
        if(f < x->x_min)
          f = x->x_min;
        SETFLOAT(x->x_matrix+k, f);
        k++;
      }
    }
    x->x_update_mode = IEM_GUI_NBXM_DRAW_MODE_UPDATE_MATRIX;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_UPDATE);
  }
}

static void numberbox_matrix_color(t_numberbox_matrix *x, t_symbol *s, int ac, t_atom *av)
{
  t_canvas *canvas=glist_getcanvas(x->x_gui.x_glist);
  
  if((ac >= 1)&&IS_A_SYMBOL(av, 0))
    x->x_front_color = atom_getsymbol(av);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_NEW);
}

static void numberbox_matrix_send(t_numberbox_matrix *x, t_symbol *s)
{iemgui_send(x, &x->x_gui, s);}

static void numberbox_matrix_receive(t_numberbox_matrix *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}


#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void *numberbox_matrix_new(t_symbol *s, int argc, t_atom *argv)
{
  t_numberbox_matrix *x = (t_numberbox_matrix *)pd_new(numberbox_matrix_class);
  t_int i, j, n=1, n_row=5, n_column=3, w=3, h=12, fs=8;
  double min=0.0, max=127.0, mm;
  t_symbol *front_color;
  t_int k;
  
  x->x_gui.x_snd = gensym("empty");
  x->x_gui.x_rcv = gensym("empty");
  x->x_gui.x_lab = gensym("empty");
  x->x_gui.x_fsf.x_font_style = 0;
  front_color = gensym("#000000");// black 
  if((argc >= 12)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
    &&IS_A_FLOAT(argv,4)&&IS_A_FLOAT(argv,5)&&IS_A_FLOAT(argv,6)
    &&(IS_A_SYMBOL(argv,7)||IS_A_FLOAT(argv,7))
    &&(IS_A_SYMBOL(argv,8)||IS_A_FLOAT(argv,8))
    &&(IS_A_SYMBOL(argv,9)||IS_A_FLOAT(argv,9))
    &&IS_A_FLOAT(argv,10)&&IS_A_FLOAT(argv,11))
  {
    n_row = (int)atom_getintarg(0, argc, argv);
    n_column = (int)atom_getintarg(1, argc, argv);
    w = (int)atom_getintarg(2, argc, argv);
    h = (int)atom_getintarg(3, argc, argv);
    min = (double)atom_getfloatarg(4, argc, argv);
    max = (double)atom_getfloatarg(5, argc, argv);
    iem_inttosymargs(&x->x_gui.x_isa, atom_getintarg(6, argc, argv));
    iemgui_new_getnames(&x->x_gui, 7, argv);
    front_color = atom_getsymbolarg(9, argc, argv);
    iem_inttofstyle(&x->x_gui.x_fsf, atom_getintarg(10, argc, argv));
    fs = (int)atom_getintarg(11, argc, argv);
  }
  else if((argc >= 2)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1))
  {
    n_row = (int)atom_getintarg(0, argc, argv);
    n_column = (int)atom_getintarg(1, argc, argv);
  }
  if(min > max)
  {
    mm = min;
    min = max;
    max = mm;
  }
  x->x_min = min;
  x->x_max = max;
  x->x_gui.x_draw = (t_iemfunptr)numberbox_matrix_draw;
  x->x_gui.x_fsf.x_snd_able = 1;
  x->x_gui.x_fsf.x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  if(!strcmp(x->x_gui.x_snd->s_name, "empty"))
    x->x_gui.x_fsf.x_snd_able = 0;
  if(!strcmp(x->x_gui.x_rcv->s_name, "empty"))
    x->x_gui.x_fsf.x_rcv_able = 0;
  if(x->x_gui.x_fsf.x_font_style == 1) strcpy(x->x_gui.x_font, "helvetica");
  else if(x->x_gui.x_fsf.x_font_style == 2) strcpy(x->x_gui.x_font, "times");
  else { x->x_gui.x_fsf.x_font_style = 0; strcpy(x->x_gui.x_font, "courier"); }
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  if(fs < 4)
    fs = 4;
  x->x_gui.x_fontsize = fs;
  if(w < 1)
    w = 1;
  x->x_gui.x_w = w;
  if(h < 8)
    h = 8;
  x->x_gui.x_h = h;
  if(n_row < 1)
    n_row = 1;
  x->x_n_row = n_row;
  if(n_column < 1)
    n_column = 1;
  x->x_n_column = n_column;
  x->x_front_color = front_color;
  x->x_buf[0] = 0;
  numberbox_matrix_calc_fontwidth(x);
  x->x_matrix = (t_atom *)getbytes((x->x_n_row*x->x_n_column+3)*sizeof(t_atom));
  SETSYMBOL(x->x_matrix, gensym("matrix"));
  SETFLOAT(x->x_matrix+1, x->x_n_row);
  SETFLOAT(x->x_matrix+2, x->x_n_column);
  k = 3;
  for(i=0; i<n_row; i++)
  {
    for(j=0; j<n_column; j++)
    {
      SETFLOAT(x->x_matrix+k, 0.0f);
      numberbox_matrix_clip_element(x, x->x_matrix+k);
      k++;
    }
  }
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  x->x_clock_reset = clock_new(x, (t_method)numberbox_matrix_tick_reset);
  x->x_clock_wait = clock_new(x, (t_method)numberbox_matrix_tick_wait);
  x->x_gui.x_fsf.x_change = 0;
  outlet_new(&x->x_gui.x_obj, &s_list);
  x->x_resid = 0;
  return (x);
}
#else
static void *numberbox_matrix_new(t_symbol *s, int argc, t_atom *argv)
{
  t_numberbox_matrix *x = (t_numberbox_matrix *)pd_new(numberbox_matrix_class);
  t_int i, j, n=1, n_row=5, n_column=3, w=3, h=12, fs=8;
  double min=0.0, max=127.0, mm;
  t_symbol *front_color;
  t_int iinit=0, ifstyle=0;
  t_iem_init_symargs *init=(t_iem_init_symargs *)(&iinit);
  t_iem_fstyle_flags *fstyle=(t_iem_fstyle_flags *)(&ifstyle);
  t_symbol *srl[3];
  t_int k;
  char str[144];
  
  x->x_gui.x_snd = gensym("empty");
  x->x_gui.x_rcv = gensym("empty");
  x->x_gui.x_lab = gensym("empty");
  srl[0] = gensym("empty");
  srl[1] = gensym("empty");
  srl[2] = gensym("empty");
  x->x_gui.x_fsf.x_font_style = 0;
  front_color = gensym("#000000");// black 
  if((argc >= 12)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1)
    &&IS_A_FLOAT(argv,2)&&IS_A_FLOAT(argv,3)
    &&IS_A_FLOAT(argv,4)&&IS_A_FLOAT(argv,5)&&IS_A_FLOAT(argv,6)
    &&(IS_A_SYMBOL(argv,7)||IS_A_FLOAT(argv,7))
    &&(IS_A_SYMBOL(argv,8)||IS_A_FLOAT(argv,8))
    &&(IS_A_SYMBOL(argv,9)||IS_A_FLOAT(argv,9))
    &&IS_A_FLOAT(argv,10)&&IS_A_FLOAT(argv,11))
  {
    n_row = (int)atom_getintarg(0, argc, argv);
    n_column = (int)atom_getintarg(1, argc, argv);
    w = (int)atom_getintarg(2, argc, argv);
    h = (int)atom_getintarg(3, argc, argv);
    min = (double)atom_getfloatarg(4, argc, argv);
    max = (double)atom_getfloatarg(5, argc, argv);
    iinit = (int)atom_getintarg(6, argc, argv);
    if(IS_A_SYMBOL(argv,7))
      srl[0] = atom_getsymbolarg(7, argc, argv);
    else if(IS_A_FLOAT(argv,7))
    {
      sprintf(str, "%d", (int)atom_getintarg(7, argc, argv));
      srl[0] = gensym(str);
    }
    if(IS_A_SYMBOL(argv,8))
      srl[1] = atom_getsymbolarg(8, argc, argv);
    else if(IS_A_FLOAT(argv,8))
    {
      sprintf(str, "%d", (int)atom_getintarg(8, argc, argv));
      srl[1] = gensym(str);
    }
    front_color = atom_getsymbolarg(9, argc, argv);
    ifstyle = (int)atom_getintarg(10, argc, argv);
    fs = (int)atom_getintarg(11, argc, argv);
  }
  else if((argc >= 2)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1))
  {
    n_row = (int)atom_getintarg(0, argc, argv);
    n_column = (int)atom_getintarg(1, argc, argv);
  }
  if(min > max)
  {
    mm = min;
    min = max;
    max = mm;
  }
  x->x_min = min;
  x->x_max = max;
  iinit &= IEM_INIT_ARGS_ALL;
  ifstyle &= IEM_FSTYLE_FLAGS_ALL;
  
  x->x_gui.x_draw = (t_iemfunptr)numberbox_matrix_draw;
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
  if(x->x_gui.x_fsf.x_font_style == 1) strcpy(x->x_gui.x_font, "helvetica");
  else if(x->x_gui.x_fsf.x_font_style == 2) strcpy(x->x_gui.x_font, "times");
  else { x->x_gui.x_fsf.x_font_style = 0; strcpy(x->x_gui.x_font, "courier"); }
  x->x_gui.x_fsf = *fstyle;
  x->x_gui.x_isa = *init;
  iemgui_first_dollararg2sym(&x->x_gui, srl);
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  
  if(fs < 4)
    fs = 4;
  x->x_gui.x_fontsize = fs;
  if(w < 1)
    w = 1;
  x->x_gui.x_w = w;
  if(h < 8)
    h = 8;
  x->x_gui.x_h = h;
  if(n_row < 1)
    n_row = 1;
  x->x_n_row = n_row;
  if(n_column < 1)
    n_column = 1;
  x->x_n_column = n_column;
  x->x_front_color = front_color;
  x->x_buf[0] = 0;
  numberbox_matrix_calc_fontwidth(x);
  x->x_matrix = (t_atom *)getbytes((x->x_n_row*x->x_n_column+3)*sizeof(t_atom));
  SETSYMBOL(x->x_matrix, gensym("matrix"));
  SETFLOAT(x->x_matrix+1, x->x_n_row);
  SETFLOAT(x->x_matrix+2, x->x_n_column);
  k = 3;
  for(i=0; i<n_row; i++)
  {
    for(j=0; j<n_column; j++)
    {
      SETFLOAT(x->x_matrix+k, 0.0f);
      numberbox_matrix_clip_element(x, x->x_matrix+k);
      k++;
    }
  }
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  x->x_clock_reset = clock_new(x, (t_method)numberbox_matrix_tick_reset);
  x->x_clock_wait = clock_new(x, (t_method)numberbox_matrix_tick_wait);
  x->x_gui.x_fsf.x_change = 0;
  outlet_new(&x->x_gui.x_obj, &s_list);
  x->x_resid = 0;
  return (x);
}
#endif

static void numberbox_matrix_free(t_numberbox_matrix *x)
{
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_unbind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  clock_free(x->x_clock_reset);
  clock_free(x->x_clock_wait);
  if(x->x_matrix)
    freebytes(x->x_matrix, (x->x_n_row*x->x_n_column+3)*sizeof(t_atom));
  gfxstub_deleteforkey(x);
}

void numberbox_matrix_setup(void)
{
  numberbox_matrix_class = class_new(gensym("nbxm"), (t_newmethod)numberbox_matrix_new,
    (t_method)numberbox_matrix_free, sizeof(t_numberbox_matrix), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)numberbox_matrix_new, gensym("numberbox_matrix"),
    A_GIMME, 0);
  class_addbang(numberbox_matrix_class,numberbox_matrix_bang);
  class_addfloat(numberbox_matrix_class,numberbox_matrix_float);
  class_addlist(numberbox_matrix_class, numberbox_matrix_list);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_matrix,
    gensym("matrix"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_col,
    gensym("col"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_row,
    gensym("row"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_element,
    gensym("element"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_click,
    gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_motion,
    gensym("motion"), A_FLOAT, A_FLOAT, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_size,
    gensym("size"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_dim,
    gensym("dim"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_delta,
    gensym("delta"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_pos,
    gensym("pos"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_range,
    gensym("range"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_color,
    gensym("color"), A_GIMME, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_send,
    gensym("send"), A_DEFSYM, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_receive,
    gensym("receive"), A_DEFSYM, 0);
  class_addmethod(numberbox_matrix_class, (t_method)numberbox_matrix_font,
    gensym("font"), A_GIMME, 0);
  
  numberbox_matrix_widgetbehavior.w_getrectfn =    numberbox_matrix_getrect;
  numberbox_matrix_widgetbehavior.w_displacefn =   iemgui_displace;
  numberbox_matrix_widgetbehavior.w_selectfn =     iemgui_select;
  numberbox_matrix_widgetbehavior.w_activatefn =   NULL;
  numberbox_matrix_widgetbehavior.w_deletefn =     iemgui_delete;
  numberbox_matrix_widgetbehavior.w_visfn =        iemgui_vis;
  numberbox_matrix_widgetbehavior.w_clickfn =      numberbox_matrix_newclick;
  
#if((PD_MAJOR_VERSION)&&(PD_MINOR_VERSION < 37))
  numberbox_matrix_widgetbehavior.w_propertiesfn = NULL;
  numberbox_matrix_widgetbehavior.w_savefn =       numberbox_matrix_save;
#else
  class_setsavefn(numberbox_matrix_class, numberbox_matrix_save);
#endif
  
  class_setwidget(numberbox_matrix_class, &numberbox_matrix_widgetbehavior);
//  class_sethelpsymbol(numberbox_matrix_class, gensym("iemhelp2/help-numberbox_matrix"));
}
