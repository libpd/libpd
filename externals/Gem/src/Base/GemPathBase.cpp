////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "GemPathBase.h"
#include "Utils/Functions.h"

// CPPEXTERN_NEW_WITH_GIMME(GemPathBase);

/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
GemPathBase :: GemPathBase(int argc, t_atom *argv)
  : m_numDimens(1), m_array(NULL),
    m_out1(NULL)
{
  m_out1 = outlet_new(this->x_obj, 0);

  if (argc >= 2)
    openMess(atom_getsymbol(&argv[1]));
  if (argc >= 1)
    {
      m_numDimens = (int)atom_getfloat(&argv[0]);
      if (m_numDimens < 1) m_numDimens = 1;
      if (m_numDimens > 64)
	{
	  error("too many dimensions, must be below 64");
	  m_numDimens = 64;
	}
    }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
GemPathBase :: ~GemPathBase()
{
  outlet_free(m_out1);
}

/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void GemPathBase :: openMess(t_symbol *arrayname)
{
  m_array = (t_garray *)pd_findbyclass(arrayname, garray_class);
  if (!m_array)
    {
      error("unable to find array %s", arrayname->s_name);
      return;
    }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void GemPathBase :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GemPathBase::openMessCallback),
		  gensym("open"), A_SYMBOL, A_NULL);
  class_addfloat(classPtr, reinterpret_cast<t_method>(&GemPathBase::floatMessCallback));    
}
void GemPathBase :: openMessCallback(void *data, t_symbol *arrayname)
{
  GetMyClass(data)->openMess(arrayname);
}
void GemPathBase :: floatMessCallback(void *data, float n)
{
  GetMyClass(data)->floatMess(n);
}

