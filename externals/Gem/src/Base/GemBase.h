/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  The base class for all of the gem objects

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger.
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
	
  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMBASE_H_
#define _INCLUDE__GEM_BASE_GEMBASE_H_

#include "Gem/GemGL.h"
#include "Gem/ContextData.h"

#include "Base/CPPExtern.h"

class GemCache;
class GemState;
/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  GemBase
    
  Base class for gem objects

  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN GemBase : public CPPExtern
{
 protected:
    	
  //////////
  // Constructor
  GemBase();
    	
  //////////
  // Destructor
  virtual     	~GemBase();

  //////////
  virtual void 	render(GemState *state) = 0;

  //////////
  void    continueRender(GemState *state);

  //////////
  // After objects below you in the chain have finished.
  // You should reset all GEM/OpenGL states here.
  virtual void 	postrender(GemState *)              { ; }

  //////////
  // Called when rendering stops

#if 1/*(jmz) this seems to be for gem2pdp*/
  virtual void 	stoprender()			{ realStopRendering(); }
#endif

  //////////
  // If you care about the start of rendering
  virtual void	startRendering()                    { ; }

  //////////
  // If you care about the stop of rendering
  virtual void	stopRendering()    	                { ; }


  //////////
  // has rendering started ?
  // deprecated, use 'getState()==RENDERING' instead
  bool            gem_amRendering;
    	
  //////////
  // If anything in the object has changed
  virtual void  	setModified();

  //////////
  // Don't mess with this unless you know what you are doing.
  GemCache    	*m_cache;
  //////////
  // check whether this object has changed
  bool             m_modified;

  //////////
  // The outlet
  t_outlet    	*m_out1;


  //////////
  // this gets called in the before the startRendering() routine
  // if it returns TRUE, the object's startRendering(), render() and stopRendering() functions will be called
  // it it returns FALSE, the object will be disabled 
  // when rendering is restarted, this function get's called again
  // the default is to enable rendering
  // this function is important if you want to disable an object because it cannot be used (e.g. missing driver support)
  virtual bool isRunnable(void);

  //////////
  // creation callback
  static void 	real_obj_setupCallback(t_class *classPtr)
    { CPPExtern::real_obj_setupCallback(classPtr); GemBase::obj_setupCallback(classPtr); }

    	
 private:
    
  void	    	realStopRendering();
  void            gem_startstopMess(int state);
  void            gem_renderMess(GemCache* state, GemState* state2);

  static inline GemBase *GetMyClass(void *data) {return((GemBase *)((Obj_header *)data)->data);}

  friend class    gemhead;
  static void 	obj_setupCallback(t_class *classPtr);
  static void 	gem_MessCallback(void *, t_symbol *,int, t_atom*);
  static void 	renderCallback(GemBase *data, GemState *state);
  static void 	postrenderCallback(GemBase *data, GemState *state);
#if 1 /*jmz this seems to be for gem2pdp*/
  static void	stoprenderCallback(GemBase *data);	//DH
#endif

  /* whether the object is internally disabled or not 
   * objects are to be disabled, if the system cannot make use of them, e.g. because of unsupported openGL features
   */
  gem::ContextData<bool>m_enabled;

  enum RenderState {INIT, ENABLED, DISABLED, RENDERING, MODIFIED};
  gem::ContextData<enum RenderState>m_state;

 protected:
  enum RenderState getState(void);

};

#endif	// for header file
