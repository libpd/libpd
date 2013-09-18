/* Copyright (c) 2002 Yves Degoyon
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* a header for pianoroll which enables 
*  to control a sequencer ( pitch and volume )
*/

#ifndef __G_PIANOROLL_H
#define __G_PIANOROLL_H

typedef struct _pianoroll
{
    t_object x_obj;
    t_glist *x_glist;
    t_symbol *x_name; 
    t_outlet *x_pitch; 
    t_outlet *x_volume; 
    int x_height; 	/* height of the pianoroll                   */
    int x_width; 	/* width of the pianoroll                    */
    t_float x_pmin; 	/* minimum value of the pitch                */
    t_float x_pmax; 	/* max value of the pitch                    */
    t_int x_nbgrades;   /* number of grades for the pitch            */
    t_int x_nbsteps; 	/* number of steps                           */
    t_int x_defvalue;   /* default value for the pitch               */
    t_float x_transpose;/* transposition value                       */
    t_float *x_peaches; /* pitch for each step                       */
    t_int *x_ipeaches;  /* pitch index for each step                 */
    t_float *x_volumes; /* volume for each step                      */
    t_int *x_ivolumes;  /* volume index for each step                */
    int x_selected; 	/* stores selected state                     */
    int x_xlines; 	/* number of vertical lines                  */
    int x_ylines; 	/* number of horizontal lines                */
    int x_scurrent; 	/* cureent step                              */
    int x_save; 	/* saving contents flag                      */
} t_pianoroll;

EXTERN t_rtext *rtext_new_without_senditup(t_glist *glist, t_text *who, t_rtext *next);

#endif
