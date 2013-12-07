#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

/* ------------------------ sfread~ ----------------------------- */

#ifdef _WIN32
#define BINREADMODE "rb"
#else
#define BINREADMODE "r"
#endif

static t_class *sfread_class;


typedef struct _sfread
{
     t_object x_obj;
     void*     x_mapaddr;
     int       x_fd;

     t_int   x_play;
     t_int   x_channels;
     t_int   x_size;
     t_int   x_loop;
     t_float x_offset;
     t_float x_skip;
     t_float x_speed;

     t_glist * x_glist;
} t_sfread;


void sfread_open(t_sfread *x,t_symbol *filename)
{
     struct stat  fstate;
     char fname[MAXPDSTRING];

     canvas_makefilename(glist_getcanvas(x->x_glist), filename->s_name,
			 fname, MAXPDSTRING);


     /* close the old file */

     if (x->x_mapaddr) munmap(x->x_mapaddr,x->x_size);
     if (x->x_fd >= 0) close(x->x_fd);

     if ((x->x_fd = sys_open(fname,O_RDONLY)) < 0)
     {
	  error("can't open %s",fname);
	  return;
     }

     /* get the size */

     fstat(x->x_fd,&fstate);
     x->x_size = fstate.st_size;

     /* map the file into memory */

     if (!(x->x_mapaddr = mmap(NULL,x->x_size,PROT_READ,MAP_PRIVATE,x->x_fd,0)))
     {
	  error("can't mmap %s",fname);
	  return;
     }
}

#define MAX_CHANS 4

static t_int *sfread_perform(t_int *w)
{
     t_sfread* x = (t_sfread*)(w[1]);
     short* buf = x->x_mapaddr;
/*     t_float *in = (t_float *)(w[2]); will we need this (indexing) ?*/
     int c = x->x_channels;

     t_float offset = x->x_offset*c;
     t_float speed = x->x_speed;
     int i,n;
     t_float* out[MAX_CHANS];
     
     for (i=0;i<c;i++)  
	  out[i] = (t_float *)(w[3+i]);
     n = (int)(w[3+c]);
     
     /* loop */

     if (x->x_skip*c >  x->x_size/sizeof(short))
	  x->x_skip = x->x_size/sizeof(short);

     if (offset + n*c*speed > x->x_size/sizeof(short)) {
	  if (!x->x_loop) 
	       x->x_play=0;
	  else {
	       offset = x->x_skip;
	  }
     }

     if (offset + n*c*speed < 0) {
	  if (!x->x_loop) 
	       x->x_play=0;
	  else {
	       offset = x->x_size/(sizeof(short));
	  }
     }



     if (x->x_play && x->x_mapaddr) {
	  float aoff = (((int)offset)>>1)<<1;
	  while (n--) {
	       for (i=0;i<c;i++)  {
		    *out[i]++ = *(buf+(int)aoff+i)/32768.;
	       }
	       offset+=speed*c;
	       aoff = (((int)offset)>>1)<<1;
	  }
     }
     else {
	  while (n--) {
	       for (i=0;i<c;i++)
		    *out[i]++ = 0.;
	  }
     }
     x->x_offset = offset/c; /* this should always be integer !! */
     return (w+c+4);
}


static void sfread_float(t_sfread *x, t_floatarg f)
{
     int t = f;
     if (t) {
	  x->x_play=1;
     }
     else {
	  x->x_play=0;
     }

}

static void sfread_loop(t_sfread *x, t_floatarg f)
{
     x->x_loop = f;
}




static void sfread_bang(t_sfread* x)
{
     x->x_offset = x->x_skip*x->x_channels;
     sfread_float(x,1.0);
}


static void sfread_dsp(t_sfread *x, t_signal **sp)
{
/*     post("sfread: dsp"); */
     switch (x->x_channels) {
     case 1:
	  dsp_add(sfread_perform, 4, x, sp[0]->s_vec, 
		  sp[1]->s_vec, sp[0]->s_n);
	  break;
     case 2:
	  dsp_add(sfread_perform, 5, x, sp[0]->s_vec, 
		  sp[1]->s_vec,sp[2]->s_vec, sp[0]->s_n);
	  break;
     case 4:
	  dsp_add(sfread_perform, 6, x, sp[0]->s_vec, 
		  sp[1]->s_vec,sp[2]->s_vec,
		  sp[3]->s_vec,sp[4]->s_vec,
		  sp[0]->s_n);
	  break;
     }
}


static void *sfread_new(t_floatarg chan,t_floatarg skip)
{
    t_sfread *x = (t_sfread *)pd_new(sfread_class);
    t_int c = chan;

    x->x_glist = (t_glist*) canvas_getcurrent();

    if (c<1 || c > MAX_CHANS) c = 1;
    floatinlet_new(&x->x_obj, &x->x_skip);
    floatinlet_new(&x->x_obj, &x->x_speed);


    x->x_fd = -1;
    x->x_mapaddr = NULL;

    x->x_loop = 0;
    x->x_channels = c;
    x->x_mapaddr=NULL;
    x->x_offset = skip;
    x->x_skip = skip;
    x->x_speed = 1.0;
    x->x_play = 0;

    while (c--) {
	 outlet_new(&x->x_obj, gensym("signal"));
    }

/*  post("sfread: x_channels = %d, x_speed = %f",x->x_channels,x->x_speed);*/

    return (x);
}

/*
 * ------------------------------------------- sfwrite~ -------------------------------
 */



static t_class *sfwrite_class;

typedef struct _sfwrite
{
     t_object x_obj;
     t_symbol* filename;
     FILE*  x_file;

     t_int rec;
     t_int x_channels;
     t_int  size;
} t_sfwrite;


static void sfwrite_open(t_sfwrite *x,t_symbol *filename)
{
     post("sfwrite: open");
     x->filename = filename;
     post("sfwrite: filename = %s",x->filename->s_name);

     if ((x->x_file = sys_fopen(x->filename->s_name,"w")) < 0)
     {
	  error("can't create %s",filename->s_name);
	  return;
     }
}

static void sfwrite_float(t_sfwrite *x, t_floatarg f)
{
  int t = f;
  if (t) {
       post("sfwrite: start", f); 
       x->rec=1;
  }
  else {
       post("sfwrite: stop", f); 
       x->rec=0;
  }

}


static short out[4*64];

static t_int *sfwrite_perform(t_int *w)
{
     t_sfwrite* x = (t_sfwrite*)(w[1]);
     t_float * in[4];
     int c = x->x_channels;
     int i,n;
     short* tout = out;

     for (i=0;i < c;i++) {
	  in[i] = (t_float *)(w[2+i]);     
     }

     n = (int)(w[2+c]);

     /* loop */

     if (x->rec && x->x_file) {

	  while (n--) {
	       for (i=0;i<c;i++)  {
		    *tout++ = *(in[i])++ * 32768.;
	       }
	  }

	  fwrite(out,sizeof(short),64*c,x->x_file);
     }
     return (w+3+c);
}



static void sfwrite_dsp(t_sfwrite *x, t_signal **sp)
{
     post("sfwrite: dsp"); 
     switch (x->x_channels) {
     case 1:
	  dsp_add(sfwrite_perform, 3, x, sp[0]->s_vec, 
		   sp[0]->s_n);
	  break;
     case 2:
	  dsp_add(sfwrite_perform, 4, x, sp[0]->s_vec, 
		  sp[1]->s_vec, sp[0]->s_n);
	  break;
     case 4:
	  dsp_add(sfwrite_perform, 6, x, sp[0]->s_vec, 
		  sp[1]->s_vec,
		  sp[2]->s_vec,
		  sp[3]->s_vec,
		  sp[0]->s_n);
	  break;
     }
     post("sfwrite: dsp end"); 
}


static void *sfwrite_new(t_floatarg chan)
{
    t_sfwrite *x = (t_sfwrite *)pd_new(sfwrite_class);
    t_int c = chan;

    post("sfwrite: x_channels = %d, ",c);

    if (c<1 || c > MAX_CHANS) c = 1;
    x->x_channels = c--;
    x->x_file=NULL;
    x->rec = 0;
    while (c--) {
	 inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    }


    return (x);
}

void sfwrite_setup(void)
{
     sfwrite_class = class_new(gensym("sfwrite~"), (t_newmethod)sfwrite_new, 0,
    	sizeof(t_sfwrite), 0,A_DEFFLOAT,0);
     class_addmethod(sfwrite_class,nullfn,gensym("signal"), 0);
     class_addmethod(sfwrite_class, (t_method) sfwrite_dsp, gensym("dsp"), 0);
     class_addmethod(sfwrite_class, (t_method) sfwrite_open, gensym("open"), A_SYMBOL,A_NULL);
     class_addfloat(sfwrite_class, sfwrite_float);
     
}


void sfread_setup(void)
{
     /* sfread */

    sfread_class = class_new(gensym("sfread~"), (t_newmethod)sfread_new, 0,
    	sizeof(t_sfread), 0,A_DEFFLOAT,A_DEFFLOAT,0);
    class_addmethod(sfread_class, nullfn, gensym("signal"), 0);
    class_addmethod(sfread_class, (t_method) sfread_dsp, gensym("dsp"), 0);
    class_addmethod(sfread_class, (t_method) sfread_open, gensym("open"), A_SYMBOL,A_NULL);
    class_addfloat(sfread_class, sfread_float);
    class_addbang(sfread_class,sfread_bang);
    class_addmethod(sfread_class,(t_method)sfread_loop,gensym("loop"),A_FLOAT,A_NULL);

}






