#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ image ----------------------------- */

static t_class *image_class;

typedef struct _image
{
    t_object x_obj;
    t_glist *x_glist;
    int x_width;
    int x_height;
    t_symbol  *x_image;
    int x_type; //0=file 1=tk_image
    t_int  x_localimage; //localimage "img%x" done
} t_image;

/* widget helper functions */

const char *image_get_filename(t_image *x,char *file)
{
    static char fname[MAXPDSTRING];
    char *bufptr;
    int fd;

    fd=open_via_path(canvas_getdir(glist_getcanvas(x->x_glist))->s_name,
                     file, "",fname, &bufptr, MAXPDSTRING, 1);
    if(fd>0)
    {
        fname[strlen(fname)]='/';
        close(fd);
        return fname;
    }
    else return 0;
}

void image_drawme(t_image *x, t_glist *glist, int firsttime)
{
    if (firsttime)
    {
        if (x->x_image == &s_) // if we have a blank image name, use the included filler
        {
            x->x_image = gensym("::moonlib::image::noimage");
            x->x_type = 1;
            pd_error(x, "[image]: no image found");
        }
        if(x->x_type)
        {
            sys_vgui(".x%lx.c create image %d %d -tags %xS\n",
                     glist_getcanvas(glist),
                     text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x);
            sys_vgui(".x%lx.c itemconfigure %xS -image %s\n",
                     glist_getcanvas(glist),x,x->x_image->s_name);
        }
        else
        {
            const char *fname=image_get_filename(x,x->x_image->s_name);
            if(!x->x_localimage)
            {
                sys_vgui("image create photo img%x\n",x);
                x->x_localimage=1;
            }
            if(fname)
                sys_vgui("::moonlib::image::configure .x%lx img%x {%s}\n",x,x,fname);
            sys_vgui(".x%lx.c create image %d %d -image img%x -tags %xS\n",
                     glist_getcanvas(glist),
                     text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x,x);
        }
        /* TODO callback from gui
          sys_vgui("image_size logo");
        */
    }
    else
    {
        sys_vgui(".x%lx.c coords %xS \
%d %d\n",
                 glist_getcanvas(glist), x,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
    }

}


void image_erase(t_image *x,t_glist *glist)
{
    sys_vgui(".x%lx.c delete %xS\n", glist_getcanvas(glist), x);
}



/* ------------------------ image widgetbehaviour----------------------------- */


static void image_getrect(t_gobj *z, t_glist *glist,
                          int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_image *x = (t_image *)z;


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
    sys_vgui(".x%lx.c coords %xSEL %d %d %d %d\n",
             glist_getcanvas(glist), x,
             text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
             text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height);

    image_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text *) x);
}

static void image_select(t_gobj *z, t_glist *glist, int state)
{
    t_image *x = (t_image *)z;
    if (state)
    {
        sys_vgui(".x%lx.c create rectangle \
%d %d %d %d -tags %xSEL -outline blue\n",
                 glist_getcanvas(glist),
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                 text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height,
                 x);
    }
    else
    {
        sys_vgui(".x%lx.c delete %xSEL\n",
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
    t_image *s = (t_image *)z;
    if (vis)
        image_drawme(s, glist, 1);
    else
        image_erase(s,glist);
}

/* can we use the normal text save function ?? */

static void image_save(t_gobj *z, t_binbuf *b)
{
    t_image *x = (t_image *)z;
    binbuf_addv(b, "ssiissi", gensym("#X"),gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_image,x->x_type);
    binbuf_addv(b, ";");
}


t_widgetbehavior   image_widgetbehavior;

void image_size(t_image *x,t_floatarg w,t_floatarg h)
{
    x->x_width = w;
    x->x_height = h;
}

void image_color(t_image *x,t_symbol *col)
{
    /*     outlet_bang(x->x_obj.ob_outlet); only bang if there was a bang ..
           so color black does the same as bang, but doesn't forward the bang
    */
}

void image_open(t_gobj *z,t_symbol *file)
{
    t_image *x = (t_image *)z;
    const char *fname;
    int oldtype=x->x_type;


    fname=image_get_filename(x,file->s_name);
    if(fname)
    {
        x->x_image=file;
        x->x_type=0;
        if(glist_isvisible(x->x_glist))
        {
            if(!x->x_localimage)
            {
                sys_vgui("image create photo img%x\n",x);
                x->x_localimage=1;
            }
            sys_vgui("img%x blank\n",x);
            sys_vgui("::moonlib::image::configure .x%lx img%x {%s}\n",x,x,fname);
            if(oldtype) sys_vgui(".x%lx.c itemconfigure %xS -image img%x\n",
                                     glist_getcanvas(x->x_glist),x,x);
        }
    }
    else
        pd_error(x, "[image]: error opening file '%s'", file->s_name);
}

void image_load(t_gobj *z,t_symbol *image,t_symbol *file)
{
    t_image *x = (t_image *)z;
    const char *fname;

    fname=image_get_filename(x,file->s_name);
    if(fname)
        sys_vgui("::moonlib::image::create_photo .x%lx %s {%s}\n",x,image->s_name,fname);
}

void image_set(t_gobj *z,t_symbol *image)
{
    t_image *x = (t_image *)z;

    x->x_image=image;
    x->x_type=1;

    if(glist_isvisible(x->x_glist))
        sys_vgui(".x%lx.c itemconfigure %xS -image %s\n",
                 glist_getcanvas(x->x_glist),x,x->x_image->s_name);
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


static void *image_new(t_symbol *image, t_float type)
{
    t_image *x = (t_image *)pd_new(image_class);

    x->x_glist = (t_glist *) canvas_getcurrent();

    x->x_width = 15;
    x->x_height = 15;
    if (type != 0)
        x->x_type= 1;
    else
        x->x_type= 0;
    x->x_localimage=0;
    x->x_image = image;

    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void image_setup(void)
{
    image_class = class_new(gensym("image"), (t_newmethod)image_new, 0,
                            sizeof(t_image),0, A_DEFSYM,A_DEFFLOAT,0);

    /*
        class_addmethod(image_class, (t_method)image_size, gensym("size"),
        	A_FLOAT, A_FLOAT, 0);

        class_addmethod(image_class, (t_method)image_color, gensym("color"),
        	A_SYMBOL, 0);
    */

    class_addmethod(image_class, (t_method)image_open, gensym("open"),
                    A_SYMBOL, 0);
    class_addmethod(image_class, (t_method)image_set, gensym("set"),
                    A_SYMBOL, 0);
    class_addmethod(image_class, (t_method)image_load, gensym("load"),
                    A_SYMBOL, A_SYMBOL, 0);

    image_setwidget();
    class_setwidget(image_class,&image_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(image_class,&image_save);
#endif

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             image_class->c_externdir->s_name,
             image_class->c_name->s_name);
}


