/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

Clamp pixel values to a fiducialtrack

Copyright (c) 1997-1998 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM. zmoelnig@iem.kug.ac.at
Copyright (c) 2002 James Tittle & Chris Clepper
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_FIDUCIALTRACK_H_
#define INCLUDE_PIX_FIDUCIALTRACK_H_

#include "Base/GemPixObj.h"

#include <vector>

#include "libfidtrack_fidtrackX.h"
#define MAX_FIDUCIAL_COUNT 512
#include "libfidtrack_segment.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_fiducialtrack
    
    Clamp pixel values to a fiducialtrack

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vec_thresh"
    Inlet for a float - "ft1"
    
    "vec_thresh" - The fiducialtrack vector
    "ft1" - Set all fiducialtracks to one value
   
-----------------------------------------------------------------*/
class GEM_EXPORT pix_fiducialtrack : public GemPixObj
{
    CPPEXTERN_HEADER(pix_fiducialtrack, GemPixObj);

    public:

        //////////
        // Constructor
    	pix_fiducialtrack(t_symbol*);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_fiducialtrack();

    	//////////
    	virtual void 	processGrayImage(imageStruct &image);

        int m_width,m_height;

        virtual void  deinit_segmenter();
        bool initialized;

	Segmenter segmenter;
        void    treeMess(t_symbol*s);
	char m_treefile[MAXPDSTRING];
        void    addMess(t_symbol*s);
	TreeIdMap treeidmap;

	FidtrackerX fidtrackerx;
	FiducialX fiducials[ 1024 ];

    private:

        t_outlet*m_infoOut;
        t_atom   m_outlist[4];
    
    	//////////
    	// Static member functions
    	static void 	treeMessCallback(void *data, t_symbol*s);
    	static void 	addMessCallback(void *data, t_symbol*s);
};

#endif	// for header file
