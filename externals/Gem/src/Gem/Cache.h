/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    The state to pass among GEM objects

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_CACHE_H_
#define _INCLUDE__GEM_GEM_CACHE_H_

#include "Gem/ExportDef.h"

class gemhead;

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemCache
    
    The cache to pass among GEM objects

DESCRIPTION
    
-----------------------------------------------------------------*/
#define GEMCACHE_MAGIC 0x1234567
class GEM_EXTERN GemCache
{
    public:
    
    	//////////
        // Constructor
    	GemCache(gemhead *parent);

      GemCache(const GemCache&);
    	
      //////////
      // Destructor
      virtual ~GemCache(void);


      virtual GemCache& operator=(const GemCache&);

    	//////////
    	// Was a modification made which will void a display list?
    	bool 	    	    dirty;
    	
    	//////////
    	// Should the image be resent?
    	bool 	    	    resendImage;

    	//////////
    	// has the Vertex-Array changed?
    	bool 	    	    vertexDirty;

		//////////
		// re-set (like creation, but without instantiating
		void reset(gemhead*parent);

    //////////
    gemhead     	    *m_parent;

    //////////
    // indicates a valid cache
    int m_magic;
};

#endif	// for header file
