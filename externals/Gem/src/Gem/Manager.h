/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  The base functions and structures
  Also includes gemwin header file

  Copyright (c) 1997-2000 Mark Danks.mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_MANAGER_H_
#define _INCLUDE__GEM_GEM_MANAGER_H_

#include "Gem/GemGL.h"
#include "Utils/GLUtil.h"

#include "Gem/ExportDef.h"

#include <string>

class gemhead;
class GemState;
class WindowInfo;

namespace gem {
  class Context;
};

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  GemMan
    
  A static class to create windows, etc.

  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN GemMan
{
 public:

  //////////
  // Should only be called once (usually by GemSetup)
  static void 	    initGem(void);

  //////////
  static void 	    addObj(gemhead *obj, float priority);
    	
  //////////
  static void 	    removeObj(gemhead *obj, float priority);

  //////////
  // Is there a window.
  static int  	    windowExists(void);

  //////////
  // Are we rendering.
  static int  	    getRenderState(void);

  //////////
  // is there a context (has its meaning under X)
  static void         createContext(char* disp);
  static int  	    contextExists(void);
    	
  //////////
  // If an object needs to know if the window changed.
  // This is important for display lists.
  static int  	    windowNumber(void);
    	
  //////////
  // reset to the initial state
  static void 	    resetState(void);

  //////////
  // Just send out one frame (if double buffered, will swap buffers)
  static void 	    render(void *);

  static void	    renderChain(t_symbol *head, bool start);
  static void	    renderChain(t_symbol *head, GemState *state);

    	    	
  //////////
  // Start a clock to do rendering.
  static void 	    startRendering(void);
    	
  //////////
  // Stop the clock to do rendering.
  static void 	    stopRendering(void);
    	
  //////////
  // Create the window with the current parameters
  static int 	    createWindow(char* disp = 0);

  //////////
  // Destroy the window
  static void 	    destroyWindow(void);
  // Destroy the window after a minimal delay
  static void 	    destroyWindowSoon(void);
	
  //////////
  // Swap the buffers.
  // If single buffered, just clears the window
  static void	    swapBuffers(void);

  //////////
  // Set the frame rate
  static void 	    frameRate(float framespersecond);
  //////////
  // Get the frame rate
  static float 	    getFramerate(void);

  static int 	    getProfileLevel(void);

  static void getDimen(int*width, int*height);
  static void getRealDimen(int*width, int*height);
  static void getOffset(int*x, int*y);
      
  //////////
  // Turn on/off lighting
  static void 	    lightingOnOff(int state);
    	
  //////////
  // Turn on/off cursor
  static void         cursorOnOff(int state);
  
  //////////
  // Turn on/off topmost position
  static void         topmostOnOff(int state);


  //////////
  // Request a lighting value - it is yours until you free it.
  // The return can be 0, in which there are too many lights
  // [in] specific - If you want a specific light.  == 0 means that you don't care.
  static GLenum	    requestLight(int specific = 0);
    	
  //////////
  // Free a lighting value
  static void 	    freeLight(GLenum lightNum);
    	
  //////////
  // Print out information
  static void 	    printInfo(void);

  //////////
  static void 	    fillGemState(GemState &);

  static int	   texture_rectangle_supported;
  
  enum GemStackId { STACKMODELVIEW, STACKCOLOR, STACKTEXTURE, STACKPROJECTION };
  static GLint     maxStackDepth[4]; // for push/pop of matrix-stacks


  static float	   m_perspect[6];	// values for the perspective matrix	
  static float	   m_lookat[9];	// values for the lookat matrix

  // LATER make this private (right now it is needed in gem2pdp)
  static int	   m_buffer;		// single(1) or double(2)

 private:
    	
  //////////
  // computer and window information
  static std::string m_title;             // title to be displayed
  static int	   m_fullscreen;	// fullscreen (1) or not (0!)
  static int       m_menuBar;		// hide (0), show(1), hide but autoshow(-1)
  static int	   m_secondscreen;	// set the second screen
  static int	   m_height;		// window height
  static int	   m_width;		// window width
  static int	   m_w;                 // the real window width (reported by gemCreateWindow())
  static int       m_h;                 // the real window height
  static int	   m_xoffset;		// window offset (x)
  static int	   m_yoffset;		// window offset (y)

  static int	   m_border;		// window border
  static int	   m_stereo;		// stereoscopic

  static int	   m_profile;		// off(0), on(1), w/o image caching(2)
  static int       m_rendering;

  static float	   m_fog;			// fog density
  enum FOG_TYPE
    { FOG_OFF = 0, FOG_LINEAR, FOG_EXP, FOG_EXP2 };
  static FOG_TYPE  m_fogMode;		// what kind of fog we have
  static GLfloat   m_fogColor[4];	// colour of the fog
  static float	   m_fogStart;		// start of the linear fog
  static float	   m_fogEnd;		// start of the linear fog

  static float     m_motionBlur;        // motion-blur factor in double-buffer mode

  static float	   fps;
  static int	   fsaa;
  static bool      pleaseDestroy;

  //////////
  // Changing these variables is likely to crash GEM
  // This is current rendering window information
  // The window is created and destroyed by the user, so
  //		if there is no window, this will contain NULL pointers.
  static WindowInfo   &getWindowInfo(void);
	
  //////////
  // Changing these variables is likely to crash GEM
  // This is constant rendering window information
  // This window is always avaliable (although not visible)
  static WindowInfo   &getConstWindowInfo(void);
  static int 	    createConstWindow(char* disp = 0);

  // gemwin is allowed to modifying "global" window attributes
  friend class gemwin;
  friend class gem::Context;
    	
  static GLfloat    m_clear_color[4];	// the frame buffer clear
  static GLbitfield m_clear_mask;		// the clear bitmask
  static GLfloat    m_mat_ambient[4];	// default ambient material
  static GLfloat    m_mat_specular[4];	// default specular material
  static GLfloat    m_mat_shininess;	// default shininess material
      
  static GLfloat    m_stereoSep;		// stereo seperation
  static GLfloat    m_stereoFocal;		// distance to focal point
  static bool	    m_stereoLine;		// draw a line between 2 stereo-screens
      
  static double	    m_lastRenderTime;	// the time of the last rendered frame
      
  // gemwin should not touch the following member variables and member functions
  static int  	    m_windowState;
  static int  	    m_windowNumber;
  static int  	    m_windowContext;
  static int        m_cursor;
  static int        m_topmost;
      
  static void 	    windowInit(void);
  static void 	    windowCleanup(void);
  static void 	    resetValues(void);

  static void resizeCallback(int xsize, int ysize, void*);
  static void dispatchWinmessCallback(void);

  //////////
  // check for supported openGL extensions we might need
  static void checkOpenGLExtensions(void);
};

#endif
