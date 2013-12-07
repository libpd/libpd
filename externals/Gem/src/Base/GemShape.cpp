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

#include "GemShape.h"
#include "Gem/State.h"
#include <ctype.h>
/////////////////////////////////////////////////////////
//
// a generic GemShape
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
GemShape :: GemShape(t_floatarg size)
  : m_linewidth(1.0f), m_size((float)size), m_drawType(GL_DEFAULT_GEM), m_blend(0),
    m_inlet(NULL),
    m_texType(0), m_texNum(0),
    m_texCoords(NULL),
    m_lighting(false)
{
  if (m_size == 0.f)m_size = 1.f;

  // the size inlet
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft1"));
}
GemShape :: GemShape()
  : m_linewidth(1.0f), m_size(1.0f), m_drawType(GL_DEFAULT_GEM), m_blend(0), 
    m_inlet(NULL),
    m_texType(0), m_texNum(0),
    m_texCoords(NULL),
    m_lighting(false)
{
  // no size inlet
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
GemShape :: ~GemShape()
{
  if(m_inlet)inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// SetVertex
// set up the texture-coordinates
/////////////////////////////////////////////////////////
void GemShape :: SetVertex(GemState* state,float x, float y, float z, float tx, float ty,int curCoord)
{
	int i;

  TexCoord*texcoords=NULL;
  int numCoords = 0;
  int numUnits = 0;

  state->get(GemState::_GL_TEX_NUMCOORDS, numCoords);
  state->get(GemState::_GL_TEX_UNITS, numUnits);


  if (numCoords) {
    tx=state->texCoordX(curCoord);
    ty=state->texCoordY(curCoord);
  }
  
  if (numUnits) {
    for( i=0; i<numUnits; i++) {
      glMultiTexCoord2fARB(GL_TEXTURE0+i, tx, ty);
    }
  } else { // no multitexturing!
    glTexCoord2f(tx, ty);
	}
  glVertex3f( x, y, z );
}

void GemShape :: SetVertex(GemState* state,float x, float y, float z, 
                           float s, float t, float r, float q,
                           int curCoord)
{
	int i;
  int numCoords = 0;
  int numUnits = 0;

  state->get(GemState::_GL_TEX_NUMCOORDS, numCoords);
  state->get(GemState::_GL_TEX_UNITS, numUnits);



  if (numCoords) {
    s*=state->texCoordX(curCoord);
    t*=state->texCoordY(curCoord);
  }

  if (numUnits) {
    for( i=0; i<numUnits; i++)
      glMultiTexCoord4fARB(GL_TEXTURE0+i, s, t, r, q);
  } else { // no multitexturing!
    glTexCoord4f(s, t, r, q);
	}

  glVertex3f( x, y, z );
}



/////////////////////////////////////////////////////////
// linewidthMess
//
/////////////////////////////////////////////////////////
void GemShape :: linewidthMess(float linewidth)
{
    m_linewidth = (linewidth < 0.0f) ? 0.0f : linewidth;
    setModified();
}

/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void GemShape :: sizeMess(float size)
{
    m_size = size;
    setModified();
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void GemShape :: typeMess(t_symbol *type)
{
  char c=toupper(*type->s_name);
  switch (c){
  case 'D': // default
    m_drawType = GL_DEFAULT_GEM;
    break;
  case 'L': // line
    m_drawType = GL_LINE_LOOP;
    break;
  case 'F': // fill
    m_drawType = GL_POLYGON;
    break;
  case 'Q': // quads
    m_drawType = GL_QUADS;
    break;
  case 'P': // point
    m_drawType = GL_POINTS;
    break;
  case 'T': // triangles
    m_drawType = GL_TRIANGLES;
    break;
  case 'S': // strip
    m_drawType = GL_TRIANGLE_STRIP;
    break;  
    
  default:
    error ("unknown draw style");
    return;
  }
  setModified();
}

/////////////////////////////////////////////////////////
// blendMess
//
/////////////////////////////////////////////////////////
void GemShape :: blendMess(float blend)
{
  m_blend = (blend>0);
  setModified();
}

void GemShape :: render(GemState *state)
{
  if (m_drawType == GL_LINE_LOOP || m_drawType == GL_LINE_STRIP || m_drawType == GL_LINES)
    glLineWidth(m_linewidth);

  if (m_blend) {
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_DONT_CARE); 
  }

  m_texType=0;
  m_texNum =0;
  m_texCoords=NULL;
  m_lighting=false;

  state->get(GemState::_GL_TEX_COORDS, m_texCoords);
  state->get(GemState::_GL_TEX_TYPE, m_texType);
  state->get(GemState::_GL_TEX_NUMCOORDS, m_texNum);
  state->get(GemState::_GL_LIGHTING, m_lighting);

  renderShape(state);

  // LATER try to restore the original state
  if (m_blend) {
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_BLEND);
  }

  if (m_drawType == GL_LINE_LOOP || m_drawType == GL_LINE_STRIP || m_drawType == GL_LINES)
    glLineWidth(1.0);
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void GemShape :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&GemShape::linewidthMessCallback),
    	    gensym("width"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&GemShape::typeMessCallback),
    	    gensym("draw"), A_SYMBOL, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&GemShape::blendMessCallback),
    	    gensym("blend"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&GemShape::sizeMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL);
}
void GemShape :: linewidthMessCallback(void *data, t_floatarg linewidth)
{
    GetMyClass(data)->linewidthMess((float)linewidth);
}
void GemShape :: typeMessCallback(void *data, t_symbol *type)
{
    GetMyClass(data)->typeMess(type);
}
void GemShape :: sizeMessCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->sizeMess((float)size);
}
void GemShape :: blendMessCallback(void *data, t_floatarg blend)
{
    GetMyClass(data)->blendMess((float)blend);
}

