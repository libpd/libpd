/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include "g_canvas.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define PD_VERSION_MINOR 32
#define BACKGROUND "-fill grey"
#define BACKGROUNDCOLOR "grey"


#define te_xpos te_xpix
#define te_ypos te_ypix

/* ------------------------ toddle ----------------------------- */



#define DEFAULTSIZE 15
#define DEFAULTCOLOR "black"

static t_class *toddle_class;

typedef struct _toddle
{
     t_object x_obj;
     t_glist * x_glist;
     int x_width;
     int x_height;
     t_symbol* x_color;
     t_clock* x_clock;
} t_toddle;

/* widget helper functions */


void toddle_drawbang(t_toddle *x,t_glist *glist,int on)
{
     if (glist_isvisible(glist)) {
	  if (on)
	       sys_vgui(".x%lx.c create oval %d %d %d %d -fill %s -tags %lxB\n",glist_getcanvas(glist),
			x->x_obj.te_xpos+1,x->x_obj.te_ypos+1,
			x->x_obj.te_xpos + x->x_width -1,
			x->x_obj.te_ypos + x->x_height -1,x->x_color->s_name,x);
	  else
	       sys_vgui(".x%lx.c delete %lxB\n",
			glist_getcanvas(glist), x);
     }
}




void toddle_drawme(t_toddle *x, t_glist *glist, int firsttime)
{
     if (firsttime) {
#if 0
	  sys_vgui(".x%lx.c create line \
%d %d %d %d %d %d %d %d %d %d -tags %lxS\n",
		   glist_getcanvas(glist),
		   x->x_obj.te_xpos, x->x_obj.te_ypos,
		   x->x_obj.te_xpos + x->x_width, x->x_obj.te_ypos,
		   x->x_obj.te_xpos + x->x_width, x->x_obj.te_ypos + x->x_height,
		   x->x_obj.te_xpos, x->x_obj.te_ypos + x->x_height,
		   x->x_obj.te_xpos, x->x_obj.te_ypos,
		   x);
#endif

	  sys_vgui(".x%lx.c create rectangle \
%d %d %d %d -tags %lxS "BACKGROUND"\n",
		   glist_getcanvas(glist),
		   x->x_obj.te_xpos, x->x_obj.te_ypos,
		   x->x_obj.te_xpos + x->x_width, x->x_obj.te_ypos + x->x_height,
		   x);
	  sys_vgui(".x%lx.c create oval \
%d %d %d %d -tags %lxP\n",glist_getcanvas(glist),
		   x->x_obj.te_xpos+1,x->x_obj.te_ypos+1,
		   x->x_obj.te_xpos + x->x_width -1,
		   x->x_obj.te_ypos + x->x_height -1,x);
	  
     }     
     else {
#if 0
	  sys_vgui(".x%lx.c coords %lxS \
%d %d %d %d %d %d %d %d %d %d\n",
		   glist_getcanvas(glist), x,
		   x->x_obj.te_xpos, x->x_obj.te_ypos,
		   x->x_obj.te_xpos + x->x_width, x->x_obj.te_ypos,
		   x->x_obj.te_xpos + x->x_width, x->x_obj.te_ypos + x->x_height,
		   x->x_obj.te_xpos, x->x_obj.te_ypos + x->x_height,
		   x->x_obj.te_xpos, x->x_obj.te_ypos);
#endif
	  sys_vgui(".x%lx.c coords %lxS \
%d %d %d %d\n",
		   glist_getcanvas(glist), x,
		   x->x_obj.te_xpos, x->x_obj.te_ypos,
		   x->x_obj.te_xpos + x->x_width, x->x_obj.te_ypos + x->x_height);


	  sys_vgui(".x%lx.c coords %lxP \
%d %d %d %d\n",glist_getcanvas(glist),x,
		   x->x_obj.te_xpos+1,x->x_obj.te_ypos+1,
		   x->x_obj.te_xpos + x->x_width-1,
		   x->x_obj.te_ypos + x->x_height-1);
     }


     {
	  int n = 1;
	  int nplus, i;
	  nplus = (n == 1 ? 1 : n-1);
	  for (i = 0; i < n; i++)
	  {
	       int onset = x->x_obj.te_xpos + (x->x_width - IOWIDTH) * i / nplus;
	       if (firsttime)
		    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxo%d\n",
			     glist_getcanvas(glist),
			     onset, x->x_obj.te_ypos + x->x_height - 1,
			     onset + IOWIDTH, x->x_obj.te_ypos + x->x_height,
			     x, i);
	       else
		    sys_vgui(".x%lx.c coords %lxo%d %d %d %d %d\n",
			     glist_getcanvas(glist), x, i,
			     onset, x->x_obj.te_ypos + x->x_height - 1,
			     onset + IOWIDTH, x->x_obj.te_ypos + x->x_height);
	  }
	  n = 1; 
	  nplus = (n == 1 ? 1 : n-1);
	  for (i = 0; i < n; i++)
	  {
	       int onset = x->x_obj.te_xpos + (x->x_width - IOWIDTH) * i / nplus;
	       if (firsttime)
		    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxi%d\n",
			     glist_getcanvas(glist),
			     onset, x->x_obj.te_ypos,
			     onset + IOWIDTH, x->x_obj.te_ypos + 1,
			     x, i);
	       else
		    sys_vgui(".x%lx.c coords %lxi%d %d %d %d %d\n",
			     glist_getcanvas(glist), x, i,
			     onset, x->x_obj.te_ypos,
			     onset + IOWIDTH, x->x_obj.te_ypos + 1);
	       
	  }
     }

}




void toddle_erase(t_toddle* x,t_glist* glist)
{
     int n;
     sys_vgui(".x%lx.c delete %lxS\n",
	      glist_getcanvas(glist), x);

     sys_vgui(".x%lx.c delete %lxP\n",
	      glist_getcanvas(glist), x);

     n = 1;

     while (n--) {
	  sys_vgui(".x%lx.c delete %lxi%d\n",glist_getcanvas(glist),x,n);
	  sys_vgui(".x%lx.c delete %lxo%d\n",glist_getcanvas(glist),x,n);
     }
}
	


/* ------------------------ toddle widgetbehaviour----------------------------- */


static void toddle_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_toddle* s = (t_toddle*)z;


    width = s->x_width;
    height = s->x_height;
    *xp1 = s->x_obj.te_xpos;
    *yp1 = s->x_obj.te_ypos;
    *xp2 = s->x_obj.te_xpos + width;
    *yp2 = s->x_obj.te_ypos + height;
}

static void toddle_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_toddle *x = (t_toddle *)z;
    x->x_obj.te_xpos += dx;
    x->x_obj.te_ypos += dy;
    toddle_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void toddle_select(t_gobj *z, t_glist *glist, int state)
{
     t_toddle *x = (t_toddle *)z;
     sys_vgui(".x%lx.c itemconfigure %lxS -fill %s\n", glist, 
	     x, (state? "blue" : BACKGROUNDCOLOR));
}


static void toddle_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void toddle_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}

       
static void toddle_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_toddle* x = (t_toddle*)z;
    if (vis)
	 toddle_drawme(x, glist, 1);
    else
	 toddle_erase(x,glist);
}

/* can we use the normal text save function ?? */

static void toddle_save(t_gobj *z, t_binbuf *b)
{
    t_toddle *x = (t_toddle *)z;
    binbuf_addv(b, "ssiissii", gensym("#X"), gensym("obj"),
                (t_int)x->x_obj.te_xpos, (t_int)x->x_obj.te_ypos,  
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_color,x->x_width,x->x_height);
    binbuf_addv(b, ";");
}


t_widgetbehavior   toddle_widgetbehavior;


void toddle_bang(t_toddle *x)
{
     toddle_drawbang(x,x->x_glist,1);
     outlet_bang(x->x_obj.ob_outlet);
     clock_delay(x->x_clock, 100);
}


static void toddle_click(t_toddle *x,
    t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl,
    t_floatarg alt)
{
     toddle_bang(x);     
}

#if (PD_VERSION_MINOR > 31) 
static int toddle_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    	if (doit)
	    toddle_click((t_toddle *)z, (t_floatarg)xpix, (t_floatarg)ypix,
	    	(t_floatarg)shift, 0, (t_floatarg)alt);
	return (1);
}
#endif

void toddle_float(t_toddle *x,t_floatarg f)
{

     toddle_drawbang(x,x->x_glist,1);
     
     clock_delay(x->x_clock, 100);
     outlet_float(x->x_obj.ob_outlet,f);
}

void toddle_on(t_toddle* x)
{
     toddle_drawbang(x,x->x_glist,1);
}

void toddle_off(t_toddle* x)
{
     toddle_drawbang(x,x->x_glist,0);
}

void toddle_tick(t_toddle* x)
{
     toddle_drawbang(x,x->x_glist,0);
    clock_unset(x->x_clock);
}


void toddle_size(t_toddle* x,t_floatarg w,t_floatarg h) {
     x->x_width = w;
     x->x_height = h;
     toddle_drawme(x, x->x_glist, 0);
}

void toddle_color(t_toddle* x,t_symbol* col)
{
     x->x_color = col;
     toddle_drawbang(x,x->x_glist,1);
     clock_delay(x->x_clock, 100);
/*     outlet_bang(x->x_obj.ob_outlet); only bang if there was a bang .. 
       so color black does the same as bang, but doesn't forward the bang 
*/
}

static void toddle_setwidget(void)
{
    toddle_widgetbehavior.w_getrectfn =     toddle_getrect;
    toddle_widgetbehavior.w_displacefn =    toddle_displace;
    toddle_widgetbehavior.w_selectfn =   toddle_select;
    toddle_widgetbehavior.w_activatefn =   toddle_activate;
    toddle_widgetbehavior.w_deletefn =   toddle_delete;
    toddle_widgetbehavior.w_visfn =   toddle_vis;
    toddle_widgetbehavior.w_clickfn = toddle_newclick;
#if (PD_MINOR_VERSION < 37) 
    toddle_widgetbehavior.w_propertiesfn = NULL; 
    toddle_widgetbehavior.w_savefn =   toddle_save;
#endif

#if PD_MINOR_VERSION >= 37
    class_setsavefn(toddle_class,&toddle_save);
#endif

}


static void *toddle_new(t_symbol* col,t_floatarg h,t_floatarg o)
{
    t_toddle *x = (t_toddle *)pd_new(toddle_class);

    x->x_glist = (t_glist*) canvas_getcurrent();
    if (h) x->x_width = h;
    else
	 x->x_width = DEFAULTSIZE;

    if (o) x->x_height = o;
    else
	 x->x_height = DEFAULTSIZE;

    if (col != &s_)
	 x->x_color = col;
    else
	 x->x_color = gensym(DEFAULTCOLOR);
    x->x_clock = clock_new(x, (t_method)toddle_tick);

    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void toddle_setup(void)
{
    toddle_class = class_new(gensym("toddle"), (t_newmethod)toddle_new, 0,
				sizeof(t_toddle),0, A_DEFSYM,A_DEFFLOAT,A_DEFFLOAT,0);

    class_addcreator((t_newmethod)toddle_new,gensym("bng"),A_DEFSYM,A_DEFFLOAT,A_DEFFLOAT,A_GIMME,0);
    class_addbang(toddle_class,toddle_bang);
    class_addfloat(toddle_class,toddle_float);

    class_addmethod(toddle_class, (t_method)toddle_click, gensym("click"),
    	A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);

    class_addmethod(toddle_class, (t_method)toddle_size, gensym("size"),
    	A_FLOAT, A_FLOAT, 0);

    class_addmethod(toddle_class, (t_method)toddle_color, gensym("color"),
    	A_SYMBOL, 0);

    class_addmethod(toddle_class, (t_method)toddle_on, gensym("on"), 0);

    class_addmethod(toddle_class, (t_method)toddle_off, gensym("off"), 0);


    toddle_setwidget();
    class_setwidget(toddle_class,&toddle_widgetbehavior);
}


