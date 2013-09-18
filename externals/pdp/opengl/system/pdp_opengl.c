
/*
 *   OpenGL Extension Module for pdp - opengl system stuff
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
#include "pdp_control.h"

#define PDP_3DP_QUEUE_LOGSIZE 16
#define PDP_3DP_QUEUE_DELTIME 1.0f



void pdp_3Dcontext_prepare_for_thread_switch(void *);
static t_pdp_procqueue _3dp_queue;


static void pdp_control_thread(void *x, t_symbol *s, int argc, t_atom *argv)
{
    int t = 0;
    float f;
    if (argc != 1) return;
    if (argv[0].a_type != A_FLOAT) return;
    f = argv[0].a_w.w_float;
    t = (f != 0.0f);
    post("3dp thread switched %s", t ? "on":"off");


    /* when we switch threads, the glx system needs to be notified
       because it has to release the render context. this is done
       in a process method, so it is run in the correct thread. */


    pdp_procqueue_add(&_3dp_queue, 0, pdp_3Dcontext_prepare_for_thread_switch, 0, 0);
    pdp_procqueue_wait(&_3dp_queue);
    

    /* fresh start: enable/disable the thread dispatching */
    pdp_procqueue_use_thread(&_3dp_queue, t);

}


/* kernel setup */
void pdp_opengl_system_setup(void)
{
    /* init the 3dp queue */
    pdp_procqueue_init(&_3dp_queue, PDP_3DP_QUEUE_DELTIME, PDP_3DP_QUEUE_LOGSIZE);

    /* scheduler uses the thread */
    pdp_procqueue_use_thread(&_3dp_queue, 1);
    //pdp_procqueue_use_thread(&_3dp_queue, 0); //DEBUG: disable 3dp thread

    /* add pdp_control method for thread */
    pdp_control_addmethod((t_method)pdp_control_thread, gensym("3dthread"));

}

t_pdp_procqueue* pdp_opengl_get_queue(void){return (&_3dp_queue);}



