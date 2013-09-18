/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  texture map a pixBlock onto a openGL-Geo

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  Copyright (c) 2002-2005 James Tittle & Chris Clepper
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_TEXTURE_H_
#define _INCLUDE__GEM_PIXES_PIX_TEXTURE_H_

#include "Base/GemBase.h"
#include "Gem/Image.h"
#include "Gem/State.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_texture
    
  Turns on texture mapping

  KEYWORDS
  pix
    
  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN pix_texture : public GemBase
{
  //////////
  // texturing in [pix_movie] is done as a friend
  friend class pix_movie;

  CPPEXTERN_HEADER(pix_texture, GemBase);

    public:

  //////////
  // Constructor
  pix_texture();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_texture();


  ////////
  // extension check
  virtual bool isRunnable(void);

  //////////
  // Do the rendering
  virtual void 	render(GemState *state);

  //////////
  // Turn back off texture mapping
  virtual void 	postrender(GemState *state);

  //////////
  // Establish texture object
  virtual void	startRendering();

  //////////
  // Delete texture object
  virtual void	stopRendering();

  //////////
  // if we need to rebuild the list
  virtual void   setModified();
	
  //////////
  // Turn on/off texture mapping
  void          textureOnOff(int on);
  int           m_textureOnOff;

  //////////
  // Set up the texture state
  void		setUpTextureState();

  //////////
  // Set the texture quality
  // [in] type - if == 0, then GL_NEAREST, else GL_LINEAR
  void          textureQuality(int type);
  GLuint		  m_textureQuality;
		
  //////////
  // Set the texture quality
  // [in] type - if == 1, then GL_REPEAT, else GL_CLAMP_TO_EDGE
  void          repeatMess(int type);
  GLuint        m_repeat, m_doRepeat;

  //////////
  // did we really do texturing in render() ??
  // good to know in the postrender()...
  bool          m_didTexture;

  //////////
  // Do we need to rebuild the display List
  gem::ContextData<GLboolean>           m_rebuildList;

  //////////
  // The size of the texture (so we can use sub image)
  int	        m_dataSize[3];

  //////////
  // The texture object number
  GLuint	    m_textureObj;

  ////////
  // an external texture (as emitted through the 2nd outlet)
  // if this is present and no image is upstream,
  // we use it as our texture
  GLuint	    m_extTextureObj;
  GLfloat     m_extWidth, m_extHeight;
  GLuint      m_extType;
  GLboolean   m_extUpsidedown;
  t_inlet         *m_inTexID;

  /* send out our texture through the 2nd outlet to be used by others */
  void sendExtTexture(GLuint texobj, GLfloat xRatio, GLfloat yRatio, GLint texType, GLboolean upsidedown);
  t_outlet	*m_outTexID;

		
  ////////
  // the texture object we are creating and destroying
  // we use it as our texture
  GLuint	    m_realTextureObj;

  //////////
  // The resizing buffer
  imageStruct   m_buffer;
  //////////
  // a buffer for colour-space conversion
  imageStruct   m_imagebuf;

  //////////
  // The texture coordinates
  TexCoord    	m_coords[4];

	
  //////////
  // this is what we get from upstream
  void pushTexCoords(GemState*);
  void popTexCoords(GemState*);
  TexCoord       *m_oldTexCoords;
  int             m_oldNumCoords;
  int             m_oldTexture;


  int m_textureType; // GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_EXT

  GLfloat m_xRatio, m_yRatio; // x- and y-size if texture

  void    textureRectangle(int mode);
  int		m_rectangle; //rectangle or power of 2
  int m_canRectangle; // openGL caps and GemSettings
		
  //////////
  // texture envirnoment mode
  void		envMess(int num);
  GLint		m_env; // GL_TEXTURE_ENV_MODE
		
  int		m_clientStorage; //for Apple's client storage extension
  int		m_yuv; // try to texture YUV-images directly when gfx-card says it is possible to do so

  GLint	m_texunit;
  GLint m_numTexUnits;


  /* using PBOs for (hopefully) optimized pixel transfers */
  void pboMess(int num_pbos);
  GLint m_numPbo;
  GLint m_curPbo;
  GLuint *m_pbo;                   // IDs of PBO


  /* upside down texture? */
  GLboolean m_upsidedown;

 private:

	//////////
	// static member functions
	static void 	floatMessCallback(void *data, float n);
	static void 	textureMessCallback(void *data, t_floatarg n);
	static void 	modeCallback(void *data, t_floatarg n);
	static void 	rectangleCallback(void *data, t_floatarg n);
	static void   envMessCallback(void *data, t_floatarg n);
	static void 	repeatMessCallback(void *data, t_floatarg n);
	static void 	clientStorageCallback(void *data, t_floatarg n);
	static void 	yuvCallback(void *data, t_floatarg n);
  static void		texunitCallback(void *data, t_floatarg unit);
  static void 	extTextureCallback(void *data, t_symbol*,int,t_atom*);

	static void 	pboCallback(void *data, t_floatarg n);
};

#endif	// for header file
