/*
  jdl@xdv.org, 200203
  calculate mean of buffer
  standard deviation + histogram
 */


#include "m_pd.h"
#include <math.h>
#include <stdlib.h>


/* cx.mean: calculate the mean from a table */
/* as defined by: add all samples together and divide by number of samples */
t_class *cxmean_class;

typedef struct _cxmean
{
  t_object  x_obj;
  t_symbol *x_arrayname;
  t_float   x_mean;
  float    *x_vec;
  t_float   f;
  int       x_nsampsintab;
} t_cxmean;

static void *cxmean_new(t_symbol *s)
{
  t_cxmean *x = (t_cxmean *)pd_new(cxmean_class);
  x->x_arrayname  = s;
  x->x_mean       = 0;
  outlet_new(&x->x_obj, &s_float);
  return (x);
}

static void cxmean_set(t_cxmean *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
    	if (*s->s_name) pd_error(x, "mean~: %s: no such array",
    	    x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_nsampsintab, &x->x_vec))
    {
    	error("%s: bad template for mean~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void cxmean_bang(t_cxmean *x)
{
  outlet_float(x->x_obj.ob_outlet,x->x_mean);
}

static void cxmean_mean(t_cxmean *x)
{
  // t_float *bl;
  t_garray *a;
  int cnt;
  t_float *fp;
  t_float xz = 0.;

  cnt = 0;
  if(!(a = (t_garray *)pd_findbyclass(x->x_arrayname,garray_class))) {
    pd_error(x, "%s: no such table", x->x_arrayname->s_name);
  }
  garray_getfloatarray(a,&x->x_nsampsintab,&x->x_vec);

  fp = x->x_vec;

  while(cnt < x->x_nsampsintab) {
    //post("cxc/mean.c: %f",*fp++);
    xz += *fp++;
    cnt++;
  }
#ifdef DEBUG
  post("cxc/mean.c: sampsum: %f",xz);
#endif
  x->x_mean = xz / x->x_nsampsintab;

  outlet_float(x->x_obj.ob_outlet, x->x_mean);
}

void cxmean_setup(void)
{
  cxmean_class = class_new(gensym("cxmean"),
			       (t_newmethod)cxmean_new,
			       0,
			       sizeof(t_cxmean),
			       CLASS_DEFAULT,
			       A_DEFSYM, 0);
  class_addcreator((t_newmethod)cxmean_new,gensym("cx.mean"),A_DEFSYM, 0);
  
  class_addmethod(cxmean_class, (t_method)cxmean_set,
		  gensym("set"), A_DEFSYM, 0);
  class_addmethod(cxmean_class, (t_method)cxmean_mean,
		  gensym("mean"), A_DEFSYM, 0);
  class_addbang(cxmean_class, cxmean_bang);
  class_sethelpsymbol(cxmean_class, gensym("statistics.pd"));
}


/* cx.avgdev: calculate the average deviation of an array from
   mean as input and array.
   takes mean as input, sum of abs values of sample-val - mean
   divided by number of samples
 */

t_class *cxavgdev_class;

typedef struct _cxavgdev
{
  t_object  x_obj;
  t_symbol *x_arrayname;
  t_float   x_avgdev;
  float    *x_vec;
  t_float   f;
  int       x_nsampsintab;
} t_cxavgdev;

static void *cxavgdev_new(t_symbol *s)
{
  t_cxavgdev *x = (t_cxavgdev *)pd_new(cxavgdev_class);
  x->x_arrayname  = s;
  x->x_avgdev       = 0;
  outlet_new(&x->x_obj, &s_float);
  return (x);
}

static void cxavgdev_set(t_cxavgdev *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
    	if (*s->s_name) pd_error(x, "mean~: %s: no such array",
    	    x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_nsampsintab, &x->x_vec))
    {
    	error("%s: bad template for mean~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void cxavgdev_bang(t_cxavgdev *x)
{
  outlet_float(x->x_obj.ob_outlet,x->x_avgdev);
}

static void cxavgdev_float(t_cxavgdev *x, t_float f)
{
  // t_float *bl;
  t_garray *a;
  int cnt;
  t_float *fp;
  t_float xz = 0.;
  t_float tz = 0.;

  cnt = 0;
  if(!(a = (t_garray *)pd_findbyclass(x->x_arrayname,garray_class))) {
    pd_error(x, "%s: no such table", x->x_arrayname->s_name);
  }
  garray_getfloatarray(a,&x->x_nsampsintab,&x->x_vec);

  fp = x->x_vec;

  while(cnt < x->x_nsampsintab) {
    tz = *fp++;
    tz = fabs(tz - f);
    xz += tz;
#ifdef DEBUG
    //post("cxc/mean.c: sampdeviation: %f",tz);
#endif
    cnt++;
  }
#ifdef DEBUG
  post("cxc/mean.c: avgsum: %f",xz);
#endif
  x->x_avgdev = xz / x->x_nsampsintab;

  outlet_float(x->x_obj.ob_outlet, x->x_avgdev);
}

void cxavgdev_setup(void)
{
  cxavgdev_class = class_new(gensym("cxavgdev"),
			       (t_newmethod)cxavgdev_new,
			       0,
			       sizeof(t_cxavgdev),
			       CLASS_DEFAULT,
			       A_DEFSYM, 0);

  class_addcreator((t_newmethod)cxavgdev_new,gensym("cx.avgdev"),A_DEFSYM, 0);
  class_addmethod(cxavgdev_class, (t_method)cxavgdev_set,
		  gensym("set"), A_DEFSYM, 0);
/*   class_addmethod(cxavgdev_class, (t_method)cxavgdev_mean, */
/* 		  gensym("mean"), A_DEFSYM, 0); */
  class_addfloat(cxavgdev_class, (t_method)cxavgdev_float);
  class_addbang(cxavgdev_class, cxavgdev_bang);
  class_sethelpsymbol(cxavgdev_class, gensym("statistics.pd"));
}

/* cx.stddev: calculate the standard deviation of an array from
   mean as input and array.
   square root of sum of power of sample - mean divided by num of
   samps - 1
 */

t_class *cxstddev_class;

typedef struct _cxstddev
{
  t_object  x_obj;
  t_symbol *x_arrayname;
  t_float   x_stddev;
  float    *x_vec;
  t_float   f;
  int       x_nsampsintab;
} t_cxstddev;

static void *cxstddev_new(t_symbol *s)
{
  t_cxstddev *x = (t_cxstddev *)pd_new(cxstddev_class);
  x->x_arrayname  = s;
  x->x_stddev       = 0;
  outlet_new(&x->x_obj, &s_float);
  return (x);
}

static void cxstddev_set(t_cxstddev *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
    	if (*s->s_name) pd_error(x, "mean~: %s: no such array",
    	    x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_nsampsintab, &x->x_vec))
    {
    	error("%s: bad template for mean~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void cxstddev_bang(t_cxstddev *x)
{
  outlet_float(x->x_obj.ob_outlet,x->x_stddev);
}

static void cxstddev_float(t_cxstddev *x, t_float f)
{
  // t_float *bl;
  t_garray *a;
  int cnt;
  t_float *fp;
  t_float xz = 0.;
  t_float tz = 0.;

  cnt = 0;
  if(!(a = (t_garray *)pd_findbyclass(x->x_arrayname,garray_class))) {
    pd_error(x, "%s: no such table", x->x_arrayname->s_name);
  }
  garray_getfloatarray(a,&x->x_nsampsintab,&x->x_vec);

  fp = x->x_vec;

  while(cnt < x->x_nsampsintab) {
    tz = *fp++;
    tz = pow(tz - f,2); // power of 2
    xz += tz;
#ifdef DEBUG
    //post("cxc/mean.c: sampdeviation: %f",tz);
#endif
    cnt++;
  }
#ifdef DEBUG
  post("cxc/mean.c: avgsum: %f",xz);
#endif
  x->x_stddev = sqrt(xz / (x->x_nsampsintab - 1));

  outlet_float(x->x_obj.ob_outlet, x->x_stddev);
}

void cxstddev_setup(void)
{
  cxstddev_class = class_new(gensym("cxstddev"),
			       (t_newmethod)cxstddev_new,
			       0,
			       sizeof(t_cxstddev),
			       CLASS_DEFAULT,
			       A_DEFSYM, 0);

  class_addcreator((t_newmethod)cxstddev_new,gensym("cx.stddev"),A_DEFSYM, 0);
  class_addmethod(cxstddev_class, (t_method)cxstddev_set,
		  gensym("set"), A_DEFSYM, 0);
/*   class_addmethod(cxstddev_class, (t_method)cxstddev_mean, */
/* 		  gensym("mean"), A_DEFSYM, 0); */
  class_addfloat(cxstddev_class, (t_method)cxstddev_float);
  class_addbang(cxstddev_class, cxstddev_bang);
  class_sethelpsymbol(cxstddev_class, gensym("statistics.pd"));
}

/* ---------- mean~ ---------- */
/* output the mean as a signal */

t_class *mean_tilde_class;

typedef struct _mean_tilde
{
  t_object  x_obj;
  t_symbol *x_arrayname;
  t_float   x_mean;
  float    *x_vec;
  t_float   f;
  int       x_nsampsintab;
} t_mean_tilde;

static void *mean_tilde_new(t_symbol *s)
{
  t_mean_tilde *x = (t_mean_tilde *)pd_new(mean_tilde_class);
  x->x_arrayname  = s;
  x->x_mean       = 0;
  outlet_new(&x->x_obj, &s_float);
  return (x);
}

static t_int *mean_tilde_perform(t_int *w)
{
  t_mean_tilde *x = (t_mean_tilde *)(w[1]);
  //t_float *out = (t_float *)(w[3]), 
  t_float *fp;
  //// t_float *in  = (t_float *)(w[2]);
  //// *out = *in;
  int n = (int)(w[2]);
  t_float xz = 0.;
  fp = x->x_vec;
  while(n--) {
    xz += abs(*fp++);
    //post("cxc/mean.c: %d : %f : %f",n,xz,fp);
  }
  x->x_mean = (t_float)(xz / n);
  //post("cxc/mean.c: %f",xz);
  return (w+3);
  //return 0;
}

static void mean_tilde_set(t_mean_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
    	if (*s->s_name) pd_error(x, "mean~: %s: no such array",
    	    x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_nsampsintab, &x->x_vec))
    {
    	error("%s: bad template for mean~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void mean_tilde_dsp(t_mean_tilde *x, t_signal **sp)
{
  mean_tilde_set(x, x->x_arrayname);
  //dsp_add(mean_tilde_perform, 2, x, sp[0]->s_vec, sp[0]->s_n);
  //dsp_add(mean_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
  dsp_add(mean_tilde_perform, 2, x, sp[0]->s_n);
}

static void mean_tilde_bang(t_mean_tilde *x)
{
  outlet_float(x->x_obj.ob_outlet,x->x_mean);
}

static void mean_tilde_mean(t_mean_tilde *x)
{
  // t_float *bl;
  t_garray *a;
  int cnt;
  t_float *fp;
  t_float xz = 0.;

  cnt = 0;
  if(!(a = (t_garray *)pd_findbyclass(x->x_arrayname,garray_class))) {
    pd_error(x, "%s: no such table", x->x_arrayname->s_name);
  }
  garray_getfloatarray(a,&x->x_nsampsintab,&x->x_vec);

  fp = x->x_vec;

  while(cnt < x->x_nsampsintab) {
    //post("cxc/mean.c: %f",*fp++);
    xz += *fp++;
    cnt++;
  }
#ifdef DEBUG
  post("cxc/mean.c: sampsum: %f",xz);
#endif
  x->x_mean = xz / x->x_nsampsintab;

  outlet_float(x->x_obj.ob_outlet, x->x_mean);
}

void mean_tilde_setup(void)
{
  //post("mean~ setup");
  mean_tilde_class = class_new(gensym("mean~"),
			       (t_newmethod)mean_tilde_new,
			       0,
			       sizeof(t_mean_tilde),
			       CLASS_DEFAULT,
			       A_DEFSYM, 0);
  //CLASS_MAINSIGNALIN(mean_tilde_class, t_mean_tilde, f);
  class_addmethod(mean_tilde_class, nullfn, gensym("signal"), 0);
  class_addmethod(mean_tilde_class,
		  (t_method)mean_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(mean_tilde_class, (t_method)mean_tilde_set,
		  gensym("set"), A_DEFSYM, 0);
  class_addmethod(mean_tilde_class, (t_method)mean_tilde_mean,
		  gensym("mean"), A_DEFSYM, 0);
  class_addbang(mean_tilde_class, mean_tilde_bang);
}
