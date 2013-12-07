/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A Glu object

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMGLUOBJ_H_
#define _INCLUDE__GEM_BASE_GEMGLUOBJ_H_

// I hate Microsoft...I shouldn't have to do this!
#ifdef _WIN32
#include <windows.h>
#endif

#include <string.h>


#include <math.h>
#ifndef M_PI
#  define M_PI (3.1415926)
#endif

#include "Base/GemShape.h"

#ifndef GLU_SILHOUETTE
# define GLU_SILHOUETTE 0
#endif

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemGluObj
    
    A Glu object

DESCRIPTION

    Inlet for an int - "in2"

    "in2" - the number of slices in the object
        
-----------------------------------------------------------------*/
class GEM_EXTERN GemGluObj : public GemShape
{
    public:
    
	    //////////
	    // Constructor
  GemGluObj(t_floatarg size, t_floatarg slices=10.f, t_floatarg stacks=0.f);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~GemGluObj();

    	//////////
    	// How the object should be drawn
    	virtual void	typeMess(t_symbol *type);

    	//////////
    	// The number of slices in the quadric
    	void	    	numSlicesMess(int numSlices);
    	void	    	numSlicesMess(int numSlices, int numStacks);

    	//////////
    	// The number of slices
    	int 	    	m_numSlices, m_numStacks;
    	
        //////////
        t_inlet         *m_sliceInlet;

    	//////////
    	// creation callback
    	static void 	real_obj_setupCallback(t_class *classPtr)
    	    { GemShape::real_obj_setupCallback(classPtr); GemGluObj::obj_setupCallback(classPtr); }
    	
    private:
    
     	static inline GemGluObj *GetMyClass(void *data) {return((GemGluObj *)((Obj_header *)data)->data);}

    	//////////
    	// Static member functions
    	static void 	obj_setupCallback(t_class *classPtr);
    	static void 	numSlicesMessCallback(void *data, t_symbol*, int, t_atom*);
};

#endif	// for header file
