/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <g_canvas.h>

/* ------------------------ qread ----------------------------- */

#include <stdio.h>

static t_class *qread_class;


#define MAXLINE 255

typedef struct _qread
{
     t_object x_obj;
     FILE* x_file;
     int x_size;
     t_clock*  x_clock;
     t_glist * x_glist;
     int x_num;
     t_symbol* x_name;
} t_qread;



static void qread_open(t_qread *x,t_symbol *filename)
{
     char fname[MAXPDSTRING];

     if (filename == &s_) {
	  post("sfread: open without filename");
	  return;
     }

     canvas_makefilename((void*)glist_getcanvas(x->x_glist), filename->s_name,
			 fname, MAXPDSTRING);


     /* close the old file */

     if (x->x_file) fclose(x->x_file);

     if (!(x->x_file = sys_fopen(fname,"r")))
     {
	  error("can't open %s",fname);
	  return;
     }


}

void qread_next(t_qread *x)
{
     int i;
     float delay;
     char name[MAXLINE];
     t_atom at[20];
     int ac=0;
     t_floatarg ff;

     if (!x->x_file) return;

     fscanf(x->x_file,"%f",&delay);
     if (feof(x->x_file)) {
	  clock_unset(x->x_clock);
	  return;
     }

     fscanf(x->x_file,"%s",name);
#ifdef DEBUG
     post("next: name = %s delay = %f",name,delay);
#endif

     for (i=0;i<=x->x_num  && !feof(x->x_file);i++) {
	  fscanf(x->x_file,"%f",&ff);
	  SETFLOAT(at+i,ff);
     }
     ac = i-1;
     fscanf(x->x_file,";");


     clock_delay(x->x_clock,delay);
     
     outlet_list(x->x_obj.ob_outlet, gensym(name), ac, at);
}

static void qread_bang(t_qread *x)
{
     if (!x->x_file) return;
     
     fseek(x->x_file,0,SEEK_SET);
     clock_delay(x->x_clock,0);

#ifdef DEBUG
     post("bang");
#endif
}

static void qread_stop(t_qread *x)
{
     clock_unset(x->x_clock);
}

static void *qread_new(t_floatarg n)
{
    t_qread *x = (t_qread *)pd_new(qread_class);
    outlet_new(&x->x_obj, &s_float);

    x->x_name = gensym("qread");
    x->x_glist = (t_glist*) canvas_getcurrent();
    x->x_clock = clock_new(x, (t_method)qread_next);
    x->x_file = NULL;
    x->x_num = n;
    return (x);
}

void qread_setup(void)
{
    qread_class = class_new(gensym("qread"), (t_newmethod)qread_new, 0,
				sizeof(t_qread), 0,A_DEFFLOAT,A_NULL);
    class_addbang(qread_class,qread_bang);
    class_addmethod(qread_class,(t_method)qread_next,gensym("next"),A_NULL);
    class_addmethod(qread_class,(t_method)qread_open,gensym("open"),A_SYMBOL,A_NULL);
    class_addmethod(qread_class,(t_method)qread_stop,gensym("stop"),A_NULL);
}


