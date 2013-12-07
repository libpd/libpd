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


#include <GL/gl.h>
#include "pdp_opengl.h"
#include "pdp_3dp_base.h"

typedef struct pdp_3d_windowcontext_struct
{
    t_pdp_3dp_base x_base;
    int x_width;
    int x_height;
    t_outlet *x_eventout;
    int x_finish_queue_id[2];
    int x_finish_queue_id_current;

} t_pdp_3d_windowcontext;


static void pdp_3d_windowcontext_sendfinish(t_pdp_3d_windowcontext *x)
{
    PDP_ASSERT(x);
    PDP_ASSERT(x->x_eventout);
    outlet_symbol(x->x_eventout, gensym("done"));
}

/* outlet methods */

/* called before the context is propagated */
static void pdp_3d_windowcontext_clearbuffer(t_pdp_3d_windowcontext *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    //post("setting up render buffer");

    // for multipass rendering
    //pdp_packet_3Dcontext_set_subwidth(p, 320);
    //pdp_packet_3Dcontext_set_subheight(p, 240);

    pdp_packet_3Dcontext_set_rendering_context(p);
    pdp_packet_3Dcontext_setup_3d_context(p);

    /* clear buffer */
    //glScissor(0,0,
    //	      pdp_packet_3Dcontext_subwidth(p),
    //	      pdp_packet_3Dcontext_subheight(p));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



}

/* called after context is propagated */
static void pdp_3d_windowcontext_swapbuffer(t_pdp_3d_windowcontext *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    //post("displaying render buffer");
    //pdp_packet_3Dcontext_set_rendering_context(p);
    pdp_packet_3Dcontext_win_swapbuffers(p);
    //pdp_packet_3Dcontext_unset_rendering_context(p);
}

void pdp_3d_windowcontext_resize(t_pdp_3d_windowcontext *x, t_floatarg width, t_floatarg height)
{
    int w = (int)width;
    int h = (int)height;
    int p = pdp_3dp_base_get_context_packet(x);
    if ((w>0) && (h>0)){
	pdp_packet_3Dcontext_win_resize(p, w, h);
	x->x_width = w;
	x->x_height = h;
    }
}

void pdp_3d_windowcontext_open(t_pdp_3d_windowcontext *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 == p){
	p  = pdp_packet_new_3Dcontext_win();
	pdp_3d_windowcontext_resize(x, x->x_width, x->x_height);
	pdp_3dp_base_set_context_packet(x, p);
    }
    
}
void pdp_3d_windowcontext_close(t_pdp_3d_windowcontext *x)
{
    t_pdp_procqueue *q = pdp_3dp_base_get_queue(x);

    /* flush all pending tasks in the queue */
    //post("preflush");
    pdp_procqueue_flush(q);
    //post("postflush");

    /* now it is safe to delete the context packet */
    pdp_packet_delete(pdp_3dp_base_move_context_packet(x));

    //post("deleted");
}

void pdp_3d_windowcontext_cursor(t_pdp_3d_windowcontext *x, t_floatarg f)
{
    int p = pdp_3dp_base_get_context_packet(x);
    bool toggle = (f != 0.0f);
    pdp_packet_3Dcontext_win_cursor(p, toggle);
}



static void pdp_3d_windowcontext_bang(t_pdp_3d_windowcontext *x)
{
    int p;
    int cur = x->x_finish_queue_id_current;
    t_pdp_list *eventlist;

    /* check if at least recent processing chain is done (two chains busy = max pipeline depth) */
    if (-1 != x->x_finish_queue_id[cur]){
	//post("pdp_3d_windowcontext_bang: bang ignored (previous rendering not finished)");
	return;
    }

    /* create a window context if needed */
    pdp_3d_windowcontext_open(x);

    /* get events and send to outlet */
    p = pdp_3dp_base_get_context_packet(x);
    eventlist = pdp_packet_3Dcontext_win_get_eventlist(p);
    if (eventlist){
	t_pdp_atom *a;
	for (a=eventlist->first; a; a=a->next){
	    outlet_pdp_list(x->x_eventout, a->w.w_list);
	}
	pdp_tree_free(eventlist);
    }

    /* bang base */
    pdp_3dp_base_bang(x);

    /* add a dummy process to the queue for synchro */
    pdp_procqueue_add(pdp_3dp_base_get_queue(x), x, 0, 0, &x->x_finish_queue_id[cur]);
    x->x_finish_queue_id_current = !cur;


    
}


static void pdp_3d_windowcontext_free(t_pdp_3d_windowcontext *x)
{
    pdp_3d_windowcontext_close(x);
    pdp_3dp_base_free(x);

}

t_class *pdp_3d_windowcontext_class;


void *pdp_3d_windowcontext_new(void)
{
    /* allocate */
    t_pdp_3d_windowcontext *x = (t_pdp_3d_windowcontext *)pd_new(pdp_3d_windowcontext_class);

    x->x_width = 320;
    x->x_height = 240;
    x->x_finish_queue_id[0] = -1;
    x->x_finish_queue_id[1] = -1;
    x->x_finish_queue_id_current =0;

    /* init super: this is mandatory */
    pdp_3dp_base_init(x);
    pdp_3dp_base_disable_active_inlet(x);

    /* set the dpd processing methods & outlets */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_windowcontext_clearbuffer, 0);
    pdp_3dp_base_add_cleanup(x, (t_pdp_method)pdp_3d_windowcontext_swapbuffer, (t_pdp_method)pdp_3d_windowcontext_sendfinish);

    /* add event outlet */
    x->x_eventout = outlet_new((t_object *)x, &s_anything);

    
    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_windowcontext_setup(void)
{
    /* create a standard pd class */
    pdp_3d_windowcontext_class = class_new(gensym("3dp_windowcontext"), (t_newmethod)pdp_3d_windowcontext_new,
   	(t_method)pdp_3d_windowcontext_free, sizeof(t_pdp_3d_windowcontext), 0, A_NULL);

    /* inherit pdp base class methods */
    pdp_3dp_base_setup(pdp_3d_windowcontext_class);

    /* register methods */
    class_addbang(pdp_3d_windowcontext_class, pdp_3d_windowcontext_bang);

    class_addmethod(pdp_3d_windowcontext_class, (t_method)pdp_3d_windowcontext_open, gensym("open"), A_NULL);
    class_addmethod(pdp_3d_windowcontext_class, (t_method)pdp_3d_windowcontext_close, gensym("close"), A_NULL);
    class_addmethod(pdp_3d_windowcontext_class, (t_method)pdp_3d_windowcontext_resize, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_3d_windowcontext_class, (t_method)pdp_3d_windowcontext_resize, gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_3d_windowcontext_class, (t_method)pdp_3d_windowcontext_cursor, gensym("cursor"), A_FLOAT, A_NULL);

}



#ifdef __cplusplus
}
#endif
