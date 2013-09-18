#ifndef __GG_ENVGEN_H__
#define __GG_ENVGEN_H__

#include "g_canvas.h"

typedef struct _wenvgen {
     t_glist* glist;
     int    width;
     int    height;
     int    numdoodles;
     int    grabbed; /* for moving points */
     int    shift; /* move 100th */
     float    pointerx;
     float    pointery;
     t_clock* numclock;
} t_wenvgen;

typedef struct _envgen
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

     t_symbol* r_sym;
     t_symbol* s_sym;

     t_float min;
     t_float max;

     t_clock* x_clock;
     t_float x_freeze;

     t_outlet* out2;
     /* widget parameters */
     t_wenvgen w;
} t_envgen;


t_widgetbehavior envgen_widgetbehavior;
void envgen_drawme(t_envgen *x, t_glist *glist, int firsttime);
int envgen_set_values(t_envgen * x);
void envgen_resize(t_envgen* x,int ns);

#endif
