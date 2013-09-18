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

#include "vertex_scale.h"

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_GIMME(vertex_scale);

/////////////////////////////////////////////////////////
//
// vertex_scale
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_scale :: vertex_scale(int argc, t_atom*argv) : GemBase(),
						      m_x(1.f), m_y(1.f), m_z(1.f), m_w(1.f), 
						      m_offset(0), m_count(0),
						      m_vertex(false), m_color(false), 
						      m_normal(false), m_texture(false)
{
  m_vertIn=inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("vertex"));
  m_parmIn=inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("param"));

  modeMess(argc, argv);
  if(!argc)m_vertex=true;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_scale :: ~vertex_scale()
{
  inlet_free(m_vertIn);
  inlet_free(m_parmIn);
}


void vertex_scale :: modeMess(int argc, t_atom*argv){
  m_vertex =false;
  m_color  =false;
  m_normal =false;
  m_texture=false;

  for(int i=0; i<argc; i++){
    char c=*atom_getsymbol(argv+i)->s_name;
    switch(c){
    case 'v': case 'V': m_vertex =true; break;
    case 'c': case 'C': m_color  =true; break;
    case 'n': case 'N': m_normal =true; break;
    case 't': case 'T': m_texture=true; break;
    default:
      error("invalid operand '%s'! skipping", atom_getsymbol(argv+i)->s_name);
      break;
    }
  }
}

void vertex_scale :: vertexMess(int offset, int count){
  m_offset = offset;
  m_count  = count;
}

void vertex_scale :: paramMess(int argc, t_atom*argv){
  m_w=1.f;

  switch (argc){
  case 4:
    m_w = atom_getfloat(argv+3);
  case 3:
    m_z = atom_getfloat(argv+2);
    m_y = atom_getfloat(argv+1);
    m_x = atom_getfloat(argv);
    break;
  default:
    error("vertex_scale: scale must be 3 or 4 values!");
    break;
  }
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_scale :: vertexProcess(int size, GLfloat*array){
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
      array[src] *= m_x;
      array[src+1] *= m_y;
      array[src+2] *= m_z;
      array[src+3] *= m_w;
      src+=4;
    }
  }else{
    int src=0;
    for (int i=0; i< size; i++){
      array[src] *= m_x;
      array[src+1] *= m_y;
      array[src+2] *= m_z;
      array[src+3] *= m_w;
      src+=4;
    }
  }
}


void vertex_scale :: render(GemState *state)
{
  if(state->VertexArraySize<=0) return;
  if(m_vertex && state->VertexArray != NULL){
    vertexProcess(state->VertexArraySize, state->VertexArray);
  }

  if(m_color && state->ColorArray != NULL){
    vertexProcess(state->VertexArraySize, state->ColorArray);
  }

  if(m_normal && state->NormalArray != NULL){
    vertexProcess(state->VertexArraySize, state->NormalArray);
  }

  if(m_texture && state->TexCoordArray != NULL){
    vertexProcess(state->VertexArraySize, state->TexCoordArray);
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_scale :: obj_setupCallback(t_class *classPtr)
{ 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_scale::modeMessCallback),
		  gensym("mode"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_scale::vertexMessCallback),
		  gensym("vertex"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_scale::paramMessCallback),
		  gensym("param"), A_GIMME, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_scale::paramMessCallback),
		  gensym("scale"), A_GIMME, A_NULL);
}

void vertex_scale :: paramMessCallback(void *data, t_symbol*, int argc, t_atom*argv)
{
  GetMyClass(data)->paramMess(argc, argv);
}

void vertex_scale :: vertexMessCallback(void *data,  t_floatarg num, t_floatarg counter)
{
  GetMyClass(data)->vertexMess(static_cast<int>(num), 
			       static_cast<int>(counter));
}
void vertex_scale :: modeMessCallback(void *data, t_symbol*, int argc, t_atom*argv)
{
  GetMyClass(data)->modeMess(argc, argv);
}
