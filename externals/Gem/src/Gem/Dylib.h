/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	- registers a loader with Pd

    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_DYLIB_H_
#define _INCLUDE__GEM_GEM_DYLIB_H_

#include "Gem/Exception.h"


/* an opaque handle to the platform specific library handle */
class GemDylibHandle;
class CPPExtern;

class GEM_EXTERN GemDylib {
 private:
  GemDylibHandle*m_handle;

 public:
  GemDylib(const CPPExtern*obj, 
	   const std::string libname, 
	   const std::string extension=std::string("")
	   ) throw(GemException);
  GemDylib(const std::string libname, 
	   const std::string extension=std::string("")
	   ) throw(GemException);

  GemDylib(const GemDylib&);

  virtual ~GemDylib(void);

  typedef void (*function_t)(void);

  virtual GemDylib& operator=(const GemDylib&);

  // if void<procname>(void) exists in dylib, run it and return "true"
  // else return false;
  bool run(const std::string procname);

  // if <procname> exists in dylib, return it, else return NULL
  function_t proc(const std::string procname);

  public:
  /**
   * LoadLib(): convenience function that searches a library named <baselibname> and then runs <procname>()
   * if "extension" is NULL, a plaform-specific default is used
   * on success "true" is returned, else "false
   */
  static bool LoadLib(const std::string procname, 
		      const std::string baselibname, 
		      const std::string fileext=std::string(""));


  static const std::string getDefaultExtension(void);
};




#endif
