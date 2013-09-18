/* (C) Guenter Geiger <geiger@epy.co.at> */

#include <m_pd.h>

#define DEBUG(x)
//#define DEBUG(x) x
/* ------------------------ envgen~ ----------------------------- */

#define NONE    0
#define ATTACK  1
#define SUSTAIN -1 
#define STATES  100

#include "envgen.h"
#include "w_envgen.h"

static t_class *envgen_class;


#define OUT_LIST(x,nr,a) \
     outlet_list(x->x_obj.ob_outlet,&s_list,nr,(t_atom*)&a);\
     if (x->s_sym != &s_ && x->s_sym->s_thing) pd_list(x->s_sym->s_thing, &s_list, nr, (t_atom*)&a);


char dumpy[2000];

/* initialize envelope with argument vector */

#include <stdio.h>

/* 
   envgen crashes frequently when reallocating memory ....
   I really don't know why, it crashes during resizebytes,
   which means it is unable to realloc() the memory ?????
   the pointer seems to be ok, I don't know what else could
   cause the problem. for the moment we prevent from reallocating
   by setting the STATES variable to 100 */

void envgen_resize(t_envgen* x,int ns)
{
    DEBUG(post("envgen_resize"););
     if (ns > x->args) {
	  int newargs = ns*sizeof(t_float); 

	  x->duration = resizebytes(x->duration,x->args*sizeof(t_float),newargs);
	  x->finalvalues = resizebytes(x->finalvalues,x->args*sizeof(t_float),newargs);
	  x->args = ns;  
     }
}



void envgen_totaldur(t_envgen* x,t_float dur)
{
    DEBUG(post("envgen_totaldur"););
     int i;
     float f = dur/x->duration[x->last_state];

     if (dur < 10) {
         pd_error(x, "envgen: duration too small %f",dur);
         return;
     }

     for (i=1;i<=x->last_state;i++)
	  x->duration[i]*=f;
}


static void envgen_dump(t_envgen* e)
{
    DEBUG(post("envgen_dump"););
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
     
}

void envgen_init(t_envgen *x,int argc,t_atom* argv)
{
    DEBUG(post("envgen_init"););
     t_float* dur;
     t_float* val;
     t_float tdur = 0;

     if (!argc) return;

     x->duration[0] = 0;

     x->last_state = argc>>1;
     envgen_resize(x,argc>>1);

     dur = x->duration;
     val = x->finalvalues;

     if (argc) {
	  *val = atom_getfloat(argv++);
	  *dur = 0.0;
     }
     dur++;val++;argc--;
     for (;argc > 0;argc--) {
	  tdur += atom_getfloat(argv++);
	  DEBUG(post("dur =%f",tdur););
	  *dur++ = tdur; 
	  argc--;
	  if (argc > 0)
	       *val++ = atom_getfloat(argv++); 
	  else
	       *val++ = 0; 
      DEBUG(post("val =%f",*(val-1)););

     }

}





void envgen_list(t_envgen *x,t_symbol* s, int argc,t_atom* argv)
{
    DEBUG(post("envgen_list"););
     envgen_init(x,argc,argv);
     if (glist_isvisible(x->w.glist)) {
	  envgen_drawme(x, x->w.glist, 0);
     }
}

void envgen_setresize(t_envgen *x, t_floatarg f)
{
    DEBUG(post("envgen_setresize"););
     x->resizeable = f;
}


void envgen_float(t_envgen *x, t_floatarg f)
{
    DEBUG(post("envgen_float"););
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


void envgen_bang(t_envgen *x)
{
    DEBUG(post("envgen_bang"););
     t_atom   a[2];

     SETFLOAT(a,x->finalvalues[NONE]);
     SETFLOAT(a+1,0);

     OUT_LIST(x,2,a);

/*       we don't force the first value anymore, so the first value
       is actually with what we have left off at the end ...
       this reduces clicks
*/
     x->x_state = ATTACK;
     x->x_val = x->finalvalues[NONE];

     SETFLOAT(a,x->finalvalues[x->x_state]*(x->max-x->min));
     SETFLOAT(a+1,x->duration[x->x_state]);

     OUT_LIST(x,2,a);
     clock_delay(x->x_clock,x->duration[x->x_state]);
}

void envgen_release(t_envgen* x) {
    DEBUG(post("envgen_release"););
     t_atom a[2];
     float del = x->duration[x->x_state] - x->duration[x->x_state-1];
     if (x->x_state <= x->sustain_state) {
	x->x_state = x->sustain_state+1; /* skip sustain state */
     	clock_delay(x->x_clock,del);
        SETFLOAT(a,x->finalvalues[x->x_state]*(x->max-x->min));
        SETFLOAT(a+1,del);
        OUT_LIST(x,2,a);
     }
}

static void envgen_sustain(t_envgen *x, t_floatarg f)
{
    DEBUG(post("envgen_sustain"););
     if (f > 0 && f < x->last_state) 
        x->sustain_state = f;
     else
		 pd_error(x,"sustain value not betweem 0 and %f, ignoring message", x->last_state);
}


static void envgen_tick(t_envgen* x)
{
    DEBUG(post("envgen_tick"););
     t_atom a[2];
     x->x_state++;
     if (x->x_state <= x->last_state && x->x_state != x->sustain_state) {
	  float del = x->duration[x->x_state] - x->duration[x->x_state-1];
	  clock_delay(x->x_clock,del);
	  SETFLOAT(a,x->finalvalues[x->x_state]*(x->max-x->min));
	  SETFLOAT(a+1,del);
	  OUT_LIST(x,2,a);
     }
//     else
//	  clock_unset(x->x_clock);
}

static void envgen_freeze(t_envgen* x, t_floatarg f)
{
    DEBUG(post("envgen_freeze"););
     x->x_freeze = f;
}


static void bindsym(t_pd* x,t_symbol* o,t_symbol* s)
{
    DEBUG(post("bindsym"););
     if (o != &s_) pd_unbind(x,o);
     o = s;
     pd_bind(x,s);
}

static void envgen_free(t_envgen* x)
{
    clock_free(x->x_clock);
}

static void *envgen_new(t_symbol *s,int argc,t_atom* argv)
{
    DEBUG(post("envgen_new"););
     t_envgen *x = (t_envgen *)pd_new(envgen_class);
     
     x->args = STATES;
     x->finalvalues = getbytes( x->args*sizeof(t_float));
     x->duration = getbytes( x->args*sizeof(t_float));
     DEBUG(post("finalvalues %lx",x->finalvalues););
     
     /* widget */
     
     x->w.grabbed = 0;
     x->resizing = 0;
     x->resizeable = 1;
     
     x->w.glist = (t_glist*) canvas_getcurrent();
     
     x->w.width = 200;
     if (argc) x->w.width = atom_getfloat(argv++),argc--;
     x->w.height = 140;
     if (argc) x->w.height = atom_getfloat(argv++),argc--;
     x->max = 1.0;
     if (argc) x->max = atom_getfloat(argv++),argc--;
     x->min = 0.0;
     if (argc) x->min = atom_getfloat(argv++),argc--;

     x->r_sym = &s_;
     if (argc) {
       t_symbol* n;

       n = atom_getsymbol(argv++);
       bindsym(&x->x_obj.ob_pd,x->r_sym,n);
       x->r_sym = n;
       argc--;
     }
     //     post("recv %s",x->r_sym->s_name);

     x->s_sym = &s_;
     if (argc) x->s_sym = atom_getsymbol(argv++),argc--;
     //     post("send %s",x->s_sym->s_name);

     if (argc)
	  envgen_init(x,argc,argv);
     else {
	  t_atom a[5];
	  SETFLOAT(a,0);
	  SETFLOAT(a+1,50);
	  SETFLOAT(a+2,1);
	  SETFLOAT(a+3,50);
	  SETFLOAT(a+4,0);
	  envgen_init(x,5,a);
     }	 
     
     x->x_val = 0.0;
     x->x_state = NONE;
     x->sustain_state = SUSTAIN;
     x->x_freeze = 0;
     
     outlet_new(&x->x_obj, &s_float);
     x->out2 = outlet_new(&x->x_obj, &s_float);
     
     x->x_clock = clock_new(x, (t_method) envgen_tick);
     return (x);
}


void envgen_motion(t_envgen *x, t_floatarg dx, t_floatarg dy);
void envgen_click(t_envgen *x,
    t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl,
    t_floatarg alt);
void envgen_key(t_envgen *x, t_floatarg f);

t_widgetbehavior envgen_widgetbehavior;



void envgen_setup(void)
{
    DEBUG(post("envgen_setup"););
    envgen_class = class_new(gensym("envgen"),
                             (t_newmethod)envgen_new,
                             (t_method) envgen_free,
                             sizeof(t_envgen),
                             0,
                             A_GIMME,
                             0);

    class_addcreator((t_newmethod)envgen_new,gensym("envgen~"),A_GIMME,0);
    class_addfloat(envgen_class, envgen_float);

    class_addbang(envgen_class,envgen_bang);
    class_addlist(envgen_class,envgen_list);
    class_addmethod(envgen_class,(t_method)envgen_sustain,gensym("sustain"),A_FLOAT,A_NULL);

    class_addmethod(envgen_class, (t_method)envgen_motion, gensym("motion"),
    	A_FLOAT, A_FLOAT, 0);
    class_addmethod(envgen_class, (t_method)envgen_key, gensym("key"),
    	A_FLOAT, 0);

    class_addmethod(envgen_class,(t_method)envgen_totaldur,gensym("duration"),A_FLOAT,NULL);
    class_addmethod(envgen_class,(t_method)envgen_freeze,gensym("freeze"),A_FLOAT,NULL);
    class_addmethod(envgen_class,(t_method)envgen_setresize,gensym("resize"),A_FLOAT,A_NULL);

    class_addmethod(envgen_class,(t_method)envgen_release,gensym("release"),A_NULL);
    envgen_widgetbehavior.w_getrectfn =   envgen_getrect;
    envgen_widgetbehavior.w_displacefn =    envgen_displace;
    envgen_widgetbehavior.w_selectfn = envgen_select;
    envgen_widgetbehavior.w_activatefn =   envgen_activate;
    envgen_widgetbehavior.w_deletefn =   envgen_delete;
    envgen_widgetbehavior.w_visfn =   envgen_vis;
    envgen_widgetbehavior.w_clickfn = envgen_newclick;
#if PD_MINOR_VERSION < 37
    envgen_widgetbehavior.w_propertiesfn = NULL;
    envgen_widgetbehavior.w_savefn =   envgen_save;
#endif

    class_setwidget(envgen_class,&envgen_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(envgen_class,&envgen_save);
#endif
    class_addmethod(envgen_class,(t_method)envgen_dump,gensym("dump"),A_NULL);


}
