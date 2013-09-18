#include <m_pd.h>
#include "g_canvas.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

/* Append " x " to the following line to show debugging messages */
#define DEBUG(x) x

// TODO: NEED TO FREE UNUSED IMAGES!



/* ------------------------ imagebutton ----------------------------- */

static t_class *imagebutton_class;
t_widgetbehavior imagebutton_widgetbehavior;


typedef struct _imagebutton
{
     t_object x_obj;
     t_glist * glist;
     int width;
     int height;
     t_symbol*  image_a;
     t_symbol*  image_b;
     t_symbol* receive;
     t_clock* clock_flash;
     t_clock* clock_brk;
     int 		flashing;
	 t_outlet* outlet;
} t_imagebutton;


static void imagebutton_bang(t_imagebutton *x)
{
	
	t_glist* glist = glist_getcanvas(x->glist);
    if(x->flashing) {
		sys_vgui(".x%x.c itemconfigure %ximage -image %x_imagebutton \n", glist, x,x->image_a);
        clock_delay(x->clock_brk, 50);
        //x->flashed = 1;
    } else  {
		sys_vgui(".x%x.c itemconfigure %ximage -image %x_imagebutton \n", glist, x,x->image_b);
        x->flashing = 1;
        
    }
    clock_delay(x->clock_flash, 250);
    
    outlet_bang(x->outlet);
}

static void imagebutton_flash_timeout(t_imagebutton *x)
{
	t_glist* glist = glist_getcanvas(x->glist);
    x->flashing = 0;
    sys_vgui(".x%x.c itemconfigure %ximage -image %x_imagebutton \n", glist, x,x->image_a);
    
}

static void imagebutton_brk_timeout(t_imagebutton *x)
{
	t_glist* glist = glist_getcanvas(x->glist);
    x->flashing = 1;
    sys_vgui(".x%x.c itemconfigure %ximage -image %x_imagebutton \n", glist, x,x->image_b);
    
}


/* widget helper functions */

static const char* imagebutton_get_filename(t_imagebutton *x,char *file) {
	static char fname[MAXPDSTRING];
	char *bufptr;
	int fd;
	
	fd=open_via_path(canvas_getdir(glist_getcanvas(x->glist))->s_name, 
	    file, "",fname, &bufptr, MAXPDSTRING, 1);
	if(fd>0){
	  	fname[strlen(fname)]='/';
	  	DEBUG(post("image file: %s",fname);)
	  	close(fd);
	  	return fname;
	} else {
		return 0;
	}
}

static int imagebutton_click(t_imagebutton *x, struct _glist *glist,
    int xpos, int ypos, int shift, int alt, int dbl, int doit) {
		DEBUG(post("x:%i y:%i dbl:%i doit:%i",xpos,ypos,dbl,doit);)
		//if ( doit) {
			//sys_vgui(".x%x.c itemconfigure %ximage -state \"%s\"\n", x->glist, x, doit?"disabled":"normal");
			if ( doit) imagebutton_bang(x) ;
		//}
		return (1);
	}




static void imagebutton_drawme(t_imagebutton *x, t_glist *glist, int firsttime) {
     if (firsttime) {	
		 
		sys_vgui(".x%x.c create image %d %d -anchor nw -image %x_imagebutton -disabledimage %x_imagebutton -tags %ximage\n", 
			glist_getcanvas(glist),
			text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x->image_a,x->image_b,x);
	  
	  
	   sys_vgui("pd [concat %s _imagesize [image width %x_imagebutton] [image height %x_imagebutton] \\;]\n",x->receive->s_name,x->image_a,x->image_a);
	   
	   
     } else {
	  sys_vgui(".x%x.c coords %ximage %d %d\n",
		   glist_getcanvas(glist), x,
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
     }

}


void imagebutton_erase(t_imagebutton* x,t_glist* glist)
{
     int n;
     sys_vgui(".x%x.c delete %ximage\n",
	      glist_getcanvas(glist), x);

}
	


/* ------------------------ image widgetbehaviour----------------------------- */


static void imagebutton_getrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_imagebutton* x = (t_imagebutton*)z;


    width = x->width;
    height = x->height;
    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = text_xpix(&x->x_obj, glist) + width;
    *yp2 = text_ypix(&x->x_obj, glist) + height;
}

static void imagebutton_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_imagebutton *x = (t_imagebutton *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    sys_vgui(".x%x.c coords %xSEL %d %d %d %d\n",
		   glist_getcanvas(glist), x,
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
		   text_xpix(&x->x_obj, glist) + x->width, text_ypix(&x->x_obj, glist) + x->height);

    imagebutton_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void imagebutton_select(t_gobj *z, t_glist *glist, int state)
{
     t_imagebutton *x = (t_imagebutton *)z;
     if (state) {
	  sys_vgui(".x%x.c create rectangle \
%d %d %d %d -tags %xSEL -outline blue\n",
		   glist_getcanvas(glist),
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
		   text_xpix(&x->x_obj, glist) + x->width, text_ypix(&x->x_obj, glist) + x->height,
		   x);
     }
     else {
	  sys_vgui(".x%x.c delete %xSEL\n",
		   glist_getcanvas(glist), x);
     }



}


static void imagebutton_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void imagebutton_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    //canvas_deletelinesfor(glist_getcanvas(glist), x);
    canvas_deletelinesfor(glist, x);
}

       
static void imagebutton_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_imagebutton* s = (t_imagebutton*)z;
    if (vis)
	 imagebutton_drawme(s, glist, 1);
    else
	 imagebutton_erase(s,glist);
}

/* can we use the normal text save function ?? */
/*
static void imagebutton_save(t_gobj *z, t_binbuf *b)
{
    t_imagebutton *x = (t_imagebutton *)z;
    binbuf_addv(b, "ssiissi", gensym("#X"),gensym("obj"),
		x->x_obj.te_xpix, x->x_obj.te_ypix,   
        atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
		x->x_image,x->x_type);
    binbuf_addv(b, ";");
}
*/


static void imagebutton_size(t_imagebutton* x,t_floatarg w,t_floatarg h) {
     x->width = w;
     x->height = h;
}

static void imagebutton_color(t_imagebutton* x,t_symbol* col)
{
/*     outlet_bang(x->x_obj.ob_outlet); only bang if there was a bang .. 
       so color black does the same as bang, but doesn't forward the bang 
*/
}

/*
static void imagebutton_open(t_gobj *z,t_symbol* file)
{
    t_imagebutton* x = (t_imagebutton*)z;
	const char *fname;
	int oldtype=x->x_type;
	

	fname=imagebutton_get_filename(x,file->s_name);
	if(fname){
		x->x_image=file;
		x->x_type=0;
	 	if(glist_isvisible(x->glist)) {
			if(!x->x_localimage) {
				sys_vgui("image create photo img%x\n",x);
				x->x_localimage=1;
			}
			sys_vgui("img%x blank\n",x);
			sys_vgui("img%x configure -file %s\n",x,fname);
			if(oldtype) sys_vgui(".x%x.c itemconfigure %xS -image img%x\n",
							glist_getcanvas(x->glist),x,x);
		}
	}
}
	*/
	
static void imagebutton_imagesize_callback(t_imagebutton *x, t_float w, t_float h) {
	DEBUG(post("received w %f h %f",w,h);)
	x->width = w;
	x->height = h;
	canvas_fixlinesfor(x->glist,(t_text*) x);
}
	
	
static void imagebutton_free(t_imagebutton *x) {
	
	// check first if variable has been unset and image is unused
    // then delete image and unset variable
     DEBUG(sys_vgui("pd [concat DEBUG b in use [image inuse %x_imagebutton] \\;]\n",x->image_b);)
     DEBUG(sys_vgui("pd [concat DEBUG a in use [image inuse %x_imagebutton] \\;]\n",x->image_a);)
    
    sys_vgui("if { [info exists %x_imagebutton] == 1 && [image inuse %x_imagebutton] == 0} { image delete %x_imagebutton \n unset %x_imagebutton\n} \n",x->image_b,x->image_b,x->image_b,x->image_b);
    sys_vgui("if { [info exists %x_imagebutton] == 1 && [image inuse %x_imagebutton] == 0} { image delete %x_imagebutton \n unset %x_imagebutton\n} \n",x->image_a,x->image_a,x->image_a,x->image_a);
    
    DEBUG(sys_vgui("pd [concat DEBUG b exists [info exists %x_imagebutton] \\;]\n",x->image_b);)
     DEBUG(sys_vgui("pd [concat DEBUG a exists [info exists %x_imagebutton] \\;]\n",x->image_a);)
    
    if (x->receive) {
		pd_unbind(&x->x_obj.ob_pd,x->receive);
	}
	clock_free(x->clock_flash);
	clock_free(x->clock_brk);
    
}
	
	
static void *imagebutton_new(t_symbol *s, int argc, t_atom *argv)
{
    t_imagebutton *x = (t_imagebutton *)pd_new(imagebutton_class);

    x->glist = (t_glist*) canvas_getcurrent();
    
    // Set up a callback to get the size
    x->width = 10;
    x->height = 10;
	
	x->flashing = 0;
	
	x->image_a = NULL;
	x->image_b = NULL;
	
	t_symbol* image_a = NULL;
	t_symbol* image_b = NULL;
	
	const char *fname;
	
	// CREATE IMAGES
	// images are only created if they have not been created yet
	// we use the symbol pointer to distinguish between image files
	
	
	if ( argc && (argv)->a_type == A_SYMBOL ) {
		image_a= atom_getsymbol(argv);
		fname = imagebutton_get_filename(x,image_a->s_name); // Get image file path
		if (fname) {
			x->image_a = gensym(fname);
			//sys_vgui("set %x_a \"%s\" \n",x,fname);
			// Create the image only if the class has not already loaded the same image (with the same symbolic path name)
			sys_vgui("if { [info exists %x_imagebutton] == 0} { image create photo %x_imagebutton -file \"%s\"\n set %x_imagebutton 1\n} \n",x->image_a,x->image_a,fname,x->image_a); 
		    //sys_vgui("pd [concat test %x_imagebutton \\;]\n",x->image_a);
		}
	}
    
    
    
    if ( argc > 1 && (argv+1)->a_type == A_SYMBOL ) {
		image_b= atom_getsymbol(argv+1);
		fname = imagebutton_get_filename(x,image_b->s_name); // Get image file path
		if (fname) {
			x->image_b = gensym(fname);
			//sys_vgui("set %x_b \"%s\" \n",x,fname);
			sys_vgui("if { [info exists %x_imagebutton] == 0} { image create photo %x_imagebutton -file \"%s\"\n set %x_imagebutton 1\n} \n",x->image_b,x->image_b,fname,x->image_b);
			//sys_vgui("pd [concat test %x_imagebutton \\;]\n",x->image_b);
		}
	}
		
	if (image_a == NULL || image_b == NULL) {
		pd_error(x,"[imagebutton] requires two gif images");
		return NULL;
	}
	char buf[MAXPDSTRING];
	sprintf(buf, "#%lx", (long)x);
	x->receive = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->receive );
    
    x->clock_flash = clock_new(x, (t_method)imagebutton_flash_timeout);
    x->clock_brk = clock_new(x, (t_method)imagebutton_brk_timeout);
    
    
   x->outlet = outlet_new(&x->x_obj, &s_float);
   

   return (x);
   
}

void imagebutton_setup(void)
{
    imagebutton_class = class_new(gensym("imagebutton"), (t_newmethod)imagebutton_new, (t_method)imagebutton_free,
				sizeof(t_imagebutton),0, A_GIMME,0);

    class_addmethod(imagebutton_class, (t_method)imagebutton_imagesize_callback,\
                     gensym("_imagesize"), A_DEFFLOAT, A_DEFFLOAT, 0);


/*
    class_addmethod(imagebutton_class, (t_method)imagebutton_size, gensym("size"),
    	A_FLOAT, A_FLOAT, 0);

    class_addmethod(imagebutton_class, (t_method)imagebutton_color, gensym("color"),
    	A_SYMBOL, 0);
*/
 /*
    class_addmethod(imagebutton_class, (t_method)imagebutton_open, gensym("open"),
    	A_SYMBOL, 0);
    	*/
    	/*
    class_addmethod(imagebutton_class, (t_method)imagebutton_set, gensym("set"),
    	A_SYMBOL, 0);
    	*/
    	    	/*
    class_addmethod(imagebutton_class, (t_method)imagebutton_load, gensym("load"),
    	A_SYMBOL, A_SYMBOL, 0);
*/
    
    imagebutton_widgetbehavior.w_getrectfn =     imagebutton_getrect;
    imagebutton_widgetbehavior.w_displacefn =    imagebutton_displace;
    imagebutton_widgetbehavior.w_selectfn =   imagebutton_select;
    imagebutton_widgetbehavior.w_activatefn =   imagebutton_activate;
    imagebutton_widgetbehavior.w_deletefn =   imagebutton_delete;
    imagebutton_widgetbehavior.w_visfn =   imagebutton_vis;

    imagebutton_widgetbehavior.w_clickfn = (t_clickfn)imagebutton_click;
    

#if PD_MINOR_VERSION < 37
	imagebutton_widgetbehavior.w_propertiesfn = NULL; 
    //imagebutton_widgetbehavior.w_savefn =   imagebutton_save;
#endif

    
    class_setwidget(imagebutton_class,&imagebutton_widgetbehavior);
#if PD_MINOR_VERSION >= 37
   // class_setsavefn(imagebutton_class,&imagebutton_save);
#endif

}


