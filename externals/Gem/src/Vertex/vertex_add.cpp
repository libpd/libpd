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

#include "vertex_add.h"

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_GIMME(vertex_add);
 
/////////////////////////////////////////////////////////
//
// vertex_add
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_add :: vertex_add(int argc, t_atom*argv) :
  m_leftType(0), m_rightType(0),
  m_rightSize(0),
  m_rightVertexArray(NULL), m_rightColorArray(NULL), m_rightTexCoordArray(NULL), m_rightNormalArray(NULL)
{
  if(argc)typeMess(argc, argv);
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd,gensym("gem_state"), gensym("gem_right"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_add :: ~vertex_add()
{
  inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// set the array-types for left- and right-hand
//
/////////////////////////////////////////////////////////
void vertex_add::typeMess(int argc, t_atom*argv){
  char c=0;
  switch(argc){
  default:
    error("GEM: dual_vertex: 'type' must have 1 (for both sides) or 2 arguments!");
    break;
  case 2:
    c=*atom_getsymbol(argv+1)->s_name;
    switch(c){
    case 'v': case 'V': m_rightType=0; break;
    case 'c': case 'C': m_rightType=1; break;
    case 't': case 'T': m_rightType=2; break;
    case 'n': case 'N': m_rightType=3; break;
    default:
      error("vertex_operator: invalid type '%s'! skipping", atom_getsymbol(argv+1)->s_name);
      return;
    }
  case 1:
    c=*atom_getsymbol(argv)->s_name;
    switch(c){
    case 'v': case 'V': m_leftType=0; break;
    case 'c': case 'C': m_leftType=1; break;
    case 't': case 'T': m_leftType=2; break;
    case 'n': case 'N': m_leftType=3; break;
    default:
      error("vertex_operator: invalid type '%s'! skipping", atom_getsymbol(argv)->s_name);
      return;
    }
    if(argc==1)m_rightType=m_leftType;
    break;
  }
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
// we assume that "lsize" and "rsize" are >0
// we assume that "larray" and "larray" point somewhere
// checking is done in render()
void vertex_add :: vertexProcess(int lsize, float*larray, int rsize, float*rarray){
  float indR=0.f; // the right-hand index
  float incR=static_cast<float>(rsize)/static_cast<float>(lsize); // the right-hand increment

  for(int i=0; i<lsize; i++){
    const int I=4*i;
    const int J=4*static_cast<int>(indR); // i know that this is expensive
    larray[I+0]+=rarray[J+0];
    larray[I+1]+=rarray[J+1];
    larray[I+2]+=rarray[J+2];
    larray[I+3]+=rarray[J+3];
    indR+=incR;
  }
}

void vertex_add :: render(GemState *state)
{ 
  int   size=state->VertexArraySize;
  if(size<=0 || m_rightSize<=0) return;

  float*leftArray=NULL;
  float*rightArray=NULL;

  switch(m_leftType){
  default: // vertex
    leftArray=state->VertexArray;
    break;
  case 1: // color
    leftArray=state->ColorArray;
    break;
  case 2: // tex
    leftArray=state->TexCoordArray;
    break;
  case 3: // normals
    leftArray=state->NormalArray;
    break;
  }
  if(leftArray==NULL)return;

  switch(m_rightType){
  default: // vertex
    rightArray=m_rightVertexArray;
    break;
  case 1: // color
    rightArray=m_rightColorArray;
    break;
  case 2: // tex
    rightArray=m_rightTexCoordArray;
    break;
  case 3: // normals
    rightArray=m_rightNormalArray;
    break;
  }
  if(rightArray==NULL)return;

  vertexProcess(size, leftArray, m_rightSize, rightArray);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_add :: postrender(GemState *state)
{
}
 
/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_add :: rightRender(GemState *state)
{
  m_rightSize          = state->VertexArraySize;

  m_rightVertexArray   = state->VertexArray;
  m_rightColorArray    = state->ColorArray;
  m_rightTexCoordArray = state->TexCoordArray;
  m_rightNormalArray   = state->NormalArray;
} 
 
 
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_add :: obj_setupCallback(t_class *classPtr)
{        
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_add::gem_rightMessCallback),
		  gensym("gem_right"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_add::typeMessCallback),
		  gensym("type"), A_GIMME, A_NULL);
}
void vertex_add :: gem_rightMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  if (argc==1 && argv->a_type==A_FLOAT){
  } else if (argc==2 && argv->a_type==A_POINTER && (argv+1)->a_type==A_POINTER){
    GetMyClass(data)->rightRender(reinterpret_cast<GemState*>((argv+1)->a_w.w_gpointer));
  } else GetMyClass(data)->error("wrong righthand arguments....");
}
void vertex_add :: typeMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  GetMyClass(data)->typeMess(argc, argv);
}
