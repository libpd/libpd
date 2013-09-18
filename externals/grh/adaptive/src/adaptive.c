/******************************************************
 *
 * Adaptive Systems for PD
 *
 * copyleft (c) Gerda Strobl, Georg Holzmann
 * 2005
 *
 * for complaints, suggestions: grh@mur.at
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

#include "adaptive.h"

#ifdef ADAPTIVE_SINGLE_OBJ
// for single externals disable the adaptive object
#else
// build as library

typedef struct _adaptive 
{
  t_object x_obj;
} t_adaptive;

t_class *adaptive_class = NULL;

static void adaptive_help(void)
{
  post("\n-----------------------------------------------");
  post("adaptive systems for PD");
  post("copyleft (c) Gerda Strobl, Georg Holzmann, 2005");
  post("");
  post("for more info look at the help patches!");
  post("-----------------------------------------------\n");
}

void *adaptive_new(void)
{
  t_adaptive *x = (t_adaptive *)pd_new(adaptive_class);
  return (void *)x;
}

//-----------------------------------------------------
// declaration of the setup functions:
void lms_tilde_setup();
void lms2_tilde_setup();
void nlms_tilde_setup();
void nlms2_tilde_setup();
void nlms3_tilde_setup();
//-end-of-declaration----------------------------------

void adaptive_setup(void) 
{
  //---------------------------------------------------
  // call all the setup functions:
  lms_tilde_setup();
  lms2_tilde_setup();
  nlms_tilde_setup();
  nlms2_tilde_setup();
  nlms3_tilde_setup();
  //-end-----------------------------------------------

  post("\nadaptive: 2005 by Gerda Strobl and Georg Holzmann");
 
  adaptive_class = class_new(gensym("adaptive"), adaptive_new, 0, sizeof(t_adaptive), 0, 0);
  class_addmethod(adaptive_class, (t_method)adaptive_help, gensym("help"), 0);
}

#endif // library

/* ---------------------- helpers ----------------------- */

void adaptation_write(const char *filename, t_int N, t_float mu, t_float *c)
{
  FILE *f=0;
  int i;

  // open file
  f = sys_fopen(filename, "w");
  if(!f)
  {
    post("adaptive, save: error open file");
    return;
  }

  // write little header, number of coefficients and mu
  fprintf(f, "adaptivePD\n");
  fprintf(f, "size: %d\n", N);
  fprintf(f, "mu: %.30f\n", mu);
  
  // write coefficients
  for(i=0; i<N; i++)
    fprintf(f, "%.30f\n", c[i]);

  // close file
  if (f) fclose(f);
  post("adaptive, save: coefficients written to file");
}

void adaptation_read(const char *filename, t_int *N, t_float *mu, 
                     t_float *c, t_float *buf)
{
  FILE *f=0;
  int i, n=0;

  // open file
  f = sys_fopen(filename, "r");
  if(!f)
  {
    post("adaptive, open: error open file");
    return;
  }
  
  // read header and nr of coefficients
  if ( fscanf(f,"adaptivePD\n") != 0) 
  {
    post("adaptive, open: error in reading file");
    return;
  }
  if ( fscanf(f,"size: %d\n",&n) != 1)
  {
    post("adaptive, open: error in reading file");
    return;
  };
  
  // change size of the filter if needed
  if(n != *N)
  {
    if(c) freebytes(c, sizeof(t_float) * (*N));
    if(buf) freebytes(buf, sizeof(t_sample) * (*N-1));
    
    *N = (n<=0) ? 1 : n;
   
    post("WARNING (adaptive): Nr. of coefficients is changed to %d!",*N);
     
    // allocate mem and init coefficients
    c = (t_float *)getbytes(sizeof(t_float) * (*N));
    
    // allocate mem for temp buffer
    buf = (t_sample *)getbytes(sizeof(t_sample) * (*N-1));
    for(i=0; i<(*N-1); i++)
      buf[i] = 0;
  }
  
  // read mu
  if ( fscanf(f,"mu: %f\n",mu) != 1)
  {
    post("adaptive, open: error in reading file");
    return;
  };

  // get coefficients:
  for(i=0; i<(*N); i++)
    if( fscanf(f, "%f\n", c+i) != 1)
    {
      post("adaptive, open: error in reading file");
      return;
    }
    //post("c_inside: %d",c);
  post("adaptive, read: coefficients readed from file");  
}
