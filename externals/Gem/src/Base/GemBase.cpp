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


/*
 * m_state: context dependent initialization state
 *
 * states:
 *
 * INIT: a context has just been created; initialize the object for this context
 * DISABLED: the object is not useable in this context
 * ENABLED: the object can run in this context; however, no context-ressources have been allocated yet (needs startRendering())
 * RENDERING: startRendering() has been called and we can render this object just fine...
 * MODIFIED: the object has been modified, and need to free it's context-ressources (stopRendering()) and request new ones (startRendering())
 *
 * state-change triggers:
 * reset(): -> INIT
 * setModified(): -> MODIFIED
 *
 * state-changers:
 * isRunnable(); INIT -> ENABLED|DISABLED
 * startRendering(): ENABLED->RENDERING
 * stopRendering(): MODIFIED->ENABLED
 *
 * 0..INIT ( => isRunnable() -> (startRendering() -> RENDERING) (-> DISABLED))
 * 1..RENDERING ( stopRendering() -> STOPPED)
 * 2..STOPPED ( -> INIT) (startRendering() -> RENDERING)
 * 3..DISABLED ( -> INIT)
 *
 *---
 * INIT -> isRunnable() -> (DISABLED) (ENABLED)
 * reset(): DISABLED -> INIT
 * ENABLED -> startRendering() -> RENDERING
 * setModified(): ENABLED
 * STOPPING -> stopRendering() -> ENABLED

 * RENDERING: render()

 * isRunnable() has to be called once for each (new) context
 * startRendering() has to be called for each context when new IDs are to be generated
 * stopRendering() has to be called for each context to free IDs

 * we need a mechanism to reset a context (e.g. because a context is destroyed and it's ID might be reused)
 */

#include "GemBase.h"
#include "Gem/Cache.h"

/////////////////////////////////////////////////////////
//
// GemBase
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
GemBase :: GemBase()
  : gem_amRendering(false), m_cache(NULL), m_modified(true),
    m_out1(NULL),
    m_enabled(true), m_state(INIT)
{
  m_out1 = outlet_new(this->x_obj, 0);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
GemBase :: ~GemBase()
{
  if (gem_amRendering){
    stopRendering();
    gem_amRendering=false;
  }

    if (m_out1)
        outlet_free(m_out1);
}

/////////////////////////////////////////////////////////
// gem_cacheMess
//
/////////////////////////////////////////////////////////
void GemBase :: gem_startstopMess(int state)
{
  // for now, this is important, as it is the only way to call the stopRendering
#if 1
  if (state && !gem_amRendering){
    m_enabled = isRunnable();
    if(m_enabled) {
      startRendering();
      m_state=RENDERING;
    }
  }
  else if (!state && gem_amRendering){
    if(m_enabled) {
      stopRendering();
      m_state=ENABLED;
    }
  }

  gem_amRendering=(state!=0);


  // continue sending out the cache message
  t_atom ap[1];
  SETFLOAT(ap, state);
  outlet_anything(this->m_out1, gensym("gem_state"), 1, ap);
#else
  post("gem_startstopMess(%d) called...please report this to the upstream developers", state);
#endif
}

/////////////////////////////////////////////////////////
// renderMess
//
/////////////////////////////////////////////////////////
void GemBase :: gem_renderMess(GemCache* cache, GemState*state)
{
  m_cache=cache;
  if(m_cache && m_cache->m_magic!=GEMCACHE_MAGIC)
    m_cache=NULL;
  if(INIT==m_state) {
    if(isRunnable()) {
      m_state=ENABLED;
    } else {
      m_state=DISABLED;
    }
  }
  if(MODIFIED==m_state) {
    stopRendering();
    m_state=ENABLED;
  }
  if(ENABLED==m_state) {
    startRendering();
    m_state=RENDERING;
  }
  if(RENDERING==m_state) {
    gem_amRendering=true;
    if(state)render(state);
    continueRender(state);
    if(state)postrender(state);
  }
  m_modified=false;
}

void GemBase :: continueRender(GemState*state){
  t_atom ap[2];
  ap->a_type=A_POINTER;
  ap->a_w.w_gpointer=(t_gpointer *)m_cache;  // the cache ?
  (ap+1)->a_type=A_POINTER;
  (ap+1)->a_w.w_gpointer=(t_gpointer *)state;
  outlet_anything(this->m_out1, gensym("gem_state"), 2, ap);
}



/////////////////////////////////////////////////////////
// setModified
//
/////////////////////////////////////////////////////////
void GemBase :: setModified()
{
  if (m_cache&& (m_cache->m_magic!=GEMCACHE_MAGIC))
    m_cache=NULL;
  if (m_cache) m_cache->dirty = true;
  m_modified=true;
  switch(m_state) {
  case DISABLED:
  case INIT:
    break;
  default:
    m_state=MODIFIED;
  }
}

/////////////////////////////////////////////////////////
// realStopRendering
//
/////////////////////////////////////////////////////////
void GemBase :: realStopRendering()
{
  /* no idea what this function is for; ask the user when it appears */
  post("realStopRendering() called...please report this to the upstream developers");
  stopRendering();
  m_cache = NULL;
  m_state=ENABLED;
}



/////////////////////////////////////////////////////////
// disabling the rendering of this object
//
/////////////////////////////////////////////////////////
bool GemBase :: isRunnable()
{
  return true;
}

enum GemBase::RenderState GemBase::getState(void) {
  return m_state;
}




/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void GemBase :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&GemBase::gem_MessCallback),
    	    gensym("gem_state"), A_GIMME, A_NULL);
}
void GemBase :: gem_MessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  if (argc==2 && argv->a_type==A_POINTER && (argv+1)->a_type==A_POINTER){
    GetMyClass(data)->gem_renderMess((GemCache *)argv->a_w.w_gpointer, (GemState *)(argv+1)->a_w.w_gpointer);
#if 1
  } else if (argc==1 && argv->a_type==A_FLOAT){
    GetMyClass(data)->gem_startstopMess(atom_getint(argv));  // start rendering (forget this !?)
#endif
  } else {
    GetMyClass(data)->error("wrong arguments in GemTrigger...");
  }
}
