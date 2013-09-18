////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
//
// Implementation file
//
//    Copyright (c) 2005 James Tittle II
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

/* LATER: respect the available number of texunits as stored in m_max */

#include "pix_multitexture.h"

#include "Gem/Manager.h"
#include "Gem/Image.h"
#include "Gem/Exception.h"

#include <stdio.h>
#ifdef _WIN32
# include <io.h>
# include <windows.h>
# define snprintf _snprintf
#endif

CPPEXTERN_NEW_WITH_ONE_ARG(pix_multitexture, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_multitexture
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_multitexture :: pix_multitexture(t_floatarg reqTexUnits)
  : m_inlet(NULL),
    m_reqTexUnits((GLint)reqTexUnits), m_max(0), m_textureType(GL_TEXTURE_2D), m_mode(0),
    m_xRatio(1.f), m_yRatio(1.f), upsidedown(false), m_texSizeX(0), m_texSizeY(0),
    m_oldTexCoords(NULL), m_oldNumCoords(0), m_oldTexture(0)
{
  if (m_reqTexUnits<=0) {
    throw (GemException("[pix_multitexture]: Please specify more than 0 texture units"));
  }

#ifdef __APPLE__
  m_mode = 1;  //default to the fastest mode for systems that support it
  m_textureType = GL_TEXTURE_RECTANGLE_EXT;
#endif
  
  m_inlet=new t_inlet*[m_reqTexUnits]; 
  char tempVt[5];
  for(int i=0;i<m_reqTexUnits; i++){
    snprintf(tempVt, 5, "#%d", i);
    tempVt[4]=0;
    m_inlet[i]=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym(tempVt));
  }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_multitexture :: ~pix_multitexture()
{ 
  if(m_inlet){
    for(int i=0;i<m_reqTexUnits; i++){
      inlet_free(m_inlet[i]);
    }
    delete[]m_inlet;
  }
}

/////////////////////////////////////////////////////////
// extension checks
//
/////////////////////////////////////////////////////////
bool pix_multitexture :: isRunnable(void) {
  if(GLEW_VERSION_1_3 && GLEW_ARB_multitexture)return true;

  error("your system lacks multitexture support");
  return false;
}

/////////////////////////////////////////////////////////
// setTexCoords
// upsidedown is derived from the imageStruct.upsidedown
// use this when loading images...
//
/////////////////////////////////////////////////////////
inline void setTexCoords(TexCoord *coords, float xRatio, float yRatio, GLboolean upsidedown=false){
  if(!upsidedown){
      coords[0].s = 0.f;
      coords[0].t = 0.f;
      coords[1].s = xRatio;
      coords[1].t = 0.f;
      coords[2].s = xRatio;
      coords[2].t = yRatio;
      coords[3].s = 0.f;
      coords[3].t = yRatio;
  } else {
      coords[3].s = 0.f;
      coords[3].t = 0.f;
      coords[2].s = xRatio;
      coords[2].t = 0.f;
      coords[1].s = xRatio;
      coords[1].t = yRatio;
      coords[0].s = 0.f;
      coords[0].t = yRatio;
  }
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_multitexture :: render(GemState *state)
{
  int textype=0;
  state->get(GemState::_GL_TEX_COORDS, m_oldTexCoords);
  state->get(GemState::_GL_TEX_NUMCOORDS, m_oldNumCoords);
  state->get(GemState::_GL_TEX_TYPE, m_oldTexture);

  state->get(GemState::_GL_TEX_UNITS, m_reqTexUnits);
	
	if (m_textureType == GL_TEXTURE_2D)
	{
	  m_xRatio = 1.0;
	  m_yRatio = 1.0;
	  textype = 1;
	}else{
	  m_xRatio = m_texSizeX;
	  m_yRatio = m_texSizeY;
	  textype = 2;
	}

	setTexCoords(m_coords, m_xRatio, m_yRatio, true);

  TexCoord*tc=m_coords;
  state->set(GemState::_GL_TEX_COORDS, tc);
  state->set(GemState::_GL_TEX_NUMCOORDS, 4);
  state->set(GemState::_GL_TEX_TYPE, textype);


	for ( int i=0; i< m_reqTexUnits; i++ )
	{
    if(GLEW_VERSION_1_3) {
      glActiveTexture( GL_TEXTURE0 + i );
    } else {
      glActiveTextureARB( GL_TEXTURE0_ARB + i );
    }

		glEnable( m_textureType );
		glBindTexture( m_textureType, m_texID[i] );
		glTexParameteri( m_textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( m_textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( m_textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( m_textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_multitexture :: postrender(GemState *state)
{
  state->set(GemState::_GL_TEX_COORDS, m_oldTexCoords);
  state->set(GemState::_GL_TEX_NUMCOORDS, m_oldNumCoords);
  state->set(GemState::_GL_TEX_TYPE, m_oldTexture);

  if(GLEW_VERSION_1_3) {
    for ( int i = m_reqTexUnits; i>0; i--)
      {
        glActiveTexture( GL_TEXTURE0 + i);
        glDisable( m_textureType );
      }
    glActiveTexture( GL_TEXTURE0 );
  } else {
    for ( int i = m_reqTexUnits; i>0; i--)
      {
        glActiveTextureARB( GL_TEXTURE0_ARB + i);
        glDisable( m_textureType );
      }
    glActiveTextureARB( GL_TEXTURE0_ARB );
  }
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_multitexture :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_multitexture::texUnitMessCallback),
                  gensym("texUnit"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_multitexture::dimenMessCallback),
                  gensym("dimen"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_multitexture::modeCallback),
		gensym("mode"), A_FLOAT, A_NULL);
  // generic inlets for texUnit
  class_addanything(classPtr, reinterpret_cast<t_method>(&pix_multitexture::parmCallback));
}
void pix_multitexture :: texUnitMessCallback(void *data, float n, float texID)
{
  GetMyClass(data)->m_texID[(int)n] = (GLint)texID;
}

void pix_multitexture :: dimenMessCallback(void *data, float sizeX, float sizeY)
{
  GetMyClass(data)->m_texSizeX = (int)sizeX;
  GetMyClass(data)->m_texSizeY = (int)sizeY;
}

void pix_multitexture :: modeCallback(void *data, t_floatarg textype)
{
  GetMyClass(data)->m_mode=((int)textype);
  if (textype)
  {
    //    GetMyClass(data)->m_oldType = GetMyClass(data)->m_textureType;
    GetMyClass(data)->m_textureType = GL_TEXTURE_RECTANGLE_EXT;
    GetMyClass(data)->post("using mode 1:GL_TEXTURE_RECTANGLE_EXT");
  }else{
    GetMyClass(data)->m_textureType = GL_TEXTURE_2D;
    GetMyClass(data)->post("using mode 0:GL_TEXTURE_2D");
  }
}

void pix_multitexture :: parmCallback(void *data, t_symbol*s, int argc, t_atom*argv){
  if(argc>0&&argv->a_type==A_FLOAT&&('#'==*s->s_name)){
    int i = atoi(s->s_name+1);
    GetMyClass(data)->m_texID[i]=(GLint)atom_getint(argv);
  } else {
     GetMyClass(data)->error("invalid texUnit specified!");
  }
}
