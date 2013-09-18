/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  ordered receive
    
  Copyright (c) 2008-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_CONTROLS_GEMRECEIVE_H_
#define _INCLUDE__GEM_CONTROLS_GEMRECEIVE_H_

#include "Base/CPPExtern.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  gemreceive
    
  [receive] with explicit order

  DESCRIPTION

  this object is really totally unrelated to Gem.
  however, it is included here, in order to replace the current
  (integrated) implementation of [gemhead] with an abstraction that makes use
  of an ordered [receive]

  the original object was [oreceive] in the iemguts library
    
  -----------------------------------------------------------------*/

EXTERN_STRUCT _bind_element;
EXTERN_STRUCT _gemreceive_proxy;

typedef struct _bind_element t_bind_element;
typedef struct _gemreceive_proxy t_gemreceive_proxy;


class GEM_EXTERN gemreceive : public CPPExtern
{
  CPPEXTERN_HEADER(gemreceive, CPPExtern);

    public:

  //////////
  // Constructor
  gemreceive(t_symbol*s, t_floatarg f=50.f);

 protected:
    	
  //////////
  // Destructor
  virtual ~gemreceive();
        
  //////////
  // keyboard-button
  virtual void receive(t_symbol*s, int argc, t_atom*argv);

  //////////
  // the symbol we are bound to
  void nameMess(t_symbol*s);
  t_symbol*m_name;

  //////////
  // the receive priority
  void priorityMess(t_float f);
  t_float m_priority;

  //////////
  // The receive outlet
  t_outlet    	*m_outlet;

  //////////
  // inlet for priority
  t_inlet*m_fltin;


 private:

  //////////
  // Static member functions
  static void     proxyCallback(t_gemreceive_proxy*, t_symbol*s, int argc, t_atom*argv);

  //////////
  static void     nameCallback(void *data, t_symbol*s);
  static void     priorityCallback(void *data, t_float f);

  static t_gemreceive_proxy*find_key(t_symbol*);
  static t_gemreceive_proxy*add_key(t_symbol*);
  static void add_element(t_gemreceive_proxy*, t_bind_element*);

  static t_gemreceive_proxy*proxy_list;


 public:
  static void bind(gemreceive*x, t_symbol*name, t_float priority);
  static void unbind(gemreceive*x, t_symbol*name);


};

#endif  // for header file
