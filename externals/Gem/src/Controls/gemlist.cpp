////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// ch@chdh.net
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "gemlist.h"

#include "Gem/Cache.h"
#include "Gem/Manager.h"

CPPEXTERN_NEW(gemlist);

/////////////////////////////////////////////////////////
//
// gemlist
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
gemlist :: gemlist(void) 
  : m_validState(false),
    m_tickTime(-1.f),
    m_lightState(false),
    m_drawType(0),
    m_mycache(new GemCache(NULL)),
    m_inlet(NULL)
{
  // create the cold inlet
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_right"));
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
gemlist :: ~gemlist()
{
  if(m_mycache)delete m_mycache; m_mycache=NULL;
  if(m_inlet)inlet_free(m_inlet); m_inlet=NULL;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void gemlist :: render(GemState *state)
{
  m_state=*state; //copy current state for later use
  m_validState=true;
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void gemlist :: postrender(GemState *)
{
  //	m_validState=false;

  // this is to early to reset the m_validState
  // when should we call this???
  // TODO : fix this.
}

void gemlist :: sendCacheState(GemCache *cache, GemState*state)
{
  if  ( !GemMan::windowExists() ) {
    // LATER: shouldn't this test for a valid context rather than an existing window??
    //	error("you should not bang the gemlist now"); 
    return;
  }
  
  if(state) {
    t_atom ap[2];
    
    if(!cache)
      cache=m_mycache;
    
    ap->a_type=A_POINTER;
    ap->a_w.w_gpointer=reinterpret_cast<t_gpointer*>(cache);
    (ap+1)->a_type=A_POINTER;
    (ap+1)->a_w.w_gpointer=reinterpret_cast<t_gpointer*>(state);
    outlet_anything(m_out1, gensym("gem_state"), 2, ap);
  }
}

/////////////////////////////////////////////////////////
// bang
//
/////////////////////////////////////////////////////////
void gemlist :: trigger()
{
  if(m_validState) {
    // outlet the current state when banged
    sendCacheState(m_cache, &m_state);
  } else {
    //    GemMan::fillGemState(state);
    sendCacheState(NULL, &m_mystate);
  }
}

///////////////////
// here come some messages for setting up a manual GemState
void gemlist :: ticktimeMess(t_float ticktime)
{
  m_tickTime=ticktime;
  m_mystate.set(GemState::_TIMING_TICK, m_tickTime);

}
void gemlist :: lightingMess(bool light)
{
  m_lightState=light;
  m_mystate.set(GemState::_GL_LIGHTING, m_lightState);
  m_mystate.set(GemState::_GL_SMOOTH, m_lightState);
}
void gemlist :: drawMess(t_atom arg)
{
  if(A_SYMBOL==arg.a_type) {
    t_symbol*type=atom_getsymbol(&arg);
    char c=*type->s_name;
    switch (c){
    case 'D': case 'd': // default
      m_drawType = GL_DEFAULT_GEM;
      break;
    case 'L': case 'l': // line
      m_drawType = GL_LINE;
      break;
    case 'F': case 'f': // fill
      m_drawType = GL_FILL;
      break;
    case 'P': case 'p': // point
      m_drawType = GL_POINT;
      break;
    default:
      m_drawType = static_cast<GLenum>(getGLdefine(&arg));
    }
  }
  else m_drawType=atom_getint(&arg);

  m_mystate.set(GemState::_GL_DRAWTYPE, m_drawType);
}

/////////////////////////////////////////////////////////
// rightRender
//
/////////////////////////////////////////////////////////
void gemlist :: rightRender(GemCache*cache, GemState *state)
{
  if(state) {
    m_state=*state;
    m_validState=true;
    // get the current state on the right inlet
  } else {
    m_validState=false;
  }
  m_cache=cache;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemlist :: obj_setupCallback(t_class *classPtr)
{
  class_addbang(classPtr, reinterpret_cast<t_method>(&gemlist::triggerMessCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&gemlist::gem_rightMessCallback), gensym("gem_right"), A_GIMME, A_NULL);


  class_addmethod(classPtr, reinterpret_cast<t_method>(&gemlist::ticktimeMessCallback), gensym("ticktime"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&gemlist::lightingMessCallback), gensym("lighting"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&gemlist::drawMessCallback), gensym("draw"), A_GIMME, A_NULL);
}

void gemlist::triggerMessCallback(void *data)
{
  GetMyClass(data)->trigger();
}

void gemlist :: ticktimeMessCallback(void *data, t_floatarg time)
{
  GetMyClass(data)->ticktimeMess(time);
}
void gemlist :: lightingMessCallback(void *data, t_floatarg light)
{
  GetMyClass(data)->lightingMess(light>0.f);
}
void gemlist :: drawMessCallback(void *data, int argc, t_atom*argv)
{
  if(argc==1)GetMyClass(data)->drawMess(argv[0]);
}

void gemlist::gem_rightMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  GemCache*cache=NULL;
  GemState*state=NULL;
  if (argc==1 && argv->a_type==A_FLOAT){
    GetMyClass(data)->rightRender(cache, state);
  } else if (argc==2 && argv->a_type==A_POINTER && (argv+1)->a_type==A_POINTER){
    cache=reinterpret_cast<GemCache*>(argv->a_w.w_gpointer);
    state=reinterpret_cast<GemState*>((argv+1)->a_w.w_gpointer);
    GetMyClass(data)->rightRender( cache, state );
  } else GetMyClass(data)->error("wrong righthand arguments....");
}
