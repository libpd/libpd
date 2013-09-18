/* Copyright (c) 2002 Yves Degoyon
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* a header for exciter which enables 
*  to trigger bang events scheduled with the mouse
*/

#ifndef __G_EXCITER_H
#define __G_EXCITER_H

typedef struct _exciter
{
    t_object x_obj;
    t_glist *x_glist;
    t_outlet **x_bangs; 
    t_int x_height; 	    /* height of the exciter                   */
    t_int x_width; 	    /* width of the exciter                    */
    t_int x_nbevents; 	    /* number of simultaneous events           */
    t_float x_timegrain;    /* time granularity for one pixel          */
    t_int *x_sbangs;        /* scheduled bangs                         */
    int x_selected; 	    /* stores selected state                   */
    int x_loop; 	    /* looping flag                            */
    int x_save; 	    /* saving contents flag                    */

     /* internal processing */
    long long x_plooptime;  /* initial time                            */ 
    long long x_reltime;    /* elapsed time since start                */ 
    long long x_looplength; /* length of a loop                        */ 
    t_int x_started;        /* start flag                              */
    t_int x_gindex;         /* last scanned index                      */
} t_exciter;

#endif
