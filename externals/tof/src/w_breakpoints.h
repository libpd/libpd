#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
//#pragma warning( disable : 4244 )
//#pragma warning( disable : 4305 )
#define abs fabs
#endif

#define BACKGROUNDCOLOR "grey86"
#define LINECOLOR "grey30"

#define BORDER 2


static void draw_inlets(t_breakpoints *x, t_glist *glist, int firsttime, int nin, int nout)
{

   if (x->r_sym == &s_) {
     int n = nout;
     int nplus, i;
     int xpos = text_xpix(&x->x_obj,glist);
     int ypos = text_ypix(&x->x_obj,glist);

     nplus = (n == 1 ? 1 : n-1);
     for (i = 0; i < n; i++)
     {
	  int onset = xpos + (x->w.width - IOWIDTH + 3 * BORDER) * i / nplus - BORDER;
	  // OUTLETS
	  if (firsttime)
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xo%d \n",
			glist_getcanvas(glist),
			onset, ypos + x->w.height - 2 + 2*BORDER,
			onset + IOWIDTH, ypos + x->w.height  - 1+ 2*BORDER,
			x, i);
	  else
	       sys_vgui(".x%x.c coords %xo%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, ypos + x->w.height - 2 + 2*BORDER,
			onset + IOWIDTH, ypos + x->w.height -1 + 2*BORDER);
     }
     n = nin; 
     nplus = (n == 1 ? 1 : n-1);
     for (i = 0; i < n; i++)
     {
	  int onset = xpos + (x->w.width - IOWIDTH + 3 * BORDER) * i / nplus - BORDER;
	  // INLETS
	  if (firsttime)
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xi%d\n",
			glist_getcanvas(glist),
			onset, ypos - BORDER + 1,
			     onset + IOWIDTH, ypos + 2 - BORDER,
			x, i);
	  else
	       sys_vgui(".x%x.c coords %xi%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, ypos - BORDER + 1,
			onset + IOWIDTH, ypos + 2 - BORDER);
	  
     }
     }
}



static int breakpoints_next_doodle(t_breakpoints *x, struct _glist *glist,
                              int xpos,int ypos)
{
    // int ret = -1;
     float xscale,yscale;
     int dxpos,dypos;
     float minval = 100000.0;
     float tval;
     int i;
     int insertpos = -1;
     float ySize = x->max - x->min;
     float yBase =  x->min;

     if (xpos > text_xpix(&x->x_obj,glist) + x->w.width) 
         xpos = text_xpix(&x->x_obj,glist) + x->w.width;

     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     dxpos = text_xpix(&x->x_obj,glist);/* + BORDER */;
     dypos = text_ypix(&x->x_obj,glist) + BORDER;

     for (i=0;i<=x->last_state;i++) {
	  float dx2 = (dxpos + (x->duration[i] * xscale)) - xpos;
	  float dy2 = (dypos + yscale - ( (x->finalvalues[i] - yBase) / ySize * yscale)) - ypos;

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
	       breakpoints_resize(x,x->args+1);

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


	  x->w.pointery = ypos;	  
	  //x->w.pointery = text_ypix(&x->x_obj,glist) +  (1.f - (x->finalvalues[i] - yBase) / ySize) * yscale;	
     }
    #ifdef DEBUG
    post("pointery =%f",x->w.pointery);
    post("insertpos =%f",insertpos);
    #endif
     x->w.grabbed = insertpos;
     return insertpos;
}

static void breakpoints_create_doodles(t_breakpoints *x, t_glist *glist)
{
     float xscale,yscale;
     int xpos,ypos;
     int i;
     char guistr[255];
     float ySize = x->max - x->min;
     float yBase =  x->min;
     float yvalue;
     
     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     xpos = text_xpix(&x->x_obj,glist);
     ypos = (int) (text_ypix(&x->x_obj,glist) + x->w.height);
     for (i=0;i<=x->last_state;i++) {
     yvalue = (x->finalvalues[i] - yBase) / ySize * yscale;
	  sprintf(guistr,".x%x.c create oval %d %d %d %d -tags %xD%d",(unsigned int)glist_getcanvas(glist),
		   (int) (xpos+(x->duration[i] * xscale) - 2),
		   (int) (ypos - yvalue - 2),
		   (int) (xpos+(x->duration[i] * xscale)+2),
		   (int) (ypos - yvalue + 2),
		   (unsigned int)x,i);

	  if (i == x->w.grabbed) {
	  	strcat(guistr," -fill red\n");
	  } else {
	   strcat(guistr," -fill "LINECOLOR"\n");
   	  }
	  sys_vgui("%s",guistr);
     }
     x->w.numdoodles = i;
}


static void breakpoints_delete_doodles(t_breakpoints *x, t_glist *glist)
{
     int i;
     for (i=0;i<=x->w.numdoodles;i++) {
	  sys_vgui(".x%x.c delete %xD%d\n",glist_getcanvas(glist),x,i);
     }
}

static void breakpoints_update_doodles(t_breakpoints *x, t_glist *glist)
{

     breakpoints_delete_doodles(x,glist);
/* LATER only create new doodles if necessary */
     breakpoints_create_doodles(x, glist);
}


static void breakpoints_delnum(t_breakpoints *x)
{
     sys_vgui(".x%x.c delete %xT\n",glist_getcanvas(x->w.glist),x); 
     
}


static void breakpoints_shownum(t_breakpoints *x,t_glist* glist) 
{
     float xscale,yscale;
     int xpos,ypos;
     int i= x->w.grabbed;
     float ySize = x->max - x->min;
     float yBase =  x->min;

     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     xpos = text_xpix(&x->x_obj,glist);
     ypos = (int) (text_ypix(&x->x_obj,glist) + x->w.height);

     breakpoints_delnum(x);

     sys_vgui(".x%x.c create text %d %d -text %.2fx%.2f -tags %xT\n",
	     (unsigned int)glist_getcanvas(x->w.glist),
	     
	     (int) (xpos+(x->duration[i] * xscale) +3),
	     (int) (ypos - (x->finalvalues[i]-yBase)/ySize*yscale - 10),
	     
	     x->finalvalues[i],
	     x->duration[i],
	     (unsigned int)x);
     clock_delay(x->w.numclock,700);
}



static void breakpoints_create(t_breakpoints *x, t_glist *glist)
{
     int i;
     static char  buf[1024];
     float xscale,yscale;
     int xpos,ypos;
     char num[40];
     float ySize = x->max - x->min;
     float yBase =  x->min;

     xpos = text_xpix(&x->x_obj,glist);
     ypos = (int) text_ypix(&x->x_obj,glist);
     x->w.numclock = clock_new(x, (t_method) breakpoints_delnum);     
     sys_vgui(".x%x.c create rectangle \
%d %d %d %d  -tags %xS -fill "BACKGROUNDCOLOR" -width %d\n",
	      glist_getcanvas(glist),
	      xpos-BORDER, ypos-BORDER,
	      xpos + x->w.width+2*BORDER, ypos + x->w.height+2*BORDER,
	      x,x->borderwidth);
     
     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     sprintf(buf,".x%x.c create line",(unsigned int)glist_getcanvas(glist));
     for (i=0;i<=x->last_state;i++) {
	  sprintf(num," %d %d ",(int)(xpos + x->duration[i]*xscale),
		                (int)(ypos + x->w.height- (x->finalvalues[i]-yBase)/ySize*yscale));
	  strcat(buf,num);
     }
     
     sprintf(num,"-tags %pP -fill "LINECOLOR"\n",x);
     strcat(buf,num);
     sys_vgui("%s",buf);
     breakpoints_create_doodles(x,glist);
}


static void breakpoints_update(t_breakpoints *x, t_glist *glist)
{
int i;
     static char  buf[1024];
     float xscale,yscale;
     char num[40];
     int xpos = text_xpix(&x->x_obj,glist);
     int ypos = text_ypix(&x->x_obj,glist);
     float ySize = x->max - x->min;
     float yBase =  x->min;

     sys_vgui(".x%x.c coords %xS \
%d %d %d %d\n",
	      glist_getcanvas(glist), x,
	      xpos - BORDER, ypos -BORDER,
	      xpos + x->w.width+2*BORDER, ypos + x->w.height+2*BORDER);
     
     
     xscale = x->w.width/x->duration[x->last_state];
     yscale = x->w.height;
     
     sprintf(buf,".x%x.c coords %pP",(unsigned int)glist_getcanvas(glist),x);

     for (i=0;i<=x->last_state;i++) {
	  sprintf(num," %d %d ",(int)(xpos + x->duration[i]*xscale),
		                (int) (ypos + x->w.height - (x->finalvalues[i]-yBase)/ySize*yscale));
	  strcat(buf,num);
     }
     strcat(buf,"\n");
     sys_vgui("%s",buf);
     breakpoints_update_doodles(x,glist);
     draw_inlets(x, glist, 0,1,3);
}



static void breakpoints_drawme(t_breakpoints *x, t_glist *glist, int firsttime)
{

     if (firsttime) breakpoints_create(x,glist);
     else breakpoints_update(x,glist);

     draw_inlets(x, glist, firsttime, 1,3);
}




static void breakpoints_erase(t_breakpoints* x,t_glist* glist)
{
     //int n;
     sys_vgui(".x%x.c delete %xS\n",
	      glist_getcanvas(glist), x);

     sys_vgui(".x%x.c delete %pP\n",
	      glist_getcanvas(glist), x);

     if (x->r_sym == &s_) {
     sys_vgui(".x%x.c delete %xi0\n",glist_getcanvas(glist),x);
     sys_vgui(".x%x.c delete %xo0\n",glist_getcanvas(glist),x);
     sys_vgui(".x%x.c delete %xo1\n",glist_getcanvas(glist),x);
     sys_vgui(".x%x.c delete %xo2\n",glist_getcanvas(glist),x);
     }
     breakpoints_delete_doodles(x,glist);
}
	


/* ------------------------ breakpoints widgetbehaviour----------------------------- */


static void breakpoints_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_breakpoints* s = (t_breakpoints*)z;

    width = s->w.width + 2*BORDER;
    height = s->w.height + 2*BORDER;
    *xp1 = text_xpix(&s->x_obj,owner)-BORDER;
    *yp1 = text_ypix(&s->x_obj,owner)-BORDER;
    *xp2 = text_xpix(&s->x_obj,owner) + width ; //+ 4
    *yp2 = text_ypix(&s->x_obj,owner) + height ; //+ 4
}

static void breakpoints_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_breakpoints *x = (t_breakpoints *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;

    breakpoints_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void breakpoints_select(t_gobj *z, t_glist *glist, int state)
{
     t_breakpoints *x = (t_breakpoints *)z;
    sys_vgui(".x%x.c itemconfigure %xS -fill %s\n", glist, 
	     x, (state? "blue" : BACKGROUNDCOLOR));
}

/* 
static void breakpoints_activate(t_gobj *z, t_glist *glist, int state)
{
   t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);
}
*/

static void breakpoints_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}

       
static void breakpoints_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_breakpoints* s = (t_breakpoints*)z;
    if (vis)
	 breakpoints_drawme(s, glist, 1);
    else
	 breakpoints_erase(s,glist);
}

/*  

static void breakpoints_save(t_gobj *z, t_binbuf *b)
{
    t_breakpoints *x = (t_breakpoints *)z;
    binbuf_addv(b, "ssiisiiffss", gensym("#X"), gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,  
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->w.width,x->w.height,x->max,x->min,x->r_sym,x->s_sym);
    binbuf_addv(b, ";");
}

*/

static void breakpoints_followpointer(t_breakpoints* x,t_glist* glist)
{
     float dur;
     float xscale = x->duration[x->last_state]/x->w.width;
     float ySize = x->max - x->min;
     float yBase =  x->min;

     if  ((x->w.grabbed > 0) && (x->w.grabbed < x->last_state)) {
	  
	  dur = (x->w.pointerx - text_xpix(&x->x_obj,glist))*xscale;
	  if (dur < x->duration[x->w.grabbed-1])
	       dur = x->duration[x->w.grabbed-1];
	  if (dur > x->duration[x->w.grabbed+1])	  
	       dur = x->duration[x->w.grabbed+1];

	  x->duration[x->w.grabbed] = dur;
     }
     
     float grabbed = (1.0f - (x->w.pointery - (float)text_ypix(&x->x_obj,glist))/(float)x->w.height);
     #ifdef DEBUG
     post("grabbed =%f",grabbed);
     #endif
     
     if (grabbed < 0.0) 
        grabbed= 0.0;
     else if (grabbed > 1.0)
        grabbed= 1.0;
	  
     x->finalvalues[x->w.grabbed] = grabbed * ySize + yBase;
     

    outlet_bang(x->out3);
    if (x->c_sym != &s_) pd_bang(x->c_sym->s_thing);

}


static void breakpoints_motion(t_breakpoints *x, t_floatarg dx, t_floatarg dy)
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
	  breakpoints_followpointer(x,x->w.glist);
     else {
	       x->w.width+=dx;
	       x->w.height+=dy;
     }
     breakpoints_shownum(x,x->w.glist);
     breakpoints_update(x,x->w.glist);
}

static void breakpoints_key(t_breakpoints *x, t_floatarg f)
{
     if (f == 8.0 && x->w.grabbed < x->last_state &&  x->w.grabbed > 0) {
	  int i;

	  for (i=x->w.grabbed;i<=x->last_state;i++) {
	       x->duration[i] = x->duration[i+1];
	       x->finalvalues[i] = x->finalvalues[i+1];
	  }

	  x->last_state--;
	  x->w.grabbed--;
	  breakpoints_update(x,x->w.glist);
	  outlet_bang(x->out3);
	  if (x->c_sym != &s_) pd_bang(x->c_sym->s_thing);
     }
}


/* 
static int bng_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if(doit)
        bng_click((t_bng *)z, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift, 0, (t_floatarg)alt);
    return (1);
}
*/

static int breakpoints_newclick(t_breakpoints *x, struct _glist *glist,
    int xpos, int ypos, int shift, int alt, int dbl, int doit)
{
    // check if user wants to resize 
     float wxpos = text_xpix(&x->x_obj,glist);
     float wypos = (text_ypix(&x->x_obj,glist));

     if (doit){
     
         #ifdef DEBUG
     
     post("clicked");
      #endif

         
          /*
          
          if ( (xpos >= wxpos + BORDER) && (xpos <= wxpos + x->w.width - BORDER) \
             && (ypos >= wypos + BORDER) && (ypos <= wypos + x->w.height - BORDER) ) {
          */
         
         
         if ( (xpos >= wxpos ) && (xpos <= wxpos + x->w.width ) \
             && (ypos >= wypos ) && (ypos <= wypos + x->w.height ) ) {
              #ifdef DEBUG
             post("inside");
              #endif
             
         breakpoints_next_doodle(x,glist,xpos,ypos);

         glist_grab(x->w.glist, &x->x_obj.te_g, (t_glistmotionfn) breakpoints_motion,
                    (t_glistkeyfn) breakpoints_key, xpos, ypos);
             
         x->w.shift = shift;
         breakpoints_followpointer(x,glist);
         breakpoints_shownum(x,glist);
         breakpoints_update(x,glist);
         
         
         }
         
     }
     return (1);
}

/*
static int breakpoints_newclick(t_breakpoints *x, struct _glist *glist,
    int xpos, int ypos, int shift, int alt, int dbl, int doit)
{
    //check if user wants to resize
     float wxpos = text_xpix(&x->x_obj,glist);
     float wypos = (int) (text_ypix(&x->x_obj,glist) + x->w.height);

     if (doit){
         breakpoints_next_doodle(x,glist,xpos,ypos);

         glist_grab(x->w.glist, &x->x_obj.te_g, (t_glistmotionfn) breakpoints_motion,
                    (t_glistkeyfn) breakpoints_key, xpos, ypos);

         x->resizing = 0;     
         if (x->resizeable && (xpos > wxpos + x->w.width) && 
             (ypos > wypos)) {
             x->resizing = 1;     
             
         }
         
         if (xpos > wxpos + BORDER && (xpos < wxpos + x->w.width - BORDER) \
             ypos > wypos + BORDER && (ypos < wypos + x->w.height - BORDER) ) {
         x->w.shift = shift;
         breakpoints_followpointer(x,glist);
         breakpoints_shownum(x,glist);
         breakpoints_update(x,glist);
         }
     }
     return (1);
}

*/

