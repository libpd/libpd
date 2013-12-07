#ifndef __GG_breakpoints_H__
#define __GG_breakpoints_H__

#include "g_canvas.h"

typedef struct _wbreakpoints {
     t_glist* glist;
     int    width;
     int    height;
     int    numdoodles;
     int    grabbed; /* for moving points */
     int    shift; /* move 100th */
     float    pointerx;
     float    pointery;
    
     t_clock* numclock;
} t_wbreakpoints;

typedef struct _breakpoints
{
     t_object x_obj;

     t_float x_val;
     
     int x_state;
     int last_state;
     int sustain_state;
     int envchanged;

     t_float* finalvalues;
     t_float* duration;
     t_float  totaldur;
     t_int    args; /* get rid of that */
     t_int resizing;
     t_int resizeable;
	t_int borderwidth;

     t_symbol* r_sym; //receive symbol
     t_symbol* s_sym; //send symbol
     t_symbol* d_sym; //dump symbol
     t_symbol* c_sym; //change symbol

     t_float min;
     t_float max;

     t_clock* x_clock;
     t_float x_freeze;

      t_float x_f;    	/* place to hold inlet's value if it's set by message */

      int state;


     t_outlet* out2;
     t_outlet* out3;
     /* widget parameters */
     t_wbreakpoints w;
} t_breakpoints;


t_widgetbehavior breakpoints_widgetbehavior;
static void breakpoints_drawme(t_breakpoints *x, t_glist *glist, int firsttime);
static int breakpoints_set_values(t_breakpoints * x);
static void breakpoints_resize(t_breakpoints* x,int ns);

#endif
