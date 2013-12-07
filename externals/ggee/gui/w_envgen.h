#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef _WIN32
#define abs fabs
#endif

#define BACKGROUNDCOLOR "grey"
#define BORDER 2


static void draw_inlets(t_envgen *x, t_glist *glist, int firsttime, int nin, int nout)
{
     int n = nout;
     int nplus, i;
     int xpos = text_xpix(&x->x_obj,glist);
     int ypos = text_ypix(&x->x_obj,glist);

     nplus = (n == 1 ? 1 : n-1);
     for (i = 0; i < n; i++)
     {
	  int onset = xpos + (x->w.width-2*BORDER) * i / nplus;
	  if (firsttime)
	       sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxo%d\n",
			glist_getcanvas(glist),
			onset, ypos + x->w.height - 1 + 2*BORDER,
			onset + IOWIDTH, ypos + x->w.height + 2*BORDER,
			x, i);
	  else
	       sys_vgui(".x%lx.c coords %lxo%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, ypos + x->w.height - 1 + 2*BORDER,
			onset + IOWIDTH, ypos + x->w.height + 2*BORDER);
     }
     n = nin; 
     nplus = (n == 1 ? 1 : n-1);
     for (i = 0; i < n; i++)
     {
	  int onset = xpos + (x->w.width - IOWIDTH) * i / nplus - BORDER;
	  if (firsttime)
	       sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxi%d\n",
			glist_getcanvas(glist),
			onset, ypos - BORDER,
			     onset + IOWIDTH, ypos + 1 - BORDER,
			x, i);
	  else
	       sys_vgui(".x%lx.c coords %lxi%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, ypos - BORDER,
			onset + IOWIDTH, ypos + 1 - BORDER);
	  
     }
}



static int envgen_next_doodle(t_envgen *x, struct _glist *glist,
                              int xpos,int ypos)
{
     int ret = -1;
     float xscale,yscale;
     int dxpos,dypos;
     float minval = 100000.0;
     float tval;
     int i;
     int insertpos = -1;

     if (xpos > text_xpix(&x->x_obj,glist) + x->w.width) 
         xpos = text_xpix(&x->x_obj,glist) + x->w.width;

     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     dxpos = text_xpix(&x->x_obj,glist);/* + BORDER */;
     dypos = text_ypix(&x->x_obj,glist) + BORDER;

     for (i=0;i<=x->last_state;i++) {
	  float dx2 = (dxpos + (x->duration[i] * xscale)) - xpos;
	  float dy2 = (dypos + yscale - (x->finalvalues[i] * yscale)) - ypos;

	  dx2*=dx2;
	  dy2*=dy2;
	  tval = sqrt(dx2+dy2);

	  if (tval <= minval) {
	    minval = tval;	    
	    insertpos = i;
	  }
     }

     /* decide if we want to make a new one */
     if (minval > /*5*/ 8 && insertpos >= 0 && !x->x_freeze) {

	  while (((dxpos + (x->duration[insertpos] * xscale)) - xpos) < 0)
	       insertpos++;
	  while (((dxpos + (x->duration[insertpos-1] * xscale)) - xpos) > 0)
	       insertpos--;

	  if (x->last_state+1 >= x->args)
	       envgen_resize(x,x->args+1);

	  for (i=x->last_state;i>=insertpos;i--) {
	       x->duration[i+1] = x->duration[i];
	       x->finalvalues[i+1] = x->finalvalues[i];
	  }
	  x->duration[insertpos] = (float)(xpos-dxpos)/x->w.width*x->duration[x->last_state++];
	  x->w.pointerx = xpos;
	  x->w.pointery = ypos;
     }
     else {
	  x->w.pointerx = text_xpix(&x->x_obj,glist) + x->duration[insertpos]*x->w.width/x->duration[x->last_state]; 


	  x->w.pointery = text_ypix(&x->x_obj,glist) + 
	       (1.f - x->finalvalues[insertpos])*x->w.height;	  
     }

     x->w.grabbed = insertpos;
     return insertpos;
}

static void envgen_create_doodles(t_envgen *x, t_glist *glist)
{
     float xscale,yscale;
     int xpos,ypos;
     int i;
     char guistr[255];
     
     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     xpos = text_xpix(&x->x_obj,glist);
     ypos = (int) (text_ypix(&x->x_obj,glist) + x->w.height);
     for (i=0;i<=x->last_state;i++) {
	  sprintf(guistr,".x%lx.c create oval %d %d %d %d -tags %lxD%d",(long unsigned int)glist_getcanvas(glist),
		   (int) (xpos+(x->duration[i] * xscale) - 2),
		   (int) (ypos - x->finalvalues[i]*yscale - 2),
		   (int) (xpos+(x->duration[i] * xscale)+2),
		   (int) (ypos - x->finalvalues[i]*yscale + 2),
		   (long unsigned int)x,i);

	  if (i == x->w.grabbed) strcat(guistr," -fill red\n");
	  else strcat(guistr,"\n");
	  sys_vgui("%s",guistr);
     }
     x->w.numdoodles = i;
}


static void envgen_delete_doodles(t_envgen *x, t_glist *glist)
{
     int i;
     for (i=0;i<=x->w.numdoodles;i++) {
	  sys_vgui(".x%lx.c delete %lxD%d\n",glist_getcanvas(glist),x,i);
     }
}

static void envgen_update_doodles(t_envgen *x, t_glist *glist)
{

     envgen_delete_doodles(x,glist);
/* LATER only create new doodles if necessary */
     envgen_create_doodles(x, glist);
}


static void envgen_delnum(t_envgen *x)
{
     sys_vgui(".x%lx.c delete %lxT\n",glist_getcanvas(x->w.glist),x); 
}


static void envgen_shownum(t_envgen *x,t_glist* glist) 
{
     float xscale,yscale;
     int xpos,ypos;
     int i= x->w.grabbed;

     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     xpos = text_xpix(&x->x_obj,glist);
     ypos = (int) (text_ypix(&x->x_obj,glist) + x->w.height);

     envgen_delnum(x);

     sys_vgui(".x%lx.c create text %d %d -text %fx%f -tags %lxT\n",
	     (long unsigned int)glist_getcanvas(x->w.glist),
	     
	     (int) (xpos+(x->duration[i] * xscale) + 12),
	     (int) (ypos - x->finalvalues[i]*yscale - 2),
	     
	     x->finalvalues[i]*(x->max-x->min),
	     x->duration[i],
	     (long unsigned int)x);
     clock_delay(x->w.numclock,700);
}



static void envgen_create(t_envgen *x, t_glist *glist)
{
     int i;
     static char  buf[1024];
     float xscale,yscale;
     int xpos,ypos;
     char num[40];

     xpos = text_xpix(&x->x_obj,glist);
     ypos = (int) text_ypix(&x->x_obj,glist);
     x->w.numclock = clock_new(x, (t_method) envgen_delnum);     
     sys_vgui(".x%lx.c create rectangle \
%d %d %d %d -tags %lxS -fill "BACKGROUNDCOLOR"\n",
	      glist_getcanvas(glist),
	      xpos-BORDER, ypos-BORDER,
	      xpos + x->w.width+2*BORDER, ypos + x->w.height+2*BORDER,
	      x);
     
     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     sprintf(buf,".x%lx.c create line",(long unsigned int)glist_getcanvas(glist));
     for (i=0;i<=x->last_state;i++) {
	  sprintf(num," %d %d ",(int)(xpos + x->duration[i]*xscale),
		                (int)(ypos + x->w.height- x->finalvalues[i]*yscale));
	  strcat(buf,num);
     }
     
     sprintf(num,"-tags %pP\n",x);
     strcat(buf,num);
     sys_vgui("%s",buf);
     envgen_create_doodles(x,glist);
}


static void envgen_update(t_envgen *x, t_glist *glist)
{
int i;
     static char  buf[1024];
     float xscale,yscale;
     char num[40];
     int xpos = text_xpix(&x->x_obj,glist);
     int ypos = text_ypix(&x->x_obj,glist);

     sys_vgui(".x%lx.c coords %lxS \
%d %d %d %d\n",
	      glist_getcanvas(glist), x,
	      xpos - BORDER, ypos -BORDER,
	      xpos + x->w.width+2*BORDER, ypos + x->w.height+2*BORDER);
     
     
     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     sprintf(buf,".x%lx.c coords %pP",(long unsigned int)glist_getcanvas(glist),x);

     for (i=0;i<=x->last_state;i++) {
	  sprintf(num," %d %d ",(int)(xpos + x->duration[i]*xscale),
		                (int) (ypos + x->w.height - x->finalvalues[i]*yscale));
	  strcat(buf,num);
     }
     strcat(buf,"\n");
     sys_vgui("%s",buf);
     envgen_update_doodles(x,glist);
     draw_inlets(x, glist, 0,1,2);
}



void envgen_drawme(t_envgen *x, t_glist *glist, int firsttime)
{

     if (firsttime) envgen_create(x,glist);
     else envgen_update(x,glist);

     draw_inlets(x, glist, firsttime, 1,2);
}




void envgen_erase(t_envgen* x,t_glist* glist)
{
     int n;
     sys_vgui(".x%lx.c delete %lxS\n",
	      glist_getcanvas(glist), x);

     sys_vgui(".x%lx.c delete %pP\n",
	      glist_getcanvas(glist), x);


     sys_vgui(".x%lx.c delete %lxi0\n",glist_getcanvas(glist),x);
     sys_vgui(".x%lx.c delete %lxo0\n",glist_getcanvas(glist),x);
     sys_vgui(".x%lx.c delete %lxo1\n",glist_getcanvas(glist),x);
     envgen_delete_doodles(x,glist);
}
	


/* ------------------------ envgen widgetbehaviour----------------------------- */


static void envgen_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_envgen* s = (t_envgen*)z;

    width = s->w.width + 2*BORDER;
    height = s->w.height + 2*BORDER;
    *xp1 = text_xpix(&s->x_obj,owner)-BORDER;
    *yp1 = text_ypix(&s->x_obj,owner)-BORDER;
    *xp2 = text_xpix(&s->x_obj,owner) + width + 4;
    *yp2 = text_ypix(&s->x_obj,owner) + height + 4;
}

static void envgen_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_envgen *x = (t_envgen *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;

    envgen_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void envgen_select(t_gobj *z, t_glist *glist, int state)
{
     t_envgen *x = (t_envgen *)z;
    sys_vgui(".x%lx.c itemconfigure %lxS -fill %s\n", glist, 
	     x, (state? "blue" : BACKGROUNDCOLOR));
}


static void envgen_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void envgen_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}

       
static void envgen_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_envgen* s = (t_envgen*)z;
    if (vis)
	 envgen_drawme(s, glist, 1);
    else
	 envgen_erase(s,glist);
}

/* can we use the normal text save function ?? */

static void envgen_save(t_gobj *z, t_binbuf *b)
{
    t_envgen *x = (t_envgen *)z;
    binbuf_addv(b, "ssiisiiffss", gensym("#X"), gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,  
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->w.width,x->w.height,x->max,x->min,x->r_sym,x->s_sym);
    binbuf_addv(b, ";");
}



static void envgen_followpointer(t_envgen* x,t_glist* glist)
{
     float dur;
     float xscale = x->duration[x->last_state]/x->w.width;

     if  ((x->w.grabbed > 0) && (x->w.grabbed < x->last_state)) {
	  
	  dur = (x->w.pointerx - text_xpix(&x->x_obj,glist))*xscale;
	  if (dur < x->duration[x->w.grabbed-1])
	       dur = x->duration[x->w.grabbed-1];
	  if (dur > x->duration[x->w.grabbed+1])	  
	       dur = x->duration[x->w.grabbed+1];

	  x->duration[x->w.grabbed] = dur;
     }
     

     x->finalvalues[x->w.grabbed] = 1.0f - (x->w.pointery - (float)text_ypix(&x->x_obj,glist))/(float)x->w.height;
     if (x->finalvalues[x->w.grabbed] < 0.0) 
	  x->finalvalues[x->w.grabbed]= 0.0;
     else if (x->finalvalues[x->w.grabbed] > 1.0)
	  x->finalvalues[x->w.grabbed]= 1.0;

}


void envgen_motion(t_envgen *x, t_floatarg dx, t_floatarg dy)
{
	if (x->w.shift) {
	  x->w.pointerx+=dx/1000.f;
	  x->w.pointery+=dy/1000.f;
     }
     else
     {
	  x->w.pointerx+=dx;
	  x->w.pointery+=dy;
     }
     if (!x->resizing)
	  envgen_followpointer(x,x->w.glist);
     else {
	       x->w.width+=dx;
	       x->w.height+=dy;
     }
     envgen_shownum(x,x->w.glist);
     envgen_update(x,x->w.glist);
}

void envgen_key(t_envgen *x, t_floatarg f)
{
     if (f == 8.0 && x->w.grabbed < x->last_state &&  x->w.grabbed > 0) {
	  int i;

	  for (i=x->w.grabbed;i<=x->last_state;i++) {
	       x->duration[i] = x->duration[i+1];
	       x->finalvalues[i] = x->finalvalues[i+1];
	  }

	  x->last_state--;
	  x->w.grabbed--;
	  envgen_update(x,x->w.glist);
     }
}


static int envgen_newclick(t_envgen *x, struct _glist *glist,
    int xpos, int ypos, int shift, int alt, int dbl, int doit)
{
    /* check if user wants to resize */
     float wxpos = text_xpix(&x->x_obj,glist);
     float wypos = (int) (text_ypix(&x->x_obj,glist) + x->w.height);

     if (doit){
         envgen_next_doodle(x,glist,xpos,ypos);

         glist_grab(x->w.glist, &x->x_obj.te_g, (t_glistmotionfn) envgen_motion,
                    (t_glistkeyfn) envgen_key, xpos, ypos);

         x->resizing = 0;     
         if (x->resizeable && (xpos > wxpos + x->w.width) && 
             (ypos > wypos)) {
             x->resizing = 1;     
             return (0);
         }
         
         x->w.shift = shift;
         envgen_followpointer(x,glist);
         envgen_shownum(x,glist);
         envgen_update(x,glist);
     }
     return (1);
}

