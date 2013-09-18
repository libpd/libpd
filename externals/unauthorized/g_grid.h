/* Copyright (c) 2002 Yves Degoyon
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* a header for grid which enables to control
*  2 parameters with the mouse cursor
*/

#ifndef __G_GRID_H
#define __G_GRID_H

typedef struct _grid
{
    t_object x_obj;
    t_glist *x_glist;
    t_symbol *x_name; 
    t_outlet *x_xoutlet; 
    t_outlet *x_youtlet; 
    int x_height; 	/* height of the grid                        */
    t_float x_min; 	/* minimum value of x                        */
    t_float x_max; 	/* max value of x                            */
    int x_width; 	/* width of the grid                         */
    t_float y_min; 	/* minimum value of y                        */
    t_float y_max; 	/* max value of y                            */
    t_float x_current; 	/* x coordinate of current position          */
    t_float y_current; 	/* y coordinate of current position          */
    int x_selected; 	/* stores selected state                     */
    int x_point; 	/* indicates if a point is plotted           */
    int x_grid; 	/* indicates if a grid is requested          */
    t_float x_xstep; 	/* sets the step ( grain ) for x             */
    t_float x_ystep; 	/* sets the step ( grain ) for y             */
    int x_xlines; 	/* number of vertical lines                  */
    int x_ylines; 	/* number of horizontal lines                */
    char *x_bgcolor; 	/* background color                          */
} t_grid;

#endif
