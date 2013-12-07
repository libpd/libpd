/* (C) Guenter Geiger <geiger@xdv.org> */


#include "m_pd.h"
#include "g_canvas.h"

/* ------------------------ gcanvas ----------------------------- */


#define DEFAULTSIZE 80

static t_class *gcanvas_class;

#define RECT 1
#define LINE 2
#define MAXELEM 1024

typedef struct _elem {
    int x;
    int y;
    int w;
    int h;
    int g;
    int type;    
    char* color;
} t_element;

typedef struct _gcanvas
{
     t_object x_obj;
     t_glist * x_glist;
     t_outlet* out2;
     t_outlet* out3;
     int x_width;
     int x_height;
     int x;
     int y;
     t_element* x_element[MAXELEM];
     int x_numelem;
} t_gcanvas;


/*
  cv .. canvas
  o  .. object identifier
  c  .. element id
  x,y,w,h .. coordinates
*/


static void rectangle(void* cv,void* o,int c,int x, int y,int w,int h,char* color) {
     sys_vgui(".x%lx.c create rectangle \
                 %d %d %d %d -tags %lx%d -fill %s\n",cv,x,y,x+w,y+h,o,c,color);
}

static void move_object(void* cv,void* o,int c,int x, int y,int w,int h) {
	  sys_vgui(".x%lx.c coords %lx%d %d %d %d %d\n",
                   cv,o,c,x,y,x+w,y+h);

}

static void color_object(void* cv,void* o,int c,char* color) {
     sys_vgui(".x%lx.c itemconfigure %lx%d -fill %s\n", cv, 
	     o, c,color);
}

static void delete_object(void* cv,void* o,int c) {
     sys_vgui(".x%lx.c delete %lx%d\n",
	      cv, o,c);
}

static void line(void* cv,void* o,int c,int x,int y,int w,int h,char* color) {
     sys_vgui(".x%lx.c create line \
                 %d %d %d %d -tags %lx%d -fill %s\n",cv,x,y,x+w,y+h,o,c,color);
}

static void gcanvas_draw_element(t_gcanvas *x,int num)
{
    t_element* e = x->x_element[num];
    if (!e) post("gcanvas_draw_element assertion failed");
    switch (e->type) {
    case RECT:
        rectangle(glist_getcanvas(x->x_glist),x,num,
                  x->x_obj.te_xpix + e->x, 
                  x->x_obj.te_ypix + e->y,
                      e->w, e->h,e->color);                            
        break;
    case LINE:
        line(glist_getcanvas(x->x_glist),x,num,
             x->x_obj.te_xpix + e->x, 
             x->x_obj.te_ypix + e->y,
             e->w, e->h,e->color);                            
            break;
    default:
        post("gcanvas: unknown element");
    }
}

static void gcanvas_move_element(t_gcanvas *x,int num)
{
    t_element* e = x->x_element[num];
    move_object(
        glist_getcanvas(x->x_glist),x,num,
        x->x_obj.te_xpix + e->x, x->x_obj.te_ypix + e->y,
        e->w, e->h);
}

static void gcanvas_delete_element(t_gcanvas *x,int num)
{
    delete_object(glist_getcanvas(x->x_glist),x,num); 
}


static void gcanvas_color_element(t_gcanvas* x,int num,char* color)
{
    t_element* e = x->x_element[num];
    e->color = color;
    color_object(glist_getcanvas(x->x_glist),x,num,color); 
}


/* widget helper functions */

void gcanvas_drawme(t_gcanvas *x, t_glist *glist, int firsttime)
{
     int i;

     if (firsttime) {
          for (i=0;i<x->x_numelem;i++)
              gcanvas_draw_element(x,i);
     }     
     else {
          for (i=0;i<x->x_numelem;i++)
              gcanvas_move_element(x,i);
     }
     
     {
       /* outlets */
	  int n = 3;
	  int nplus, i;
	  nplus = (n == 1 ? 1 : n-1);
	  for (i = 0; i < n; i++)
	  {
	       int onset = x->x_obj.te_xpix + (x->x_width - IOWIDTH) * i / nplus;
	       if (firsttime)
		    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxo%d\n",
			     glist_getcanvas(glist),
			     onset, x->x_obj.te_ypix + x->x_height - 1,
			     onset + IOWIDTH, x->x_obj.te_ypix + x->x_height,
			     x, i);
	       else
		    sys_vgui(".x%lx.c coords %lxo%d %d %d %d %d\n",
			     glist_getcanvas(glist), x, i,
			     onset, x->x_obj.te_ypix + x->x_height - 1,
			     onset + IOWIDTH, x->x_obj.te_ypix + x->x_height);
	  }
	  /* inlets */
	  n = 0; 
	  nplus = (n == 1 ? 1 : n-1);
	  for (i = 0; i < n; i++)
	  {
	       int onset = x->x_obj.te_xpix + (x->x_width - IOWIDTH) * i / nplus;
	       if (firsttime)
		    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxi%d\n",
			     glist_getcanvas(glist),
			     onset, x->x_obj.te_ypix,
			     onset + IOWIDTH, x->x_obj.te_ypix + 1,
			     x, i);
	       else
		    sys_vgui(".x%lx.c coords %lxi%d %d %d %d %d\n",
			     glist_getcanvas(glist), x, i,
			     onset, x->x_obj.te_ypix,
			     onset + IOWIDTH, x->x_obj.te_ypix + 1);
	       
	  }
     }

}




void gcanvas_erase(t_gcanvas* x,t_glist* glist)
{
    int n,i;

     for (i=0;i<x->x_numelem;i++)
         gcanvas_delete_element(x,i);

     n = 3;
     while (n--) {
	  sys_vgui(".x%lx.c delete %lxo%d\n",glist_getcanvas(glist),x,n);
     }
}
	


/* ------------------------ gcanvas widgetbehaviour----------------------------- */


static void gcanvas_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_gcanvas* s = (t_gcanvas*)z;


    width = s->x_width;
    height = s->x_height;
    *xp1 = s->x_obj.te_xpix;
    *yp1 = s->x_obj.te_ypix;
    *xp2 = s->x_obj.te_xpix + width;
    *yp2 = s->x_obj.te_ypix + height;
}

static void gcanvas_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_gcanvas *x = (t_gcanvas *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    gcanvas_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void gcanvas_select(t_gobj *z, t_glist *glist, int state)
{
     t_gcanvas *x = (t_gcanvas *)z;
     color_object(glist,x,0,state ? "blue" : x->x_element[0]->color);
}


static void gcanvas_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void gcanvas_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}

       
static void gcanvas_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_gcanvas* x = (t_gcanvas*)z;
    if (vis)
	 gcanvas_drawme(x, glist, 1);
    else
	 gcanvas_erase(x,glist);
}

/* can we use the normal text save function ?? */

static void gcanvas_save(t_gobj *z, t_binbuf *b)
{
    t_gcanvas *x = (t_gcanvas *)z;
    binbuf_addv(b, "ssiisii", gensym("#X"),gensym("obj"),
		(t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,  
		gensym("gcanvas"),x->x_width,x->x_height);
    binbuf_addv(b, ";");
}


t_widgetbehavior   gcanvas_widgetbehavior;

static void gcanvas_motion(t_gcanvas *x, t_floatarg dx, t_floatarg dy)
{
  x->x += dx;
  x->y += dy;
  outlet_float(x->out2,x->y);
  outlet_float(x->x_obj.ob_outlet,x->x);
}

void gcanvas_key(t_gcanvas *x, t_floatarg f)
{
  post("key");
}


static void gcanvas_click(t_gcanvas *x,
    t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl,
    t_floatarg doit,int up)
{
    glist_grab(x->x_glist, &x->x_obj.te_g, (t_glistmotionfn) gcanvas_motion,
		(t_glistkeyfn) NULL, xpos, ypos);

    x->x = xpos - x->x_obj.te_xpix;
    x->y = ypos - x->x_obj.te_ypix;
    outlet_float(x->out2,x->y);
    outlet_float(x->x_obj.ob_outlet,x->x);
    outlet_float(x->out3,0);
}

static int gcanvas_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    	if (doit)
	    gcanvas_click((t_gcanvas *)z, (t_floatarg)xpix, (t_floatarg)ypix,
	    	(t_floatarg)shift, 0, (t_floatarg)alt,dbl);

    if (dbl) outlet_float(((t_gcanvas*)z)->out3,1);
    return (1);
}

void gcanvas_size(t_gcanvas* x,t_floatarg w,t_floatarg h) {
     x->x_width = w;
     x->x_height = h;
     gcanvas_drawme(x, x->x_glist, 0);
}

static void gcanvas_setwidget(void)
{
    gcanvas_widgetbehavior.w_getrectfn =     gcanvas_getrect;
    gcanvas_widgetbehavior.w_displacefn =    gcanvas_displace;
    gcanvas_widgetbehavior.w_selectfn =   gcanvas_select;
    gcanvas_widgetbehavior.w_activatefn =   gcanvas_activate;
    gcanvas_widgetbehavior.w_deletefn =   gcanvas_delete;
    gcanvas_widgetbehavior.w_visfn =   gcanvas_vis;
    gcanvas_widgetbehavior.w_clickfn = gcanvas_newclick;
    class_setsavefn(gcanvas_class,gcanvas_save);
}


static void gcanvas_rect(t_gcanvas* x,t_symbol* c,float xp,float y,float w,float h)
{
    t_element* e = getbytes(sizeof(t_element));
    x->x_element[x->x_numelem++] = e;
    
    e->type = RECT;
    e->x = xp;
    e->y = y;
    e->w = w;
    e->h = h;
    e->color = c->s_name;
    gcanvas_draw_element(x,x->x_numelem-1);
}

static void gcanvas_line(t_gcanvas* x,t_symbol* c,float xp,float y,float w,float h)
{
    if (x->x_numelem < MAXELEM-1) {
        t_element* e = getbytes(sizeof(t_element));
        x->x_element[x->x_numelem++] = e;
        e->type = LINE;
        e->x = xp;
        e->y = y;
        e->w = w;
        e->h = h;
        e->g = 1;
        e->color = c->s_name;
        gcanvas_draw_element(x,x->x_numelem-1);
    }
}

static void gcanvas_move(t_gcanvas* x,float num,float xp,float y,float w,float h)
{
    t_element* e = x->x_element[(int)num];
    if (e) {
        e->x = xp;
        e->y = y;
        e->w = w;
        e->h = h;
        gcanvas_move_element(x,(int)num);
    }    
}

static void gcanvas_color(t_gcanvas* x,t_symbol* c,float num)
{
    gcanvas_color_element(x,(int)num,c->s_name);
}


void gcanvas_deletenum(t_gcanvas* x,float num)
{
    int i = (int) num;
    if (x->x_element[i]) {
        gcanvas_delete_element(x,i);
        freebytes(x->x_element[i],sizeof(t_element));
        x->x_element[i] = NULL;
    }
}

static void gcanvas_reset(t_gcanvas* x) {
    int i;
    for (i=1;i<x->x_numelem;i++)
        gcanvas_deletenum(x,i);
    x->x_numelem = 1;
}

static void *gcanvas_new(t_symbol* s,t_int ac,t_atom* at)
{
    int i;
    t_gcanvas *x = (t_gcanvas *)pd_new(gcanvas_class);

    x->x_glist = (t_glist*) canvas_getcurrent();
    
    for (i=0;i<MAXELEM;i++)
        x->x_element[i] = NULL;
    x->x_numelem = 0;


    /* Fetch the width */

    x->x_width = DEFAULTSIZE;
    if (ac-- > 0) {
         if (at->a_type != A_FLOAT)
              error("gcanvas: wrong argument type");
         else
              x->x_width = atom_getfloat(at++);
         
         if (x->x_width < 0 || x->x_width > 2000) {
              error("gcanvas: unallowed width %f",x->x_width);
              x->x_width = DEFAULTSIZE;
         }
    }

    /* Fetch the height */

    x->x_height = DEFAULTSIZE;
    if (ac-- > 0) {
         if (at->a_type != A_FLOAT)
              error("gcanvas: wrong argument type");
         else 
              x->x_height = atom_getfloat(at++);
         
         if (x->x_height < 0 || x->x_height > 2000) {
              error("gcanvas: unallowed height %f",x->x_height);
              x->x_width = DEFAULTSIZE;
         }
    }

    x->x_element[0] = getbytes(sizeof(t_element));
    x->x_numelem++;
    
    x->x_element[0]->type = RECT;
    x->x_element[0]->x = 0;
    x->x_element[0]->y = 0;
    x->x_element[0]->w = x->x_width;
    x->x_element[0]->h = x->x_height;
    x->x_element[0]->color = "white";
   

    outlet_new(&x->x_obj, &s_float);
    x->out2 = outlet_new(&x->x_obj, &s_float);
    x->out3 = outlet_new(&x->x_obj, &s_float);
    return (x);
}



void gcanvas_setup(void)
{
    gcanvas_class = class_new(gensym("gcanvas"), (t_newmethod)gcanvas_new, 0,
				sizeof(t_gcanvas),0, A_GIMME,0);

    class_addmethod(gcanvas_class, (t_method)gcanvas_click, gensym("click"),
    	A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gcanvas_class, (t_method)gcanvas_size, gensym("size"),
    	A_FLOAT, A_FLOAT, 0);

    class_addmethod(gcanvas_class, (t_method)gcanvas_line, gensym("line"),
    	A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT,A_FLOAT,0);
    class_addmethod(gcanvas_class, (t_method)gcanvas_rect, gensym("rect"),
    	A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT,A_FLOAT,0);
    class_addmethod(gcanvas_class, (t_method)gcanvas_move, gensym("move"),
    	A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT,A_FLOAT,0);
    class_addmethod(gcanvas_class, (t_method)gcanvas_color, gensym("color"),
    	A_SYMBOL,A_FLOAT,0);
    class_addmethod(gcanvas_class, (t_method)gcanvas_deletenum,gensym("delete"),
    	A_FLOAT,0);

    class_addmethod(gcanvas_class, (t_method)gcanvas_reset,gensym("reset"),0);
    gcanvas_setwidget();
    class_setwidget(gcanvas_class,&gcanvas_widgetbehavior);
}


