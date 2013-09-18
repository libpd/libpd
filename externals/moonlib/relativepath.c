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

typedef struct _relativepath
{
    t_object x_obj;
    t_canvas *x_canvas;
    int x_dolzero;
    int x_realized;
} t_relativepath;

t_class *relativepath_class;

void relativepath_setup(void);

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

static void relativepath_symbol(t_relativepath *x,t_symbol *sym)
{
    t_canvas *can=0;
    t_symbol *s=sym;
    char *instr=sym->s_name,*outstr=instr,
          *candir,
          canname[MAXPDSTRING],totaldir[MAXPDSTRING],
          *cnamedir;
    unsigned int n,i=0;

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

    strcpy(canname,x->x_canvas->gl_name->s_name);
    //post("canname=%s",canname);
    cnamedir=dirname(canname);

    if (strcmp(cnamedir,"."))
    {
        sprintf(totaldir,"%s/%s",candir,cnamedir);
    }
    else
        strcpy(totaldir,candir);

    //post("dir=%s",totaldir);

    n=strlen(totaldir);
    if(strlen(instr)<=n) goto end;

    while(i<n)
    {
        if(instr[i]!=totaldir[i]) goto end;
        i++;
    }

    if(instr[n]=='/')
    {
        outstr=strdup(instr+n+1);
        s=gensym(outstr);
    }

end:
    outlet_symbol(x->x_obj.ob_outlet,s);
}


static void *relativepath_new(t_float dolzero)
{
    t_relativepath *x = (t_relativepath *)pd_new(relativepath_class);
    int d0;

    outlet_new(&x->x_obj, 0);
    x->x_canvas = canvas_getcurrent();
    x->x_dolzero = dolzero;
    x->x_realized=dolzero?0:1;

    return (void *)x;
}

void relativepath_setup(void)
{
    relativepath_class = class_new(gensym("relativepath"),(t_newmethod)relativepath_new,
                                   0, sizeof(t_relativepath), 0,A_DEFFLOAT,0);

    class_addsymbol(relativepath_class, relativepath_symbol);
}

#endif /* NOT _WIN32 */
