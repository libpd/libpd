#include <string.h>
#include <m_pd.h>
#include "g_canvas.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ image ----------------------------- */

static t_class *image_class;

typedef struct _image
{
     t_object x_obj;
     t_glist * x_glist;
     int x_width;
     int x_height;
     t_symbol*  x_fname;
} t_image;

/* widget helper functions */

void image_drawme(t_image *x, t_glist *glist, int firsttime)
{
	if (firsttime) {
		char fname[MAXPDSTRING];
		canvas_makefilename(glist_getcanvas(x->x_glist), x->x_fname->s_name,
							fname, MAXPDSTRING);

	  sys_vgui("image create photo img%lx -file {%s}\n",x,fname);
	  sys_vgui(".x%lx.c create image %d %d -image img%lx -tags %lxS\n", 
			   glist_getcanvas(glist),text_xpix(&x->x_obj, glist), 
			   text_ypix(&x->x_obj, glist),x,x);

	  /* TODO callback from gui
	    sys_vgui("image_size logo");
	  */
     }     
     else {
		 sys_vgui(".x%lx.c coords %lxS %d %d\n",
				  glist_getcanvas(glist), x,
				  text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
     }
	
}


void image_erase(t_image* x,t_glist* glist)
{
     int n;
     sys_vgui(".x%lx.c delete %lxS\n",
	      glist_getcanvas(glist), x);

}
	

static t_symbol *get_filename(t_int argc, t_atom *argv)
{
    t_symbol *fname;
    fname = atom_getsymbolarg(0, argc, argv);
    if(argc > 1)
    {
        int i;
        char buf[MAXPDSTRING];
        strcpy(buf, fname->s_name);
        for(i = 1; i < argc; i++)
        {
            strcat(buf, " ");
            strcat(buf, atom_getsymbolarg(i, argc, argv)->s_name);
        }
        fname = gensym(buf);
    }
    return fname;
}

/* ------------------------ image widgetbehaviour----------------------------- */


static void image_getrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_image* x = (t_image*)z;


    width = x->x_width;
    height = x->x_height;
    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = text_xpix(&x->x_obj, glist) + width;
    *yp2 = text_ypix(&x->x_obj, glist) + height;
}

static void image_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_image *x = (t_image *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    sys_vgui(".x%lx.c coords %lxSEL %d %d %d %d\n",
		   glist_getcanvas(glist), x,
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
		   text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height);

    image_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void image_select(t_gobj *z, t_glist *glist, int state)
{
     t_image *x = (t_image *)z;
     if (state) {
	  sys_vgui(".x%lx.c create rectangle \
%d %d %d %d -tags %lxSEL -outline blue\n",
		   glist_getcanvas(glist),
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
		   text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height,
		   x);
     }
     else {
	  sys_vgui(".x%lx.c delete %lxSEL\n",
		   glist_getcanvas(glist), x);
     }



}


static void image_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void image_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}

       
static void image_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_image* x = (t_image*)z;
    if (vis)
	 image_drawme(x, glist, 1);
    else
	 image_erase(x,glist);
}

/* can we use the normal text save function ?? */

static void image_save(t_gobj *z, t_binbuf *b)
{
    t_image *x = (t_image *)z;
    binbuf_addv(b, "ssiiss", gensym("#X"), gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix,   
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_fname);
    binbuf_addv(b, ";");
}


t_widgetbehavior   image_widgetbehavior;

void image_size(t_image* x,t_floatarg w,t_floatarg h) {
     x->x_width = w;
     x->x_height = h;
}

void image_color(t_image* x,t_symbol* col)
{
/*     outlet_bang(x->x_obj.ob_outlet); only bang if there was a bang .. 
       so color black does the same as bang, but doesn't forward the bang 
*/
}

void image_open(t_image* x, t_symbol *s, t_int argc, t_atom *argv)
{
    x->x_fname = get_filename(argc, argv);
    image_erase(x, x->x_glist);
    image_drawme(x, x->x_glist, 1);
}

static void image_setwidget(void)
{
    image_widgetbehavior.w_getrectfn =     image_getrect;
    image_widgetbehavior.w_displacefn =    image_displace;
    image_widgetbehavior.w_selectfn =   image_select;
    image_widgetbehavior.w_activatefn =   image_activate;
    image_widgetbehavior.w_deletefn =   image_delete;
    image_widgetbehavior.w_visfn =   image_vis;
#if (PD_VERSION_MINOR > 31) 
    image_widgetbehavior.w_clickfn = NULL;
    image_widgetbehavior.w_propertiesfn = NULL; 
#endif
#if PD_MINOR_VERSION < 37
    image_widgetbehavior.w_savefn =   image_save;
#endif
}


static void *image_new(t_symbol *s, t_int argc, t_atom *argv)
{
    t_image *x = (t_image *)pd_new(image_class);

    x->x_glist = (t_glist*) canvas_getcurrent();

    x->x_width = 15;
    x->x_height = 15;

    x->x_fname = get_filename(argc, argv);
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void image_setup(void)
{
    image_class = class_new(gensym("image"), (t_newmethod)image_new, 0,
				sizeof(t_image),0, A_GIMME,0);

    class_addmethod(image_class, (t_method)image_size, gensym("size"),
    	A_FLOAT, A_FLOAT, 0);

/*
    class_addmethod(image_class, (t_method)image_color, gensym("color"),
    	A_SYMBOL, 0);
*/

    class_addmethod(image_class, (t_method)image_open, gensym("open"),
    	A_GIMME, 0);
	
    image_setwidget();
    class_setwidget(image_class,&image_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(image_class,&image_save);
#endif
}


