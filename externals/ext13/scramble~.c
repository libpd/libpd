#include "m_pd.h"
#include "ext13.h"
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <netdb.h>
#include <sys/errno.h>
#include <sys/socket.h>
#endif

/* ------------------------ scramble_tilde~ ----------------------------- */

static t_class *scramble_tilde_class;

typedef struct _scramble_grain
{
    t_float* L;
    t_float* R;
    int size;
    t_float maximum;
    struct _scramble_grain *next;
}t_scramble_grain;

typedef struct _scramble_tilde
{
     t_object x_obj;
     int x_n;
     t_int x_channels;
     t_float play, analize;
     t_float dir, current_dir, pitch, actual_pitch, grain_r, autopitch;
     t_float valsum, valavg;
     int valsumcount, valsummax;
     int autofollow, playmode, semitones;
     t_scramble_grain *firstgrain;
     t_scramble_grain *workgrain;
     t_scramble_grain *ring;
     int grains, newgrains, w_grain, r_grain, n_grain, gotagrain, flush;
     int r_d, w_d, dist, mindist, lowptr, hiptr, lastlowptr;
     t_float lowval, hival, prevval, lowborder, normalize;
     int nsamples;
     t_outlet *trigger1;
     t_outlet *trigger2;
} t_scramble_tilde;


/* grain functions*/
static t_scramble_grain* scramble_tilde_newgrain(){
     t_scramble_grain* thegrain;
     thegrain = getbytes( sizeof(t_scramble_grain));
     thegrain->L = NULL;
     thegrain->R = NULL;
     thegrain->size = 0;
     thegrain->next = NULL;
     return (thegrain);
}


static t_scramble_grain* scramble_tilde_getgrain(t_scramble_grain* firstgrain, int n){
     t_scramble_grain* thegrain = firstgrain;
     while (n--){
     if (thegrain->next){
         thegrain = thegrain->next;
     }else
         return (NULL);
     }
     return (thegrain);
}

static int scramble_tilde_getsomegrain(t_scramble_grain* firstgrain,int g){
    t_scramble_grain* thegrain ;
    int r;
    do{
      r =  rand() % g;
      thegrain = scramble_tilde_getgrain(firstgrain, r);
    }while (thegrain->size == 0);
    return (r);
}

static void scramble_tilde_grainbuf(t_scramble_grain* grain, int c, int n){
     if (!grain->size)
        grain->L =  getbytes(n * sizeof(t_float));
     else
        grain->L =  resizebytes(grain->L, grain->size * sizeof(t_float), n * sizeof(t_float)); 

     if (c == 2){
         if (!grain->size)
             grain->R =  getbytes(n * sizeof(t_float));
         else
             grain->R =  resizebytes(grain->R, grain->size * sizeof(t_float), n * sizeof(t_float));
     }
     grain->size = n;
}

static void scramble_tilde_freegrain(t_scramble_grain* grain, int c){
     if (grain->size){
        freebytes(grain->L, grain->size * sizeof(t_float));
        if (c == 2) freebytes(grain->R, grain->size * sizeof(t_float));
        grain->size = 0;
        grain->next = NULL;
     }
}


t_int *scramble_tilde_perform(t_int *w)
{
    t_scramble_tilde*  x = (t_scramble_tilde*)(w[1]);
    int i;
    int erg=0;
    int n;
    t_float val, valL, valR, killval;
#ifndef _WIN32
    t_float* out[x->x_channels];
    t_float* in[x->x_channels];
#else
    t_float** out = (t_float**)malloc(x->x_channels*sizeof(t_float*));
    t_float** in = (t_float**)malloc(x->x_channels*sizeof(t_float*));
#endif

    float n_factor, frac,  a,  b,  c,  d, cminusb;
    int index;
    float *fp;
    t_atom at[2];

    /* anything changed?*/
    if (x->flush){
      int i = x->grains;
      x->flush = 0;
      x->gotagrain = 0;
      while (i--)
        scramble_tilde_grainbuf(scramble_tilde_getgrain(x->firstgrain,i),x->x_channels,0);
    }
    
    if (x->newgrains){
      int tmp = x->grains;
      if (x->newgrains > x->grains){
        x->workgrain = scramble_tilde_getgrain(x->firstgrain,x->grains - 1); /*-1 ???*/
        tmp = x->newgrains;
        x->newgrains -= x->grains;
        x->grains = tmp;
        while (x->newgrains--){
           x->workgrain->next = scramble_tilde_newgrain();
           x->workgrain = x->workgrain->next;
        }
 //       post ("now %d grains",x->grains);
      }else{
        if (x->newgrains < x->grains){
           t_scramble_grain*  tmpgrain;

           x->grains = x->newgrains;
           x->workgrain = scramble_tilde_getgrain(x->firstgrain,x->grains - 1);

           /* delete old grains */
           while (x->workgrain->next){
             tmpgrain = x->workgrain->next;
             scramble_tilde_freegrain(x->workgrain,x->x_channels);
             x->workgrain = tmpgrain;
           }

           /* reset readpointer if needed*/
           if (x->r_grain >=  x->grains){
             x->r_grain = 0;
             x->grain_r = -1;
           }
//           post ("now %d grains",x->grains);
        }
      }
      x->newgrains=0;
    }

    if ((x->ring->size > x->x_n) || (x->ring->size < x->x_n) ){
//       post ("scramble~: new size for ringbuffer:%d samples, %d channels, oldsize:%d",x->x_n,x->x_channels,x->ring->size);
       scramble_tilde_grainbuf(x->ring, x->x_channels ,x->x_n);
       x->x_n = x->ring->size;
       x->dist = 0;
       x->lowptr = x->r_d;
       x->lastlowptr = -1;
       x->lowval = x->lowborder;
    }
    
    for (i = 0; i < x->x_channels ;i++)
       in[i] = (t_float *)(w[2 + i]);

    for (i = 0;i < x->x_channels ;i++)
       out[i] = (t_float *)(w[2 + x->x_channels + i]);

    n = (int)(w[2 + x->x_channels * 2]);/*number of samples*/
//    post ("n:%d",n);
    
    while (n--){
    /*read from input*/
       if (++x->r_d > x->x_n){
           x->r_d = 0;
       }
       valL = *(t_float*)(x->ring->L +  x->r_d) = *in[0]++;
       if (valL < 0) valL *= -1;
       if (x->x_channels == 2){
         valR = *(t_float*)(x->ring->R + x->r_d) = *in[1]++;
         if (valR < 0) valR *= -1;
         val =  valL + valR / 2.0;
         if (valL > x->hival){
            x->hiptr = x->r_d;
            x->hival = valL;
         }
         if (valR > x->hival){
            x->hiptr = x->r_d;
            x->hival = valR;
         }
       }else {
         val = valL;
          if (valL > x->hival){
            x->hiptr = x->r_d;
            x->hival = valL;
          }
       }

//       if (val < 0) val *= -1;

       x->valsum += val;
//       if (x->valsumcount++ > x->mindist * 10){
       if (x->valsumcount++  && (x->r_d == 0)){
          x->valavg = x->valsum / x->valsumcount;
          x->valsumcount = 0;
          x->valsum = 0;
          if (x->autofollow && ( x->valavg > 0.003)) {
            x->lowborder = x->valavg;
//            post ("lowborder = %f",x->lowborder);
          }
       }

       if ((val < x->lowborder) && (x->prevval > x->lowborder)){
       /* a new low-period */
         x->dist = -1;
         x->lowptr =  x->r_d;
         x->lowval = val;
//           post ("low");
       }
       if ((x->r_d + 1) == x->lastlowptr){
         /* one round without a point to cut */
          x->lastlowptr = -1; 
          x->lowval = x->lowborder;
          x->hival = 0;
//          post ("lastlowptr: reset");
       }
       
       if (val < x->lowborder){x->dist++;}

       if (val <= x->lowval) {
          x->lowptr = x->r_d;
          x->lowval = val;
          /*found a point to cut*/
       }

       if ((val > x->lowborder) && (x->prevval < x->lowborder) && ( x->dist < x->mindist)){
         /*too short low-period*/
          x->dist = 0;
          x->lowptr = x->r_d;
          x->lowval = x->lowborder;
//         post ("low too short");
       }
       
       if ((val > x->lowborder) && (x->prevval < x->lowborder) && ( x->dist > x->mindist)){
       /*found a low-period to cut */
         if ((x->lastlowptr != -1) ){
           int grainlen = 0;
           int i = 0;
           int wp = 1; /*first and last sample of grain should be 0.0*/

           x->gotagrain = 1;
           /* how long is the new grain */
           if (x->lastlowptr > x->lowptr){
             grainlen = x->x_n - x->lastlowptr + x->lowptr;
           }else{
             grainlen = x->lowptr - x->lastlowptr;
           }
           
           if (x->analize){
             /*find and prepare the grain*/
             if (++x->w_grain >= x->grains ) x->w_grain = 0;
             x->workgrain = scramble_tilde_getgrain (x->firstgrain, x->w_grain);
             scramble_tilde_grainbuf(x->workgrain, x->x_channels, grainlen + 2);
  
             *(t_float*)(x->workgrain->L) = 0.0;
             *(t_float*)(x->workgrain->L + x->workgrain->size -1) = 0.0;
             if (x->x_channels == 2){
               *(t_float*)(x->workgrain->R) = 0.0;
               *(t_float*)(x->workgrain->R + x->workgrain->size -1) = 0.0;
             }
             x->workgrain->maximum = x->hival;

             /*notify the world*/
             SETFLOAT(at, grainlen);
             SETFLOAT(at+1, x->w_grain + 1);
             outlet_list(x->trigger1, 0, 2, at);

             /*copy to the grain*/
             i =  x->lastlowptr;
             while (grainlen--){
                if (++i >= x->x_n) i = 0;
                *(t_float*)(x->workgrain->L + wp ) = *(t_float*)(x->ring->L + i);
                if (x->x_channels == 2)
                  *(t_float*)(x->workgrain->R + wp ) = *(t_float*)(x->ring->R + i);
                wp++;
             }
           }/*end if analize*/
//           post ("copied: w_grain: %d",x->w_grain);
         }/* end lastlowptr != -1*/
         x->dist = 0;
         x->hival = 0;
         x->lastlowptr = x->lowptr; 
       }/*end found a low-period to cut */

       x->prevval = val;
   }/*end while n-- (read from input)*/


/*--------------------playback--------------*/
   n = (int)(w[2 + x->x_channels * 2]);/*number of samples*/

   x->workgrain = scramble_tilde_getgrain (x->firstgrain, x->r_grain);
   if (x->normalize && x->workgrain) n_factor = x->normalize / x->workgrain->maximum;
   else  n_factor = 1;
   
   while (n--){
     int wgs;
     if (x->workgrain) wgs = x->workgrain->size - 2;
     else wgs = 0;
     if (( (x->grain_r >=  wgs) || (x->grain_r < 1) || (x->workgrain == NULL) ) && x->play && x->gotagrain){
        if (x->playmode < 2){
            x->r_grain = scramble_tilde_getsomegrain(x->firstgrain, x->grains);
            x->workgrain = scramble_tilde_getgrain (x->firstgrain, x->r_grain);
        }else{
           if (x->n_grain == -1){
             x->play = 0;
             x->r_grain = 0;
             x->workgrain = NULL;
           }else{
              x->r_grain = x->n_grain;
              x->workgrain = scramble_tilde_getgrain (x->firstgrain, x->r_grain);
              x->n_grain = -1;
              if ((x->r_grain == x->w_grain) || (x->workgrain == NULL)){
                x->play = 0;
                x->r_grain = 0;
                x->workgrain = NULL;
              }  else if (!x->workgrain->size){
                x->play = 0;
                x->r_grain = 0;
                x->workgrain = NULL;
              }
           }
        }/*end if playmode < 2*/

        if (x->workgrain){
          if (((rand() % 200) / 100.0  - 1.0 ) < x->dir){
            x->current_dir = 1;
            x->grain_r = 1;
          }
          else{
            x->current_dir = -1;
            x->grain_r = x->workgrain->size -3;
          }

          if ( ( (x->autopitch >= 1.) && (x->semitones) ) || ( (x->autopitch) && (! x->semitones) ) ){
            if (x->semitones){
              int ap = (int)x->autopitch;
              int rauf = 0;
              int count ;

              if (rand() % 2 == 1){ rauf = 1;}
/*              post ("rauf:%d",rauf); */

              x->actual_pitch = x->pitch;

              for (count = (rand() % ap); count >= 0; count--){
                /*1.05946 = 12te wurzel aus 2 */
                if (rauf){
                  x->actual_pitch = x->actual_pitch * 1.05946;
                }else{
                  x->actual_pitch = x->actual_pitch / 1.05946;
                }
              }
            }else{
              if (((rand() % 200) / 100.0  - 1.0 ) > 0){ 
                 x->actual_pitch = x->pitch + x->pitch * ((rand() % 100 ) / 100.0 * x->autopitch);
              }else{
                 x->actual_pitch = x->pitch - x->pitch / ((rand() % 100 ) / 100.0 * x->autopitch);
              }
            }/*end if semitones*/
          } else {
            x->actual_pitch = x->pitch;
          }/* end if autopitch*/

/*          post ("x->actual_pitch:%f, x->autopitch:%f",x->actual_pitch,x->autopitch); */
          
          if (x->normalize) n_factor = x->normalize / x->workgrain->maximum;
          else  n_factor = 1;

          SETFLOAT(at, (x->workgrain->size - 2) / x->actual_pitch);
          SETFLOAT(at+1, x->r_grain + 1);
          outlet_list(x->trigger2, 0, 2, at);

        }/*end if workgrain !=NULL */
     }/* end finding a new grain*/
         
     if (x->play && x->gotagrain){
         /*write graincontent to output*/
         /* 4 point interpolation taken from ../src/d_array.c tabread4~ */
         index = x->grain_r;
         if (index < 1)
           index = 1, frac = 0;
         else if (index > x->workgrain->size - 3)
           index = x->workgrain->size - 3, frac = 1;
         else
            frac = x->grain_r - index;

         fp = (t_float*)(x->workgrain->L + index);
         a = fp[-1];
         b = fp[0];
         c = fp[1];
         d = fp[2];

         cminusb = c-b;
         *out[0]++ = (b + frac * (
                  cminusb - 0.5f * (frac-1.) * (
                    (a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
                  )
         )) * n_factor;

         if (x->x_channels == 2){
           fp = (t_float*)(x->workgrain->R + index);
           a = fp[-1];
           b = fp[0];
           c = fp[1];
           d = fp[2];
           cminusb = c-b;
           *out[1]++ = (b + frac * (
                      cminusb - 0.5f * (frac-1.) * (
                          (a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
                      )
           )) * n_factor;
         }
         x->grain_r += x->current_dir * x->actual_pitch;
     }else/* if play*/{
        *out[0]++ = 0;
        if (x->x_channels == 2)
          *out[1]++ = 0;
     }/*end if play */
   }/*end while n-- */
#ifdef _WIN32
   free(in);
   free(out);
#endif
   return (w + x->x_channels * 2 + 3);
}

static void scramble_tilde_dsp(t_scramble_tilde *x, t_signal **sp)
{
    switch (x->x_channels) {
       case 1:
          dsp_add(scramble_tilde_perform, 4, x, sp[0]->s_vec,
          sp[1]->s_vec, sp[0]->s_n);
//          post ("1 channel");
          break;
       case 2:
          dsp_add(scramble_tilde_perform, 6, x, sp[0]->s_vec,
          sp[1]->s_vec,sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
//          post ("2 channels");
          break;
     }
}


static void scramble_tilde_free(t_scramble_tilde *x){
  int n = x->grains - 1;
  while (n--){
    scramble_tilde_freegrain (scramble_tilde_getgrain(x->firstgrain,n),x->x_channels);
    scramble_tilde_freegrain (x->ring,x->x_channels);
  }
}


static void *scramble_tilde_new(t_floatarg c,t_floatarg b)
{
    t_scramble_tilde *x = (t_scramble_tilde *)pd_new(scramble_tilde_class);
    int i;
//    x->bufL = NULL;
//    x->bufR = NULL;
    x->x_channels = (t_int)c;
    if (x->x_channels > 2) {
       x->x_channels = 2;
       post ("maximum: 2 channels");
    }
    if (x->x_channels < 1) x->x_channels = 1;

    outlet_new(&x->x_obj, gensym("signal"));
    if (x->x_channels == 2){
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
      outlet_new(&x->x_obj, gensym("signal"));
    }

    x->trigger1 = outlet_new(&x->x_obj, &s_float);
    x->trigger2 = outlet_new(&x->x_obj, &s_float);
    x->dir = 1;
    x->pitch = 1;
    x->actual_pitch = 1;
    x->autopitch = 0;
    x->semitones = 1;
    x->autofollow = 1;
    x->playmode = 1;
    x->normalize = 0;
    x->analize = 1;
    x->flush = 0;
    x->x_n = (int)b;
    if (x->x_n >882000 ){x->x_n = 882000;}
    if (x->x_n < 88200 ){x->x_n = 88200;}
/*    x->rR = x->rL = x->wR = x->wL = NULL;*/
    x->lowptr = 0;
    x->lastlowptr = x->r_d = x->grain_r = -1;
    x->mindist = 1024;
    x->lowborder = 0.35;
//    scramble_tilde_tempbuf(x,x->x_n);
    x->ring = scramble_tilde_newgrain();
    scramble_tilde_grainbuf(x->ring,x->x_channels,x->x_n);

    x->valsum = x->valavg = x->valsumcount = 0;
    x->valsummax = 1024;

    /* the grains:*/
    x->grains = 50;
    x->r_grain = 0;
    x->w_grain = x->n_grain = -1;
    x->firstgrain = x->workgrain = scramble_tilde_newgrain();
    for (i = 1;i < x->grains;i++){
        x->workgrain->next = scramble_tilde_newgrain();
        x->workgrain = x->workgrain->next;
    }
    return (x);
}

void scramble_tilde_float(t_scramble_tilde* x, t_float n){
  x->play = n;
  if (x->playmode == 2) {
    x->n_grain = (int)n - 1;
    x->grain_r = -1;
  }
}

void scramble_tilde_buffer(t_scramble_tilde* x, t_float n){
  if (n > 64) x->x_n = (int)n;
//  post ("buffersize now:%d",x->x_n);
}

void scramble_tilde_threshold(t_scramble_tilde* x, t_float t){
    if (t >0) {
      x->lowborder = t;
      x->autofollow = 0;
    }else{
      post ("threshold must be a positive value (0.1 - 0.8 makes sense)");
    }
    
}

void scramble_tilde_grains(t_scramble_tilde* x, t_float g){
    if ((g > 1) && (g < 2048) ) x->newgrains = (int)g;
    else post ("scramble~: minimum # of grains must be 2 an maximum # is 2048");
}

void scramble_tilde_mindist(t_scramble_tilde* x, t_float t){
    if ((t > 0)  && (t < x->x_n)) x->mindist = (int)t;
    else post ("scramble~: minimum distance must be positive value lower than buffersize");
}

void scramble_tilde_direction(t_scramble_tilde* x, t_float d){
     if (d > 1) d = 1;
     if (d < -1) d = -1;
     x->dir = d;
}

void scramble_tilde_autofollow(t_scramble_tilde* x){
     x->autofollow = 1;
}

void scramble_tilde_pitch(t_scramble_tilde* x, t_float p){
     if (p > 0) x->pitch = p;
     else post ("scramble~: pitch must be  > 0");
}

void scramble_tilde_autopitch(t_scramble_tilde* x, t_float p){
     x->autopitch = p;
}

void scramble_tilde_semitones(t_scramble_tilde* x, t_float p){
     x->semitones = (int)p;
}


void scramble_tilde_normalize(t_scramble_tilde* x, t_float n){
     x->normalize = n;
}

void scramble_tilde_analize(t_scramble_tilde* x, t_float f){
     x->analize = f;
}

void scramble_tilde_flush(t_scramble_tilde* x){
    x->flush = 1;
}

void scramble_tilde_playmode(t_scramble_tilde* x, t_float p){
    x->playmode = (int)p;
    if (x->playmode < 0) x->playmode = 0;
    if (x->playmode < 1) x->playmode = 2;
    switch (x->playmode){
      case 0: post ("scramble~: playmode off");
              break;
      case 1: post ("scramble~: active playmode");
              break;
      case 2: post ("scramble~: passive playmode");
              break;
      default: post ("scramble~: invalid playmode");        
    }
}


void scramble_tilde_setup(void)
{
    scramble_tilde_class = class_new(gensym("scramble~"), (t_newmethod) scramble_tilde_new, 0,
    	sizeof(t_scramble_tilde), 0, A_DEFFLOAT,A_DEFFLOAT, 0);
    class_addfloat(scramble_tilde_class,scramble_tilde_float);
    class_addmethod(scramble_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_dsp, gensym("dsp"), 0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_buffer, gensym("buffer"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_threshold, gensym("threshold"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_grains, gensym("grains"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_mindist, gensym("min_length"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_direction, gensym("direction"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_autofollow, gensym("autofollow"),0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_pitch, gensym("pitch"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_autopitch, gensym("autopitch"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_semitones, gensym("semitones"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_flush, gensym("flush"), 0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_normalize, gensym("normalize"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_analize, gensym("analize"), A_DEFFLOAT,0);
    class_addmethod(scramble_tilde_class, (t_method) scramble_tilde_playmode, gensym("playmode"), A_DEFFLOAT,0);
}
