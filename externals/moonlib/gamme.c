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
#include <math.h>
#include <stdlib.h>
#include <m_pd.h>
#include "g_canvas.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ gamme ----------------------------- */
#define BACKGROUND "-fill grey"
#define BACKGROUNDCOLOR "grey"

#ifndef  PD_VERSION_MINOR
#define PD_VERSION_MINOR 32
#endif

#define IS_A_POINTER(atom,index) ((atom+index)->a_type == A_POINTER)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)
#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)


#define DEFAULTSIZE 15
#define DEFAULTWIDTH 90
#define DEFAULTHEIGHT 40

#define DEFAULTCOLOR "black"
#define BLACKCOLOR "black"
#define WHITECOLOR "white"
#define SELBLACKCOLOR "gold"
#define SELWHITECOLOR "yellow"


static t_class *gamme_class;

static char *NoteNames[]=
{ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
static char NoteColPos[]=
{ 1,-1,2,-2,3,4,-4,5,-5,6,-6,7 };
static char Whites[]= {0,2,4,5,7,9,11};
static char Blacks[]= {1,3,6,8,10};
static char BlacksWhites[]= {1,3,6,8,10,0,2,4,5,7,9,11};
static char WhitesBlacks[]= {0,2,4,5,7,9,11,1,3,6,8,10};

#define ISWHITE(x) (NoteColPos[x]>0)
#define ISBLACK(x) (!ISWHITE(x))

#define te_xpos te_xpix
#define te_ypos te_ypix

typedef struct _gamme
{
    t_object x_obj;
    t_outlet *x_out_n; /*gives the number of selected notes when change occurs*/
    t_outlet *x_out_note; /*gives the number and new value of the changed notes when change occurs*/
    t_glist *x_glist;
    int x_width;
    int x_height;
    char x_n;
    char x_notes[12];
    char x_on_notes[12];
} t_gamme;

/* widget helper functions */


#define INTERSPACE 0.02
#define NOTEWIDTH ((1-INTERSPACE*6.0)/7.0)
#define BLACK1st ((NOTEWIDTH+INTERSPACE)/2.0)
#define BLACKH 0.6
static void note_get_rel_rect(int x, float *xp1, float *yp1, float *xp2, float *yp2)
{
    int cp=NoteColPos[x];

    *xp1=(abs(cp)-1)*(NOTEWIDTH+INTERSPACE) + (cp<0)*BLACK1st;
    *xp2=*xp1+NOTEWIDTH;

    *yp1=0;
    *yp2=cp<0?BLACKH:1;
}

static int get_touched_note(float x, float y)
{
    int i,j;
    float xp1,xp2,yp1,yp2;

    for(j=0; j<12; j++)
    {
        i=BlacksWhites[j];
        note_get_rel_rect(i,&xp1,&yp1,&xp2,&yp2);
        if((x>=xp1)&&(x<=xp2)&&(y>=yp1)&&(y<=yp2))
            return i;
    }
    /*post("gamme::get_touched_note note not found: x=%f y=%f",x,y);*/
    return -1;
}

static void draw_inlets(t_gamme *x, t_glist *glist, int firsttime, int nin, int nout)
{
    int n = nout;
    int nplus, i;
    int xpos=text_xpix(&x->x_obj, glist);
    int ypos=text_ypix(&x->x_obj, glist);

    nplus = (n == 1 ? 1 : n-1);
    for (i = 0; i < n; i++)
    {
        int onset = xpos + (x->x_width - IOWIDTH) * i / nplus;
        if (firsttime)
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %xo%d\n",
                     glist_getcanvas(glist),
                     onset, ypos + x->x_height - 1,
                     onset + IOWIDTH, ypos + x->x_height,
                     x, i);
        else
            sys_vgui(".x%lx.c coords %xo%d %d %d %d %d\n",
                     glist_getcanvas(glist), x, i,
                     onset, ypos + x->x_height - 1,
                     onset + IOWIDTH, ypos + x->x_height);
    }
    n = nin;
    nplus = (n == 1 ? 1 : n-1);
    for (i = 0; i < n; i++)
    {
        int onset = xpos + (x->x_width - IOWIDTH) * i / nplus;
        if (firsttime)
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %xi%d\n",
                     glist_getcanvas(glist),
                     onset, ypos,
                     onset + IOWIDTH, ypos + 1,
                     x, i);
        else
            sys_vgui(".x%lx.c coords %xi%d %d %d %d %d\n",
                     glist_getcanvas(glist), x, i,
                     onset, ypos,
                     onset + IOWIDTH, ypos + 1);

    }
}

void gamme_drawme(t_gamme *x, t_glist *glist, int firsttime)
{
    int i,j;
    float x1,y1,x2,y2;
    int xi1,yi1,xi2,yi2;
    char *color;
    int xpos=text_xpix(&x->x_obj, glist);
    int ypos=text_ypix(&x->x_obj, glist);

    if (firsttime)
    {
        sys_vgui(".x%lx.c create rectangle \
%d %d %d %d -tags %xS "BACKGROUND"\n",
                 glist_getcanvas(glist),
                 xpos, ypos,
                 xpos + x->x_width, ypos + x->x_height,
                 x);

    }
    else
    {
        sys_vgui(".x%lx.c coords %xS \
%d %d %d %d\n",
                 glist_getcanvas(glist), x,
                 xpos, ypos,
                 xpos + x->x_width, ypos + x->x_height);
    }

    for(j=0; j<12; j++)
    {
        i=WhitesBlacks[j];
        note_get_rel_rect(i,&x1,&y1,&x2,&y2);
        xi1=xpos + x->x_width*x1;
        xi2=xpos + x->x_width*x2;
        yi1=ypos + x->x_height*y1;
        yi2=ypos + x->x_height*y2;

        if (firsttime)
        {
            color=x->x_notes[i]?	(ISWHITE(i)?SELWHITECOLOR:SELBLACKCOLOR):
                      (ISWHITE(i)?WHITECOLOR:BLACKCOLOR);
            sys_vgui(".x%lx.c create rectangle \
%d %d %d %d -tags %x%s -fill %s\n",
                     glist_getcanvas(glist),xi1,yi1,xi2,yi2,
                     x,NoteNames[i],color);
        }
        else
    {
            sys_vgui(".x%lx.c coords %x%s \
%d %d %d %d\n",
                     glist_getcanvas(glist),x,NoteNames[i],xi1,yi1,xi2,yi2);
        }
    }

    draw_inlets(x, glist, firsttime, 1,3);

}

void gamme_erase(t_gamme *x,t_glist *glist)
{
    int n;
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c delete %xS\n",canvas, x);

    for(n=0; n<12; n++)
        sys_vgui(".x%lx.c delete %x%s\n",canvas,x,NoteNames[n]);

    n = 1;
    while (n--)
    {
        sys_vgui(".x%lx.c delete %xi%d\n",canvas,x,n);
    }
    n = 3;
    while (n--)
    {
        sys_vgui(".x%lx.c delete %xo%d\n",canvas,x,n);
    }
}



/* ------------------------ gamme widgetbehaviour----------------------------- */


static void gamme_getrect(t_gobj *z, t_glist *glist,
                          int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_gamme *x = (t_gamme *)z;
    int width, height;
    t_gamme *s = (t_gamme *)z;


    width = s->x_width;
    height = s->x_height;
    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = *xp1 + width;
    *yp2 = *yp1 + height;
}

static void gamme_displace(t_gobj *z, t_glist *glist,
                           int dx, int dy)
{
    t_gamme *x = (t_gamme *)z;
    x->x_obj.te_xpos += dx;
    x->x_obj.te_ypos += dy;
    gamme_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text *) x);
}

static void gamme_select(t_gobj *z, t_glist *glist, int state)
{
    t_gamme *x = (t_gamme *)z;
    sys_vgui(".x%lx.c itemconfigure %xS -fill %s\n", glist,
             x, (state? "blue" : BACKGROUNDCOLOR));
}


static void gamme_activate(t_gobj *z, t_glist *glist, int state)
{
    /*    t_text *x = (t_text *)z;
        t_rtext *y = glist_findrtext(glist, x);
        if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void gamme_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}


static void gamme_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_gamme *s = (t_gamme *)z;
    if (vis)
        gamme_drawme(s, glist, 1);
    else
        gamme_erase(s,glist);
}

/* can we use the normal text save function ?? */

static void gamme_save(t_gobj *z, t_binbuf *b)
{
    t_gamme *x = (t_gamme *)z;
    char *c=x->x_notes;

    binbuf_addv(b, "ssiisiiiiiiiiiiiiii", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpos, (t_int)x->x_obj.te_ypos,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_width,x->x_height,
                c[0],c[1],c[2],c[3],c[4],c[5],c[6],c[7],c[8],c[9],c[10],c[11]);
    binbuf_addv(b, ";");
}

void gamme_getn(t_gamme *x)
{
    outlet_float(x->x_out_n,x->x_n);
}

void gamme_out_changed(t_gamme *x,int note)
{
    t_atom ats[2];
    SETFLOAT(&ats[0],note);
    SETFLOAT(&ats[1],x->x_notes[note]);

    outlet_list(x->x_out_note,0,2,ats);
}

inline float my_mod(float x,int n)
{
    float y=fmod(x,n);
    return y<0?y+n:y;
}

#define my_div(x,y) (floor(x/y))
#define tonotei(x) (my_mod(rint(x),12U))

void gamme_set(t_gamme *x,t_floatarg note,t_floatarg on)
{
    unsigned int i,notei=tonotei(note),changed=0;
    char *color;
    t_canvas *canvas=glist_getcanvas(x->x_glist);


    if(x->x_notes[notei]!=on) changed=1;
    if(on<0) x->x_notes[notei]=!(x->x_notes[notei]);
    else x->x_notes[notei]=on;
    if(changed) gamme_out_changed(x,notei);

    color=x->x_notes[notei]?(ISWHITE(notei)?SELWHITECOLOR:SELBLACKCOLOR):
              (ISWHITE(notei)?WHITECOLOR:BLACKCOLOR);

    if(glist_isvisible(x->x_glist))
        sys_vgui(".x%lx.c itemconfigure %x%s -fill %s\n", canvas,
                 x, NoteNames[notei],color);

    x->x_n=0;
    for(i=0; i<12; i++) if(x->x_notes[i]) x->x_on_notes[(int)(x->x_n++)]=i;
    gamme_getn(x);
}

#define getnote(n) \
	(my_div(n,(int)x->x_n)*12+x->x_on_notes[(int)my_mod(n,x->x_n)])
void gamme_get(t_gamme *x,t_floatarg ref_octave,t_floatarg note)
{
    int no0,no1,ni0,ni1,n0,n1,n;
    float xn,xx,nn;

    if(!x->x_n) return;
    no0=floor(note);
    no1=ceil(note);
    xx=note-no0;

    nn=getnote((float)no0)*(1-xx)+getnote((float)(no0+1))*xx+ref_octave*12;
    n=getnote((float)no0)+ref_octave*12;
    outlet_float(x->x_obj.ob_outlet,nn);
}

static void gamme_click(t_gamme *x, t_floatarg xpos, t_floatarg ypos,
                        t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    int note;
    int x0=text_xpix(&x->x_obj, x->x_glist);
    int y0=text_ypix(&x->x_obj, x->x_glist);

    note=get_touched_note(
             (xpos-x0)/x->x_width,
             (ypos-y0)/x->x_height);

    if(note>=0) gamme_set(x,note,!x->x_notes[note]);
}

static int gamme_newclick(t_gobj *z, struct _glist *glist,
                          int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_gamme *x = (t_gamme *)z;

    if(doit)
    {
        gamme_click( x, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift,
                     0, (t_floatarg)alt);
    }
    return (1);
}

void gamme_float(t_gamme *x,t_floatarg f)
{
    unsigned int notei=tonotei(f);

    /*post("notei=%d",notei);*/
    if(x->x_notes[notei])
        outlet_float(x->x_obj.ob_outlet,f);
}

void gamme_round(t_gamme *x,t_floatarg f,t_floatarg round)
{
    unsigned int notei=tonotei(f);
    int imin=floor(f),imax=ceil(f);
    float norm;

    if(!x->x_n) return;

    while(!x->x_notes[(int)my_mod((imin),12U)]) imin--;
    while(!x->x_notes[(int)my_mod((imax),12U)]) imax++;

    /*post("min: %d  max: %d",imin,imax);*/

    if((imin!=imax)&&round)
    {
        round*=round;
        norm=(f-imin)/(imax-imin)*2-1;
        norm=norm/sqrt(1+round*norm*norm)*sqrt(1+round)/2+.5;
        f=norm*(imax-imin)+imin;
    }
    outlet_float(x->x_obj.ob_outlet,f);
}

void gamme_setall(t_gamme *x,t_symbol *s, int argc, t_atom *argv)
{
    int i=0,err=0;

    if(argc==12)
    {
        for(i=0; i<12; i++) err+=!IS_A_FLOAT(argv,i);
        if(!err) for(i=0; i<12; i++) gamme_set(x,i,atom_getfloat(&argv[i]));
    }
}

void gamme_getall(t_gamme *x)
{
    int i=0;

    for(i=0; i<12; i++) gamme_out_changed(x,i);
    gamme_getn(x);
}

extern int sys_noloadbang;
static void gamme_loadbang(t_gamme *x)
{
    int i;

    if(sys_noloadbang) return;
    for(i=0; i<12; i++) gamme_out_changed(x,i);
    gamme_getn(x);
}

void gamme_size(t_gamme *x,t_floatarg w,t_floatarg h)
{
    x->x_width = w;
    x->x_height = h;
    gamme_drawme(x, x->x_glist, 0);
}

t_widgetbehavior   gamme_widgetbehavior;

static void gamme_setwidget(void)
{
    gamme_widgetbehavior.w_getrectfn =		gamme_getrect;
    gamme_widgetbehavior.w_displacefn =	gamme_displace;
    gamme_widgetbehavior.w_selectfn =		gamme_select;
    gamme_widgetbehavior.w_activatefn =	gamme_activate;
    gamme_widgetbehavior.w_deletefn =		gamme_delete;
    gamme_widgetbehavior.w_visfn =			gamme_vis;
    gamme_widgetbehavior.w_clickfn =		gamme_newclick;
    //gamme_widgetbehavior.w_propertiesfn =	NULL;
    //gamme_widgetbehavior.w_savefn =			gamme_save;
}


static void *gamme_new(t_symbol *s, int argc, t_atom *argv)
{
    int i=0,err=0;

    t_gamme *x = (t_gamme *)pd_new(gamme_class);

    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_width = DEFAULTWIDTH;
    x->x_height = DEFAULTHEIGHT;
    outlet_new(&x->x_obj, &s_float);
    x->x_out_n=outlet_new(&x->x_obj, &s_float);
    x->x_out_note=outlet_new(&x->x_obj, &s_float);

    x->x_n=0;
    for(i=0; i<12; i++) x->x_notes[i]=0;
    for(i=0; i<12; i++) x->x_on_notes[i]=0;

    if((argc>1)&&IS_A_FLOAT(argv,0)&&IS_A_FLOAT(argv,1))
    {
        if(atom_getfloat(&argv[0])) x->x_width = atom_getfloat(&argv[0]);
        if(atom_getfloat(&argv[1])) x->x_height = atom_getfloat(&argv[1]);

        if(argc==14)
        {
            for(i=0; i<12; i++) err+=(!IS_A_FLOAT(argv,i+2));
            if(!err)
            {
                for(i=0; i<12; i++) if(x->x_notes[i]=atom_getfloat(&argv[i+2]))
                        x->x_on_notes[(int)(x->x_n++)]=i;
                /*gamme_set(x,i,atom_getfloat(&argv[i+2]));gamme_getn(x);*/
            }
            else post("gamme_new : error in creation arguments");
        }
        /*if(argc==14) gamme_setall(x,s,argc-2,&argv[2]);*/
    }

    return (x);
}

void gamme_setup(void)
{
    gamme_class = class_new(gensym("gamme"), (t_newmethod)gamme_new, 0,
                            sizeof(t_gamme),0, A_GIMME,0);

    class_addfloat(gamme_class,gamme_float);

    class_addmethod(gamme_class, (t_method)gamme_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);

    class_addmethod(gamme_class, (t_method)gamme_size, gensym("size"),
                    A_FLOAT, A_FLOAT, 0);

    class_addmethod(gamme_class, (t_method)gamme_set, gensym("set"),
                    A_FLOAT, A_FLOAT, 0);

    class_addmethod(gamme_class, (t_method)gamme_get, gensym("get"),
                    A_FLOAT, A_FLOAT, 0);

    class_addmethod(gamme_class, (t_method)gamme_round, gensym("round"),
                    A_FLOAT, A_FLOAT, 0);

    class_addmethod(gamme_class, (t_method)gamme_setall, gensym("setall"),
                    A_GIMME, 0);

    class_addmethod(gamme_class, (t_method)gamme_getall, gensym("getall"), 0);

    class_addmethod(gamme_class, (t_method)gamme_getn, gensym("getn"), 0);

    /*class_addmethod(gamme_class, (t_method)gamme_loadbang, gensym("loadbang"), 0);*/


    gamme_setwidget();
    class_setwidget(gamme_class,&gamme_widgetbehavior);
    class_setsavefn(gamme_class, gamme_save);

}


