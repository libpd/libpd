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

#ifndef _INCLUDE__GEM_PIXES_PIX_CUBEMAP_H_
#define _INCLUDE__GEM_PIXES_PIX_CUBEMAP_H_

#include "Base/GemBase.h"
#include "Gem/Image.h"
#include "Gem/State.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_cubemap
    
  Turns on texture mapping

  KEYWORDS
  pix
    
  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN pix_cubemap : public GemBase
{
  CPPEXTERN_HEADER(pix_cubemap, GemBase);
    
    public:
  
  //////////
  // Constructor
  pix_cubemap();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_cubemap();


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
  GLuint	m_textureQuality;
		
  //////////
  // Set the texture quality
  // [in] type - if == 1, then GL_REPEAT, else GL_CLAMP_TO_EDGE
  void          repeatMess(int type);
  GLuint        m_repeat;

  //////////
  // did we really do texturing in render() ??
  // good to know in the postrender()...
  bool          m_didTexture;

  //////////
  // Do we need to rebuild the display List
  int           m_rebuildList;

  //////////
  // The size of the texture (so we can use sub image)
  int	        m_dataSize[3];

  //////////
  // The texture object number
  GLuint	    m_textureObj;

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


  GLint m_textureType; // GL_TEXTURE_CUBE_MAP

  GLfloat m_xRatio, m_yRatio; // x- and y-size if texture

  //////////
  // texture envirnoment mode
  void		envMess(int num);
  GLint		m_env; // GL_TEXTURE_ENV_MODE
		
  GLint	m_texunit;
  GLint	m_numTexUnits;

  GLuint m_map;

  void applyTex(GLint, imageStruct*);
  void rightImage(int id, GemState *state);
  imageStruct*m_img[6];

  t_inlet*m_imgIn[6];


 private:

  //////////
  // static member functions
  static void floatMessCallback(void *data, float n);
  static void textureMessCallback(void *data, t_floatarg n);
  static void modeCallback(void *data, t_floatarg n);
  static void envMessCallback(void *data, t_floatarg n);
  static void repeatMessCallback(void *data, t_floatarg n);
  static void texunitCallback(void *data, t_floatarg unit);
  static void mapCallback(void *data, t_floatarg unit);

  static void rightImage_callback(void*, t_symbol*, int, t_atom*);

};

#endif	// for header file
