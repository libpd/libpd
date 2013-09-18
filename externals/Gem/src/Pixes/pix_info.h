/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Get pixel information

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_INFO_H_
#define _INCLUDE__GEM_PIXES_PIX_INFO_H_

#include "Base/GemBase.h"
#include "Gem/Image.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

    pix_info
    
	Get image information

KEYWORDS
    pix
    
DESCRIPTION

 outputs xsize, ysize, csize, format, data

-----------------------------------------------------------------*/
class GEM_EXTERN pix_info : public GemBase
{
    CPPEXTERN_HEADER(pix_info, GemBase);

    public:

	    //////////
    	// Constructor
    	pix_info();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_info();

    	//////////
    	// All we want is the pixel information, so this is a complete override.
	//  	virtual void 	render(GemState *state);

    	//////////
        virtual void 	render(GemState*);


        //////////
        void		trigger();

        //////////
        // The color outlet
        t_outlet    	*m_x, *m_y, *m_c; // xsize; ysize; csize
	t_outlet        *m_format;        // format
	t_outlet        *m_misc;          // type, upsidedown, notowned
	t_outlet        *m_pixblock;      // newimage, newfilm
	t_outlet        *m_data;          // data
};

#endif	// for header file
