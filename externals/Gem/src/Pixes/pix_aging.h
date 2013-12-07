/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    age an image

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    this is based on EffecTV by Fukuchi Kentauro
    * AgingTV - film-aging effect.
    * Copyright (C) 2001 FUKUCHI Kentarou

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_AGING_H_
#define _INCLUDE__GEM_PIXES_PIX_AGING_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_aging
    
    Make pix look old

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/
typedef struct _scratch
{
	int life;
	int x;
	int dx;
	int init;
} t_scratch;

class GEM_EXTERN pix_aging : public GemPixObj
{
    CPPEXTERN_HEADER(pix_aging, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_aging();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_aging();

    	//////////
    	// Do the processing
    	virtual void 	processImage(imageStruct &image);
        
	void scratchMess(int scratchlines);

	////////
	// which FX do we want ?
	int m_coloraging;
	int m_scratching;
	int m_pits;
	int m_dusts;


	////////
	// scratching
	t_scratch *m_scratch;
	int m_scratchlines;

	////////
	// pits
	int m_pitinterval;
	int m_areascale;

	////////
	// dust
	int m_dustinterval;

 private:
	
	//////////
	// Static member callbacks
	static void colorMessCallback(void *dump, t_float state);
	static void scratchMessCallback(void *dump, t_float state);
	static void pitsMessCallback(void *dump, t_float state);
	static void dustMessCallback(void *dump, t_float state);
};

#endif	// for header file
