
/* text entry widget for PD                                              *
 * Based on button from GGEE by Guenter Geiger                           *
 * Copyright Ben Bogart 2004 ben@ekran.org                               * 

 * This program is distributed under the terms of the GNU General Public *
 * License                                                               *

 * entry is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *

 * entry is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          */

#include <m_pd.h>
#include <g_canvas.h>
#include <stdio.h>
#include <string.h>

/* TODO: get Ctrl-A working to select all */
/* TODO: set message doesnt work with a loadbang */


#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifndef IOWIDTH 
#define IOWIDTH 4
#endif

#define DEFAULT_COLOR           "grey70"

#define TKW_HANDLE_HEIGHT       15
#define TKW_HANDLE_WIDTH        15

#define TKW_HANDLE_INSET        -2
#define ENTRY_DEFAULT_WIDTH     130
#define ENTRY_DEFAULT_HEIGHT    130
#define ENTRY_MIN_WIDTH         66
#define ENTRY_MIN_HEIGHT        34

#define TOTAL_INLETS            1
#define TOTAL_OUTLETS           2

#define DEBUG(x)
//#define DEBUG(x) x

typedef struct _entry
{
    t_object   x_obj;
    t_canvas  *x_canvas;
    t_glist   *x_glist;

    t_symbol  *x_receive_name;

    int        x_height;
    int        x_width;
    int        x_resizing;

    t_symbol  *x_bgcolour;
    t_symbol  *x_fgcolour;
    
    t_symbol  *x_font_face;
    t_int      x_font_size;
    t_symbol  *x_font_weight;

    t_float    x_border;
    t_symbol  *x_relief;
    t_int      x_have_scrollbar;
    t_int      x_selected;
    
    /* IDs for Tk widgets */
    char *tcl_namespace;       
    char *canvas_id;  
    char *frame_id;       
    char *text_id;        
    char *scrollbar_id;   
    char *handle_id;      
    char *window_tag;      
    char *all_tag;
    
    t_outlet* x_data_outlet;
    t_outlet* x_status_outlet;
} t_entry;


static t_class *entry_class;


static t_symbol *backspace_symbol;
static t_symbol *return_symbol;
static t_symbol *space_symbol;
static t_symbol *tab_symbol;
static t_symbol *escape_symbol;
static t_symbol *left_symbol;
static t_symbol *right_symbol;
static t_symbol *up_symbol;
static t_symbol *down_symbol;

/* function prototypes */

static void entry_getrect(t_gobj *z, t_glist *owner, int *xp1, int *yp1, int *xp2, int *yp2);
static void entry_displace(t_gobj *z, t_glist *glist, int dx, int dy);
static void entry_select(t_gobj *z, t_glist *glist, int state);
static void entry_activate(t_gobj *z, t_glist *glist, int state);
static void entry_delete(t_gobj *z, t_glist *glist);
static void entry_vis(t_gobj *z, t_glist *glist, int vis);
//static int entry_click(t_gobj *z, t_glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit);
static void entry_save(t_gobj *z, t_binbuf *b);


static t_widgetbehavior   entry_widgetbehavior = {
w_getrectfn:  entry_getrect,
w_displacefn: entry_displace,
w_selectfn:   entry_select,
w_activatefn: NULL,
w_deletefn:   entry_delete,
w_visfn:      entry_vis,
w_clickfn:    NULL,
}; 

/* widget helper functions */
static void set_tk_widget_ids(t_entry *x, t_canvas *canvas)
{
    char buf[MAXPDSTRING];

    x->x_canvas = canvas;

    /* Tk ID for the current canvas that this object is drawn in */
    sprintf(buf,".x%lx.c", (long unsigned int) canvas);
    x->canvas_id = getbytes(strlen(buf) + 1);
    strcpy(x->canvas_id, buf);

    /* Tk ID for the "frame" the other things are drawn in */
    sprintf(buf,"%s.frame%lx", x->canvas_id, (long unsigned int)x);
    x->frame_id = getbytes(strlen(buf) + 1);
    strcpy(x->frame_id, buf);

    sprintf(buf,"%s.text%lx", x->frame_id, (long unsigned int)x);
    x->text_id = getbytes(strlen(buf) + 1);
    strcpy(x->text_id, buf);    /* Tk ID for the "text", the meat! */

    sprintf(buf,"%s.window%lx", x->canvas_id, (long unsigned int)x);
    x->window_tag = getbytes(strlen(buf) + 1);
    strcpy(x->window_tag, buf);    /* Tk ID for the resizing "window" */

    sprintf(buf,"%s.handle%lx", x->canvas_id, (long unsigned int)x);
    x->handle_id = getbytes(strlen(buf) + 1);
    strcpy(x->handle_id, buf);    /* Tk ID for the resizing "handle" */

    sprintf(buf,"%s.scrollbar%lx", x->frame_id, (long unsigned int)x);
    x->scrollbar_id = getbytes(strlen(buf) + 1);
    strcpy(x->scrollbar_id, buf);    /* Tk ID for the optional "scrollbar" */

    sprintf(buf,"all%lx", (long unsigned int)x);
    x->all_tag = getbytes(strlen(buf) + 1);
    strcpy(x->all_tag, buf);    /* Tk ID for the optional "scrollbar" */
}

static int calculate_onset(t_entry *x, t_glist *glist, 
                           int current_iolet, int total_iolets)
{
    DEBUG(post("calculate_onset"););
    return(text_xpix(&x->x_obj, glist) + (x->x_width - IOWIDTH)    \
           * current_iolet / (total_iolets == 1 ? 1 : total_iolets - 1));
}

static void draw_inlets(t_entry *x, t_glist *glist, int firsttime, 
                        int total_inlets, int total_outlets)
{
    DEBUG(post("draw_inlets in: %d  out: %d", total_inlets, total_outlets););

    int i, onset;
    
    /* inlets */
    for (i = 0; i < total_inlets; i++)
    {
        onset = calculate_onset(x, glist, i, total_inlets);
        if (firsttime)
        {
            sys_vgui("%s create rectangle %d %d %d %d -tags {%xi%d %xi %s}\n",
                     x->canvas_id, onset, text_ypix(&x->x_obj, glist) - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist),
                     x, i, x, x->all_tag);
        }
    }
    for (i = 0; i < total_outlets; i++) /* outlets */
    {
        onset = calculate_onset(x, glist, i, total_outlets);
        if (firsttime)
        {
            sys_vgui("%s create rectangle %d %d %d %d -tags {%xo%d %xo %s}\n",
                     x->canvas_id, onset, text_ypix(&x->x_obj, glist) + x->x_height,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_height + 2,
                     x, i, x, x->all_tag);
        }
    }
    DEBUG(post("draw inlet end"););
}

static void erase_inlets(t_entry *x)
{
    DEBUG(post("erase_inlets"););
/* Added tag for all inlets/outlets of one instance */
    sys_vgui("%s delete %xi\n", x->canvas_id, x); 
    sys_vgui("%s delete %xo\n", x->canvas_id, x); 

}

static void draw_scrollbar(t_entry *x)
{
    sys_vgui("pack %s -side right -fill y -before %s \n",
             x->scrollbar_id, x->text_id);
    x->x_have_scrollbar = 1;
}

static void erase_scrollbar(t_entry *x)
{
    sys_vgui("pack forget %s \n", x->scrollbar_id);
    x->x_have_scrollbar = 0;
}

static void bind_button_events(t_entry *x)
{
    /* mouse buttons */
    sys_vgui("bind %s <Button> {pdtk_canvas_mouse %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b 0}\n",
             x->text_id, x->canvas_id, x->canvas_id, x->canvas_id);
    sys_vgui("bind %s <ButtonRelease> {pdtk_canvas_mouseup %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->text_id, x->canvas_id, x->canvas_id, x->canvas_id);
    sys_vgui("bind %s <Shift-Button> {pdtk_canvas_mouse %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b 1}\n",
             x->text_id, x->canvas_id, x->canvas_id, x->canvas_id);
    sys_vgui("bind %s <Button-2> {pdtk_canvas_rightclick %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->text_id, x->canvas_id, x->canvas_id, x->canvas_id);
    sys_vgui("bind %s <Button-3> {pdtk_canvas_rightclick %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->text_id, x->canvas_id, x->canvas_id, x->canvas_id);
    sys_vgui("bind %s <$::modifier-Button> {pdtk_canvas_rightclick %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->text_id, x->canvas_id, x->canvas_id, x->canvas_id);
    /* mouse motion */
    sys_vgui("bind %s <Motion> {pdtk_canvas_motion %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] 0}\n",
             x->text_id, x->canvas_id, x->canvas_id, x->canvas_id);
}

static void create_widget(t_entry *x)
{
    DEBUG(post("create_widget"););
    /* I guess this is for fine-tuning of the rect size based on width and height? */

    sys_vgui("namespace eval entry%lx {} \n", x);
    
    /* Seems we have to delete the widget in case it already exists (Provided by Guenter)*/
    sys_vgui("destroy %s\n", x->frame_id);
    sys_vgui("frame %s \n", x->frame_id);
    sys_vgui("text %s -font {{%s} %d %s} -border 1 \
    -highlightthickness 1 -relief sunken -bg \"%s\" -fg \"%s\"  \
    -yscrollcommand {%s set} \n",
             x->text_id, 
             x->x_font_face->s_name, x->x_font_size, x->x_font_weight->s_name,
             x->x_bgcolour->s_name, x->x_fgcolour->s_name,
             x->scrollbar_id);
    sys_vgui("scrollbar %s -command {%s yview}\n",
             x->scrollbar_id, x->text_id);
    sys_vgui("pack %s -side left -fill both -expand 1 \n", x->text_id);
    sys_vgui("pack %s -side bottom -fill both -expand 1 \n", x->frame_id);

    bind_button_events(x);
    sys_vgui("bind %s <KeyRelease> {+pdsend {%s keyup %%N}} \n", 
             x->text_id, x->x_receive_name->s_name);
}

static void entry_drawme(t_entry *x, t_glist *glist, int firsttime)
{
    DEBUG(post("entry_drawme: firsttime %d canvas %lx glist %lx", firsttime, x->x_canvas, glist););
    set_tk_widget_ids(x,glist_getcanvas(glist));	
    if (firsttime) 
    {
        create_widget(x);	
        draw_inlets(x, glist, firsttime, TOTAL_INLETS, TOTAL_OUTLETS);
        if(x->x_have_scrollbar) draw_scrollbar(x);
        sys_vgui("%s create window %d %d -anchor nw -window %s    \
                  -tags {%s %s} -width %d -height %d \n", x->canvas_id,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                 x->frame_id, x->window_tag, x->all_tag, x->x_width, x->x_height);
    }     
    else 
    {
        post("NO MORE COORDS");
//        sys_vgui("%s coords %s %d %d\n", x->canvas_id, x->all_tag,
//                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
    }
}


static void entry_erase(t_entry* x,t_glist* glist)
{
    DEBUG(post("entry_erase: canvas %lx glist %lx", x->x_canvas, glist););

    set_tk_widget_ids(x,glist_getcanvas(glist));
    erase_inlets(x);
    sys_vgui("destroy %s\n", x->frame_id);
    sys_vgui("%s delete %s\n", x->canvas_id, x->all_tag);
}
	


/* ------------------------ text widgetbehaviour----------------------------- */


static void entry_getrect(t_gobj *z, t_glist *owner, 
                          int *xp1, int *yp1, int *xp2, int *yp2)
{
//    DEBUG(post("entry_getrect");); /* this one is very chatty :D */
    t_entry *x = (t_entry*)z;
    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = *xp1 + x->x_width;
    *yp2 = *yp1 + x->x_height + 2; // add 2 to give space for outlets
}

static void entry_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_entry *x = (t_entry *)z;
    DEBUG(post("entry_displace: canvas %lx glist %lx", x->x_canvas, glist););
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
        set_tk_widget_ids(x,glist_getcanvas(glist));
        sys_vgui("%s move %s %d %d\n", x->canvas_id, x->all_tag, dx, dy);
        sys_vgui("%s move RSZ %d %d\n", x->canvas_id, dx, dy);
/*        sys_vgui("%s coords %s %d %d %d %d\n", x->canvas_id, x->all_tag,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                 text_xpix(&x->x_obj, glist) + x->x_width, 
                 text_ypix(&x->x_obj, glist) + x->x_height-2);*/
//        entry_drawme(x, glist, 0);
        canvas_fixlinesfor(glist, (t_text*) x);
    }
    DEBUG(post("displace end"););
}

static void entry_select(t_gobj *z, t_glist *glist, int state)
{
    t_entry *x = (t_entry *)z;
    DEBUG(post("entry_select: canvas %lx glist %lx state %d", x->x_canvas, glist, state););
    
    if( (state) && (!x->x_selected))
    {
        sys_vgui("%s configure -bg #bdbddd -state disabled -cursor $cursor_editmode_nothing\n",
                 x->text_id);
        x->x_selected = 1;
    }
    else if (!state)
    {
        sys_vgui("%s configure -bg grey -state normal -cursor xterm\n",
                 x->text_id);
        /* activatefn never gets called with 0, so destroy here */
        sys_vgui("destroy %s\n", x->handle_id);
        x->x_selected = 0;
    }
    entry_activate(z, glist, state);
}

static void entry_activate(t_gobj *z, t_glist *glist, int state)
{
    DEBUG(post("entry_activate %d", state););    
    t_entry *x = (t_entry *)z;
 	int x1, y1, x2, y2;

    if(state)
    {
        entry_getrect(z, glist, &x1, &y1, &x2, &y2);
        sys_vgui("canvas %s -width %d -height %d -bg #ddd -bd 0 \
-highlightthickness 3 -highlightcolor {#f00} -cursor bottom_right_corner\n",
                 x->handle_id, TKW_HANDLE_WIDTH, TKW_HANDLE_HEIGHT);
        int handle_x1 = x2 - TKW_HANDLE_WIDTH;
        int handle_y1 = y2 - (TKW_HANDLE_HEIGHT - TKW_HANDLE_INSET);
//        int handle_x2 = x2;
//        int handle_y2 = y2 - TKW_HANDLE_INSET;
/* no worky, this should draw MAC OS X style lines on the resize handle */
/*         sys_vgui("%s create line %d %d %d %d -fill black -tags RESIZE_LINES\n",  */
/*                  x->handle_id, handle_x2, handle_y1, handle_x1, handle_y2); */
        sys_vgui("%s create window %d %d -anchor nw -width %d -height %d -window %s -tags RSZ\n",
                 x->canvas_id, handle_x1, handle_y1,
                 TKW_HANDLE_WIDTH, TKW_HANDLE_HEIGHT,
                 x->handle_id, x->all_tag);
        sys_vgui("raise %s\n", x->handle_id);
        sys_vgui("bind %s <Button> {pdsend {%s resize_click 1}}\n",
                 x->handle_id, x->x_receive_name->s_name);
        sys_vgui("bind %s <ButtonRelease> {pdsend {%s resize_click 0}}\n",
                 x->handle_id, x->x_receive_name->s_name);
        sys_vgui("bind %s <Motion> {pdsend {%s resize_motion %%x %%y }}\n",
                 x->handle_id, x->x_receive_name->s_name);
    }
}

static void entry_delete(t_gobj *z, t_glist *glist)
{
    DEBUG(post("entry_delete: glist %lx", glist););    
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}

       
static void entry_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_entry *x = (t_entry*)z;
    DEBUG(post("entry_vis: vis %d canvas %lx glist %lx", vis, x->x_canvas, glist););
    if (vis) {
        entry_drawme(x, glist, 1);
    }
    else {
        entry_erase(x, glist);
    }
}

/*  the clickfn is only called in run mode
static int entry_click(t_gobj *z, t_glist *glist, int xpix, int ypix, 
                       int shift, int alt, int dbl, int doit)
{
    t_entry *x = (t_entry *)z;
    DEBUG(post("entry_click x:%d y:%d edit: %d", xpix, ypix, x->x_canvas->gl_edit););    
    return 0;
}
*/

static void entry_append(t_entry* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("entry_append"););
    int i;
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */
    t_float tmp_float;

    for(i=0; i<argc ; i++)
    {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        if(tmp_symbol == &s_)
        {
            tmp_float = atom_getfloatarg(i, argc , argv);
            sys_vgui("lappend ::%s::list %g \n", x->tcl_namespace, tmp_float );
        }
        else 
        {
            sys_vgui("lappend ::%s::list %s \n", x->tcl_namespace, tmp_symbol->s_name );
        }
    }
    sys_vgui("append ::%s::list \" \"\n", x->tcl_namespace);
    sys_vgui("%s insert end $::%s::list ; unset ::%s::list \n", 
               x->text_id, x->tcl_namespace, x->tcl_namespace );
    sys_vgui("%s yview end-2char \n", x->text_id );
}

static void entry_key(t_entry* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("entry_key"););
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */
    t_int tmp_int;

    tmp_symbol = atom_getsymbolarg(0, argc, argv);
    if(tmp_symbol == &s_)
    {
        tmp_int = (t_int) atom_getfloatarg(0, argc , argv);
        if(tmp_int < 10)
        {
            sys_vgui("%s insert end %d\n", x->text_id, tmp_int);
        }
        else if(tmp_int == 10)
        {
            sys_vgui("%s insert end {\n}\n", x->text_id);
        }
        else
        {
            sys_vgui("%s insert end [format \"%c\" %d]\n", x->text_id, tmp_int);
        }
    }
    else 
    {
        sys_vgui("%s insert end %s\n", x->text_id, tmp_symbol->s_name );
    }
    sys_vgui("%s yview end-2char \n", x->text_id );
}

/* Clear the contents of the text widget */
static void entry_clear(t_entry* x)
{
    sys_vgui("%s delete 0.0 end \n", x->text_id);
}

/* Function to reset the contents of the entry box */
static void entry_set(t_entry* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("entry_set"););

    entry_clear(x);
    entry_append(x, s, argc, argv);
}

static void entry_output(t_entry* x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_data_outlet, s, argc, argv );
}

/* Pass the contents of the text widget onto the entry_output fuction above */
static void entry_bang_output(t_entry* x)
{
    /* With "," and ";" escaping thanks to JMZ */
    sys_vgui("pdsend \"%s output [string map {\",\" \"\\\\,\" \";\" \"\\\\;\"} \
              [%s get 0.0 end]] \"\n", 
             x->x_receive_name->s_name, x->text_id);
}

static void entry_keyup(t_entry *x, t_float f)
{
/*     DEBUG(post("entry_keyup");); */
    int keycode = (int) f;
    char buf[10];
    t_symbol *output_symbol;

    if( (keycode > 32 ) && (keycode < 65288) )
    {
        snprintf(buf, 2, "%c", keycode);
        output_symbol = gensym(buf);
    } else
        switch(keycode)
        {
        case 32: /* space */
            output_symbol = space_symbol;
            break;
        case 65293: /* return */
            output_symbol = return_symbol;
            break;
        case 65288: /* backspace */
            output_symbol = backspace_symbol;
            break;
        case 65289: /* tab */
            output_symbol = tab_symbol;
            break;
        case 65307: /* escape */
            output_symbol = escape_symbol;
            break;
        case 65361: /* left */
            output_symbol = left_symbol;
            break;
        case 65363: /* right */
            output_symbol = right_symbol;
            break;
        case 65362: /* up */
            output_symbol = up_symbol;
            break;
        case 65364: /* down */
            output_symbol = down_symbol;
            break;
        default:
            snprintf(buf, 10, "key_%d", keycode);
            DEBUG(post("keyup: %d", keycode););
            output_symbol = gensym(buf);
        }
    outlet_symbol(x->x_status_outlet, output_symbol);
}

static void entry_save(t_gobj *z, t_binbuf *b)
{
    t_entry *x = (t_entry *)z;
    
    binbuf_addv(b, "ssiisiiss;", &s__X, gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix, 
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_width, x->x_height, x->x_bgcolour, x->x_fgcolour);
}


static void entry_option_float(t_entry* x, t_symbol *option, t_float value)
{
	sys_vgui("%s configure -%s %f \n", 
               x->text_id, option->s_name, value);
}

static void entry_option_symbol(t_entry* x, t_symbol *option, t_symbol *value)
{
	sys_vgui("%s configure -%s {%s} \n", 
               x->text_id, option->s_name, value->s_name);
}

static void entry_option(t_entry *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */

    tmp_symbol = atom_getsymbolarg(1, argc, argv);
    if(tmp_symbol == &s_)
    {
        entry_option_float(x,atom_getsymbolarg(0, argc, argv),
                           atom_getfloatarg(1, argc, argv));
    }
    else
    {
        entry_option_symbol(x,atom_getsymbolarg(0, argc, argv),tmp_symbol);
    }
}

static void entry_scrollbar(t_entry *x, t_float f)
{
    if(f > 0)
        draw_scrollbar(x);
    else
        erase_scrollbar(x);
}


/* function to change colour of text background */
void entry_bgcolour(t_entry* x, t_symbol* bgcol)
{
	x->x_bgcolour = bgcol;
	sys_vgui("%s configure -background \"%s\" \n", 
             x->text_id, x->x_bgcolour->s_name);
}

/* function to change colour of text foreground */
void entry_fgcolour(t_entry* x, t_symbol* fgcol)
{
	x->x_fgcolour = fgcol;
	sys_vgui("%s configure -foreground \"%s\" \n", 
             x->text_id, x->x_fgcolour->s_name);
}

static void entry_fontsize(t_entry *x, t_float font_size)
{
    DEBUG(post("entry_fontsize"););
    if(font_size > 8) 
    {
        x->x_font_size = (t_int)font_size;
        sys_vgui("%s configure -font {{%s} %d %s} \n", 
                 x->text_id,
                 x->x_font_face->s_name, x->x_font_size, 
                 x->x_font_weight->s_name);
    }
    else
        pd_error(x,"entry: invalid font size: %f",font_size);
}

static void entry_size(t_entry *x, t_float width, t_float height)
{
    DEBUG(post("entry_size"););
    x->x_height = height;
    x->x_width = width;
    if(glist_isvisible(x->x_glist))
    {
        sys_vgui("%s itemconfigure %s -width %d -height %d\n",
                 x->canvas_id, x->window_tag, x->x_width, x->x_height);
        canvas_fixlinesfor(x->x_glist, (t_text *)x);  // 2nd inlet
    }
}

/* callback functions */
static void entry_click_callback(t_entry *x, t_floatarg f)
{
    if( (x->x_glist->gl_edit) && (x->x_glist == x->x_canvas) )
    {	
        entry_select((t_gobj *)x, x->x_glist, f);
    }
}

static void entry_resize_click_callback(t_entry *x, t_floatarg f)
{
    t_canvas *canvas = (glist_isvisible(x->x_glist) ? x->x_canvas : 0);
    int newstate = (int)f;
    if (x->x_resizing && newstate == 0)
    {
        if (canvas)
        {
            draw_inlets(x, canvas, 1, TOTAL_INLETS, TOTAL_OUTLETS);
            canvas_fixlinesfor(x->x_glist, (t_text *)x);  // 2nd inlet
        }
    }
    else if (!x->x_resizing && newstate)
    {
        erase_inlets(x);
    }
    x->x_resizing = newstate;
}

static void entry_resize_motion_callback(t_entry *x, t_floatarg f1, t_floatarg f2)
{
    DEBUG(post("entry_resize_motion_callback"););
    if (x->x_resizing)
    {
        int dx = (int)f1, dy = (int)f2;
        if (glist_isvisible(x->x_glist))
        {
            x->x_width += dx;
            x->x_height += dy;
            sys_vgui("%s itemconfigure %s -width %d -height %d\n",
                     x->canvas_id, x->window_tag, 
                     x->x_width, x->x_height);
            sys_vgui("%s move RSZ %d %d\n",
                     x->canvas_id, dx, dy);
        }
    }
}

static void entry_free(t_entry *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_receive_name);
}

static void *entry_new(t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("entry_new"););
    t_entry *x = (t_entry *)pd_new(entry_class);
    char buf[MAXPDSTRING];
    
    x->x_height = 1;
    x->x_font_face = gensym("helvetica");
    x->x_font_size = 10;
    x->x_font_weight = gensym("normal");
    x->x_have_scrollbar = 0;
    x->x_selected = 0;

    if (argc < 4)
	{
		post("entry: You must enter at least 4 arguments. Default values used.");
		x->x_width = ENTRY_DEFAULT_WIDTH;
		x->x_height = ENTRY_DEFAULT_HEIGHT;
		x->x_bgcolour = gensym(DEFAULT_COLOR);
		x->x_fgcolour = gensym("black");
		
	} else {
		/* Copy args into structure */
		x->x_width = atom_getint(argv);
		x->x_height = atom_getint(argv+1);
		x->x_bgcolour = atom_getsymbol(argv+2);
		x->x_fgcolour = atom_getsymbol(argv+3);
    }

    x->x_data_outlet = outlet_new(&x->x_obj, &s_float);
    x->x_status_outlet = outlet_new(&x->x_obj, &s_symbol);

    sprintf(buf,"entry%lx",(long unsigned int)x);
    x->tcl_namespace = getbytes(strlen(buf) + 1);
    strcpy(x->tcl_namespace, buf);    

    sprintf(buf,"#%s", x->tcl_namespace);
    x->x_receive_name = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_receive_name);

    x->x_glist = canvas_getcurrent();
    set_tk_widget_ids(x, x->x_glist);

    return (x);
}

void entry_setup(void) {
    entry_class = class_new(gensym("entry"), (t_newmethod)entry_new, 
                            (t_method)entry_free, sizeof(t_entry),0,A_GIMME,0);
				
	class_addbang(entry_class, (t_method)entry_bang_output);

    class_addmethod(entry_class, (t_method)entry_keyup,
                    gensym("keyup"),
                    A_DEFFLOAT,
                    0);

    class_addmethod(entry_class, (t_method)entry_scrollbar,
                    gensym("scrollbar"),
                    A_DEFFLOAT,
                    0);

    class_addmethod(entry_class, (t_method)entry_option,
                    gensym("option"),
                    A_GIMME,
                    0);

    class_addmethod(entry_class, (t_method)entry_size,
                    gensym("size"),
                    A_DEFFLOAT,
                    A_DEFFLOAT,
                    0);

    class_addmethod(entry_class, (t_method)entry_fontsize,
                    gensym("fontsize"),
                    A_DEFFLOAT,
                    0);
	
	class_addmethod(entry_class, (t_method)entry_output,
                    gensym("output"),
                    A_GIMME,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_set,
                    gensym("set"),
                    A_GIMME,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_append,
                    gensym("append"),
                    A_GIMME,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_key,
                    gensym("key"),
                    A_GIMME,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_clear,
                    gensym("clear"),
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_bgcolour,
                    gensym("bgcolour"),
                    A_DEFSYMBOL,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_fgcolour,
                    gensym("fgcolour"),
                    A_DEFSYMBOL,
                    0);

    class_addmethod(entry_class, (t_method)entry_click_callback,
                    gensym("click"), A_FLOAT, 0);
    class_addmethod(entry_class, (t_method)entry_resize_click_callback,
                    gensym("resize_click"), A_FLOAT, 0);
    class_addmethod(entry_class, (t_method)entry_resize_motion_callback,
                    gensym("resize_motion"), A_FLOAT, A_FLOAT, 0);
								  
    class_setwidget(entry_class,&entry_widgetbehavior);
    class_setsavefn(entry_class,&entry_save);

    backspace_symbol = gensym("backspace");
    return_symbol = gensym("return");
	space_symbol = gensym("space");
	tab_symbol = gensym("tab");
	escape_symbol = gensym("escape");
	left_symbol = gensym("left");
	right_symbol = gensym("right");
	up_symbol = gensym("up");
	down_symbol = gensym("down");
}


