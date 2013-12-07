////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) GÂžnther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "vertex_mul.h"

CPPEXTERN_NEW_WITH_GIMME(vertex_mul);
 
/////////////////////////////////////////////////////////
//
// vertex_mul
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_mul :: vertex_mul(int argc, t_atom*argv) : vertex_add(argc, argv)
{}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_mul :: ~vertex_mul()
{}


/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
// we assume that "lsize" and "rsize" are >0
// we assume that "larray" and "larray" point somewhere
// checking is done in render()
void vertex_mul :: vertexProcess(int lsize, float*larray, int rsize, float*rarray){
  float indR=0.f; // the right-hand index
  float incR=static_cast<float>(rsize) / static_cast<float>(lsize); // the right-hand increment

  for(int i=0; i<lsize; i++){
    const int I=4*i;
    const int J=static_cast<int>(4.*indR); // i know that this is expensive
    //const int J=4*(int)indR; // i know that this is expensive
    larray[I+0]*=rarray[J+0];
    larray[I+1]*=rarray[J+1];
    larray[I+2]*=rarray[J+2];
    larray[I+3]*=rarray[J+3];
    indR+=incR;
  }
}

 
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_mul :: obj_setupCallback(t_class *classPtr)
{}
