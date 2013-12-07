/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemgui written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include "g_canvas.h"
#include "g_all_guis.h"
#include <stdio.h>
#include <string.h>

#ifdef MSW
#include <io.h>
#else
#include <unistd.h>
#endif

#if     _MSC_VER >= 1300  /* since VSC 7.0 */
#define close _close
#endif

/* ------------------------ setup routine ------------------------- */

t_widgetbehavior iem_image_widgetbehavior;
static t_class *iem_image_class;

typedef struct _iem_image
{
  t_iemgui  x_gui;
  t_symbol  *x_gifsym;
  t_atom    x_at_out[2];
  int       x_have_image;
} t_iem_image;

static t_symbol *iem_image_calc_size(t_iem_image *x)
{
  char dirbuf[MAXPDSTRING], *namebufptr;
  char namebuf[MAXPDSTRING];
  unsigned char buf[222];
  unsigned int i;
  char *c;
  int fd;
  FILE *fh;
  size_t items;
  
  if(!x->x_gifsym || !x->x_gifsym->s_name)
  {
    post("iem_image-ERROR: no gifname");
    x->x_gifsym = (t_symbol *)0;
    return((t_symbol *)0);
  }
  fd = open_via_path(canvas_getdir(glist_getcanvas(x->x_gui.x_glist))->s_name, x->x_gifsym->s_name,
    "", dirbuf, &namebufptr, MAXPDSTRING, 1);
  if (fd < 0)
  {
    post("iem_image-ERROR: cannot open %s first time", x->x_gifsym->s_name);
    x->x_gifsym = (t_symbol *)0;
    return((t_symbol *)0);
  }
  else
  {
    if(fd >= 0)
      close(fd);
    strcpy(namebuf, dirbuf);
    strcat(namebuf, "/");
    strcat(namebuf, namebufptr);
    fh = sys_fopen(namebuf, "r");
    if(fh == NULL)
    {
      post("iem_image-ERROR: cannot open %s second time", namebuf);
      x->x_gifsym = (t_symbol *)0;
      return((t_symbol *)0);
    }
    else
    {
        items=fread(buf, 22, sizeof(unsigned char), fh);
  	if((items < 1)||(strlen((char*)buf)<7)) {
	post("iem_image-ERROR: can not read header in %s, only %d items read: %s.", namebuf, strlen((char*)buf), (char*) buf);
        x->x_gifsym = (t_symbol *)0;
        return((t_symbol *)0);
        };
      fclose(fh);
      c = (char *)buf;
      if((c[0] != 'G')||(c[1] != 'I')||(c[2] != 'F'))
      {
        post("iem_image-ERROR: %s is not a GIF-file", namebuf);
        x->x_gifsym = (t_symbol *)0;
        return((t_symbol *)0);
      }
      i = 256*(unsigned int)buf[7];
      i += (unsigned int)buf[6];
      x->x_gui.x_w = (int)i;
      i = 256*(unsigned int)buf[9];
      i += (unsigned int)buf[8];
      x->x_gui.x_h = (int)i;
      SETFLOAT(x->x_at_out, (t_float)x->x_gui.x_w);
      SETFLOAT(x->x_at_out+1, (t_float)x->x_gui.x_h);
      outlet_list(x->x_gui.x_obj.ob_outlet, &s_list, 2, x->x_at_out);
      if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_list(x->x_gui.x_snd->s_thing, &s_list, 2, x->x_at_out);
      return(gensym(namebuf));
    }
  }
}

static void iem_image_draw_new(t_iem_image *x, t_glist *glist)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  t_symbol *correct_name;

  if(correct_name = iem_image_calc_size(x))
  {
    sys_vgui("image create photo %lxPHOTOIMAGE -file {%s} -format gif -width %d -height %d\n",
      x, correct_name->s_name, x->x_gui.x_w, x->x_gui.x_h);
    sys_vgui(".x%lx.c create image %d %d -image %lxPHOTOIMAGE -tags %lxPHOTO\n",
      canvas, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2, x, x);

    x->x_have_image=1;
  } 
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline #%6.6x -tags %lxBASE\n",
    canvas, xpos, ypos, xpos + x->x_gui.x_w, ypos + x->x_gui.x_h, IEM_GUI_COLOR_SELECTED, x);
}

static void iem_image_draw_move(t_iem_image *x, t_glist *glist)
{
  int xpos=text_xpix(&x->x_gui.x_obj, glist);
  int ypos=text_ypix(&x->x_gui.x_obj, glist);
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_have_image)
    sys_vgui(".x%lx.c coords %lxPHOTO %d %d\n", canvas, x, xpos+x->x_gui.x_w/2, ypos+x->x_gui.x_h/2);
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d\n",
    canvas, x, xpos, ypos, xpos + x->x_gui.x_w, ypos + x->x_gui.x_h);
  canvas_fixlinesfor(glist_getcanvas(x->x_gui.x_glist), (t_text*)x);
}

static void iem_image_draw_erase(t_iem_image* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
  if(x->x_have_image)
  {
    sys_vgui("image delete %lxPHOTOIMAGE\n", x);
    sys_vgui(".x%lx.c delete %lxPHOTO\n", canvas, x);    
    x->x_have_image=0;
  }
}

static void iem_image_draw_select(t_iem_image* x, t_glist* glist)
{
  t_canvas *canvas=glist_getcanvas(glist);
  
  if(x->x_gui.x_fsf.x_selected)
  {
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline #%6.6x -tags %lxBASE\n",
      canvas, xpos, ypos, xpos + x->x_gui.x_w,
      ypos + x->x_gui.x_h, IEM_GUI_COLOR_SELECTED, x);
  }
  else
    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
}

static void iem_image_draw(t_iem_image *x, t_glist *glist, int mode)
{
  if(mode == IEM_GUI_DRAW_MODE_MOVE)
    iem_image_draw_move(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_NEW)
    iem_image_draw_new(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_SELECT)
    iem_image_draw_select(x, glist);
  else if(mode == IEM_GUI_DRAW_MODE_ERASE)
    iem_image_draw_erase(x, glist);
}

/* ------------------------ cnv widgetbehaviour----------------------------- */

static void iem_image_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
  t_iem_image *x = (t_iem_image *)z;
  
  *xp1 = text_xpix(&x->x_gui.x_obj, glist);
  *yp1 = text_ypix(&x->x_gui.x_obj, glist);
  *xp2 = *xp1 + x->x_gui.x_w;
  *yp2 = *yp1 + x->x_gui.x_h;
}

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void iem_image_save(t_gobj *z, t_binbuf *b)
{
  t_iem_image *x = (t_iem_image *)z;
  t_symbol *srl[3];
  
  srl[0] = x->x_gui.x_snd;
  srl[1] = x->x_gui.x_rcv;
  srl[2] = gensym("empty");
  if(x->x_gifsym)
    binbuf_addv(b, "ssiissiiss", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    x->x_gifsym, iem_symargstoint(&x->x_gui.x_isa), 
    iem_fstyletoint(&x->x_gui.x_fsf), srl[0], srl[1]);
  else
    binbuf_addv(b, "ssiisiiiss", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    0, iem_symargstoint(&x->x_gui.x_isa), 
    iem_fstyletoint(&x->x_gui.x_fsf), srl[0], srl[1]);
  binbuf_addv(b, ";");
}
#else
static void iem_image_save(t_gobj *z, t_binbuf *b)
{
  t_iem_image *x = (t_iem_image *)z;
  int *ip1, *ip2;
  t_symbol *srl[3];
  
  srl[0] = x->x_gui.x_snd;
  srl[1] = x->x_gui.x_rcv;
  srl[2] = gensym("empty");
  iemgui_all_unique2dollarzero(&x->x_gui, srl);
  iemgui_all_sym2dollararg(&x->x_gui, srl);
  ip1 = (int *)(&x->x_gui.x_isa);
  ip2 = (int *)(&x->x_gui.x_fsf);
  if(x->x_gifsym)
    binbuf_addv(b, "ssiissiiss", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    x->x_gifsym, (*ip1)&IEM_INIT_ARGS_ALL,
    (*ip2)&IEM_FSTYLE_FLAGS_ALL, srl[0], srl[1]);
  else
    binbuf_addv(b, "ssiisiiiss", gensym("#X"),gensym("obj"),
    (t_int)x->x_gui.x_obj.te_xpix, (t_int)x->x_gui.x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_gui.x_obj.te_binbuf)),
    0, (*ip1)&IEM_INIT_ARGS_ALL,
    (*ip2)&IEM_FSTYLE_FLAGS_ALL, srl[0], srl[1]);
  binbuf_addv(b, ";");
}
#endif

static void iem_image_clear(t_iem_image *x)
{
  (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
  x->x_gifsym = (t_symbol *)0;
}

static void iem_image_open(t_iem_image *x, t_symbol *name)
{
  if(name && name->s_name)
  {
    if(x->x_gifsym)
    {
      (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_ERASE);
      x->x_gifsym = (t_symbol *)0;
    }
    x->x_gifsym = name;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_GUI_DRAW_MODE_NEW);
  }
}

static void iem_image_delta(t_iem_image *x, t_symbol *s, int ac, t_atom *av)
{iemgui_delta((void *)x, &x->x_gui, s, ac, av);}

static void iem_image_pos(t_iem_image *x, t_symbol *s, int ac, t_atom *av)
{iemgui_pos((void *)x, &x->x_gui, s, ac, av);}

static void iem_image_send(t_iem_image *x, t_symbol *s)
{iemgui_send(x, &x->x_gui, s);}

static void iem_image_receive(t_iem_image *x, t_symbol *s)
{iemgui_receive(x, &x->x_gui, s);}

#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
static void *iem_image_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iem_image *x = (t_iem_image *)pd_new(iem_image_class);
  t_symbol *gifsym=(t_symbol *)0;
  x->x_gui.x_snd = gensym("empty");
  x->x_gui.x_rcv = gensym("empty");
  x->x_gui.x_lab = gensym("empty");
  x->x_gui.x_fsf.x_font_style = 0;
  if(argc >= 1)
  {
    if(IS_A_SYMBOL(argv,0))
      gifsym = atom_getsymbolarg(0, argc, argv);
    else if(IS_A_FLOAT(argv,0))
      gifsym = (t_symbol *)0;
  }
  else if(argc >= 5)
  {
    if(IS_A_SYMBOL(argv,0))
      gifsym = atom_getsymbolarg(0, argc, argv);
    else if(IS_A_FLOAT(argv,0))
      gifsym = (t_symbol *)0;
    iem_inttosymargs(&x->x_gui.x_isa, atom_getintarg(1, argc, argv));
    iem_inttofstyle(&x->x_gui.x_fsf, atom_getintarg(2, argc, argv));
    iemgui_new_getnames(&x->x_gui, 3, argv);
  }

  x->x_gui.x_draw = (t_iemfunptr)iem_image_draw;
  x->x_gui.x_fsf.x_snd_able = 1;
  x->x_gui.x_fsf.x_rcv_able = 1;
  x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
  if(!strcmp(x->x_gui.x_snd->s_name, "empty"))
    x->x_gui.x_fsf.x_snd_able = 0;
  if(!strcmp(x->x_gui.x_rcv->s_name, "empty"))
    x->x_gui.x_fsf.x_rcv_able = 0;
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  x->x_gui.x_w = 100;
  x->x_gui.x_h = 60;
  x->x_gifsym = gifsym;
  x->x_have_image=0;
  x->x_gui.x_fsf.x_selected = 0;
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  outlet_new(&x->x_gui.x_obj, &s_list);
  return(x);
}
#else
static void *iem_image_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iem_image *x = (t_iem_image *)pd_new(iem_image_class);
  t_symbol *gifsym=(t_symbol *)0;
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
  if(argc >= 1)
  {
    if(IS_A_SYMBOL(argv,0))
      gifsym = atom_getsymbolarg(0, argc, argv);
    else if(IS_A_FLOAT(argv,0))
      gifsym = (t_symbol *)0;
  }
  else if(argc >= 5)
  {
    if(IS_A_SYMBOL(argv,0))
      gifsym = atom_getsymbolarg(0, argc, argv);
    else if(IS_A_FLOAT(argv,0))
      gifsym = (t_symbol *)0;
    iinit = (int)atom_getintarg(1, argc, argv);
    ifstyle = (int)atom_getintarg(2, argc, argv);
    if(IS_A_SYMBOL(argv, 3))
      srl[0] = atom_getsymbolarg(3, argc, argv);
    else if(IS_A_FLOAT(argv,3))
    {
      sprintf(str, "%d", (int)atom_getintarg(3, argc, argv));
      srl[0] = gensym(str);
    }
    if(IS_A_SYMBOL(argv,4))
      srl[1] = atom_getsymbolarg(4, argc, argv);
    else if(IS_A_FLOAT(argv,4))
    {
      sprintf(str, "%d", (int)atom_getintarg(4, argc, argv));
      srl[1] = gensym(str);
    }
  }
  
  iinit &= IEM_INIT_ARGS_ALL;
  ifstyle &= IEM_FSTYLE_FLAGS_ALL;
  x->x_gui.x_draw = (t_iemfunptr)iem_image_draw;
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
  iemgui_first_dollararg2sym(&x->x_gui, sr);
  if(x->x_gui.x_fsf.x_rcv_able)
    pd_bind(&x->x_gui.x_obj.ob_pd, x->x_gui.x_rcv);
  x->x_gui.x_w = 100;
  x->x_gui.x_h = 60;
  x->x_gifsym = gifsym;
  x->x_gui.x_fsf.x_selected = 0;
  iemgui_verify_snd_ne_rcv(&x->x_gui);
  outlet_new(&x->x_gui.x_obj, &s_list);
  return(x);
}
#endif

static void iem_image_ff(t_iem_image *x)
{
  gfxstub_deleteforkey(x);
}

void iem_image_setup(void)
{
  iem_image_class = class_new(gensym("iem_image"), (t_newmethod)iem_image_new,
    (t_method)iem_image_ff, sizeof(t_iem_image), 0, A_GIMME, 0);  
  class_addmethod(iem_image_class, (t_method)iem_image_open, gensym("open"), A_SYMBOL, 0);
  class_addmethod(iem_image_class, (t_method)iem_image_clear, gensym("clear"), 0);
  class_addmethod(iem_image_class, (t_method)iem_image_delta,
    gensym("delta"), A_GIMME, 0);
  class_addmethod(iem_image_class, (t_method)iem_image_pos,
    gensym("pos"), A_GIMME, 0);
  class_addmethod(iem_image_class, (t_method)iem_image_send,
    gensym("send"), A_DEFSYM, 0);
  class_addmethod(iem_image_class, (t_method)iem_image_receive,
    gensym("receive"), A_DEFSYM, 0);
  
  iem_image_widgetbehavior.w_getrectfn = iem_image_getrect;
  iem_image_widgetbehavior.w_displacefn = iemgui_displace;
  iem_image_widgetbehavior.w_selectfn = iemgui_select;
  iem_image_widgetbehavior.w_activatefn = NULL;
  iem_image_widgetbehavior.w_deletefn = iemgui_delete;
  iem_image_widgetbehavior.w_visfn = iemgui_vis;
  iem_image_widgetbehavior.w_clickfn = NULL;
  
#if defined(PD_MAJOR_VERSION) && (PD_MINOR_VERSION >= 37)
  class_setsavefn(iem_image_class, iem_image_save);
#else
  iem_image_widgetbehavior.w_propertiesfn = NULL;
  iem_image_widgetbehavior.w_savefn = iem_image_save;
#endif
  
  class_setwidget(iem_image_class, &iem_image_widgetbehavior);
//  class_sethelpsymbol(iem_image_class, gensym("iemhelp2/help-iem_image"));
  
  //  post("iem_image library loaded!");
}
