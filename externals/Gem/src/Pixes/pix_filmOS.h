/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a pix block
(OS independant parent-class)

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.


-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_FILMOS_H_
#define _INCLUDE__GEM_PIXES_PIX_FILMOS_H_

#define GEM_MOVIE_NONE 0
#define GEM_MOVIE_AVI  1
#define GEM_MOVIE_MPG  2
#define GEM_MOVIE_MOV  3

#include <string.h>
#include <stdio.h>

#include "Base/GemBase.h"
#include "Gem/Image.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_filmOS
    
  Loads in a movie
    
  KEYWORDS
  pix
    
  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN pix_filmOS : public GemBase
{
  CPPEXTERN_HEADER(pix_filmOS, GemBase);
    
    public:
  
  //////////
  // Constructor
  pix_filmOS(t_symbol *filename);
  //  pix_filmOS();

 protected:
    
  //////////
  // Destructor
  virtual ~pix_filmOS();

  //////////
  // create and delete buffers
  virtual void createBuffer();
  virtual void deleteBuffer();

  //////////
  // close the movie file
  virtual void closeMess(void){}
  //////////
  // open a movie up
  virtual void openMess(t_symbol *filename, int colorspace=0);

  // really open the file ! (OS dependent)
  virtual void realOpen(char *filename) {}

  //////////
  // prepare for texturing (on open)
  virtual void prepareTexture() {}

  //////////
  // Do the rendering
  virtual void render(GemState *state);
  virtual void getFrame() {}
  virtual void texFrame(GemState *state, int doit) {}


  //////////
  // Clear the dirty flag on the pixBlock
  virtual void postrender(GemState *state);

  //////////
  virtual void startRendering();

  //////////
  // Delete texture object
  virtual void stopRendering() {}
    	
  //////////
  virtual void setUpTextureState() {}
		
  //////////
  // Change which image to display
  virtual void changeImage(int imgNum, int trackNum);
  //////////
  // change the colorspace
  virtual void csMess(int format);	
  //-----------------------------------
  // GROUP:	Movie data
  //-----------------------------------

  //////////
  // what we got from upstream
  pixBlock* m_oldImage;

  //////////
  // the current file
  t_symbol *x_filename;
	
  //////////
  // If a movie was loaded and what kind of Movie this is
  int 	    	m_haveMovie;

  int           m_auto;

  //////////
  // frame infromation
  int           m_numFrames;
  int 	    	m_reqFrame;
  int 	    	m_curFrame;

  //////////
  // track information
  int           m_numTracks;
  int           m_track;

  //////////
  // frame data
  unsigned char*m_frame;  /* this points to the main texture (might be black) */
  unsigned char*m_data;   /* this points always to the real data */

  pixBlock    	m_pixBlock;
  imageStruct   m_imageStruct;
  

  int		m_xsize;
  int		m_ysize;
  int       	m_csize;


  bool      	m_film; // are we in film- or in movie-mode
  int		m_newFilm;
  int 		newImage;
  int		m_colorspace;
  int		m_format;

  //////////
  // a outlet for information like #frames and "reached end"
  t_outlet     *m_outNumFrames;
  t_outlet     *m_outEnd;

 protected:
	
  //////////
  // static member functions
  static void openMessCallback   (void *data, t_symbol *, int, t_atom*);
  static void changeImageCallback(void *data, t_symbol *, int argc, t_atom *argv);
  static void autoCallback       (void *data, t_floatarg state);
  // static void colorspaceCallback(void *data, t_floatarg state);
  static void colorspaceCallback(void *data, t_symbol *state);
};
#endif	// for header file
