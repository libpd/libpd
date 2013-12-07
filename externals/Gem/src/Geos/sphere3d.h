/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A sphere3d

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_SPHERE_D_H_
#define _INCLUDE__GEM_GEOS_SPHERE_D_H_

#include "Base/GemGluObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    sphere3d
    
    Creates a sphere3d

KEYWORDS
    geo

DESCRIPTION
        
-----------------------------------------------------------------*/
class GEM_EXTERN sphere3d : public GemGluObj
{
    CPPEXTERN_HEADER(sphere3d, GemGluObj);

    public:

        //////////
        // Constructor
  sphere3d(t_floatarg size, t_floatarg slice=10.0, t_floatarg stack=0.0);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~sphere3d();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

        virtual void  	createSphere3d();

        virtual void    setCartesian(int i, int j, GLfloat x, GLfloat y, GLfloat z);
        virtual void    setSpherical(int i, int j, GLfloat r, GLfloat azimuth, GLfloat elevation);

        virtual void    print(int slice, int stack);
        virtual void    print(void);

        GLfloat		*m_x;
        GLfloat		*m_y;
        GLfloat		*m_z;
        int 		oldStacks, oldSlices;
        GLenum		oldDrawType;
	int             oldTexture;

        GLuint          m_displayList;
        
 private:
        static void setCartMessCallback(void*, t_floatarg  i, t_floatarg j,
                                        t_floatarg x, t_floatarg y, t_floatarg z);
        static void setSphMessCallback(void*, t_floatarg  i, t_floatarg j,
                                       t_floatarg r, t_floatarg phi, t_floatarg theta);
        static void printMessCallback(void*);
};
#endif	// for header file
