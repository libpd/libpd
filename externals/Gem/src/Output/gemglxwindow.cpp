///////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "Gem/GemConfig.h"

#ifdef HAVE_LIBX11
#include "gemglxwindow.h"
#include "Gem/GemGL.h"
#include <stdio.h>
#include <stdlib.h>

#include "RTE/MessageCallbacks.h"
#include "Gem/Exception.h"


#ifdef HAVE_LIBXXF86VM
#  include <X11/extensions/xf86vmode.h>
#endif
#include <X11/cursorfont.h>

CPPEXTERN_NEW(gemglxwindow);

#define EVENT_MASK                                                      \
  ExposureMask|StructureNotifyMask|PointerMotionMask|ButtonMotionMask | \
  ButtonReleaseMask | ButtonPressMask | KeyPressMask | KeyReleaseMask | DestroyNotify

// window creation variables
static int snglBuf24[] = {GLX_RGBA, 
                          GLX_RED_SIZE, 8, 
                          GLX_GREEN_SIZE, 8, 
                          GLX_BLUE_SIZE, 8, 
                          GLX_DEPTH_SIZE, 16, 
                          GLX_STENCIL_SIZE, 8, 
                          GLX_ACCUM_RED_SIZE, 8,
                          GLX_ACCUM_GREEN_SIZE, 8,
                          GLX_ACCUM_BLUE_SIZE, 8,
                          None};
static int snglBuf24Stereo[] = {GLX_RGBA, 
                                GLX_RED_SIZE, 8, 
                                GLX_GREEN_SIZE, 8, 
                                GLX_BLUE_SIZE, 8, 
                                GLX_DEPTH_SIZE, 16, 
                                GLX_STENCIL_SIZE, 8, 
                                GLX_ACCUM_RED_SIZE, 8,
                                GLX_ACCUM_GREEN_SIZE, 8,
                                GLX_ACCUM_BLUE_SIZE, 8,
                                GLX_STEREO,
                                None};
static int dblBuf24[] =  {GLX_RGBA, 
                          GLX_RED_SIZE, 4, 
                          GLX_GREEN_SIZE, 4, 
                          GLX_BLUE_SIZE, 4, 
                          GLX_DEPTH_SIZE, 16, 
                          GLX_STENCIL_SIZE, 8, 
                          GLX_ACCUM_RED_SIZE, 8,
                          GLX_ACCUM_GREEN_SIZE, 8,
                          GLX_ACCUM_BLUE_SIZE, 8,
                          GLX_DOUBLEBUFFER, 
                          None};
static int dblBuf24Stereo[] =  {GLX_RGBA, 
                                GLX_RED_SIZE, 4, 
                                GLX_GREEN_SIZE, 4, 
                                GLX_BLUE_SIZE, 4, 
                                GLX_DEPTH_SIZE, 16, 
                                GLX_STENCIL_SIZE, 8, 
                                GLX_ACCUM_RED_SIZE, 8,
                                GLX_ACCUM_GREEN_SIZE, 8,
                                GLX_ACCUM_BLUE_SIZE, 8,
                                GLX_DOUBLEBUFFER, 
                                GLX_STEREO,
                                None};
static int snglBuf8[] =  {GLX_RGBA, 
                          GLX_RED_SIZE, 3, 
                          GLX_GREEN_SIZE, 3, 
                          GLX_BLUE_SIZE, 2, 
                          GLX_DEPTH_SIZE, 16, 
                          None};
static int snglBuf8Stereo[] =  {GLX_RGBA, 
                                GLX_RED_SIZE, 3, 
                                GLX_GREEN_SIZE, 3, 
                                GLX_BLUE_SIZE, 2, 
                                GLX_DEPTH_SIZE, 16, 
                                GLX_STEREO,
                                None};
static int dblBuf8[] =   {GLX_RGBA, 
                          GLX_RED_SIZE, 1, 
                          GLX_GREEN_SIZE, 2, 
                          GLX_BLUE_SIZE, 1, 
                          GLX_DEPTH_SIZE, 16, 
                          GLX_DOUBLEBUFFER, 
                          None};

static int dblBuf8Stereo[] =   {GLX_RGBA, 
                                GLX_RED_SIZE, 1, 
                                GLX_GREEN_SIZE, 2, 
                                GLX_BLUE_SIZE, 1, 
                                GLX_DEPTH_SIZE, 16, 
                                GLX_DOUBLEBUFFER, 
                                GLX_STEREO,
                                None};

static int xerr;
static int ErrorHandler (Display *dpy, XErrorEvent *event)
{
  // we don't really care about the error
  // let's hope for the best
  if(event)
    xerr=event->error_code;  

  if ( event->error_code != BadWindow ) {
    char buf[256];
    XGetErrorText (dpy, event->error_code, buf, sizeof(buf));
    error("Xwin: %s\n", buf);
  } else
    error("Xwin: BadWindow (%d)\n", xerr);
  return (0);
}

static Bool WaitForNotify(Display *, XEvent *e, char *arg)
{
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}


 

struct gemglxwindow::PIMPL {
  int         fs;                 // FullScreen

  Display     *dpy;               // X Display
  Window      win;                // X Window
  int         screen;             // X Screen
  Colormap    cmap;               // X color map
  GLXContext  context;            // OpenGL context

  Atom        delete_atom;
  
#ifdef HAVE_LIBXXF86VM
  XF86VidModeModeInfo deskMode; // originale ModeLine of the Desktop
#endif

  XIM inputMethod;
  XIC inputContext;


  bool        have_border;

  bool doDispatch;

  PIMPL(void) : 
    fs(0),
    dpy(NULL), 
    win(0), 
    cmap(0), 
    context(NULL), 
    delete_atom(0),
#ifdef HAVE_LIBXXF86VM
    //    deskMode(0),
#endif
    inputMethod(NULL),
    inputContext(NULL),

    have_border(false),
    doDispatch(false)
  {
  }
  ~PIMPL(void) {
  }

  std::string key2string(XKeyEvent* kb) {
#define KEYSTRING_SIZE 10
    char keystring[KEYSTRING_SIZE];
    KeySym keysym_return;
    int len=0;

    if(inputContext) {
      len=Xutf8LookupString(inputContext, kb,keystring,KEYSTRING_SIZE,&keysym_return,NULL);
    }
    if(len<1) {
      len=XLookupString(kb,keystring,2,&keysym_return,NULL);
    }

    if ( (keysym_return & 0xff00)== 0xff00 ) {
      //non alphanumeric key: use keysym
      return std::string(XKeysymToString(keysym_return));
    }
    
    if (len==0) {
      //modifier key:use keysym
      //triggerKeyboardEvent(XKeysymToString(keysym_return), kb->keycode, 1);
    } else if(len<KEYSTRING_SIZE) {
      keystring[len]=0;
    } else {
      keystring[KEYSTRING_SIZE-1]=0;
    }
    
    return std::string(keystring);
  }

  bool create(std::string display, int buffer, bool fullscreen, bool border, int&x, int&y, unsigned int&w, unsigned int&h) {
    int modeNum=4;
    int bestMode=0;
#ifdef HAVE_LIBXXF86VM
    XF86VidModeModeInfo **modes;
#endif

    XSetErrorHandler (ErrorHandler);

    if ( (dpy = XOpenDisplay(display.c_str())) == NULL) { 
      ::error("Could not open display %s",display.c_str());
      return false;
    }
    screen  = DefaultScreen(dpy);

    if ( !glXQueryExtension(dpy, NULL, NULL) ) {
      throw(GemException("X server has no OpenGL GLX extension"));
      return false;
    } 

    if (fullscreen){
      if (!display.empty()){
        throw(GemException("fullscreen not available on remote display"));
        fullscreen=false;
      } else {
#ifdef HAVE_LIBXXF86VM
        XF86VidModeGetAllModeLines(dpy, screen, &modeNum, &modes);
        deskMode = *modes[0];
#else
        throw(GemException("no xxf86vm-support: cannot switch to fullscreen"));
#endif
      }
    }
    XVisualInfo *vi;
    // the user wants double buffer
    if (buffer == 2) {
      // try for a double-buffered on 24bit machine (try stereo first)
      vi = glXChooseVisual(dpy, screen, dblBuf24Stereo);
      if (vi == NULL)
        vi = glXChooseVisual(dpy, screen, dblBuf24);
      if (vi == NULL) {
        // try for a double buffered on a 8bit machine (try stereo first)
        vi = glXChooseVisual(dpy, screen, dblBuf8Stereo);
        if(vi == NULL)
          vi = glXChooseVisual(dpy, screen, dblBuf8);
        if (vi == NULL) {
          throw(GemException("Unable to create double buffer window"));
          return false;
        }
        ::post("Only using 8 color bits");
      }
    }
    // the user wants single buffer
    else {
      // try for a single buffered on a 24bit machine (try stereo first)
      vi = glXChooseVisual(dpy, screen, snglBuf24Stereo);
      if (vi == NULL)
        vi = glXChooseVisual(dpy, screen, snglBuf24);
      if (vi == NULL) {
        // try for a single buffered on a 8bit machine (try stereo first)
        vi = glXChooseVisual(dpy, screen, snglBuf8Stereo);
        if (vi == NULL)
          vi = glXChooseVisual(dpy, screen, snglBuf8);
        if (vi == NULL) {
          throw(GemException("Unable to create single buffer window"));
          return false;
        }
        ::post("Only using 8 color bits");
      }
    }

    if (vi->c_class != TrueColor && vi->c_class != DirectColor) {
      ::error("TrueColor visual required for this program (got %d)", vi->c_class);
      return false;
    }
    // create the rendering context
    try {
      context = glXCreateContext(dpy, vi, masterContext, GL_TRUE);
      // this masterContext should only be initialized once by a static PIMPL
      // see below in gemglxwindow::create()
      if(!masterContext) 
        masterContext=context;
    } catch(void*e){
      context=NULL;
    }
    if (context == NULL) {
      throw(GemException("Could not create rendering context"));
      return false;
    }
    // create the X color map
    cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), 
                                    vi->visual, AllocNone);
    if (!cmap) {
      throw(GemException("Could not create X colormap"));
      return false;
    }

    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.border_pixel = 0;
    // event_mask creates signal that window has been created
    swa.event_mask = EVENT_MASK;

    int flags;
#ifdef HAVE_LIBXXF86VM
    if (fullscreen){
      /* look for mode with requested resolution */
      for (int i = 0; i < modeNum; i++) {
        if ((modes[i]->hdisplay == w) && (modes[i]->vdisplay == w)) {
          bestMode = i;
        }
      }
    
      XF86VidModeSwitchToMode(dpy, screen, modes[bestMode]);
      XF86VidModeSetViewPort(dpy, screen, 0, 0);
      w = modes[bestMode]->hdisplay;
      h = modes[bestMode]->vdisplay;
      x=y=0;
      XFree(modes);

      swa.override_redirect = True;
      flags=CWBorderPixel|CWColormap|CWEventMask|CWOverrideRedirect;
    } else
#endif
      { // !fullscren
        if (border){
          swa.override_redirect = False;
          flags=CWBorderPixel|CWColormap|CWEventMask|CWOverrideRedirect;
        } else {
          swa.override_redirect = True;
          flags=CWBorderPixel|CWColormap|CWEventMask|CWOverrideRedirect;
        }
      }
    fs = fullscreen;

    win = XCreateWindow(dpy, RootWindow(dpy, vi->screen),
                                 x, y, w, h,
                                 0, vi->depth, InputOutput, 
                                 vi->visual, flags, &swa);
    if (!win) {
      throw(GemException("Could not create X window"));
      return false;
    }

    have_border=(True==swa.override_redirect);

    XSelectInput(dpy, win, EVENT_MASK);

    inputMethod = XOpenIM(dpy, NULL, NULL, NULL);
    if(inputMethod) {
      XIMStyle style=0;
      XIMStyles *stylePtr=NULL;
      const char *preedit_attname = NULL;
      XVaNestedList preedit_attlist = NULL;

      if ((XGetIMValues(inputMethod, XNQueryInputStyle, &stylePtr, NULL) != NULL)) {
        stylePtr=NULL;
      }


      /*
       * Select the best input style supported by both the IM and Tk.
       */
      int i=0;
      if(stylePtr) {
        for (i = 0; i < stylePtr->count_styles; i++) {
          XIMStyle thisStyle = stylePtr->supported_styles[i];
          if (thisStyle == (XIMPreeditPosition | XIMStatusNothing)) {
            style = thisStyle;
            break;
          } else if (thisStyle == (XIMPreeditNothing | XIMStatusNothing)) {
            style = thisStyle;
          }
        }
        XFree(stylePtr);
      }


      if (style & XIMPreeditPosition) {
        XPoint spot = {0, 0};
        XFontSet inputXfs;
        preedit_attname = XNPreeditAttributes;
        preedit_attlist = XVaCreateNestedList(0,
                                              XNSpotLocation, &spot,
                                              XNFontSet, inputXfs,
                                              NULL);
      }


      inputContext=XCreateIC(inputMethod,
                                      XNInputStyle, style,
                                      XNClientWindow, win,
                                      XNFocusWindow, win,
                                      preedit_attname, preedit_attlist,
                                      NULL);
    }



    /* found a bit at
     * http://biology.ncsa.uiuc.edu/library/SGI_bookshelves/SGI_Developer/books/OpenGL_Porting/sgi_html/apf.html
     * LATER think about reacting on this event...
     */
    delete_atom = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
    if (delete_atom != None)
      XSetWMProtocols(dpy, win, &delete_atom,1);

    try{
      xerr=0;
      glXMakeCurrent(dpy, win, context);

      if(xerr!=0) {
        /* seems like the error-handler was called; so something did not work the way it should
         * should we really prevent window-creation in this case?
         * LATER re-think the entire dual-context thing
         */

        throw(GemException("problems making glX-context current: refusing to continue"));
        throw(GemException("try setting the environment variable GEM_SINGLE_CONTEXT=1"));
        return false;
      }
      Window winDummy;
      unsigned int depthDummy;
      unsigned int borderDummy;
      int x, y;
      XGetGeometry(dpy, win,
                   &winDummy, 
                   &x, &y,
                   &w, &h,
                   &borderDummy, &depthDummy);
    }catch(void*e){
      throw(GemException("Could not make glX-context current"));
      return false;
    }
    return true;
  }

  static GLXContext  masterContext;// The GLXcontext to share rendering with
  static gem::Context*masterGemContext;
};
GLXContext   gemglxwindow::PIMPL::masterContext=NULL;
gem::Context*gemglxwindow::PIMPL::masterGemContext=NULL;

/////////////////////////////////////////////////////////
//
// gemglxwindow
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
gemglxwindow :: gemglxwindow(void) :
  m_display(std::string("")),
  m_pimpl(new PIMPL())
{
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
gemglxwindow :: ~gemglxwindow()
{
  if(m_pimpl->win) 
    destroyMess();

  delete m_pimpl;
}


bool gemglxwindow :: makeCurrent(void){
  if(!m_pimpl->dpy || !m_pimpl->win || !m_pimpl->context)
    return false;

  xerr=0;
  glXMakeCurrent(m_pimpl->dpy, m_pimpl->win, m_pimpl->context);
  if(xerr!=0) {
    return false;
  }
  return true;
}

void gemglxwindow :: swapBuffers(void) {
  glXSwapBuffers(m_pimpl->dpy, m_pimpl->win);
}

void gemglxwindow::dispatch(void) {
  if(!m_pimpl->doDispatch)return;
  XEvent event; 
  XButtonEvent* eb = (XButtonEvent*)&event; 
  XKeyEvent* kb  = (XKeyEvent*)&event; 
  char keystring[2];
  KeySym keysym_return;

  while (XCheckWindowEvent(m_pimpl->dpy,m_pimpl->win,
                           StructureNotifyMask |
                           KeyPressMask | KeyReleaseMask |
                           PointerMotionMask | 
                           ButtonMotionMask |
                           ButtonPressMask | 
                           ButtonReleaseMask,
                           &event))
    {
      switch (event.type)
        {
        case ButtonPress: 
          button(eb->button-1, 1);
          motion(eb->x, eb->y);
          break; 
        case ButtonRelease: 
          button(eb->button-1, 0);
          motion(eb->x, eb->y);
          break; 
        case MotionNotify: 
          motion(eb->x, eb->y);
          if(!m_pimpl->have_border) {
            int err=XSetInputFocus(m_pimpl->dpy, m_pimpl->win, RevertToParent, CurrentTime);
            err=0;
          }
          break; 
        case KeyPress:
          key(m_pimpl->key2string(kb), kb->keycode, 1);
          break;
        case KeyRelease:
          key(m_pimpl->key2string(kb), kb->keycode, 0);
          break;
        case ConfigureNotify:
          if ((event.xconfigure.width != m_width) || 
              (event.xconfigure.height != m_height)) {
            m_width=event.xconfigure.width;
            m_height=event.xconfigure.height;
            XResizeWindow(m_pimpl->dpy, m_pimpl->win, m_width, m_height);
            dimension(m_width, m_height);
          }
          if ((event.xconfigure.send_event) && 
              ((event.xconfigure.x != m_xoffset) || 
               (event.xconfigure.y != m_yoffset))) {
            m_xoffset=event.xconfigure.x;
            m_yoffset=event.xconfigure.y;
            position(m_xoffset, m_yoffset);
          }
          break;
        default:
          // post("event %d", event.type);
          break; 
        }
    }
  
  if (XCheckTypedEvent(m_pimpl->dpy,  ClientMessage, &event)) {
    info("window", "destroy");
    //    GemMan::destroyWindowSoon();
  }
}


/////////////////////////////////////////////////////////
// bufferMess
//
/////////////////////////////////////////////////////////
void gemglxwindow :: bufferMess(int buf)
{
  switch(buf) {
  case 1: case 2:
    m_buffer=buf;
    break;
  default:
    error("buffer can only be '1' (single) or '2' (double) buffered");
    break;
  }
}

/////////////////////////////////////////////////////////
// fsaaMess
//
/////////////////////////////////////////////////////////
void gemglxwindow :: fsaaMess(int value)
{
  m_fsaa=value;
}

/////////////////////////////////////////////////////////
// titleMess
//
/////////////////////////////////////////////////////////
void gemglxwindow :: titleMess(std::string s)
{
  m_title=s;
  if(m_pimpl->dpy && m_pimpl->win) {
    XSetStandardProperties(m_pimpl->dpy, m_pimpl->win,
                           m_title.c_str(), "gem", 
                           None, 0, 0, NULL);
  }

}
/////////////////////////////////////////////////////////
// border
//
/////////////////////////////////////////////////////////
void gemglxwindow :: borderMess(bool setting)
{
  m_border=setting;
}
/////////////////////////////////////////////////////////
// dimensionsMess
//
/////////////////////////////////////////////////////////
void gemglxwindow :: dimensionsMess(unsigned int width, unsigned int height)
{
  if (width <= 0) {
    error("width must be greater than 0");
    return;
  }
    
  if (height <= 0 ) {
    error ("height must be greater than 0");
    return;
  }

  m_width=width;
  m_height=height;
}
/////////////////////////////////////////////////////////
// fullscreenMess
//
/////////////////////////////////////////////////////////
void gemglxwindow :: fullscreenMess(bool on)
{
  m_fullscreen = on;
}

/////////////////////////////////////////////////////////
// offsetMess
//
/////////////////////////////////////////////////////////
void gemglxwindow :: offsetMess(int x, int y)
{
  m_xoffset=x;
  m_yoffset=y;
}

/////////////////////////////////////////////////////////
// createMess
//
/////////////////////////////////////////////////////////
bool gemglxwindow :: create(void)
{
  bool success=true;

  static gemglxwindow::PIMPL*constPimpl=NULL;
  if(!constPimpl) {
    constPimpl=new PIMPL();

    try {
      int x=0, y=0;
      unsigned int w=1, h=1;
      success=constPimpl->create("", 2, false, false, x, y, w, h);
      constPimpl->masterContext=constPimpl->context;
    } catch (GemException&x) {
      error("const context creation failed: %s", x.what());
      logpost(NULL, 4, "continuing at your own risk!");
    }
    if(!constPimpl->masterGemContext) {
      try {
	constPimpl->masterGemContext = createContext();
      } catch (GemException&x) {
	constPimpl->masterGemContext = NULL;
	error("context creation failed: %s", x.what());
      }
    }
  }

  if(constPimpl->masterGemContext && !m_context) {
    m_context=constPimpl->masterGemContext;
  }

  int modeNum=4;
  int bestMode=0;
#ifdef HAVE_LIBXXF86VM
  XF86VidModeModeInfo **modes;
#endif
  int fullscreen=m_fullscreen;

  char svalue[3];
  snprintf(svalue, 3, "%d", m_fsaa);
  svalue[2]=0;
  if (m_fsaa!=0) setenv("__GL_FSAA_MODE", svalue, 1); // this works only for NVIDIA-cards

 
  try {
    success=m_pimpl->create(m_display, m_buffer, m_fullscreen, m_border, m_xoffset, m_yoffset, m_width, m_height);
  } catch (GemException&x) {
    x.report();
    success=false;
  }
  if(!success)return false;

  XMapRaised(m_pimpl->dpy, m_pimpl->win);
  //  XMapWindow(m_pimpl->dpy, m_pimpl->win);
  XEvent report;
  XIfEvent(m_pimpl->dpy, &report, WaitForNotify, (char*)m_pimpl->win);
  if (glXIsDirect(m_pimpl->dpy, m_pimpl->context))
    post("Direct Rendering enabled!");

  cursorMess(m_cursor);
  titleMess(m_title);
  return createGemWindow();
}
void gemglxwindow :: createMess(std::string display)
{
  if(m_pimpl->win) {
    error("window already made");
    return;
  }

  m_display=display;
  if(!create()) {
    destroyMess();
    return;
  }
  dimension(m_width, m_height);
  m_pimpl->doDispatch=true;
}
/////////////////////////////////////////////////////////
// destroy window
//
/////////////////////////////////////////////////////////
void gemglxwindow :: destroy(void)
{
  /* both glXMakeCurrent() and XCloseDisplay() will crash the application
   * if the handler of the display (m_pimpl->dpy) is invalid, e.g. because
   * somebody closed the Gem-window with xkill or by clicking on the "x" of the window
   */
  if (m_pimpl->dpy) {
    int err=0;
    /* patch by cesare marilungo to prevent the crash "on my laptop" */
    glXMakeCurrent(m_pimpl->dpy, None, NULL); /* this crashes if no window is there! */
    
    if (m_pimpl->win)
      err=XDestroyWindow(m_pimpl->dpy, m_pimpl->win);
    if (m_pimpl->context) {
      // this crashes sometimes on my laptop:
      glXDestroyContext(m_pimpl->dpy, m_pimpl->context);
    }
    if (m_pimpl->cmap)
      err=XFreeColormap(m_pimpl->dpy, m_pimpl->cmap);
    
#ifdef HAVE_LIBXXF86VM
    if (m_pimpl->fs){
      XF86VidModeSwitchToMode(m_pimpl->dpy, m_pimpl->screen, &m_pimpl->deskMode);
      XF86VidModeSetViewPort(m_pimpl->dpy, m_pimpl->screen, 0, 0);
      m_pimpl->fs=0;
    }
#endif
    
    err=XCloseDisplay(m_pimpl->dpy); /* this crashes if no window is there */
  }
  m_pimpl->dpy = NULL;
  m_pimpl->win = 0;
  m_pimpl->cmap = 0;
  m_pimpl->context = NULL;
  if(m_pimpl->delete_atom)m_pimpl->delete_atom=None; /* not very sophisticated destruction...*/
  
  destroyGemWindow();
}
void gemglxwindow :: destroyMess(void)
{
  m_pimpl->doDispatch=false;
  if(makeCurrent()) {
    destroy();
  } else {
    error("unable to destroy current window");
  }
}


/////////////////////////////////////////////////////////
// cursorMess
//
/////////////////////////////////////////////////////////
void gemglxwindow :: cursorMess(bool state)
{
  m_cursor=state;
  if(!m_pimpl->dpy || !m_pimpl->win)
    return;

  if (!state) {
    static char data[1] = {0};
    XColor dummy;

    Pixmap blank = XCreateBitmapFromData(m_pimpl->dpy, m_pimpl->win,
				  data, 1, 1);
    Cursor cursor = XCreatePixmapCursor(m_pimpl->dpy, blank, blank,
				 &dummy, &dummy, 0, 0);
    XFreePixmap(m_pimpl->dpy, blank);
    XDefineCursor(m_pimpl->dpy, m_pimpl->win, cursor);
  }
  else
    XUndefineCursor(m_pimpl->dpy, m_pimpl->win);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemglxwindow :: obj_setupCallback(t_class *classPtr)
{
}

#endif /* HAVE_GL_GLX_H */
