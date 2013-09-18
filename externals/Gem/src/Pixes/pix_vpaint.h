/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Snap a pix of the render buffer

	Copyright (c) 2005 tigital. tigital@mac.com
    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_VPAINT_H_
#define _INCLUDE__GEM_PIXES_PIX_VPAINT_H_

#include "Base/GemPixObj.h"
#include "Gem/Image.h"
#include "Gem/PBuffer.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_vpaint
    
    Creates voronoi regions based on pixel color
    
KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vert_size"
    
-----------------------------------------------------------------*/
class GEM_EXTERN pix_vpaint : public GemPixObj
{
    CPPEXTERN_HEADER(pix_vpaint, GemPixObj);

    public:

        //////////
        // Constructor
    	pix_vpaint();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_vpaint();

      // extension check
      bool isRunnable(void);
    	
    	//////////
    	// Do the processing
    	virtual void 	processImage(imageStruct &image);

    	//////////
    	// When a size message is received
    	virtual void	sizeMess(int width, int height);
    	
		
		//////////
		//
		void	makepoints();
		void	makecone();
		void	init();
		int		m_initialized;
		int		maxPoints, numPoints, viewImage, useStrokes, drawEdges, moving, canDrawEdges;
		int		mouseX, mouseY, lastX, lastY, sampleMenu, strokeMenu;
		
		/*
		* Point structure 
		*/
		typedef struct {
			int x, y;
			unsigned char r, g, b;
		} cPoint;
		cPoint *points;
    	
    	//////////
    	// The imageStruct with the current image
		imageStruct     m_imageStruct;
    	//////////
		// PBuffer
		PBuffer		*m_pbuffer;
    	//////////
    	// The x position
    	int     	m_x;
    	
    	//////////
    	// The y position
    	int     	m_y;
    	
    	//////////
    	// The width
    	int     	m_w;
    	
    	//////////
    	// The height
    	int     	m_h;
		
		//////////
		// recalc the random points
		bool		m_banged;
    	
    private:

    t_inlet *m_sizinlet;
    	
    //////////
    // static member functions
		static void 	bangMessCallback(void *data);
    static void 	sizeMessCallback(void *data, t_floatarg width, t_floatarg height );
};

#endif	// for header file
