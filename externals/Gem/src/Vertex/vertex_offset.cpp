////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "vertex_offset.h"

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_GIMME(vertex_offset);

/////////////////////////////////////////////////////////
//
// vertex_offset
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_offset :: vertex_offset(int argc, t_atom*argv) : vertex_scale(argc, argv),
						      m_x(0.f), m_y(0.f), m_z(0.f), m_w(0.f) 
{}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_offset :: ~vertex_offset()
{ }

void vertex_offset :: paramMess(int argc, t_atom*argv){
  m_w=0.f;

  switch (argc){
  case 4:
    m_w = atom_getfloat(argv+3);
  case 3:
    m_z = atom_getfloat(argv+2);
    m_y = atom_getfloat(argv+1);
    m_x = atom_getfloat(argv);
    break;
  default:
    error("offset must be 3 or 4 values!");
    break;
  }
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_offset :: vertexProcess(int size, GLfloat*array){
  int count;
   
  if (m_offset < 0) m_offset = 0;
  if (m_offset > size) m_offset = size;
  count = m_count;
  
  if (count < 1) count = 1;
  if ((count + m_offset-1) > size) count = size - m_offset;// -1;
  
  //needs some altivec 
  if (m_offset){
    int src = (m_offset-1) * 4;
    for (int i=0; i< count; i++){
      array[src] += m_x;
      array[src+1] += m_y;
      array[src+2] += m_z;
      array[src+3] += m_w;
      src+=4;
    }
  }else{
    int src=0;
    for (int i=0; i< size; i++){
      array[src] += m_x;
      array[src+1] += m_y;
      array[src+2] += m_z;
      array[src+3] += m_w;
      src+=4;
    }
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_offset :: obj_setupCallback(t_class *classPtr)
{ 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_offset::paramMessCallback),
		  gensym("offset"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_offset::paramMessCallback),
		  gensym("param"), A_GIMME, A_NULL);
}
