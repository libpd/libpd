/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A textextruded

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2005 Georg Holzmann <grh@mur.at>
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_TEXTEXTRUDED_H_
#define _INCLUDE__GEM_GEOS_TEXTEXTRUDED_H_

#include "Base/TextBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    textextruded
    
    Creates a textextruded string

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN textextruded : public TextBase
{
    CPPEXTERN_HEADER(textextruded, TextBase);

    public:

    //////////
    // Constructor
    textextruded(int argc, t_atom *argv);
    	
    protected:
    	
    //////////
    // Destructor
    virtual ~textextruded();

#ifdef FTGL
    virtual FTFont*makeFont(const char*fontname);
#else
    virtual void    destroyFont(){}
    virtual int     makeFontFromFace(){return 0;}
    virtual void    render(GemState*){}
#endif

    //////////
    // Set the precision for rendering
    virtual void	setDepth(float depth);
 private:
    static void 	depthMessCallback(void *data, t_floatarg depth);

};

#endif	// for header file
