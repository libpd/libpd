////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Created by tigital on 10/04/2005.
// Copyright 2005 James Tittle
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) GÂžnther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "glsl_geometry.h"
#include "Gem/Manager.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
# include <io.h>
# define close _close
#else
# include <unistd.h>
#endif

CPPEXTERN_NEW_WITH_ONE_ARG(glsl_geometry, t_symbol *, A_DEFSYM);

/////////////////////////////////////////////////////////
//
// glsl_geometry
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
glsl_geometry :: glsl_geometry() :
  m_shaderTarget(0),
  m_shader(0),
  m_shaderARB(0),
  m_compiled(0), m_size(0),
  m_shaderString(NULL), 
  m_shaderFilename(NULL),
  m_shaderID(0)
{
  // create an outlet to send shader object ID
  m_outShaderID = outlet_new(this->x_obj, &s_float);
}
glsl_geometry :: glsl_geometry(t_symbol *filename) :
  m_shaderTarget(0),
  m_shader(0),
  m_shaderARB(0),
  m_compiled(0), m_size(0), 
  m_shaderString(NULL), 
  m_shaderFilename(NULL),
  m_shaderID(0)
{
  openMess(filename);

  // create an outlet to send shader object ID
  m_outShaderID = outlet_new(this->x_obj, &s_float);
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
glsl_geometry :: ~glsl_geometry()
{
  closeMess();
}

////////////////////////////////////////////////////////
// closeMess
//
/////////////////////////////////////////////////////////
void glsl_geometry :: closeMess(void)
{
  if(m_shaderString)delete [] m_shaderString;
  m_shaderString=NULL;
  m_size=0;
  if(m_shader)
    glDeleteShader( m_shader );
  if(m_shaderARB)
    glDeleteObjectARB( m_shaderARB );

  gem::utils::glsl::delshader(m_shader);
  gem::utils::glsl::delshader(m_shaderARB);


  m_shader=0;
  m_shaderARB = 0;

  m_compiled=0;
}

////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
bool glsl_geometry :: openMessGL2(void)
{
  if (m_shader) {
    glDeleteShader( m_shader );
    gem::utils::glsl::delshader(m_shader);
  }
  m_shader = glCreateShader(m_shaderTarget);

  if (!m_shader)
    {
      error("could not create GLSL shader object");
      return false;
    }
  const char * vs = m_shaderString;
  glShaderSource( m_shader, 1, &vs, NULL );
  glCompileShader( m_shader );
  glGetShaderiv( m_shader, GL_COMPILE_STATUS, &m_compiled );
  if (!m_compiled) {
    GLint	length;
    GLchar* log;
    glGetShaderiv( m_shader, GL_INFO_LOG_LENGTH, &length );
    log = (GLchar*)malloc( length * sizeof(GLchar) );
    glGetShaderInfoLog( m_shader, length, NULL, log );
    post("compile Info_log:");
    post("%s", log );
    error("shader not loaded");
    free(log);
    return false;
  }
  return true;
}

bool glsl_geometry :: openMessARB(void)
{
  if(m_shaderARB) {
    glDeleteObjectARB( m_shaderARB );
    gem::utils::glsl::delshader(m_shaderARB);
  }
  m_shaderARB = glCreateShaderObjectARB(m_shaderTarget);

  if (!m_shaderARB)
    {
      error("could not create ARB shader object");
      return false;
    }
  const char * vs = m_shaderString;
  glShaderSourceARB( m_shaderARB, 1, &vs, NULL );
  glCompileShaderARB( m_shaderARB );
  glGetObjectParameterivARB( m_shaderARB, GL_OBJECT_COMPILE_STATUS_ARB, &m_compiled );
  if (!m_compiled) {
    GLint	length;
    GLcharARB* log;
    glGetObjectParameterivARB( m_shaderARB, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length );
    log = (GLcharARB*)malloc( length * sizeof(GLcharARB) );
    glGetInfoLogARB( m_shaderARB, length, NULL, log );
    post("compile Info_log:");
    post("%s", log );
    error("shader not loaded");
    free(log);
    return false;
  }

  return true;
}

void glsl_geometry :: openMess(t_symbol *filename)
{
  if(NULL==filename || NULL==filename->s_name)return;
  if(&s_==filename)return;
  if( !GLEW_VERSION_1_1 ) { /* stupid check whether we have a valid context */
    post("shader '%s' will be loaded when rendering is turned on (openGL context needed)", filename->s_name);
    m_shaderFilename=filename;
    return;
  }

  if(!isRunnable()) {
    return;
  }

  // Clean up any open files
  closeMess();

  std::string fn = findFile(filename->s_name);
  const char*buf=fn.c_str();

  FILE *file = fopen(buf,"rb");
  if(file) {
    fseek(file,0,SEEK_END);
    long size = ftell(file);
    if(size<0){fclose(file);error("error reading filesize");return;}
    m_shaderString = new char[size + 1];
    memset(m_shaderString,0,size + 1);
    fseek(file,0,SEEK_SET);
    size_t count=fread(m_shaderString,1,size,file);
    int err=ferror(file);
    fclose(file);
    if(err){error("error %d reading file (%d<%d)", err, count, size); return;}
  } else {
    error("could not find shader-file: '%s'", buf);
    return;
    /*
      // assuming that the "filename" is actually a shader-program per se
      m_shaderString = new char[strlen(buf) + 1];
      strcpy(m_shaderString,buf);
    */
  }
  m_size=strlen(m_shaderString);

  if(GLEW_EXT_geometry_shader4) // GLEW_VERSION_2_1
    openMessGL2();
  else if (GLEW_ARB_geometry_shader4) 
    openMessARB();

  logpost(NULL, 5, "Loaded file: %s", buf);
  m_shaderFilename=NULL;

  if (m_shader || m_shaderARB)
    {
      t_atom a;
      // send shaderID to outlet
      gem::utils::glsl::atom_setshader(a, m_shader?m_shader:m_shaderARB);

      outlet_list(m_outShaderID, gensym("list"), 1, &a);
    }
}

////////////////////////////////////////////////////////
// extension check
//
/////////////////////////////////////////////////////////
bool glsl_geometry :: isRunnable() {
  if(GLEW_EXT_geometry_shader4) { // GLEW_VERSION_2_1 ??
    m_shaderTarget = GL_GEOMETRY_SHADER_EXT;
    return true;
  } else if (GLEW_ARB_geometry_shader4) {
    m_shaderTarget = GL_GEOMETRY_SHADER_ARB;
    return true;
  }

  error("need OpenGL-2.1 (or at least the geometry-shader ARB-extension) to run GLSL");
  return false;
}

////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void glsl_geometry :: startRendering()
{
  if(NULL!=m_shaderFilename)
    openMess(m_shaderFilename);

  if (m_shaderString == NULL)
    {
      error("need to load a shader");
      return;
    }
}

////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void glsl_geometry :: render(GemState *state)
{
}

////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void glsl_geometry :: postrender(GemState *state)
{
}

////////////////////////////////////////////////////////
// printInfo
//
/////////////////////////////////////////////////////////
void glsl_geometry :: printInfo()
{
  if(GLEW_EXT_geometry_shader4 || GLEW_ARB_geometry_shader4) {
    GLint bitnum = 0;
    post("glsl_geometry Hardware Info");
    post("===========================");
    if(GLEW_EXT_geometry_shader4) {
      glGetIntegerv( GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT, &bitnum );
      post("MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT: %d", bitnum);
      glGetIntegerv( GL_MAX_VARYING_FLOATS, &bitnum );
      post("MAX_VARYING_FLOATS: %d", bitnum);
      glGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &bitnum );
      post("MAX_COMBINED_TEXTURE_IMAGE_UNITS: %d", bitnum);
      glGetIntegerv( GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT, &bitnum );
      post("MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT: %d", bitnum);
      glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &bitnum );
      post("MAX_TEXTURE_IMAGE_UNITS: %d", bitnum);
      glGetIntegerv( GL_MAX_TEXTURE_COORDS, &bitnum );
      post("MAX_TEXTURE_COORDS: %d", bitnum);
      if(m_shader) {
        post("compiled last shader to ID: %d", m_shader);
      }
    } else {
      glGetIntegerv( GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB, &bitnum );
      post("MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB: %d", bitnum);
      glGetIntegerv( GL_MAX_VARYING_FLOATS_ARB, &bitnum );
      post("MAX_VARYING_FLOATS: %d", bitnum);
      glGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &bitnum );
      post("MAX_COMBINED_TEXTURE_IMAGE_UNITS: %d", bitnum);
      glGetIntegerv( GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB, &bitnum );
      post("MAX_GEOMETRY_TEXTURE_IMAGE_UNITS: %d", bitnum);
      glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &bitnum );
      post("MAX_TEXTURE_IMAGE_UNITS: %d", bitnum);
      glGetIntegerv( GL_MAX_TEXTURE_COORDS_ARB, &bitnum );
      post("MAX_TEXTURE_COORDS: %d", bitnum);
     if(m_shaderARB) {
        post("compiled last shaderARB to ID: %d", m_shaderARB);
      }
    }
  } else post("no GLSL support");
}

////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void glsl_geometry :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&glsl_geometry::openMessCallback),
                  gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&glsl_geometry::printMessCallback),
                  gensym("print"), A_NULL);
}
void glsl_geometry :: openMessCallback(void *data, t_symbol *filename)
{
  GetMyClass(data)->openMess(filename);
}
void glsl_geometry :: printMessCallback(void *data)
{
  GetMyClass(data)->printInfo();
}
