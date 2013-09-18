/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a texture

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
Copyright (c) 2002 James Tittle & Chris Clepper
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_MOVIE_H_
#define _INCLUDE__GEM_PIXES_PIX_MOVIE_H_

#include "Pixes/pix_film.h"
#include "Pixes/pix_texture.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_movie
    
  Loads in a movie
    
  KEYWORDS
  pix
    
  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN pix_movie : public pix_film
{
  CPPEXTERN_HEADER(pix_movie, pix_film);
  
    public:

  //////////
  // Constructor
  pix_movie(t_symbol *filename);

 protected:
    
  //////////
  // Destructor
  virtual ~pix_movie();

  //////////
  // Do the rendering
  virtual void render(GemState *state);
  //////////
  // Clear the dirty flag on the pixBlock
  virtual void postrender(GemState *state);
  //////////
  virtual void startRendering();
  //////////
  // Delete texture object
  virtual void stopRendering();

  pix_texture    m_pixtexture;
  //////////
  // Set the texture quality
  // [in] type - if == 0, then GL_NEAREST, else GL_LINEAR
  void          textureQuality(int type){m_pixtexture.textureQuality(type);}
  void          repeatMess(int type){m_pixtexture.repeatMess(type);}
  void          modeMess(int mode){m_pixtexture.m_rectangle=mode;}

 protected:
	
  //////////
  // static member functions
  static void 	textureMessCallback(void *data, t_floatarg n);
  static void 	modeCallback(void *data, t_floatarg n);
  static void 	rectangleCallback(void *data, t_floatarg n);
  static void 	repeatMessCallback(void *data, t_floatarg n);

};

#endif	// for header file
