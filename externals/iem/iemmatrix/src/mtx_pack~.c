#include "iemmatrix.h"
#define MTX_PACK_MAXCHANNELS 200

static t_class *mtx_pack_tilde_class;

typedef struct _mtx_pack_tilde {
   t_object x_obj;
   int block_size;
   int num_chan;
   t_float **sig_in;
   t_atom *list_out;
   t_outlet *message_outlet;
} mtx_pack_tilde;

void *newMtxPackTilde (t_floatarg f)
{
   int num_chan=1;
   mtx_pack_tilde *x = (mtx_pack_tilde*) pd_new(mtx_pack_tilde_class);
   num_chan=(int)f;
   if ((num_chan<1) || (num_chan>MTX_PACK_MAXCHANNELS)) {
      num_chan=1;
   }
   x->num_chan=num_chan;
   x->sig_in=0;
   x->list_out=0;
   while (num_chan--) {
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
   }
   x->sig_in = (t_float**)getbytes(sizeof(t_float*)*x->num_chan);
   x->message_outlet=(t_outlet*)outlet_new(&x->x_obj,&s_list);

   return (void *) x;
}
void deleteMtxPackTilde (mtx_pack_tilde *x)
{
   if (x->sig_in)
      freebytes (x->sig_in, x->num_chan * sizeof (t_float));
   if (x->list_out)
      freebytes (x->list_out, (x->num_chan * x->block_size + 2)*sizeof(t_atom));
}
static t_int *mTxPackTildePerform (t_int *arg)
{
   mtx_pack_tilde *x = (mtx_pack_tilde *) (arg[1]);
   int chan;
   int samp;
   t_atom *lptr=x->list_out+2;
   SETFLOAT(x->list_out,(t_float)x->num_chan);
   SETFLOAT(x->list_out+1,(t_float)x->block_size);

   for (chan=0; chan<x->num_chan; chan++) {
      for (samp=0; samp<x->block_size; samp++,lptr++) {
         SETFLOAT(lptr, x->sig_in[chan][samp]);
      }
   }

   outlet_anything(x->message_outlet,gensym("matrix"),
         x->block_size*x->num_chan+2,x->list_out);

   return(arg+2);

}

static void mTxPackTildeDsp (mtx_pack_tilde *x, t_signal **sp)
{
   int chan;
   for (chan=0; chan<x->num_chan; chan++)
       x->sig_in[chan]=sp[chan]->s_vec;

   x->block_size=sp[0]->s_n;
   x->list_out = (t_atom*) getbytes ((x->num_chan * x->block_size + 2) *sizeof(t_atom));
   dsp_add(mTxPackTildePerform,1,x);
}

void mtx_pack_tilde_setup (void)
{
   mtx_pack_tilde_class = class_new(gensym("mtx_pack~"), (t_newmethod)newMtxPackTilde, (t_method) deleteMtxPackTilde, sizeof(mtx_pack_tilde), CLASS_NOINLET, A_DEFFLOAT, 0);
   class_addmethod (mtx_pack_tilde_class, (t_method) mTxPackTildeDsp, gensym("dsp"),0);
}

void iemtx_pack__setup(void)
{
   mtx_pack_tilde_setup();
}


