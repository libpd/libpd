////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "part_targetsize.h"


#include "papi.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(part_targetsize, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// part_targetsize
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
part_targetsize :: part_targetsize(t_floatarg size, t_floatarg scale)
{
 	if (size != 0.f)
		sizeMess(size, size, size);
	else
		sizeMess(1.f, 1.f, 1.f);

 	if (scale != 0.f)
		scaleMess(scale, scale, scale);
	else
		scaleMess(.05f, 0.05f, 0.05f);

    // create the new inlet
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft1"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft2"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
part_targetsize :: ~part_targetsize()
{ }

/////////////////////////////////////////////////////////
// renderParticles
//
/////////////////////////////////////////////////////////
void part_targetsize :: renderParticles(GemState *state)
{
  if (m_tickTime > 0.f)
	{
		pTargetSize(m_size[0], m_size[1], m_size[2],
			    m_scale[0], m_scale[1], m_scale[2]);
	}
}

/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void part_targetsize :: sizeMess(float sizex, float sizey, float sizez)
{
    m_size[0] = sizex;
    m_size[1] = sizey;
    m_size[2] = sizez;
    setModified();
}

/////////////////////////////////////////////////////////
// scaleMess
//
/////////////////////////////////////////////////////////
void part_targetsize :: scaleMess(float scaleX,float scaleY,float scaleZ)
{
  m_scale[0] = scaleX;
  m_scale[1] = scaleY;
  m_scale[2] = scaleZ;
  setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void part_targetsize :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&part_targetsize::sizeMessCallback),
		    gensym("ft1"), A_GIMME, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&part_targetsize::scaleMessCallback),
		    gensym("ft2"), A_GIMME, A_NULL); 
}
void part_targetsize :: sizeMessCallback(void *data, t_symbol*s, int argc, t_atom *argv)
{
  t_float size=1.0, sizeX=1.0, sizeY=1.0, sizeZ=1.0;
  switch (argc){
  case 1:
    size=atom_getfloat(argv++);
    GetMyClass(data)->sizeMess(size, size, size);
    break;
  case 3:
    sizeX=atom_getfloat(argv++);
    sizeY=atom_getfloat(argv++);
    sizeZ=atom_getfloat(argv++);
    GetMyClass(data)->sizeMess(sizeX, sizeY, sizeZ);
    break;
  }
}
void part_targetsize :: scaleMessCallback(void *data, t_symbol*s, int argc, t_atom *argv)
{
  t_float scale=1.0, scaleX=1.0, scaleY=1.0, scaleZ=1.0;
  switch (argc){
  case 1:
    scale=atom_getfloat(argv++);
    GetMyClass(data)->scaleMess(scale, scale, scale);
    break;
  case 3:
    scaleX=atom_getfloat(argv++);
    scaleY=atom_getfloat(argv++);
    scaleZ=atom_getfloat(argv++);
    GetMyClass(data)->scaleMess(scaleX, scaleY, scaleZ);
    break;
  }
}
