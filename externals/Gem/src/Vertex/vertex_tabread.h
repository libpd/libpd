/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_tabread

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_TABREAD_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_TABREAD_H_

#include "Base/GemVertex.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_tabread
    
    Creates a vertex_tabread

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_tabread : public GemVertex
{
    CPPEXTERN_HEADER(vertex_tabread, GemVertex);

    public:

        //////////
        // Constructor
  vertex_tabread(int, t_atom*);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~vertex_tabread(void);

        GLfloat	*m_VertexArray;
        GLfloat	*m_ColorArray;
		GLfloat *m_NormalArray;
        GLfloat	*m_TexCoordArray;
		int		 m_size;

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
		
		//////////
		// set the dirty flag
		bool m_doit;
		void bangMess(void);

	//////////
	// set the tables that hold the interleaved data (vertex, color, ...)
	virtual void tableMess(int,t_atom*);
	t_symbol *m_Vtable, *m_Ctable, *m_Ttable, *m_Ntable;

 private:
        static void 	tableMessCallback(void *data, t_symbol*,int,t_atom*);
		static void     bangMessCallback(void*data);

};

#endif	// for header file
