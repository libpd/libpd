/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef INCLUDE_IEM16_TABLE_H__
#define INCLUDE_IEM16_TABLE_H__

/* sampling */

#include "iem16.h"
#include <string.h>
/* ------------------------- table16 -------------------------- */
/* a 16bit table */

t_class *table16_class;
typedef struct _table16 {
  t_object x_obj;

  t_symbol *x_tablename;
  long      x_size;
  t_iem16_16bit    *x_table; /* hold the data */

  int x_usedindsp;
  t_canvas *x_canvas; /* for file i/o */
} t_table16;


EXTERN int table16_getarray16(t_table16*x, int*size,t_iem16_16bit**vec);
EXTERN void table16_usedindsp(t_table16*x);



union tabfudge
{
    double tf_d;
    int32 tf_i[2];
};

#endif
