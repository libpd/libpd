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
//    Copyright (c) 2002 tigital
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef _MSC_VER
# pragma NOTE("memory(484): warning C4150: Löschen eines Zeigers auf den nicht definierten Typ 'gem::GLStack::Data'. Destruktor wurde nicht aufgerufen.")
#endif

#include "Gem/GLStack.h"
#include "Gem/RTE.h"

/* need GLUtil for glReportError */ 
#include "Gem/GemGL.h"
#include "Utils/GLUtil.h"
#include <map>

#define GLDEBUG if(glReportError())::startpost("glError @ %s:%d[%s] ", __FILE__, __LINE__, __FUNCTION__), ::post

using namespace gem;

namespace gem {
  class GLStack::Data {
public:
  Data(void) {
    int i=0;
    for(i=0; i<4; i++) {
      stackDepth[i]=0;
      maxDepth[i]=0;
      orgDepth[i]=0;
    }
  }
  int stackDepth[4];
  int maxDepth[4];
  int orgDepth[4];
};
};

namespace {
  static std::map<enum GLStack::GemStackId, GLenum>s_id2mode;
  static std::map<enum GLStack::GemStackId, GLenum>s_id2depth;
  static std::map<enum GLStack::GemStackId, GLenum>s_id2maxdepth;
  static std::map<enum GLStack::GemStackId, bool>s_id2init;
}


GLStack:: GLStack(bool haveValidContext) : data(new Data()) {
  static bool firsttime=true;
  if(firsttime) {
    s_id2mode[MODELVIEW] =GL_MODELVIEW;
    s_id2mode[PROJECTION]=GL_PROJECTION;
    s_id2mode[TEXTURE]   =GL_TEXTURE;
    s_id2mode[COLOR]     =GL_COLOR;

    s_id2depth[MODELVIEW] =GL_MODELVIEW_STACK_DEPTH;
    s_id2depth[PROJECTION]=GL_PROJECTION_STACK_DEPTH;
    s_id2depth[TEXTURE]   =GL_TEXTURE_STACK_DEPTH;
    s_id2depth[COLOR]     =GL_COLOR_MATRIX_STACK_DEPTH;

    s_id2maxdepth[MODELVIEW] =GL_MAX_MODELVIEW_STACK_DEPTH;
    s_id2maxdepth[PROJECTION]=GL_MAX_PROJECTION_STACK_DEPTH;
    s_id2maxdepth[TEXTURE]   =GL_MAX_TEXTURE_STACK_DEPTH;
    s_id2maxdepth[COLOR]     =GL_MAX_COLOR_MATRIX_STACK_DEPTH;

    s_id2init[MODELVIEW] =false;
    s_id2init[PROJECTION]=false;
    s_id2init[TEXTURE]   =false;
    s_id2init[COLOR]     =false;
  }
  firsttime=false;

  if(haveValidContext) {
    reset();
  }
}
GLStack::~GLStack() {
}

#ifdef __GNUC__
# warning push/pop texture matrix has to be done per texunit
  // each texunit has it's own matrix to be pushed/popped
  // changing the texunit (e.g. in [pix_texture]) makes the 
  // local depthcounter a useless, and we get a lot of 
  // stack under/overflows
#endif


/** push the given matrix to the stack if the maximum has not been reached 
 *   returns true on success and false otherwise (stack overflow)
 * NOTE: needs valid openGL context
 */
bool GLStack::push(enum GemStackId id) {
  GLenum mode=s_id2mode[id];
  if(!mode)return false;
  if(data->stackDepth[id]<data->maxDepth[id]) {
    glMatrixMode(mode);
    glPushMatrix();
    data->stackDepth[id]++;
    return true;
  }

  data->stackDepth[id]++;
  return false;
}

void GLStack::push() {
  push(COLOR);
  push(TEXTURE);
  push(PROJECTION);
  push(MODELVIEW);
}


/** pop the given matrix from the stack if the maximum has not been reached 
 *   returns true on success and false otherwise (stack underlow)
 * NOTE: needs valid openGL context
 */
bool GLStack::pop(enum GemStackId id) {
  GLenum mode=s_id2mode[id];
  if(!mode)return false;

  data->stackDepth[id]--;
  if(data->stackDepth[id]<data->maxDepth[id]) {
    glMatrixMode(mode);
    glPopMatrix();
    return true;
  }
  return false;
}

void GLStack::pop() {
  pop(COLOR);
  pop(TEXTURE);
  pop(PROJECTION);
  pop(MODELVIEW);
}
/** 
 * reset the maximum stack depth of the given stack 
 * NOTE: needs valid openGL context
 */
void GLStack::reset() {
  reset(MODELVIEW);
  reset(PROJECTION);
  reset(TEXTURE);
  reset(COLOR);
}

/** 
 * reset the maximum stack depth of all stacks
 * NOTE: needs valid openGL context
 */
int GLStack::reset(enum GemStackId id) {
  bool firsttime=!(s_id2init[id]);
  if(firsttime) {
    s_id2init[id]=true;

    if(COLOR == id && !GLEW_ARB_imaging) {
      s_id2maxdepth[id]=0;
      s_id2depth[id]=0;
    }
    glReportError(); // clear any errors so far
  }


  GLenum maxdepth=s_id2maxdepth[id];
  GLenum depth=s_id2depth[id];

  if(maxdepth && depth) {
    /* hmm, some ati-cards (with fglrx) report GLEW_ARB_imaging support but fail the 'depth' test for COLOR */
    
    glGetIntegerv(maxdepth, data->maxDepth+id);
    if(firsttime && glReportError())s_id2maxdepth[id]=0;

    glGetIntegerv(depth, data->stackDepth+id);
    if(firsttime && glReportError())s_id2depth[id]=0;

    data->orgDepth[id]=data->stackDepth[id];
    return data->stackDepth[id];
  }
  return -1;
}

void GLStack::print() {
  post("MODELVIEW: %02d/%02d",  data->stackDepth[MODELVIEW], data->maxDepth[MODELVIEW]);
  post("PROJECTION: %02d/%02d",  data->stackDepth[PROJECTION], data->maxDepth[PROJECTION]);
  post("TEXTURE: %02d/%02d",  data->stackDepth[TEXTURE], data->maxDepth[TEXTURE]);
  post("COLOR: %02d/%02d",  data->stackDepth[COLOR], data->maxDepth[COLOR]);
}
