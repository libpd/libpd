/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  The state to pass among GEM objects

  Copyright (c) 1997-2000 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_STATE_H_
#define _INCLUDE__GEM_GEM_STATE_H_

#include "Gem/ExportDef.h"
#include "Gem/GemGL.h"

#include "Gem/RTE.h"
#include "Utils/any.h"

struct pixBlock;
class TexCoord;

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  TexCoord
    

  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN TexCoord
{
 public:
  TexCoord() : s(0.f), t(0.f) { }
    TexCoord(float s_, float t_) : s(s_), t(t_) { }
      float   	    s;
      float   	    t;
};


/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  GemState
    
  The state to pass among GEM objects

  DESCRIPTION
    
  -----------------------------------------------------------------*/

class GemStateData;
class GEM_EXTERN GemState
{
 public:
  typedef enum {
    _DIRTY,
    _TIMING_TICK,
    _PIX,
    _GL_STACKS,
    _GL_DISPLAYLIST,
    _GL_LIGHTING,
    _GL_SMOOTH,
    _GL_DRAWTYPE,
    _GL_TEX_TYPE,
    _GL_TEX_COORDS,
    _GL_TEX_NUMCOORDS,
    _GL_TEX_UNITS,





    _LAST
  } key_t;

  //////////
  // Has something changed since the last time?
  // deprecated: use property 'dirty' instead
  GEM_DEPRECATED  bool                 dirty;

  //////////
  // Are we in a display list creation?
  // deprecated: use property 'gl.displaylist' instead
  GEM_DEPRECATED  bool 	    	    inDisplayList;
    	
  //////////
  // Lighting on?
  // deprecated: use property 'gl.lighting' instead
  GEM_DEPRECATED  bool 	    	    lighting;
    	
  //////////
  // Smooth shading (flat is other type)
  // deprecated: use property 'gl.smooth' instead
  GEM_DEPRECATED  bool 	    	    smooth;
    	
  //////////
  // Texture mapping on?
  // 0..off
  // 1..normalized texture
  // 2..rectangle texture
  // deprecated: use property 'gl.tex.type' instead
  GEM_DEPRECATED  int 	    	    texture;
    	
  //////////
  // The image to texture map
  // deprecated: use property 'pix' instead
  GEM_DEPRECATED  pixBlock	        *image;
    	
  //////////
  // Texture coordinates.
  // This can be NULL if there aren't any coordinates
  // deprecated: use property 'gl.tex.coords' instead
  GEM_DEPRECATED  TexCoord    	    *texCoords;
    	
  //////////
  // The number of TexCoords
  // deprecated: use property 'gl.tex.numcoords' instead
  GEM_DEPRECATED  int 	    	    numTexCoords;
  
  //////////
  // The number of multiTexUnits
  //   default = 0, max = 7
  // deprecated: use property 'gl.tex.units' instead
  GEM_DEPRECATED  int 	    	    multiTexUnits;

  //////////
  // Milliseconds since last frame
  // If in Stereoscopic mode, then it is the same number for both left
  //		and right renderings
  // deprecated: use property 'timing.tick' instead
  GEM_DEPRECATED  float				tickTime;

  //////////////////
  // the default draw-type (might be overriden within a Geo)
  // deprecated: use property 'gl.drawtype' instead
  GEM_DEPRECATED  GLenum				drawType;

  //////////
  // how deep is the current stack /* 4 fields for the 4 stacks */
  // deprecated: use property 'gl.stacks' instead
  GEM_DEPRECATED  int stackDepth[4];
	
  ////////////
  //vertex-array data
  // deprecated: use property 'vertex.dirty' instead
  GEM_DEPRECATED  int                 VertexDirty; // the vertex-arrays has changed
  // deprecated: use property 'vertex.array.vertex' instead
  GEM_DEPRECATED  GLfloat				*VertexArray;
  // deprecated: use property 'vertex.array.vertex' instead
  GEM_DEPRECATED  int					VertexArraySize;
  // deprecated: use property 'vertex.array.vertex' instead
  GEM_DEPRECATED  int					VertexArrayStride;

  // deprecated: use property 'vertex.array.color' instead
  GEM_DEPRECATED  GLfloat				*ColorArray;
  // deprecated: use property 'vertex.array.color' instead
  GEM_DEPRECATED  bool		 			HaveColorArray;

  // deprecated: use property 'vertex.array.normal' instead
  GEM_DEPRECATED  GLfloat				*NormalArray;
  // deprecated: use property 'vertex.array.normal' instead
  GEM_DEPRECATED  bool					HaveNormalArray;

  // deprecated: use property 'vertex.array.texcoord' instead
  GEM_DEPRECATED  GLfloat				*TexCoordArray;
  // deprecated: use property 'vertex.array.texcoord' instead
  GEM_DEPRECATED  bool					HaveTexCoordArray;

  //////////
  // Constructor
  GemState(void);
  GemState(const GemState&);

  //////////
  // Destructor
  virtual ~GemState(void);

  float texCoordX(int num) const;

  float texCoordY(int num) const;

  /* reset (parts of?) the GemState: to be called from [gemhead] */
  void reset(void);


  /* get a named property */
  /* if the property exists (as the given type), 
   * the value of the 2nd argument is set accordingly and <code>true</code> is returned
   * if the key does not exist (or the type is wrong) the value is not touched and <code>false</code> is returned instead
   */
  virtual bool get(const key_t key, gem::any&value);

  template<class T>
    bool get(const key_t key, T&value) {
    try {
      gem::any val;
      if(!get(key,val)) {
        // key not found
        return false;
      }
      value=gem::any_cast<T>(val);
      return true;
    } catch (gem::bad_any_cast&x) {
      ::verbose(3, "%s:%d [%s] %d :: %s", __FILE__, __LINE__, __FUNCTION__, key, x.what().c_str());
      // type problem
    }
    return false;
  };
  /* set a named property */
  virtual bool set(const key_t key, gem::any value);
  /* remove a named property */
  virtual bool remove(const key_t key);

  // Copy assignment
  GemState& operator=(const GemState&);

 protected:
  GemStateData*data;
};
    	
#endif	// for header file
