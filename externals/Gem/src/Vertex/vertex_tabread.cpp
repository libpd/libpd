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

#include "vertex_tabread.h"

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_GIMME(vertex_tabread);

/////////////////////////////////////////////////////////
//
// vertex_tabread
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_tabread :: vertex_tabread(int argc, t_atom*argv) :
  m_VertexArray(NULL),  m_ColorArray(NULL),  m_NormalArray(NULL),  m_TexCoordArray(NULL),
  m_size(0), m_doit(false),
  m_Vtable(NULL), m_Ctable(NULL), m_Ttable(NULL), m_Ntable(NULL)
{
  if(argc)tableMess(argc, argv);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_tabread :: ~vertex_tabread()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////

///////////////
// check if array exists and whether it is a floatarray
//
///////////////
static t_float* checkarray(t_symbol *s, int &length)
{
  t_garray *a;
  t_float  *fp;
  length = 0;

  if (!(a = reinterpret_cast<t_garray*>(pd_findbyclass(s, garray_class))))    {
    if (*s->s_name) error("vertex_tabread: %s: no such array", s->s_name);
    fp = 0;
  } else if (!garray_getfloatarray(a, &length, &fp))   {
    error("%s: bad template for vertex_tabread", s->s_name);
    fp = 0;
  }
  
  if (length==0){
    error("vertex_tabread: table %s is zero-lengthed", s->s_name);
    fp=0;
  }
  return fp;
}

void vertex_tabread :: render(GemState *state)
{
  int      length=0;
  int      size=0;
  GLfloat*dummy=NULL;

  state->VertexArray   = NULL; state->VertexArraySize   = 0;
  state->ColorArray    = NULL; state->HaveColorArray    = 0;
  state->NormalArray   = NULL; state->HaveNormalArray   = 0;
  state->TexCoordArray = NULL; state->HaveTexCoordArray = 0;

  if(m_Vtable){
    dummy=checkarray(m_Vtable, length);
    size=length;
    if(dummy && length>0){
      state->VertexArray = dummy;
      state->VertexArraySize = length >> 2;

      state->VertexArrayStride = 3;
    }
  }
  if (size){
    if(m_Ctable){
      dummy=checkarray(m_Ctable, length);
      if(dummy && length==size){
	state->ColorArray = dummy;
	state->HaveColorArray = 1;
      }
    }
    if(m_Ntable){
      dummy=checkarray(m_Ntable, length);
      if(dummy && length==size){
	state->NormalArray = dummy;
	state->HaveNormalArray = 1;
      }
    }
    if(m_Ttable){
      dummy=checkarray(m_Ttable, length);
      if(dummy && length==size){
	state->TexCoordArray = dummy;
	state->HaveTexCoordArray = 1;
      }
    }
  }
  setModified();

  if (m_doit){
    state->VertexDirty=1;
    m_doit=false;
  }
}

void vertex_tabread :: tableMess(int argc, t_atom*argv){
  t_symbol*Vtable=NULL;
  t_symbol*Ctable=NULL;
  t_symbol*Ntable=NULL;
  t_symbol*Ttable=NULL;

  switch(argc){
  case 4:
    if((argv+3)->a_type!=A_SYMBOL){
      error("only symbolic table-names are accepted");
      return;
    }
    Ttable=atom_getsymbol(argv+3);
  case 3:
    if((argv+2)->a_type!=A_SYMBOL){
      error("only symbolic table-names are accepted");
      return;
    }
    Ntable=atom_getsymbol(argv+2);
  case 2:
    if((argv+1)->a_type!=A_SYMBOL){
      error("only symbolic table-names are accepted");
      return;
    }
    Ctable=atom_getsymbol(argv+1);
  case 1:
    if((argv+0)->a_type!=A_SYMBOL){
      error("only symbolic table-names are accepted");
      return;
    }
    Vtable=atom_getsymbol(argv+0);
    break;
  default:
    error("table must have 1, 2, 3 or 4 arguments");
    return;
  }

  m_Vtable=Vtable;
  m_Ctable=Ctable;
  m_Ntable=Ntable;
  m_Ttable=Ttable;
}

void vertex_tabread :: bangMess()
{
  m_doit = true;
}

 
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_tabread :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_tabread::tableMessCallback),
		  gensym("table"), A_GIMME, A_NULL);
  class_addbang(classPtr, reinterpret_cast<t_method>(&vertex_tabread::bangMessCallback));
}

void vertex_tabread :: tableMessCallback(void *data, t_symbol*, int argc, t_atom*argv)
{
  GetMyClass(data)->tableMess(argc, argv);
}

void vertex_tabread :: bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}
