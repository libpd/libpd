/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

create a window

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMWINCREATE_H_
#define _INCLUDE__GEM_BASE_GEMWINCREATE_H_
#include "Gem/GemConfig.h"
#include "Gem/GemGL.h"

#if defined _WIN32
# include <windows.h>
#elif defined __APPLE__
# import <AGL/agl.h>
#elif defined __linux__ || defined HAVE_GL_GLX_H
# ifdef HAVE_LIBXXF86VM
#  include <X11/extensions/xf86vmode.h>
# endif
#else
# error Define OS specific window creation
#endif

#include "Gem/ExportDef.h"

// I hate Microsoft...I shouldn't have to do this!
#ifdef _WIN32
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
# pragma warning( disable : 4091 )
#endif

#include <string.h>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  WindowInfo
    
  All of the relavent information about an OpenGL window

  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN WindowInfo
{
 public:
	
  // Constructor
  WindowInfo() :
    fs(0), 
#if defined _WIN32
    win(NULL), dc(NULL), context(NULL),
#elif defined __APPLE__
    pWind(NULL), context(NULL), offscreen(NULL), pixelSize(32),
    pixMap(NULL), rowBytes(0), baseAddr(NULL),
#elif defined __linux__ || defined HAVE_GL_GLX_H
    dpy(NULL), win(0), screen(0), cmap(0), context(NULL), delete_atom(0), have_border(false),
#else
#endif
    have_constContext(0)
    {}
  int         fs;                 // FullScreen
  
#if defined _WIN32

  HWND        win;                // Window handle
  HDC         dc;                 // Device context handle
  HGLRC       context;            // OpenGL context

#elif defined __APPLE__

    WindowPtr		pWind;		// GEM window reference for gemwin
    AGLContext		context;	// OpenGL context
    GWorldPtr		offscreen;	// Macintosh offscreen buffer
    long		pixelSize;	//
    Rect		r;		//
    PixMapHandle	pixMap;		// PixMap Handle
    long		rowBytes;	// 
    void 		*baseAddr;	// 
    short		fontList;	// Font

#elif defined __linux__ || defined HAVE_GL_GLX_H

  Display     *dpy;               // X Display
  Window      win;                // X Window
  int         screen;             // X Screen
  Colormap    cmap;               // X color map
  GLXContext  context;            // OpenGL context
  Atom        delete_atom;

  bool        have_border;

# ifdef HAVE_LIBXXF86VM
  XF86VidModeModeInfo deskMode; // originale ModeLine of the Desktop
# endif


#else
# error Define OS specific window data
#endif
  int         have_constContext;  // 1 if we have a constant context
};

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  WindowHints
    
  Hints for window creation

  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN WindowHints
{
 public:

  //////////
  // Should the window be realized
  int         actuallyDisplay;

  //////////
  // Single or double buffered
  int         buffer;

  //////////
  // The width/x of the window
  int         width;
  //////////
  // The height/y of the window
  int         height;

  //////////
  // the real width/height of the window (set by createGemWindow())
  int real_w, real_h;

  //////////
  // The offset/x of the window (likely tobe overridden by the window-manager)
  int         x_offset;
  //////////
  // The offset/y of the window (likely tobe overridden by the window-manager)
  int         y_offset;

  //////////
  // Should we do fullscreen ?
  int			fullscreen;
  
  //////////
  // Is there a second screen ?
  int			secondscreen;

  //////////
  // Should there be a window border?
  int			border;

  //////////
  // mode for full-screen antialiasing
  int                   fsaa;

  ///// if we can use a different display , this has its meaning under X
  char* display;

  //////////////
  // display some title....
  char* title;

  //////////
  // The GLXcontext to share rendering with
#if defined _WIN32
    HGLRC       shared;
#elif defined __APPLE__
    AGLContext	shared;
#elif defined __linux__ || defined HAVE_GL_GLX_H
    GLXContext  shared;
#else
#error Define OS specific OpenGL context
#endif
};

//////////
// Create a new window
GEM_EXTERN extern int createGemWindow(WindowInfo &info, WindowHints &hints);

//////////
// Destroy a window
GEM_EXTERN extern void destroyGemWindow(WindowInfo &info);

//////////
// Set the cursor
GEM_EXTERN extern int cursorGemWindow(WindowInfo &info, int state);

//////////
// Set the topmost position
GEM_EXTERN extern int topmostGemWindow(WindowInfo &info, int state);

//////////
// swap the buffers (get's called in double-buffered mode)
GEM_EXTERN extern void gemWinSwapBuffers(WindowInfo &nfo);
/////////
// reestablish a context
GEM_EXTERN extern void gemWinMakeCurrent(WindowInfo &nfo);

/////////
// init OS-specific stuff
GEM_EXTERN extern bool initGemWin(void);

/////////
// prepare a WindowInfo for context-sharing
GEM_EXTERN void initWin_sharedContext(WindowInfo &info, WindowHints &hints);



/////////
// 
GEM_EXTERN extern void dispatchGemWindowMessages(WindowInfo &nfo);



#endif  // for header file
