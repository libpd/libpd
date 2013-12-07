/* Copyright (c) 2002 Yves Degoyon
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* probalizer : outputs integer values according to a drawn probability curve
*/

#ifndef __G_PROBALIZER_H
#define __G_PROBALIZER_H

typedef struct _probalizer
{
    t_object x_obj;
    t_glist *x_glist;
    t_int x_height; 	    /* height of the probalizer                */
    t_int x_width; 	    /* width of the probalizer                 */
    t_int x_nvalues; 	    /* number of values                        */
    t_int x_noccurrences;   /* max number of occurrences in a serial   */
    t_int *x_probs; 	    /* probability of each event               */
    t_int *x_ovalues; 	    /* number of outputs of each event         */
    int x_selected; 	    /* stores selected state                   */
    int x_save; 	    /* saving contents flag                    */
    t_outlet *x_endoutlet;  /* outlet to signal the end of the serial  */
} t_probalizer;

#endif
