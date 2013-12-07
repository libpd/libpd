////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "GemContext.h"
#include "Gem/Manager.h"
#include "Gem/Exception.h"

#include "Gem/RTE.h"

#include <stack>
#include <set>

#ifdef GEM_MULTICONTEXT
# warning multicontext rendering currently under development
#endif /* GEM_MULTICONTEXT */

static GLEWContext*s_glewcontext=NULL;
static GemGlewXContext*s_glewxcontext=NULL;

using namespace gem;

class Context::PIMPL {
public:
  PIMPL(void) : 
#ifdef GEM_MULTICONTEXT
    context(new GLEWContext), xcontext(new GemGlewXContext), 
#else
    context(NULL), xcontext(NULL), 
#endif
    contextid(makeID())
  {
    /* check the stack-sizes */
    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH,    maxStackDepth+GemMan::STACKMODELVIEW);
    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH,      maxStackDepth+GemMan::STACKTEXTURE);
    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH,   maxStackDepth+GemMan::STACKPROJECTION);

    if(GLEW_ARB_imaging)
      glGetIntegerv(GL_MAX_COLOR_MATRIX_STACK_DEPTH, maxStackDepth+GemMan::STACKCOLOR);
    else 
      maxStackDepth[GemMan::STACKCOLOR]=0;
  }

  PIMPL(const PIMPL&p) : 
#ifdef GEM_MULTICONTEXT
    context(new GLEWContext(*p.context)), xcontext(new GemGlewXContext(*p.xcontext)), 
#else
    context(NULL), xcontext(NULL), 
#endif
    contextid(makeID())
  {
    /* check the stack-sizes */
    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH,    maxStackDepth+GemMan::STACKMODELVIEW);
    glGetIntegerv(GL_MAX_COLOR_MATRIX_STACK_DEPTH, maxStackDepth+GemMan::STACKCOLOR);
    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH,      maxStackDepth+GemMan::STACKTEXTURE);
    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH,   maxStackDepth+GemMan::STACKPROJECTION);
  }

  ~PIMPL(void) {
    freeID(contextid);
#ifdef GEM_MULTICONTEXT
    if(context )delete context; context=NULL;
    if(xcontext)delete xcontext; xcontext=0;
#endif
  }

  GLint maxStackDepth[4];

  GLEWContext    *context;
  GemGlewXContext*xcontext;

  unsigned int contextid;

  // LATER: reusing IDs prevents a memleak in gem::ContextData
  // LATER: reusing IDs might make us re-use invalid gem::ContextData!
  static std::set<unsigned int>s_takenIDs;
  static unsigned int makeID(void) //  GemContext_newid
  {
    unsigned int id=0;
#ifdef GEM_MULTICONTEXT
    while(s_takenIDs.find(id) != s_takenIDs.end())
      id++;
#endif /* GEM_MULTICONTEXT */
    s_takenIDs.insert(id);
    return id;
  }
  static void freeID(unsigned int id)
  {
    /* LATER reuse freed ids */
    /* LATER remove this ID from the s_contextid stack and related (x)context */
    s_takenIDs.erase(id);
  }

  static unsigned int s_contextid;
  static GLEWContext*s_context;
  static GemGlewXContext*s_xcontext;
};
unsigned int    Context::PIMPL::s_contextid=0;
GLEWContext*    Context::PIMPL::s_context=NULL;
GemGlewXContext*Context::PIMPL::s_xcontext=NULL;
std::set<unsigned int>      Context::PIMPL::s_takenIDs;

Context::Context(void) 
  : m_pimpl(new PIMPL())
{
  push();
  std::string errstring="";

  GLenum err = glewInit();
  
  if (GLEW_OK != err) {
    if(GLEW_ERROR_GLX_VERSION_11_ONLY == err) {
      errstring="failed to init GLEW (glx): continuing anyhow - please report any problems to the gem-dev mailinglist!";
    } else if (GLEW_ERROR_GL_VERSION_10_ONLY) {
      errstring="failed to init GLEW: your system only supports openGL-1.0";
    } else {
      errstring="failed to init GLEW";
    }
  }

  post("GLEW version %s",glewGetString(GLEW_VERSION));

  if(!m_pimpl) {
    errstring="failed to init GemContext";
  }

  pop();

  if(!errstring.empty()) {
    if(m_pimpl)delete m_pimpl; m_pimpl=NULL;
    throw(GemException(errstring));
  }
  GemMan::m_windowState++;
}

Context::Context(const Context&c) 
  : m_pimpl(new PIMPL(*(c.m_pimpl)))
{
  push();
  post("foo GLEW version %s",glewGetString(GLEW_VERSION));
  pop();
}

Context&Context::operator=(const Context&c) {
  if(&c == this || c.m_pimpl == m_pimpl)
    return (*this);

  if(m_pimpl)delete m_pimpl;
  m_pimpl=new PIMPL(*c.m_pimpl);
  push();
  pop();

  return(*this);
}


Context::~Context(void) {
  if(m_pimpl)delete m_pimpl; m_pimpl=NULL;
  GemMan::m_windowState--;
}

bool Context::push(void) {
  GemMan::maxStackDepth[GemMan::STACKMODELVIEW]= m_pimpl->maxStackDepth[GemMan::STACKMODELVIEW];
  GemMan::maxStackDepth[GemMan::STACKCOLOR]=     m_pimpl->maxStackDepth[GemMan::STACKCOLOR];
  GemMan::maxStackDepth[GemMan::STACKTEXTURE]=   m_pimpl->maxStackDepth[GemMan::STACKTEXTURE];
  GemMan::maxStackDepth[GemMan::STACKPROJECTION]=m_pimpl->maxStackDepth[GemMan::STACKPROJECTION];

  m_pimpl->s_context=m_pimpl->context;
  m_pimpl->s_xcontext=m_pimpl->xcontext;
  m_pimpl->s_contextid=m_pimpl->contextid;
  return true;
}

bool Context::pop(void) {
  return true;
}

unsigned int Context::getContextId(void) {
  return PIMPL::s_contextid;
}

/* returns the last GemWindow that called makeCurrent()
 * LATER: what to do if this has been invalidated (e.g. because the context was destroyed) ? 
 */
GLEWContext*Context::getGlewContext(void) {
  return PIMPL::s_context;
}
GemGlewXContext*Context::getGlewXContext(void) {
  return PIMPL::s_xcontext;
}

GLEWContext*glewGetContext(void)     {return  gem::Context::getGlewContext();}
GemGlewXContext*wglewGetContext(void){return  gem::Context::getGlewXContext();}
GemGlewXContext*glxewGetContext(void){return  gem::Context::getGlewXContext();}

