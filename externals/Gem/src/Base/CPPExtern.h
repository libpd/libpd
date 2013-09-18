/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    The base class for all externs written in C++

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_CPPEXTERN_H_
#define _INCLUDE__GEM_BASE_CPPEXTERN_H_

#include "Gem/ExportDef.h"

#include "Gem/RTE.h"
#include "Gem/Version.h"

#include <new>
#include <string>

class CPPExtern;

/* forward declaration of a generic exception handler for GemExceptions */
namespace gem {
  GEM_EXTERN void catchGemException(const char*objname, const t_object*obj);
};

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    Obj_header
    
    The obligatory object header

DESCRIPTION
    
    This is in a separate struct to assure that when PD uses the
    class, the t_object is the very first thing.  If it were the 
    first thing in CPPExtern, then there could be problems with
    the vtable.
    
-----------------------------------------------------------------*/
struct GEM_EXTERN Obj_header
{
    	//////////
    	// The obligatory object header
    	t_object    	    pd_obj;

    	//////////
    	// Our data structure
        CPPExtern           *data;

  // This has a dummy arg so that NT won't complain
  void *operator new(size_t, void *location, void *dummy);
};

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    CPPExtern
    
    The base class for all externs written in C++

DESCRIPTION
    
    Each extern which is written in C++ needs to use the #defines at the
    end of this header file.  Currently, the operator new(size_t) and
    operator delete(void *) are not overridden.  This will be a problem
    when PD expects everything to fit in its memory space and control
    all memory allocation.
    
    The define
    
        CPPEXTERN_HEADER(NEW_CLASS, PARENT_CLASS);
    
    should be somewhere in your header file.
    One of the defines like
    
    CPPEXTERN_NEW(NEW_CLASS);
    CPPEXTERN_NEW_WITH_TWO_ARGS(NEW_CLASS, t_floatarg, A_FLOAT, t_floatarg, A_FLOAT);
    
    should be the first thing in your implementation file.
    NEW_CLASS is the name of your class and PARENT_CLASS is the 
    parent of your class.
        
-----------------------------------------------------------------*/
class GEM_EXTERN CPPExtern
{
    public:

        //////////
        // Constructor
    	CPPExtern(void);

        //////////
        // The Pd header
        t_object          *x_obj;

    	//////////
    	// Destructor
    	virtual ~CPPExtern(void) = 0;
    	
        //////////
        // Get the object's canvas
        const t_canvas            *getCanvas(void) const       { return(m_canvas); }

        //////////
        // This is a holder - don't touch it
        static t_object     *m_holder;

        //////////
        // my name
        static char          *m_holdname;
        t_symbol             *m_objectname;

    protected:
    	
    	//////////
    	// Creation callback
    	static void 	real_obj_setupCallback(t_class *) {}

    private:

      //////////
      // The canvas that the object is in
      t_canvas            *m_canvas;

 public:
      // these call pd's print-functions, and eventually prepend the object's name
      void            startpost(const char*format, ...) const;
      void            post(const char*format, ...) const;
      void            endpost(void) const;
      void            verbose(const int level, const char*format, ...) const;
      void            error(const char*format, ...) const; /* internally uses pd_error() */

      // searches for a file based on the parent abstraction's path
      // wraps open_via_path() and canvas_makefilename()
      // the full filename is returned
      // if the file does not exist, it is constructed
      std::string findFile(const std::string filename, const std::string ext) const;
      std::string findFile(const std::string filename) const;

 private:
	mutable bool m_endpost; /* internal state for startpost/post/endpost */
	static bool checkGemVersion(const int major, const int minor);
  CPPExtern(const CPPExtern&);
  virtual CPPExtern&operator=(const CPPExtern&);
};

////////////////////////////////////////
// This should be used in the header
////////////////////////////////////////

#define CPPEXTERN_HEADER(NEW_CLASS, PARENT_CLASS)    	    	\
public:     	    	    	    	    	    	    	\
static void obj_freeCallback(void *data)    	    	    	\
{ CPPExtern *mydata = ((Obj_header *)data)->data; delete mydata; \
  ((Obj_header *)data)->Obj_header::~Obj_header(); }   	    	\
static void real_obj_setupCallback(t_class *classPtr)  	    	\
{ PARENT_CLASS::real_obj_setupCallback(classPtr);    	    	\
  NEW_CLASS::obj_setupCallback(classPtr); }  	    	    	\
private:    	    	    	    	    	    	    	\
static inline NEW_CLASS *GetMyClass(void *data) {return((NEW_CLASS *)((Obj_header *)data)->data);} \
static void obj_setupCallback(t_class *classPtr);


////////////////////////////////////////
// This should be the first thing in the implementation file
////////////////////////////////////////

//
// NO ARGUMENTS
/////////////////////////////////////////////////
#define CPPEXTERN_NEW(NEW_CLASS)    	    	     \
  REAL_NEW__CLASS(NEW_CLASS);                    \
  static void* create_ ## NEW_CLASS (void)       \
    REAL_NEW__CREATE1(NEW_CLASS)                 \
    obj->data = new NEW_CLASS();                 \
    REAL_NEW__CREATE2(NEW_CLASS)                 \
    REAL_NEW__SETUP1(NEW_CLASS)                  \
    REAL_NEW__SETUP2(NEW_CLASS)

//
// ONE ARGUMENT
/////////////////////////////////////////////////
#define CPPEXTERN_NEW_WITH_ONE_ARG(NEW_CLASS, TYPE, PD_TYPE)    \
  REAL_NEW__CLASS(NEW_CLASS);                    \
  static void* create_ ## NEW_CLASS (TYPE arg)   \
    REAL_NEW__CREATE1(NEW_CLASS)                 \
    obj->data = new NEW_CLASS(arg);              \
    REAL_NEW__CREATE2(NEW_CLASS)                 \
    REAL_NEW__SETUP1(NEW_CLASS)                  \
         PD_TYPE,                                \
    REAL_NEW__SETUP2(NEW_CLASS)

//
// GIMME ARGUMENT
/////////////////////////////////////////////////
#define CPPEXTERN_NEW_WITH_GIMME(NEW_CLASS)  	    	    	\
  REAL_NEW__CLASS(NEW_CLASS);                    \
  static void* create_ ## NEW_CLASS (t_symbol*s, int argc, t_atom*argv) \
    REAL_NEW__CREATE1(NEW_CLASS)                 \
       obj->data = new NEW_CLASS(argc,argv);     \
    REAL_NEW__CREATE2(NEW_CLASS)                 \
    REAL_NEW__SETUP1(NEW_CLASS)                  \
         A_GIMME,              \
    REAL_NEW__SETUP2(NEW_CLASS)

//
// TWO ARGUMENTS
/////////////////////////////////////////////////
#define CPPEXTERN_NEW_WITH_TWO_ARGS(NEW_CLASS, TYPE, PD_TYPE, TTWO, PD_TWO)	\
  REAL_NEW__CLASS(NEW_CLASS);                     \
  static void* create_ ## NEW_CLASS (TYPE arg, TTWO arg2) \
    REAL_NEW__CREATE1(NEW_CLASS)           \
    obj->data = new NEW_CLASS(arg, arg2);  \
    REAL_NEW__CREATE2(NEW_CLASS)           \
    REAL_NEW__SETUP1(NEW_CLASS)            \
         PD_TYPE, PD_TWO,                  \
    REAL_NEW__SETUP2(NEW_CLASS)

//
// THREE ARGUMENTS
/////////////////////////////////////////////////
#define CPPEXTERN_NEW_WITH_THREE_ARGS(NEW_CLASS, TYPE, PD_TYPE, TTWO, PD_TWO, TTHREE, PD_THREE)	\
  REAL_NEW__CLASS(NEW_CLASS);                        \
  static void* create_ ## NEW_CLASS (TYPE arg, TTWO arg2, TTHREE arg3)  \
    REAL_NEW__CREATE1(NEW_CLASS)                    \
       obj->data = new NEW_CLASS(arg, arg2, arg3);  \
    REAL_NEW__CREATE2(NEW_CLASS)                    \
    REAL_NEW__SETUP1(NEW_CLASS)                     \
         PD_TYPE, PD_TWO, PD_THREE,                 \
    REAL_NEW__SETUP2(NEW_CLASS)

//
// FOUR ARGUMENTS
/////////////////////////////////////////////////
#define CPPEXTERN_NEW_WITH_FOUR_ARGS(NEW_CLASS, TYPE, PD_TYPE, TTWO, PD_TWO, TTHREE, PD_THREE, TFOUR, PD_FOUR) \
  REAL_NEW__CLASS(NEW_CLASS);                             \
  static void* create_ ## NEW_CLASS (TYPE arg, TTWO arg2, TTHREE arg3, TFOUR arg4) \
    REAL_NEW__CREATE1(NEW_CLASS)                         \
       obj->data = new NEW_CLASS(arg, arg2, arg3, arg4); \
    REAL_NEW__CREATE2(NEW_CLASS)                         \
    REAL_NEW__SETUP1(NEW_CLASS)                          \
         PD_TYPE, PD_TWO, PD_THREE, PD_FOUR,             \
    REAL_NEW__SETUP2(NEW_CLASS)

//
// FIVE ARGUMENTS
/////////////////////////////////////////////////
#define CPPEXTERN_NEW_WITH_FIVE_ARGS(NEW_CLASS, TYPE, PD_TYPE, TTWO, PD_TWO, TTHREE, PD_THREE, TFOUR, PD_FOUR, TFIVE, PD_FIVE) \
  REAL_NEW__CLASS(NEW_CLASS);                                   \
  static void* create_ ## NEW_CLASS (TYPE arg, TTWO arg2, TTHREE arg3, TFOUR arg4, TFIVE arg5) \
    REAL_NEW__CREATE1(NEW_CLASS)                               \
       obj->data = new NEW_CLASS(arg, arg2, arg3, arg4, arg5); \
    REAL_NEW__CREATE2(NEW_CLASS)                               \
    REAL_NEW__SETUP1(NEW_CLASS)                                \
         PD_TYPE, PD_TWO, PD_THREE, PD_FOUR, PD_FIVE           \
    REAL_NEW__SETUP2(NEW_CLASS)


//////////////////////////////////////////////////////////////////////////////
// These should never be called or used directly!!!
//
//
///////////////////////////////////////////////////////////////////////////////

#define REAL_NEW__CLASS(NEW_CLASS)  STATIC_CLASS t_class * NEW_CLASS ## _class
#define REAL_NEW__CREATE1(NEW_CLASS)  {                                          \
   try{                                                                          \
    Obj_header *obj = new (pd_new(NEW_CLASS ## _class),(void *)NULL) Obj_header; \
    CPPExtern::m_holder = &obj->pd_obj;                                            \
    CPPExtern::m_holdname=(char*)#NEW_CLASS;

#define REAL_NEW__CREATE2(NEW_CLASS) \
  CPPExtern::m_holder = NULL;                                   \
  CPPExtern::m_holdname=NULL;                                   \
  return(obj);                                                  \
  } catch (...) {gem::catchGemException(CPPExtern::m_holdname, CPPExtern::m_holder); return NULL;} \
  }

#define REAL_NEW__SETUP1(NEW_CLASS) \
  extern "C" {                                                  \
    GEM_EXPORT void NEW_CLASS ## _setup(void)                                  \
    {                                                           \
    static int recalled=0; if(recalled)return; recalled=1;      \
    NEW_CLASS ## _class = class_new(                          \
                                      gensym(#NEW_CLASS),               \
                                      (t_newmethod)create_ ## NEW_CLASS, \
                                      (t_method)&NEW_CLASS::obj_freeCallback, \
                                      sizeof(Obj_header), GEM_CLASSFLAGS,
#define REAL_NEW__SETUP2(NEW_CLASS)                         \
  A_NULL);                                                  \
  SET_HELPSYMBOL(NEW_CLASS);                                \
  NEW_CLASS::real_obj_setupCallback(NEW_CLASS ## _class);   \
  }                                                         \
  }                                                         \
  AUTO_REGISTER_CLASS(NEW_CLASS);

	
///////////////////////////////////////////////////////////////////////////////
// static class:
//   by default classes are declared static
//   however, sometimes we need classes not-static, so we can refer to them
//   from other classes
///////////////////////////////////////////////////////////////////////////////
#ifdef NO_STATIC_CLASS
# define STATIC_CLASS
#else
# define STATIC_CLASS static
#endif

///////////////////////////////////////////////////////////////////////////////
// auto registering a class
// this creates a dummy class, whose constructor calls the setup-function 
// (registering the class with pd)
// a static copy of this class is created at runtime, to actually do the setup-call
///////////////////////////////////////////////////////////////////////////////
#ifdef NO_AUTO_REGISTER_CLASS
// if NO_AUTO_REGISTER_CLASS is defined, we will not register the class
# define AUTO_REGISTER_CLASS(NEW_CLASS) \
  static int NEW_CLASS ## _dummyinstance
#else
// for debugging we can show the which classes are auto-registering
# if 0
#  define POST_AUTOREGISTER(NEW_CLASS) post("auto-registering: "#NEW_CLASS)
# else
#  define POST_AUTOREGISTER(NEW_CLASS)
# endif
# define AUTO_REGISTER_CLASS(NEW_CLASS)			\
  class NEW_CLASS ## _cppclass {					\
  public:								\
  NEW_CLASS ## _cppclass(void) {POST_AUTOREGISTER(NEW_CLASS); NEW_CLASS ## _setup(); } \
};									\
  static NEW_CLASS ## _cppclass NEW_CLASS ## _instance
#endif

///////////////////////////////////////////////////////////////////////////////
// setting the help-symbol
///////////////////////////////////////////////////////////////////////////////
#if defined HELPSYMBOL_BASE || defined HELPSYMBOL
# ifndef HELPSYMBOL_BASE
#  define HELPSYMBOL_BASE ""
# endif

# ifndef HELPSYMBOL
#  define SET_HELPSYMBOL(NEW_CLASS)                                     \
  class_sethelpsymbol(NEW_CLASS ## _class, gensym(HELPSYMBOL_BASE #NEW_CLASS))
# else
#  define SET_HELPSYMBOL(NEW_CLASS)				\
    class_sethelpsymbol(NEW_CLASS ## _class, gensym(HELPSYMBOL_BASE HELPSYMBOL))
# endif

#else 
# define SET_HELPSYMBOL(NEW_CLASS)
#endif /* HELPSYMBOL */

///////////////////////////////////////////////////////////////////////////////
// setting the class-flags
///////////////////////////////////////////////////////////////////////////////
#ifndef GEM_CLASSFLAGS
# define GEM_CLASSFLAGS 0
#endif

// macros for boilerplate code to object messages
#include "RTE/MessageCallbacks.h"

#endif	// for header file
