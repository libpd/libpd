/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Base class for shapes

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMSHAPE_H_
#define _INCLUDE__GEM_BASE_GEMSHAPE_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemShape
    
    Base class for shapes

DESCRIPTION
    
    Inlet for a float - "ft1"

    "ft1" - the size of the shape
    "draw" - the drawing style
    "width" - the line width when drawing with lines
    
-----------------------------------------------------------------*/
class TexCoord;
class GEM_EXTERN GemShape : public GemBase
{
    public:

        //////////
        // Constructor
        // [in] size - A size of 0. means to just use the default (ie, 1.)
    	GemShape(t_floatarg size);
	GemShape();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~GemShape();

	    //-----------------------------------
	    // GROUP:	Access functions
	    //-----------------------------------
    
    	//////////
    	// The width of the lines in line draw mode
    	void	    	linewidthMess(float linewidth);
    	
    	//////////
    	// The size of the object
    	void	    	sizeMess(float size);

    	//////////
    	// How the object should be drawn
    	virtual void	typeMess(t_symbol *type);
    	
	    //-----------------------------------
	    // GROUP:	Utility functions
	    //-----------------------------------
	
		void SetVertex(GemState* state,float x, float y, float z, 
                   float tx, float ty, 
                   int curCoord);

		void SetVertex(GemState* state,float x, float y, float z, 
                   float s, float t, float r, float q,
                   int curCoord);

	    //-----------------------------------
	    // GROUP:	Member variables
	    //-----------------------------------
    
    //////////
    // The line width for GL_LINE mode
    GLfloat	    	m_linewidth;

    //////////
    // The size of the object
    GLfloat	    	m_size;
	
    //////////
    // The drawing style (GL_LINE, GL_POLYGON, etc)
    GLenum	    	m_drawType;

    //////////
    // do we want blending?
    GLboolean	    	m_blend;
    void  blendMess(float blend);


    ////////
    // override this memberfunction to automatically enable softblended rendering,...
    virtual void renderShape(GemState *state) {;}

    // OR
    // override this memberfunction if you don't want softblending
    virtual void render(GemState *state);

        
    //////////
    // The size inlet
    t_inlet         *m_inlet;

    //-----------------------------------
    // GROUP:	Setup functions
    //-----------------------------------
    
    	//////////
    	// creation callback
    	static void 	real_obj_setupCallback(t_class *classPtr)
        { GemBase::real_obj_setupCallback(classPtr); GemShape::obj_setupCallback(classPtr); }
    	
    private:
    
     	static inline GemShape *GetMyClass(void *data) {return((GemShape *)((Obj_header *)data)->data);}

       	//////////
    	// static member functions
    	static void 	obj_setupCallback(t_class *classPtr);
    	static void 	linewidthMessCallback(void *data, t_floatarg linewidth);
    	static void 	typeMessCallback(void *data, t_symbol *type);
    	static void 	sizeMessCallback(void *data, t_floatarg size);
    	static void 	blendMessCallback(void *data, t_floatarg size);


 protected:
      int m_texType, m_texNum;
      TexCoord*m_texCoords;
      bool m_lighting;
};

#endif	// for header file
