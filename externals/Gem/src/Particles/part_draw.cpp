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

#include "part_draw.h"

#include <string.h>
#include "Gem/State.h"

#include "papi.h"

CPPEXTERN_NEW(part_draw);

/////////////////////////////////////////////////////////
//
// part_draw
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
part_draw :: part_draw()
		   : m_drawType(GL_LINES)
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
part_draw :: ~part_draw()
{ }

/////////////////////////////////////////////////////////
// renderParticles
//
/////////////////////////////////////////////////////////
void part_draw :: renderParticles(GemState *state)
{
  bool lighting=false;
  state->get(GemState::_GL_LIGHTING, lighting);
	if (lighting)
	{
		glDisable(GL_LIGHTING);
	}
	if (m_tickTime > 0.f)
	{
		pMove();
	}
	pDrawGroupp(m_drawType);
	if (lighting)
	{
		glEnable(GL_LIGHTING);
	}
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void part_draw :: typeMess(int ac,t_atom* av)
{
  if(ac&&A_SYMBOL==av->a_type) {
    t_symbol*s=atom_getsymbolarg(0,ac,av);
    switch(*(s->s_name)) {
    case 'l': case 'L': m_drawType=GL_LINES; break;
    case 'p': case 'P': m_drawType=GL_POINTS; break;
    default: 
      error("unknown draw style");
      return;
    }
  } else {
    m_drawType = (int)atom_getfloatarg(0,ac,av);
  }
}


/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void part_draw :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&part_draw::typeMessCallback),
    	    gensym("draw"), A_GIMME, A_NULL);
}
void part_draw :: typeMessCallback(void *data,  t_symbol *s, int ac,t_atom* av)
{
    GetMyClass(data)->typeMess(ac,av);
}


