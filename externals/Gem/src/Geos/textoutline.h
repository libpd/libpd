/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A textoutline

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2005 Georg Holzmann <grh@mur.at>
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_TEXTOUTLINE_H_
#define _INCLUDE__GEM_GEOS_TEXTOUTLINE_H_

#include "Base/TextBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    textoutline
    
    Creates a textoutline string

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN textoutline : public TextBase
{
    CPPEXTERN_HEADER(textoutline, TextBase);

    public:

		//////////
		// Constructor
    	textoutline(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~textoutline();

#ifdef FTGL
	virtual FTFont*makeFont(const char*fontname);
#endif
};

#endif	// for header file
