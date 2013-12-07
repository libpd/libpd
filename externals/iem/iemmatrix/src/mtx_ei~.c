#include "iemmatrix.h"

static t_class *mtx_ei_tilde_class;

typedef struct _mtx_ei_tilde {
   t_object x_obj;
   int bs;
   int sd;
   int sg;
   int se;
   t_float *g;
   t_float *sig_in_left;
   t_float *sig_in_right;
   t_float *sl;
   t_float *sr;
   t_float *ei;
   t_atom *list_out;
   t_outlet *message_outlet;
} mtx_ei_tilde;

void *newMtxEITilde(t_symbol *s, int argc, t_atom *argv)
{
   int sd=1;
   int sg=1;
   mtx_ei_tilde *x = (mtx_ei_tilde*) pd_new(mtx_ei_tilde_class);
   x->sig_in_left=0;
   x->sig_in_right=0;
   x->ei=0;
   x->list_out=0;
   x->g=0;
   inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
   inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
   x->message_outlet=(t_outlet*)outlet_new(&x->x_obj,&s_list);

   if (argc > 2) {
      sd = atom_getint(argv++);
      sg = argc-1;
      sg = (sg<0)?0:sg;
      sd = (sd<1)?1:sd;
   }
   x->sd=sd;
   x->sg=sg;
   x->se=(2*sd-1)*(2*sg-1);
   post("size delay %d, size gain %d",x->sd,x->sg);

   x->list_out = (t_atom*) getbytes ((x->se + 2) * sizeof(t_atom));
   
   if (x->sg) {
      x->g = (t_float*) getbytes (x->sg*sizeof(t_float));
      for (sg=0;sg<x->sg;sg++){ 
         x->g[sg] = atom_getfloat (argv++);
	 post("g[%d]=%f",sg,x->g[sg]);
      }
   }


   x->ei = (t_float*) getbytes (x->se * sizeof(t_float));

   return (void *) x;
}
void deleteMtxEITilde (mtx_ei_tilde *x)
{
   if (x->sl)
      freebytes (x->sl, x->sd * sizeof (t_float));
   if (x->sr)
      freebytes (x->sr, x->sd * sizeof (t_float));
   if (x->list_out)
      freebytes (x->list_out, (x->se + 2) * sizeof(t_atom));
   if (x->ei)
      freebytes (x->ei, x->se * sizeof(t_float));

}

t_float computeEIBlock (t_float *sl, t_float gl, t_float *sr, t_float gr, int n)
{
    t_float ei=0;
    t_float *exch;
    int n16 = (n >> 4);
    if ((gl!=1.0f)&&(gr!=1.0f)) {
       while(n16--) {
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
           ei += fabs(gl * *sl++ - gr * *sr++);
		n-=16;
       }
       while(n--) {
           ei += fabs(gl * *sl++ - gr * *sr++);
       }
     }
    else {
       if (gr==1.0f) {
	  exch = sr;
	  sr = sl;
	  sl = exch;
	  gr = gl;
       }
       while(n16--) {
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
           ei += fabs(*sl++ - gr * *sr++);
		n-=16;
       }
       while(n--) {
           ei += fabs(*sl++ - gr * *sr++);
       }
    }
    return ei;
}

static t_int *mTxEITildePerform (t_int *arg) 
{
   mtx_ei_tilde *x = (mtx_ei_tilde *) (arg[1]);
   int gi,zi,eii = 0;
   t_float scale=1.0f/x->bs;
   SETFLOAT(x->list_out,(t_float)2*x->sg-1);
   SETFLOAT(x->list_out+1,(t_float)2*x->sd-1);

   memcpy(x->sl, x->sl+x->bs, sizeof(t_float)*x->sd);
   memcpy(x->sr, x->sr+x->bs, sizeof(t_float)*x->sd);
   memcpy(x->sl+x->sd, x->sig_in_left, sizeof(t_float)*x->bs);
   memcpy(x->sr+x->sd, x->sig_in_right, sizeof(t_float)*x->bs);

   for (gi=0; gi<x->sg; gi++) {
       if (gi>0) {
           for (zi=0; zi<x->sd; zi++) {
               if (zi>0) 
                   x->ei[eii++]=computeEIBlock(&x->sl[zi],x->g[gi],
                           &x->sr[x->sd-zi],x->g[x->sg-gi],
                           x->bs);
               x->ei[eii++]=computeEIBlock(&x->sl[zi],x->g[gi],
                       &x->sr[x->sd-zi-1],x->g[x->sg-gi],
                       x->bs);
           }
       }
       for (zi=0; zi<x->sd; zi++) {
           if (zi>0) 
               x->ei[eii++]=computeEIBlock(&x->sl[zi],x->g[gi],
                       &x->sr[x->sd-zi],x->g[x->sg-gi-1],
                       x->bs);
           x->ei[eii++]=computeEIBlock(&x->sl[zi],x->g[gi],
                   &x->sr[x->sd-zi-1],x->g[x->sg-gi-1],
                   x->bs);
       }
   }

   for (eii=0; eii<x->se; eii++)
       SETFLOAT(x->list_out+eii+2, x->ei[eii]*scale);

   outlet_anything(x->message_outlet,gensym("matrix"),
         x->se+2,x->list_out);

   return(arg+2);

}

static void mTxEITildeDsp (mtx_ei_tilde *x, t_signal **sp)
{
   int chan;
   x->sig_in_left=sp[0]->s_vec;
   x->sig_in_right=sp[1]->s_vec;

   x->bs=sp[0]->s_n;

   if (x->sl)
      freebytes (x->sl, x->sd * sizeof (t_float));
   if (x->sr)
      freebytes (x->sr, x->sd * sizeof (t_float));

   if (x->sd) {
      x->sl = (t_float*) getbytes ((x->sd+x->bs)*sizeof(t_float));
      x->sr = (t_float*) getbytes ((x->sd+x->bs)*sizeof(t_float));
   }

   dsp_add(mTxEITildePerform,1,x);
}

void mtx_ei_tilde_setup (void)
{
   mtx_ei_tilde_class = class_new(gensym("mtx_ei~"), (t_newmethod)newMtxEITilde, (t_method) deleteMtxEITilde, sizeof(mtx_ei_tilde), CLASS_NOINLET, A_GIMME, 0);
   class_addmethod (mtx_ei_tilde_class, (t_method) mTxEITildeDsp, gensym("dsp"),0);
}

void iemtx_ei__setup(void)
{
   mtx_ei_tilde_setup();
}


