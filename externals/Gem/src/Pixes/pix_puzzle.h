/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    play puzzle with a sequence of pixBufs

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_PUZZLE_H_
#define _INCLUDE__GEM_PIXES_PIX_PUZZLE_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_puzzle
  
  shuffle areas of the pixBuf
  
  KEYWORDS
  pix
    
  DESCRIPTION
   
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_puzzle : public GemPixObj
{
  CPPEXTERN_HEADER(pix_puzzle, GemPixObj);

    public:

  //////////
  // Constructor
  pix_puzzle();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_puzzle();

  //////////
  // Do the processing
  virtual void 	processImage(imageStruct &image);
  virtual void processYUVImage(imageStruct &image);

  imageStruct    myImage;

  //////////
  // Make a puzzle
  virtual void 	makePuzzleBlocks(int xsize, int ysize, int csize);
  virtual void  shuffle();
  virtual void  sizeMess(int width, int height);
  virtual void  moveMess(int direction);
 
  int blocksize, blockxsize,blockysize,  blocknum, spacepos;
  int blockw, blockh;
  int *blockpos;
  int *blockoffset;
  int marginw, marginh;

  int m_force;

  //////////
  // anyone wants to play the famous puzzle game ?
  int m_game;

 private:
  
  //////////
  // static member functions
  static void bangMessCallback(void *data);
  static void sizeMessCallback(void *data, t_floatarg width, t_floatarg height);
  static void moveMessCallback(void *data, t_floatarg state);
};

#endif	// for header file
