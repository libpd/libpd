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


/*

  pdp_loop: a looping packet delay line
  messages:
     record x: start recording at position x (default = 0)
     stop:     stop recording
     float:    output packet at position
     bang:     output next packet
     rewind:   rewind
     loop:     set looping mode

*/


#include "pdp.h"
#include "pdp_internals.h"


typedef struct pdp_loop_struct
{
    t_object x_obj;
    t_float x_f;
  
    t_outlet *x_outlet0;
    t_outlet *x_outlet1;

    int *x_packet;
    int x_order;       /* size of the packet loop */
    int x_play_head;   /* next position to play back */
    int x_record_head; /* next position to record to */

    int x_loop;
    int x_recording_frames;    /* nb frames left to record */
    //int x_recording_shot; /* single frame recording is on */
} t_pdp_loop;





static void pdp_loop_input_0(t_pdp_loop *x, t_symbol *s, t_floatarg f)
{
  int in;
  int out;
  int packet;

  
  /* if recording is off, ignore packet */
  if ((!x->x_recording_frames)) return;

  /* store a packet on register ro */
  if (s == gensym("register_ro")){

      /* delete old & store new */
      in = x->x_record_head; //% x->x_order;
      pdp_packet_mark_unused(x->x_packet[in]);
      packet = pdp_packet_copy_ro((int)f);
      x->x_packet[in] = packet;

      /* advance head & decrease record counter */
      x->x_recording_frames--;
      x->x_record_head++;

      /* turn off recording if we are at the end */
      if (x->x_record_head == x->x_order) x->x_recording_frames = 0;
  }
}


static void pdp_loop_bang(t_pdp_loop *x){
    int out;
    int packet;

    out = x->x_play_head;

    /* don't play if we're at the end of the sequence and looping is disabled */
    if ((!x->x_loop) && (out >= x->x_order)) return;

    /* wrap index */
    out %=  x->x_order;

    /* output the current packet */
    packet = x->x_packet[out];
    outlet_float(x->x_outlet1, (float)out); // output location
    if (-1 != packet) outlet_pdp(x->x_outlet0, packet); // output packet

    /* advance playback head */
    x->x_play_head++;

}






static void pdp_loop_reset(t_pdp_loop *x)
{
  int i;
  for (i=0; i<x->x_order; i++) {
      pdp_packet_mark_unused(x->x_packet[i]);
      x->x_packet[i] = -1;
  }
  x->x_play_head = 0;
  x->x_record_head = 0;

}

static void pdp_loop_record(t_pdp_loop *x, t_floatarg fstart, t_floatarg fdur)
{
    int istart = (int)fstart;
    int idur = (int)fdur;
    istart %= x->x_order;
    if (istart<0) istart+= x->x_order;
    if (idur <= 0) idur = x->x_order - istart;

    x->x_record_head = istart;
    x->x_recording_frames = idur;
}

static void pdp_loop_store(t_pdp_loop *x, t_floatarg f)
{
    int i = (int)f;
    i %= x->x_order;
    if (i<0) i+= x->x_order;

    x->x_record_head = i;
    x->x_recording_frames = 1;
}

static void pdp_loop_seek(t_pdp_loop *x, t_floatarg f)
{
    int i = (int)f;
    i %= x->x_order;
    if (i<0) i+= x->x_order;

    x->x_play_head = i;
}

static void pdp_loop_seek_hot(t_pdp_loop *x, t_floatarg f)
{
    pdp_loop_seek(x, f);
    pdp_loop_bang(x);
}


static void pdp_loop_stop(t_pdp_loop *x)
{
    x->x_recording_frames = 0;
}

static void pdp_loop_loop(t_pdp_loop *x, t_floatarg f)
{
    if (f == 0.0f) x->x_loop = 0;
    if (f == 1.0f) x->x_loop = 1;

}
static void pdp_loop_free(t_pdp_loop *x)
{
  pdp_loop_reset(x);
  pdp_dealloc (x->x_packet);
}

static int pdp_loop_realsize(float f)
{
    int order = (int)f;
    if (order <= 2) order = 2;
    return order;
}


static void pdp_loop_resize(t_pdp_loop *x, t_floatarg f)
{
    int i;
    int order = pdp_loop_realsize(f);
    int *newloop;

    /* if size didn't change, do nothing */
    if (x->x_order == order) return;

    /* create new array */
    newloop = (int *)pdp_alloc(sizeof(int) * order);


    /* extend it */
    if (x->x_order < order){

	/* copy old packets */
	for (i=0; i<x->x_order; i++) newloop[i] = x->x_packet[i];

	/* loop extend the rest */
	for (i=x->x_order; i<order; i++) newloop[i] = pdp_packet_copy_ro(x->x_packet[i % x->x_order]);

    }

    /* or shrink it */
    else {
	/* copy part of old packets */
	for (i=0; i<order; i++) newloop[i] = x->x_packet[i];

	/* delete the other part of old packets */
	for (i=order; i<x->x_order; i++) pdp_packet_mark_unused(x->x_packet[i]);

	/* adjust heads */
	x->x_play_head %= order;
	x->x_record_head %= order;

    }

    /* delete old line & store new */
    pdp_dealloc (x->x_packet);
    x->x_packet = newloop;
    x->x_order = order;
    

}


t_class *pdp_loop_class;



void *pdp_loop_new(t_floatarg f)
{
    int i;
    int order = pdp_loop_realsize(f);
    t_pdp_loop *x = (t_pdp_loop *)pd_new(pdp_loop_class);

    x->x_order = order;
    x->x_packet = (int *)pdp_alloc(sizeof(int)*order);
    for(i=0; i<order; i++) x->x_packet[i] = -1;

    x->x_play_head = 0;
    x->x_record_head = 0;
    x->x_recording_frames = 0;

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("seek"));
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_outlet1 = outlet_new(&x->x_obj, &s_anything); 

    x->x_loop = 1;
    
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_loop_setup(void)
{


    pdp_loop_class = class_new(gensym("pdp_loop"), (t_newmethod)pdp_loop_new,
			      (t_method)pdp_loop_free, sizeof(t_pdp_loop), 0, A_DEFFLOAT, A_NULL);
    
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_record, gensym("record"),  A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_store, gensym("store"),  A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_reset, gensym("reset"),  A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_bang, gensym("bang"),  A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_stop, gensym("stop"),  A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_seek, gensym("seek"),  A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_resize, gensym("size"),  A_FLOAT, A_NULL);
    class_addmethod(pdp_loop_class, (t_method)pdp_loop_loop, gensym("loop"),  A_FLOAT, A_NULL);
    class_addfloat(pdp_loop_class, (t_method)pdp_loop_seek_hot);

}

#ifdef __cplusplus
}
#endif
