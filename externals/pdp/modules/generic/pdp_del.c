/*
 *   Pure Data Packet module.
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */



#include "pdp.h"


typedef struct pdp_del_struct
{
    t_object x_obj;
    t_float x_f;
  
    t_outlet *x_outlet0;

    t_outlet **x_outlet;

    int *x_packet;
    int x_order;
    int x_head;
    int x_delay;
} t_pdp_del;





static void pdp_del_input_0(t_pdp_del *x, t_symbol *s, t_floatarg f)
{
  int in;
  int out;
  int packet;
  
    /* if this is a register_ro message or register_rw message, register with packet factory */
    /* if this is a process message, start the processing + propagate stuff to outputs */

    if (s == gensym("register_ro")){
      in = (x->x_head % x->x_order);
      //post("pdp_del: marking unused packed id=%d on loc %d", x->x_packet[0], in);
      pdp_packet_mark_unused(x->x_packet[in]);
      packet = pdp_packet_copy_ro((int)f);


      // TODO TODO TODO !!!!

      //pdp_packet_print_debug((int)f);

      


      x->x_packet[in] = packet;
      //post("pdp_del: writing packed id=%d on loc %d", packet, in);
    }
    else if (s == gensym("process")){
      out = (((x->x_head + x->x_delay)) % x->x_order);
      packet = x->x_packet[out];

      // originally, we wouldn't keep the packet in the delay line to save memory
      // however, this behaviour is very annoying, and doesn't allow ''scratching''
      // so we send out a copy instead.
      // pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet[out]);
      int p = pdp_packet_copy_ro(packet);
      pdp_packet_pass_if_valid(x->x_outlet0, &p);


/*
      if (-1 != packet){
	//post("pdp_del: packet %d has id %d", out, packet);
	pdp_packet_mark_unused(packet);
	outlet_pdp(x->x_outlet0, packet);
	x->x_packet[out] = -1;
      }


      else {
	//post("pdp_del: packet %d is empty", out);
      }
*/

      x->x_head = (x->x_head + x->x_order - 1) % x->x_order;

/*
      int i;
      for (i=0; i<x->x_order; i++){
	  fprintf(stderr, " %d", x->x_packet[i]);
      }
      fprintf(stderr, "\n");
*/

    }


}





static void pdp_del_delay(t_pdp_del *x, t_floatarg fdel)
{
  int del = (int)fdel;
  if (del < 0) del = 0;
  if (del >= x->x_order) del = x->x_order - 1;

  x->x_delay = del;

}

static void pdp_del_reset(t_pdp_del *x)
{
  int i;
  for (i=0; i<x->x_order; i++) {
      pdp_packet_mark_unused(x->x_packet[i]);
      x->x_packet[i] = -1;
  }
  x->x_head = 0;

}

static void pdp_del_debug(t_pdp_del *x)
{
  int i;
  post ("order %d", x->x_order);
  post ("delay %d", x->x_delay);
  post ("head %d", x->x_head);
  for (i=0; i<x->x_order; i++) {
      post("%d ", x->x_packet[i]);
  }
}

static void pdp_del_free(t_pdp_del *x)
{
  pdp_del_reset(x);
  pdp_dealloc (x->x_packet);
}

t_class *pdp_del_class;



void *pdp_del_new(t_floatarg forder, t_floatarg fdel)
{
  int order = (int)forder;
  int del;
  int logorder;
  int i;
  t_pdp_del *x = (t_pdp_del *)pd_new(pdp_del_class);

  del = order;
  order++;

  if (del < 0) del = 0;
  if (order <= 2) order = 2;

  //post("pdp_del: order = %d", order);

  x->x_order = order;
  x->x_packet = (int *)pdp_alloc(sizeof(int)*order);
  for(i=0; i<order; i++) x->x_packet[i] = -1;
  x->x_delay = del;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("delay"));
  x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 


  return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_del_setup(void)
{


    pdp_del_class = class_new(gensym("pdp_del"), (t_newmethod)pdp_del_new,
			      (t_method)pdp_del_free, sizeof(t_pdp_del), 0, A_DEFFLOAT, A_NULL);
    
    class_addmethod(pdp_del_class, (t_method)pdp_del_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_del_class, (t_method)pdp_del_delay, gensym("delay"),  A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_del_class, (t_method)pdp_del_reset, gensym("reset"),  A_NULL);

    class_addmethod(pdp_del_class, (t_method)pdp_del_debug, gensym("_debug"),  A_NULL);

}

#ifdef __cplusplus
}
#endif
