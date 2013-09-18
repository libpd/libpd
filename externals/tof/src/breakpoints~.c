/* (C) Guenter Geiger <geiger@epy.co.at> */

#include <m_pd.h>

//#define DEBUG
/* ------------------------ breakpoints~ ----------------------------- */

#define NONE    0
#define ATTACK  1
#define SUSTAIN -1 
#define STATES  100



#include "breakpoints~.h"

#include "w_breakpoints.h"

static t_class *breakpoints_class;


#define OUT_LIST(x,nr,a) \
     outlet_list(x->x_obj.ob_outlet,&s_list,nr,(t_atom*)&a);\
     if (x->s_sym != &s_ && x->s_sym->s_thing) pd_list(x->s_sym->s_thing, &s_list, nr, (t_atom*)&a);


char dumpy[2000];

/* initialize envelope with argument vector */

#include <stdio.h>

/* 
   breakpoints crashes frequently when reallocating memory ....
   I really don't know why, it crashes during resizebytes,
   which means it is unable to realloc() the memory ?????
   the pointer seems to be ok, I don't know what else could
   cause the problem. for the moment we prevent from reallocating
   by setting the STATES variable to 100 */

static void breakpoints_resize(t_breakpoints* x,int ns)
{
     if (ns > x->args) {
	  int newargs = ns*sizeof(t_float); 

	  x->duration = resizebytes(x->duration,x->args*sizeof(t_float),newargs);
	  x->finalvalues = resizebytes(x->finalvalues,x->args*sizeof(t_float),newargs);
	  x->args = ns;  
     }
}

 /* this is the actual performance routine which acts on the samples.
    It's called with a single pointer "w" which is our location in the
    DSP call list.  We return a new "w" which will point to the next item
    after us.  Meanwhile, w[0] is just a pointer to dsp-perform itself
    (no use to us), w[1] and w[2] are the input and output vector locations,
    and w[3] is the number of points to calculate. */
static t_int *breakpointssig_perform(t_int *w)
{
	
	t_breakpoints *x = (t_breakpoints *)(w[1]);
    t_sample *in = (t_float *)(w[2]);
    t_sample *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    
    //int state = x->state; 
    t_sample f;
    t_sample val;
    
     if (x->state > x->last_state) x->state = x->last_state;
    while (n--) {
		f = *in;
		 
		
   
     
		while ( (x->state > 0) && (f < x->duration[x->state-1]) ) x->state--;
		while ( (x->state <  x->last_state) && (x->duration[x->state] < f) ) x->state++;
		// Interpolate
		 if (x->state == 0 || f >= x->duration[x->last_state]) {
			  val = x->finalvalues[x->state];
		  } else {
			  val = x->finalvalues[x->state-1] + 
			  (f - x->duration[x->state-1])*
			  (x->finalvalues[x->state] - x->finalvalues[x->state-1])/ 
			  (x->duration[x->state] - x->duration[x->state-1]);
		  }
		  
		// Output
		*out= val;
		out++;
		in++;
		//*out++=x->state;
    }
    //x->state = state;
    return (w+5);
}



    /* called to start DSP.  Here we call Pd back to add our perform
    routine to a linear callback list which Pd in turn calls to grind
    out the samples. */
static void breakpointssig_dsp(t_breakpoints *x, t_signal **sp)
{
    dsp_add(breakpointssig_perform, 4, x,sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void breakpoints_totaldur(t_breakpoints* x,t_float dur)
{
     int i;
     float f = dur/x->duration[x->last_state];

     if (dur < 10) {
	post("breakpoints: duration too small %f",dur);
        return;
     }

     for (i=1;i<=x->last_state;i++)
	  x->duration[i]*=f;
}


static void breakpoints_dump(t_breakpoints* e)
{
     t_atom argv[50];
     int argc= 0;
     t_atom* a = argv;
     int i;

     SETFLOAT(a,e->finalvalues[0]);argc++;
     for (i=1;i <= e->last_state;i++) {
	  SETFLOAT(argv+argc,e->duration[i] - e->duration[i-1]);
	  argc++;
	  SETFLOAT(argv+argc,e->finalvalues[i]);
	  argc++;
     }
     outlet_list(e->out2,&s_list,argc,(t_atom*)&argv);
     if (e->d_sym != &s_) pd_list(e->d_sym->s_thing, &s_list, argc, argv);
     //pd_float(x->s_sym->s_thing, val);
     //EXTERN void pd_float(t_pd *x, t_float f);
     //EXTERN void outlet_list(t_outlet *x, t_symbol *s, int argc, t_atom *argv);
     //EXTERN void pd_list(t_pd *x, t_symbol *s, int argc, t_atom *argv);
     /////////////////////////////////////////////////////
}

static void breakpoints_init(t_breakpoints *x,int argc,t_atom* argv)
{
     t_float* dur;
     t_float* val;
     t_float tdur = 0;

     if (!argc) return;

     x->duration[0] = 0;

     x->last_state = argc>>1;
     breakpoints_resize(x,argc>>1);

     dur = x->duration;
     val = x->finalvalues;
     
     // get the first value
     if (argc) {
      *val = atom_getfloat(argv++);
      *dur = 0.0;
      x->max = *val;
      x->min = *val;
     }
     dur++;val++;argc--;
     // get the following
     for (;argc > 0;argc--) {
	  tdur += atom_getfloat(argv++);
#ifdef DEBUG
	  post("dur =%f",tdur);
#endif
	  *dur++ = tdur; 
	  argc--;
	  if (argc > 0) {
	       *val = atom_getfloat(argv++); 
	       if (*val > x->max ) x->max = *val;
	       if (*val < x->min ) x->min = *val;
	       *val++;
	   } else {
	       *val = 0; 
	       if (*val > x->max ) x->max = *val;
	       if (*val < x->min ) x->min = *val;
	       *val++;
	   }
#ifdef DEBUG
	  post("val =%f",*(val-1));
#endif

     }
     
    if ( x->max == x->min ) {
        if ( x->max == 0 ) {
          x->max = 1;
        } else {
          if (x->max > 0) {
            x->min = 0;
            if (x->max < 1) x->max =1;
          } else {
            x->max = 0;
          }
        }
     }

}





static void breakpoints_list(t_breakpoints *x,t_symbol* s, int argc,t_atom* argv)
{
     breakpoints_init(x,argc,argv);
     if (glist_isvisible(x->w.glist)) {
	  breakpoints_drawme(x, x->w.glist, 0);
     }
}

static void breakpoints_setresize(t_breakpoints *x, t_floatarg f)
{
     x->resizeable = f;
}




/*
void breakpoints_float(t_breakpoints *x, t_floatarg f)
{
     int state = 0;
     float val;
     
     

     while (x->duration[state] < f && state <  x->last_state) state++;

     if (state == 0 || f >= x->duration[x->last_state]) {
          val = x->finalvalues[state]*(x->max-x->min);
	  outlet_float(x->x_obj.ob_outlet,f);
	  if (x->s_sym != &s_) pd_float(x->s_sym->s_thing, f);
	  return;
     }

     val = x->finalvalues[state-1] + 
		  (f - x->duration[state-1])*
		  (x->finalvalues[state] - x->finalvalues[state-1])/ 
		  (x->duration[state] - x->duration[state-1]);

     val *= (x->max - x->min);
     outlet_float(x->x_obj.ob_outlet,val);
     if (x->s_sym != &s_) pd_float(x->s_sym->s_thing, val);
}
*/

static void bindsym(t_pd* x,t_symbol* o,t_symbol* s)
{
     if (o != &s_) pd_unbind(x,o);
     o = s;
     pd_bind(x,s);
}


static void *breakpoints_new(t_symbol *s,int argc,t_atom* argv)
{
     t_breakpoints *x = (t_breakpoints *)pd_new(breakpoints_class);
     
     
     x->state = 0;
     
     x->borderwidth = 2;
     
     x->x_f = 0;
     x->args = STATES;
     x->finalvalues = getbytes( x->args*sizeof(t_float));
     x->duration = getbytes( x->args*sizeof(t_float));
#ifdef DEBUG
     post("finalvalues %x",x->finalvalues);
#endif
     /* widget */
     
     x->w.grabbed = 0;
     x->resizing = 0;
     x->resizeable = 0;
     x->w.glist = (t_glist*) canvas_getcurrent();
     
     x->w.width = 200;
     if (argc) x->w.width = atom_getfloat(argv++),argc--;
     x->w.height = 140;
     if (argc) x->w.height = atom_getfloat(argv++),argc--;
     
     t_float initialDuration = 100;
     if (argc) initialDuration = atom_getfloat(argv++),argc--;
         
     x->r_sym = &s_;
     if (argc) {
       t_symbol* n;

       n = atom_getsymbol(argv++);
       bindsym(&x->x_obj.ob_pd,x->r_sym,n);
       x->r_sym = n;
       argc--;
     }
     #ifdef DEBUG
     post("recv %s",x->r_sym->s_name);
     #endif
     /*
     x->s_sym = &s_;
     if (argc) x->s_sym = atom_getsymbol(argv++),argc--;
     #ifdef DEBUG
     post("send %s",x->s_sym->s_name);
     #endif
     */
     x->d_sym = &s_;
     if (argc) x->d_sym = atom_getsymbol(argv++),argc--;
     #ifdef DEBUG
     post("dump %s",x->d_sym->s_name);
     #endif 
     
     x->c_sym = &s_;
     if (argc) x->c_sym = atom_getsymbol(argv++),argc--;
     
     if (argc>2)
	  breakpoints_init(x,argc,argv);
     else {
	  t_atom a[5];
	  SETFLOAT(a,0);
	  SETFLOAT(a+1,50);
	  SETFLOAT(a+2,1);
	  SETFLOAT(a+3,50);
	  SETFLOAT(a+4,0);
	  breakpoints_init(x,5,a);
     }	 
     
     x->x_val = 0.0;
     x->x_state = NONE;
     x->sustain_state = SUSTAIN;
     x->x_freeze = 0;
     
     breakpoints_totaldur(x,initialDuration);
     
     outlet_new(&x->x_obj, gensym("signal"));
     //outlet_new(&x->x_obj, &s_float);
     x->out2 = outlet_new(&x->x_obj, &s_float);
     x->out3 = outlet_new(&x->x_obj, &s_bang);
     
     //x->x_clock = clock_new(x, (t_method) breakpoints_tick);
     return (x);
}




static void breakpoints_motion(t_breakpoints *x, t_floatarg dx, t_floatarg dy);
static void breakpoints_click(t_breakpoints *x,
    t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl,
    t_floatarg alt);
static void breakpoints_key(t_breakpoints *x, t_floatarg f);

t_widgetbehavior breakpoints_widgetbehavior;



static void breakpoints_free(t_breakpoints *x) {
       
     
     if (x->r_sym != &s_)  pd_unbind(&x->x_obj.ob_pd, x->r_sym);
     
}

void breakpoints_tilde_setup(void)
{
    breakpoints_class = class_new(gensym("breakpoints~"), (t_newmethod)breakpoints_new, (t_method)breakpoints_free,
    	sizeof(t_breakpoints), 0,A_GIMME,0);

   CLASS_MAINSIGNALIN(breakpoints_class, t_breakpoints, x_f);
    	/* here we tell Pd about the "dsp" method, which is called back
	when DSP is turned on. */
    class_addmethod(breakpoints_class, (t_method)breakpointssig_dsp, gensym("dsp"), 0);


    //class_addcreator((t_newmethod)breakpoints_new,gensym("breakpoints~"),A_GIMME,0);
    //class_addfloat(breakpoints_class, breakpoints_float);

    //class_addbang(breakpoints_class,breakpoints_bang);
    class_addlist(breakpoints_class,breakpoints_list);
    //class_addmethod(breakpoints_class,(t_method)breakpoints_sustain,gensym("sustain"),A_FLOAT,A_NULL);

    class_addmethod(breakpoints_class, (t_method)breakpoints_motion, gensym("motion"),A_FLOAT, A_FLOAT, 0);
    class_addmethod(breakpoints_class, (t_method)breakpoints_key, gensym("key"),
    	A_FLOAT, 0);

    class_addmethod(breakpoints_class,(t_method)breakpoints_totaldur,gensym("duration"),A_FLOAT,NULL);
    //class_addmethod(breakpoints_class,(t_method)breakpoints_freeze,gensym("freeze"),A_FLOAT,NULL);
    class_addmethod(breakpoints_class,(t_method)breakpoints_setresize,gensym("resize"),A_FLOAT,A_NULL);

    //class_addmethod(breakpoints_class,(t_method)breakpoints_release,gensym("release"),A_NULL);
    breakpoints_widgetbehavior.w_getrectfn =   breakpoints_getrect;
    breakpoints_widgetbehavior.w_displacefn =    breakpoints_displace;
    breakpoints_widgetbehavior.w_selectfn = breakpoints_select;
    //breakpoints_widgetbehavior.w_activatefn =   breakpoints_activate;
    breakpoints_widgetbehavior.w_activatefn =   NULL;
    breakpoints_widgetbehavior.w_deletefn =   breakpoints_delete;
    breakpoints_widgetbehavior.w_visfn =   breakpoints_vis;
    breakpoints_widgetbehavior.w_clickfn = (t_clickfn) breakpoints_newclick;
    //breakpoints_widgetbehavior.w_clickfn = NULL;
#if PD_MINOR_VERSION < 37
    breakpoints_widgetbehavior.w_propertiesfn = NULL;
    //breakpoints_widgetbehavior.w_savefn =   breakpoints_save;
    breakpoints_widgetbehavior.w_savefn =  NULL;
#endif

    class_setwidget(breakpoints_class,&breakpoints_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    //class_setsavefn(breakpoints_class,&breakpoints_save);
#endif
    class_addmethod(breakpoints_class,(t_method)breakpoints_dump,gensym("dump"),A_NULL);


}


