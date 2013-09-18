////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "part_source.h"


#include <string.h>

CPPEXTERN_NEW_WITH_ONE_ARG(part_source, t_floatarg, A_DEFFLOAT);

  /////////////////////////////////////////////////////////
//
// part_source
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
part_source :: part_source(t_floatarg num)
  : m_numberToAdd(150), m_domain(PDSphere)
{
  int i=9;while(i--)m_arg[i]=0.0;
  m_arg[3]=0.2f;
  if (num > 0) m_numberToAdd = num;

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("numToAdd"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("symbol"), gensym("domain"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vector"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
part_source :: ~part_source()
{ }
/////////////////////////////////////////////////////////
// vel.domain
//
/////////////////////////////////////////////////////////
void part_source :: domainMess(t_symbol*s)
{
  char *str=s->s_name;

       if (!strcmp(str,"point"    ))m_domain=PDPoint;
  else if (!strcmp(str,"line"     ))m_domain=PDLine;
  else if (!strcmp(str,"triangle" ))m_domain=PDTriangle;
  else if (!strcmp(str,"plane"    ))m_domain=PDPlane;
  else if (!strcmp(str,"box"      ))m_domain=PDBox;
  else if (!strcmp(str,"sphere"   ))m_domain=PDSphere;
  else if (!strcmp(str,"cylinder" ))m_domain=PDCylinder;
  else if (!strcmp(str,"cone"     ))m_domain=PDCone;
  else if (!strcmp(str,"blob"     ))m_domain=PDBlob;
  else if (!strcmp(str,"disc"     ))m_domain=PDDisc;
  else if (!strcmp(str,"rectangle"))m_domain=PDRectangle;
  else error("GEM: particles: unknown domain");
}
void part_source :: vectorMess(int argc, t_atom*argv){
  int i=9;
  while(i--)if(argc>i)m_arg[i]=atom_getfloat(argv+i);
}

/////////////////////////////////////////////////////////
// renderParticles
//
/////////////////////////////////////////////////////////
void part_source :: renderParticles(GemState *state)
{
  if (m_tickTime > 0.f) {
    pSource((float)m_numberToAdd, m_domain, 
	    m_arg[0],m_arg[1],m_arg[2],m_arg[3],m_arg[4],m_arg[5],m_arg[6],m_arg[7],m_arg[8]);
  }
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void part_source :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&part_source::numberMessCallback),
		  gensym("numToAdd"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&part_source::domainMessCallback),
		  gensym("domain"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&part_source::vectorMessCallback),
		  gensym("vector"), A_GIMME, A_NULL);
}
void part_source :: numberMessCallback(void *data, t_floatarg num)
{
  GetMyClass(data)->numberMess(num);
}
void part_source :: domainMessCallback(void *data, t_symbol*s)
{
  GetMyClass(data)->domainMess(s);
}
void part_source :: vectorMessCallback(void *data, t_symbol*, int argc, t_atom*argv)
{
  GetMyClass(data)->vectorMess(argc, argv);
}

