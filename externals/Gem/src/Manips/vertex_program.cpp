////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Created by tigital on 10/16/04.
// Copyright 2004-2006 James Tittle
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "vertex_program.h"

#include <string.h>
#include <stdio.h>
#ifdef _WIN32
# include <io.h>
# define close _close
#else
# include <unistd.h>
#endif

#include "Utils/GLUtil.h"

CPPEXTERN_NEW_WITH_ONE_ARG(vertex_program, t_symbol *, A_DEFSYM);

/////////////////////////////////////////////////////////
//
// vertex_program
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_program :: vertex_program() :
  m_programType(GEM_PROGRAM_none), 
  m_programID(0), 
  m_programString(NULL), m_size(0),
  m_buf(NULL),
  m_envNum(-1)
{
}
vertex_program :: vertex_program(t_symbol *filename) :
  m_programType(GEM_PROGRAM_none), 
  m_programID(0), 
  m_programString(NULL), m_size(0),
  m_envNum(-1)
{
  openMess(filename);
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_program :: ~vertex_program()
{
  closeMess();
}


/////////////////////////////////////////////////////////
// extension check
//
bool vertex_program :: isRunnable() {
  if(GLEW_ARB_vertex_program || GLEW_NV_vertex_program)
    return true;

  error("need ARB (or NV) vertex_program extension for shaders");
  return false;
}


////////////////////////////////////////////////////////
// closeMess
//
/////////////////////////////////////////////////////////
void vertex_program :: closeMess(void)
{
  delete [] m_programString;
  m_programString=NULL;
  m_size=0;
  if(m_programID){
    switch(m_programType){
    case(GEM_PROGRAM_NV):
      glDeleteProgramsNV(1,&m_programID);
      break;
    case(GEM_PROGRAM_ARB):
      glDeleteProgramsARB(1,&m_programID);
      break;
    default:
      break;
    }
  }

  m_programID=0;
  m_programType=GEM_PROGRAM_none;
}
////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
GLint vertex_program :: queryProgramtype(char*program)
{
  if(!strncmp(program,"!!ARBvp1.0",10)){
    m_programTarget=GL_VERTEX_PROGRAM_ARB;
    return(GEM_PROGRAM_ARB);
  }
  if(!strncmp(program,"!!VP1.0",7)){
    m_programTarget=GL_VERTEX_PROGRAM_NV;
    return(GEM_PROGRAM_NV);
  }
  return GEM_PROGRAM_none;
}


void vertex_program :: openMess(t_symbol *filename)
{
  if(NULL==filename || NULL==filename->s_name || &s_==filename || 0==*filename->s_name)return;

  // Clean up any open files
  closeMess();
  std::string fn = findFile(filename->s_name);
  m_buf=fn.c_str();

  FILE *file = fopen(m_buf,"rb");
  if(file) {
    fseek(file,0,SEEK_END);
    long size = ftell(file);
    if(size<0){fclose(file);error("error reading filesize");return;}
    m_programString = new char[size + 1];
    memset(m_programString,0,size + 1);
    fseek(file,0,SEEK_SET);
    size_t count = fread(m_programString,1,size,file);
    int err=ferror(file);
    fclose(file);
    if(err){error("error %d reading file (%d<%d)", err, count, size); return;}
  } else {
    m_programString = new char[strlen(m_buf) + 1];
    strcpy(m_programString,m_buf);
  }
  m_size=strlen(m_programString);
  m_programType=queryProgramtype(m_programString);
  if(m_programType==GEM_PROGRAM_none){
    m_programID = 0;
    char *s = m_programString;
    while(*s && *s != '\n') s++;
    *s = '\0';
    post("unknown program header \"%s\" or error open \"%s\" file\n",
         m_programString,
         filename->s_name);
    
    delete m_programString; m_programString=NULL;
    m_size=0;
    return;
  }

  post("Loaded file: %s\n", m_buf);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_program :: LoadProgram(void)
{
  if(NULL==m_programString)return;
  GLint err=-1;

  if((GEM_PROGRAM_NV == m_programType) && (!GLEW_NV_vertex_program)) {
    error("NV vertex programs not supported by this system");
    return;
  }

  if((GEM_PROGRAM_ARB == m_programType) && (!GLEW_ARB_vertex_program)) {
    error("ARB vertex programs not supported by this system");
    return;
  }

  switch(m_programType){
  case  GEM_PROGRAM_NV:
    if (m_programID==0)
      {
        glEnable(m_programTarget);
        glGenProgramsNV(1, &m_programID);
        glBindProgramNV(m_programTarget, m_programID);
        glLoadProgramNV(m_programTarget, m_programID, m_size, (GLubyte*)m_programString);
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &err);
      } else {
      glEnable(m_programTarget);
      glBindProgramNV(m_programTarget, m_programID);
      return;
    }
    break;
  case  GEM_PROGRAM_ARB:
    if (m_programID==0)
      {
        glEnable(m_programTarget);
        glGenProgramsARB(1, &m_programID);
        glBindProgramARB( m_programTarget, m_programID);
        glProgramStringARB( m_programTarget, GL_PROGRAM_FORMAT_ASCII_ARB, m_size, m_programString);
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err);
      } else {
      glEnable(m_programTarget);
      glBindProgramARB(m_programTarget, m_programID);
      return;
    }
    break;
  default:
    return;
  }

  if(err != -1) {
    int line = 0;
    char *s = m_programString;
    while(err-- && *s) if(*s++ == '\n') line++;
    while(s >= m_programString && *s != '\n') s--;
    char *e = ++s;
    while(*e != '\n' && *e != '\0') e++;
    *e = '\0';
    error("program error at line %d:\n\"%s\"\n",line,s);
    post("%s\n", glGetString(GL_PROGRAM_ERROR_STRING_ARB));
  }

  if(GLEW_ARB_vertex_program) {
    GLint isUnderNativeLimits;
    glGetProgramivARB( m_programTarget, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isUnderNativeLimits);
  
    // If the program is over the hardware's limits, print out some information
    if (isUnderNativeLimits!=1)
      {
        // Go through the most common limits that are exceeded
        error("is beyond hardware limits");

        GLint aluInstructions, maxAluInstructions;
        glGetProgramivARB(m_programTarget, GL_PROGRAM_ALU_INSTRUCTIONS_ARB, &aluInstructions);
        glGetProgramivARB(m_programTarget, GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB, &maxAluInstructions);
        if (aluInstructions>maxAluInstructions)
          post("[%s]: Compiles to too many ALU instructions (%d, limit is %d)\n", m_buf, aluInstructions, maxAluInstructions);

        GLint textureInstructions, maxTextureInstructions;
        glGetProgramivARB(m_programTarget, GL_PROGRAM_TEX_INSTRUCTIONS_ARB, &textureInstructions);
        glGetProgramivARB(m_programTarget, GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB, &maxTextureInstructions);
        if (textureInstructions>maxTextureInstructions)
          post("[%s]: Compiles to too many texture instructions (%d, limit is %d)\n", m_buf, textureInstructions, maxTextureInstructions);

        GLint textureIndirections, maxTextureIndirections;
        glGetProgramivARB(m_programTarget, GL_PROGRAM_TEX_INDIRECTIONS_ARB, &textureIndirections);
        glGetProgramivARB(m_programTarget, GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB, &maxTextureIndirections);
        if (textureIndirections>maxTextureIndirections)
          post("[%s]: Compiles to too many texture indirections (%d, limit is %d)\n", m_buf, textureIndirections, maxTextureIndirections);

        GLint nativeTextureIndirections, maxNativeTextureIndirections;
        glGetProgramivARB(m_programTarget, GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, &nativeTextureIndirections);
        glGetProgramivARB(m_programTarget, GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, &maxNativeTextureIndirections);
        if (nativeTextureIndirections>maxNativeTextureIndirections)
          post("[%s]: Compiles to too many native texture indirections (%d, limit is %d)\n", m_buf, nativeTextureIndirections, maxNativeTextureIndirections);

        GLint nativeAluInstructions, maxNativeAluInstructions;
        glGetProgramivARB(m_programTarget, GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, &nativeAluInstructions);
        glGetProgramivARB(m_programTarget, GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, &maxNativeAluInstructions);
        if (nativeAluInstructions>maxNativeAluInstructions)
          post("[%s]: Compiles to too many native ALU instructions (%d, limit is %d)\n", m_buf, nativeAluInstructions, maxNativeAluInstructions);
      }
  }
}

void vertex_program :: startRendering()
{
  if (m_programString == NULL)
    {
      error("need to load a program");
      return;
    }

  LoadProgram();
}

void vertex_program :: render(GemState *state)
{
  LoadProgram();

  /* actually glProgramEnvParameter4fvARB really depends on GL_ARB_vertex_program
   * and cannot be used with _only_ GL_NV_vertex_program
   */
  if(GLEW_ARB_vertex_program) {
    if(m_programID&&(m_envNum>=0)){
      glProgramEnvParameter4fvARB(m_programTarget, m_envNum, m_param);
    }
  }
} 

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void vertex_program :: postrender(GemState *state)
{
  if(m_programID){
    switch(m_programType){
    case  GEM_PROGRAM_NV:  case  GEM_PROGRAM_ARB:
      glDisable( m_programTarget );
      break;
    default:
      break;
    }
  }
}

void vertex_program :: paramMess(int envNum, t_float param1, t_float param2, t_float param3, t_float param4)
{
  if(envNum>=0){
    //float param[4] = {param1, param2, param3, param4};
	
    m_param[0] = param1;
    m_param[1] = param2;
    m_param[2] = param3;
    m_param[3] = param4;

    m_envNum = envNum;
  } else 
    m_envNum = -1;
}
/////////////////////////////////////////////////////////
// printInfo
//
/////////////////////////////////////////////////////////
void vertex_program :: printInfo()
{
  GLint bitnum = 0;

  if(!GLEW_ARB_vertex_program) {
    post("no ARB vertex support!");
    return;
  }

	post("Vertex_Program Hardware Info");
	post("============================");
	glGetIntegerv( GL_MAX_VERTEX_ATTRIBS_ARB, &bitnum );
	post("MAX_VERTEX_ATTRIBS: %d", bitnum);
	glGetIntegerv( GL_MAX_PROGRAM_MATRICES_ARB, &bitnum );
	post("MAX_PROGRAM_MATRICES: %d", bitnum);
	glGetIntegerv( GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB, &bitnum );
	post("MAX_PROGRAM_MATRIX_STACK_DEPTH: %d", bitnum);
	
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_INSTRUCTIONS_ARB, &bitnum);
	post("MAX_PROGRAM_INSTRUCTIONS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, &bitnum);
	post("MAX_PROGRAM_NATIVE_INSTRUCTIONS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_TEMPORARIES_ARB, &bitnum);
	post("MAX_PROGRAM_TEMPORARIES: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB, &bitnum);
	post("MAX_PROGRAM_NATIVE_TEMPORARIES: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_PARAMETERS_ARB, &bitnum);
	post("MAX_PROGRAM_PARAMETERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB, &bitnum);
	post("MAX_PROGRAM_NATIVE_PARAMETERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ATTRIBS_ARB, &bitnum);
	post("MAX_PROGRAM_ATTRIBS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB, &bitnum);
	post("MAX_PROGRAM_NATIVE_ATTRIBS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB, &bitnum);
	post("MAX_PROGRAM_ADDRESS_REGISTERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, &bitnum);
	post("MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &bitnum);
	post("MAX_PROGRAM_LOCAL_PARAMETERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, &bitnum);
	post("MAX_PROGRAM_ENV_PARAMETERS: %d", bitnum);
	post("");
	
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_INSTRUCTIONS_ARB, &bitnum);
	post("PROGRAM_INSTRUCTIONS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB, &bitnum);
	post("PROGRAM_NATIVE_INSTRUCTIONS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_TEMPORARIES_ARB, &bitnum);
	post("PROGRAM_TEMPORARIES: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_TEMPORARIES_ARB, &bitnum);
	post("PROGRAM_NATIVE_TEMPORARIES: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_PARAMETERS_ARB, &bitnum);
	post("PROGRAM_PARAMETERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_PARAMETERS_ARB, &bitnum);
	post("PROGRAM_NATIVE_PARAMETERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_ATTRIBS_ARB, &bitnum);
	post("PROGRAM_ATTRIBS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_ATTRIBS_ARB, &bitnum);
	post("PROGRAM_NATIVE_ATTRIBS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_ADDRESS_REGISTERS_ARB, &bitnum);
  post("PROGRAM_ADDRESS_REGISTERS: %d", bitnum);
	glGetProgramivARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, &bitnum);
	post("PROGRAM_NATIVE_ADDRESS_REGISTERS: %d", bitnum);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_program :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_program::openMessCallback),
                  gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_program::printMessCallback),
                  gensym("print"), A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_program::paramMessCallback),
                  gensym("parameter"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT,A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_program::paramMessCallback),
                  gensym("param"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT,A_NULL);
}
void vertex_program :: openMessCallback(void *data, t_symbol *filename)
{
  GetMyClass(data)->openMess(filename);
}
void vertex_program :: printMessCallback(void *data)
{
	GetMyClass(data)->printInfo();
}

void vertex_program :: paramMessCallback(void *data, t_float envNum, t_float param1, t_float param2, t_float param3, t_float param4)
{
  GetMyClass(data)->paramMess(static_cast<int>(envNum), param1, param2, param3, param4);
}
