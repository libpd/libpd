#ifndef _WIN32
/*
Copyright (C) 2002 Antoine Rousseau

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*#include <m_imp.h>*/
#include "m_pd.h"
#include "g_canvas.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>

extern t_canvas *canvas_list;	    	    /* list of all root canvases */
extern int canvas_getdollarzero( void);

struct _canvasenvironment
{
    t_symbol *ce_dir;	/* directory patch lives in */
    int ce_argc;    	/* number of "$" arguments */
    t_atom *ce_argv;	/* array of "$" arguments */
    int ce_dollarzero;	/* value of "$0" */
};

typedef struct _absolutepath
{
    t_object x_obj;
    t_canvas *x_canvas;
    int x_dolzero;
    int x_realized;
} t_absolutepath;

t_class *absolutepath_class;

void absolutepath_setup(void);

static t_glist *getcanvas(t_glist *can,int d0)
{
    t_canvas *retcan=0;
    t_gobj *ob;

    if((can->gl_env)&&(can->gl_env->ce_dollarzero==d0))
    {
        return can;
    }

    ob=can->gl_list;
    while(ob&&(retcan==0))
    {
        if (pd_class(&ob->g_pd) == canvas_class)
            retcan=getcanvas((t_glist *)ob,d0);
        ob=ob->g_next;
    }

    if((!retcan)&&(can->gl_next)) retcan=getcanvas((t_glist *)can->gl_next,d0);
    return retcan;
}


static void absolutepath_symbol(t_absolutepath *x,t_symbol *sym)
{
    t_canvas *can=0;
    char buf[MAXPDSTRING], *bufptr,
         *instr=sym->s_name,
          canname[MAXPDSTRING],totaldir[MAXPDSTRING],
          *cnamedir,
          *candir;
    unsigned int n,i=0;
    int fd;

    if(!x->x_realized) can=(t_canvas *)getcanvas(canvas_list,x->x_dolzero);
    if(can)
    {
        x->x_canvas = can;
        x->x_realized = 1;
        //post("found $0 canvas : %x %d ",x->x_canvas, x->x_canvas->gl_env->ce_dollarzero );
    }

    if(!instr) return;

    candir=canvas_getdir(x->x_canvas)->s_name;
    if(!candir) candir="";

    //post("input= %s   candir= %s   glname=%s",instr,candir,x->x_canvas->gl_name->s_name);

    strcpy(canname,x->x_canvas->gl_name->s_name);
    cnamedir=dirname(canname);

    if (strcmp(cnamedir,"."))
    {
        sprintf(totaldir,"%s/%s",candir,cnamedir);
        fd=open_via_path(totaldir,instr ,"",buf, &bufptr, MAXPDSTRING, 1);
    }
    else
        fd=open_via_path(candir, instr, "",buf, &bufptr, MAXPDSTRING, 1);

    if (fd>=0)
    {
        close(fd);
        buf[strlen(buf)]='/';
        outlet_symbol(x->x_obj.ob_outlet,gensym(buf));
    }
    return;
}


static void *absolutepath_new(t_float dolzero)
{
    t_absolutepath *x = (t_absolutepath *)pd_new(absolutepath_class);
    t_canvas *can=canvas_list;
    int d0;

    outlet_new(&x->x_obj, 0);
    x->x_canvas = canvas_getcurrent();
    x->x_dolzero = dolzero;
    x->x_realized=dolzero?0:1;

    return (void *)x;
}

void absolutepath_setup(void)
{
    absolutepath_class = class_new(gensym("absolutepath"),(t_newmethod)absolutepath_new,
                                   0, sizeof(t_absolutepath), 0,A_DEFFLOAT, 0);

    class_addsymbol(absolutepath_class, absolutepath_symbol);
}

#endif /* NOT _WIN32 */
