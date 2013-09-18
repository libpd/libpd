/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_draw

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_DRAW_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_DRAW_H_

#include "Base/GemVertex.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_draw
    
    Creates a vertex_draw

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_draw : public GemVertex
{
    CPPEXTERN_HEADER(vertex_draw, GemVertex);

    public:

        //////////
        // Constructor
    	vertex_draw(void);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~vertex_draw(void);
        
	////////////
	// use VertexArrayObjects
        int m_vao;

	////////
	// do we want to use the colorArray if it exists ?
	int m_color;

	////////
	// do we want to use the texCoordArray if it exists ?
	int m_texcoord;

	//////////
	// VertexBufferObjects
	// these are only used if "__VBO" is defined
		GLuint	m_nVBOVertices, m_nVBOColor, m_nVBOTexCoords, m_nVBONormals;
        
    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);


	//////////
	// our own draw-style
	virtual void    typeMess(t_symbol*s);
	GLint m_drawType;

	////////
	// do we want to use the default draw-style (from GemState) or our own ?
	int 	m_defaultDraw;

 private:
        static void 	colorMessCallback(void *data, t_floatarg size);
        static void 	texcoordMessCallback(void *data, t_floatarg t);
        static void 	defaultMessCallback(void *data, t_floatarg size);
        static void 	typeMessCallback(void *data, t_symbol*s);

};

#endif	// for header file
