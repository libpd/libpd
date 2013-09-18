/* Copyright (c) 2002 Yves Degoyon
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* a header for 2d_space which enables to spatialize
*  several sound inputs with the mouse  
*/

#ifndef __G_2D_SPACE_H
#define __G_2D_SPACE_H

#define AUDIENCE_NONE 0
#define AUDIENCE_INPUT 1
#define AUDIENCE_OUTPUT 2

typedef struct _audience_tilde
{
    t_object x_obj;
    t_glist *x_glist;
    t_int   x_nbinputs;
    t_int   x_nboutputs;
    t_int   *x_inputs_x;
    t_int   *x_inputs_y;
    t_int   *x_outputs_x;
    t_int   *x_outputs_y;
    t_inlet **x_inputs; 
    t_outlet **x_outputs; 
    t_int x_allocate;        /* indicates that audio buffer is            */
                             /* beeing reallocated                        */
    t_int x_audiobuffersize; /* audio buffer size                         */
    t_int x_audiowritepos;   /* audio writing position                    */
    t_float **x_audiobuffer; /* audio buffer                              */
    t_int   x_type_selected; /* type of selected object                   */
                             /* e.g. inpout or output or none             */
    t_int   x_nselected;     /* index of item selected                    */
    t_int x_height; 	     /* height of the 2d_space object             */
    t_int x_width; 	     /* width of the 2d_space object              */
    t_int x_selected; 	     /* stores selected state                     */
    t_float x_attenuation;   /* sound attenuation per meter               */
    t_int x_applydelay;      /* optional delay due to the distance        */
    t_float x_f;             /* classical float for signal input          */
} t_audience_tilde;

#endif
