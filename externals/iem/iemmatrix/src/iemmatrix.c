/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) IOhannes m zmölnig, forum::für::umläute
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */
#include "iemmatrix.h"

void iemmatrix_sources_setup(void);

void iemmatrix_setup(){
  post("");
  post("iemmatrix "VERSION);
  post("\tobjects for manipulating 2d-matrices");
  post("\t(c) IOhannes m zmölnig, Thomas Musil, Franz Zotter :: iem, 2001-2011");
  post("\tcompiled "__DATE__" : "__TIME__);
  post("");

  iemmatrix_sources_setup();
}
