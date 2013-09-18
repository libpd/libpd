/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  handling of diverse openGL stacks

  Copyright (c) 1997-2000 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/
#ifndef _INCLUDE__GEM_GEM_GLSTACK_H_
#define _INCLUDE__GEM_GEM_GLSTACK_H_

#include "Gem/ExportDef.h"

#include <memory>

namespace gem {
class GEM_EXTERN GLStack {
 public:
  GLStack(bool haveValidContext=false);
  virtual ~GLStack(void);

  enum GemStackId { MODELVIEW, COLOR, TEXTURE, PROJECTION };

  /** push the given matrix to the stack if the maximum has not been reached 
   *   returns true on success and false otherwise (stack overflow)
   * NOTE: needs valid openGL context
   * NOTE: might change the current matrix mode; you have to manually reset it
   */
  bool push(enum GemStackId);

  /** pushes all matrices possible
   * NOTE: finishes in MODELVIEW matrix mode
   */
  void push(void);

  /** pop the given matrix from the stack if the maximum has not been reached 
   *   returns true on success and false otherwise (stack underflow)
   * NOTE: needs valid openGL context
   * NOTE: might change the current matrix mode; you have to manually reset it
   */
  bool pop(enum GemStackId);
  /** pops all matrices possible
   * NOTE: finishes in MODELVIEW matrix mode
   */
  void pop(void);

  /** 
   * reset the maximum stack depth of the given stack 
   * NOTE: needs valid openGL context
   */
  void reset(void);
  /** 
   * reset the maximum stack depth of all stacks
   *  returns the current stack depth (-1 on failure)
   * NOTE: needs valid openGL context
   */
  int reset(enum GemStackId);

  void print(void);

 private:
  class Data;
  std::auto_ptr<Data>data;
};

} /* namespace gem */


#endif /* _INCLUDE__GEM_GEM_GLSTACK_H_ */
