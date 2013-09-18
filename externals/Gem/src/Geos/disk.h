/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A disk

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

/////////////////////////////////////////////////////////
// 1905:forum::für::umläute:2000
/////////////////////////////////////////////////////////
// added the gluDisk
/////////////////////////////////////////////////////////

#ifndef _INCLUDE__GEM_GEOS_DISK_H_
#define _INCLUDE__GEM_GEOS_DISK_H_

#include "Base/GemGluObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	disk
    
    Creates a disk

KEYWORD
    geo
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN disk : public GemGluObj
{
    CPPEXTERN_HEADER(disk, GemGluObj);

    public:

	    //////////
	    // Constructor
    	disk(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~disk();

    	//////////
		// Inner radius of the disk
		float			m_innerRadius;

    	//////////
		// Set the inner radius
		void			innerRadius(float radius);

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
	
	private:

    	//////////
    	// Static member functions
    	static void 	innerRadiusCallback(void *data, t_floatarg radius);
};

#endif	// for header file
