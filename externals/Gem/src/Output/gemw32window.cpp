////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "Gem/GemConfig.h"
#ifdef _WIN32
# include "gemw32window.h"

#define DEBUGLINE ::startpost("%s:%d[%s] ", __FILE__, __LINE__, __FUNCTION__), ::post

# include <stdlib.h>

# ifdef HAVE_QUICKTIME
#  include <QTML.h>
#  include <Movies.h>
# endif /* HAVE_QUICKTIME */

# include "Gem/Event.h"
# include "Gem/GemGL.h"
# include "RTE/MessageCallbacks.h"
# include "Gem/Exception.h"

# include <map>

static bool initGemWin(void) {
# ifdef HAVE_QUICKTIME
	OSErr		err = noErr;

	// Initialize QuickTime Media Layer
	err = InitializeQTML(0);
	if (err)
    {
      error("GEM Man: Could not initialize quicktime: error %d\n", err);
      return false;
    }	
	// Initialize QuickTime
	EnterMovies();
	if (err)
    {
      error("GEM Man: Could not initialize quicktime: error %d\n", err);
      return false;
    }	
	logpost(NULL, 5, "Gem Man: QT init OK");
# endif /* HAVE_QUICKTIME */
  return true;
}


class gemw32window :: Window {
public:
  HWND win;
  HDC dc;
  HGLRC context;

  static HGLRC sharedContext;
  Window(gemw32window*parent, HINSTANCE hInstance, int buffer, bool fullscreen, bool border, std::string title, int &x, int &y, unsigned int &w, unsigned int &h) :
    win(NULL),
    dc(NULL),
    context(NULL) {
    try {
      create(hInstance, buffer, fullscreen, border, title, x, y, w, h);
    } catch(GemException&x) {
      destroy();
      throw(x);
    }
    if(parent && win)
        s_winmap[win]=parent;
  }
  ~Window(void) {
    destroy();
  }
  static RECT getRealRect(int x, int y, unsigned int w, unsigned int h, 
                          bool border, bool fullscreen, 
                          DWORD &style, DWORD&exStyle) {
      exStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
      style  =WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
      
      if (fullscreen){
        exStyle  = WS_EX_APPWINDOW;
        style     |= WS_POPUP;
      } else {
        exStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        if (border)
          style |= WS_OVERLAPPEDWINDOW;
        else
          style |= WS_POPUP;
      }

      RECT newSize;
      newSize.left = x;
      newSize.top  = y;
      newSize.right  = w+x;
      newSize.bottom = h+y;
    
      AdjustWindowRectEx(&newSize, style, FALSE, exStyle); // no menu
      if (newSize.left<0 && x>=0){
        newSize.right-=newSize.left;
        newSize.left=0;
      }
      if (newSize.top<0 && y>=0){
        newSize.bottom-=newSize.top;
        newSize.top=0;
      }
      return newSize;
  }

private:
  static std::map<HWND, gemw32window*>s_winmap;
  void create(HINSTANCE hInstance, int buffer, bool fullscreen, bool border, std::string title, int &x, int &y, unsigned int &w, unsigned int &h) {
    DWORD dwExStyle;
    DWORD style;
    
    if (fullscreen){
        DEVMODE dmScreenSettings;								// Device Mode
    
        if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings)){
            ::error("GEM: couldn't get screen capabilities!");
        } else {
        w = dmScreenSettings.dmPelsWidth;
        h = dmScreenSettings.dmPelsHeight;
        }
    
        x=y=0;

        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
        dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth	= w;			// Selected Screen Width
        dmScreenSettings.dmPelsHeight	= h;			// Selected Screen Height
        dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
        dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
          dmScreenSettings.dmPelsWidth	= w;
          dmScreenSettings.dmPelsHeight	= h;
          if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
              ::error("couldn't switch to fullscreen");
            fullscreen=false;
          }
        }
      }

      // Since Windows uses some of the window for the border, etc,
      //		we have to ask how big the window should really be
      RECT newSize = getRealRect(x, y, w, h, 
                          border, fullscreen, 
                          style, dwExStyle);
      // Create the window
      win = CreateWindowEx (
                            dwExStyle,
                            "GEM",
                            title.c_str(),
                            style,
                            newSize.left,
                            newSize.top,
                            newSize.right - newSize.left,
                            newSize.bottom - newSize.top,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);
      if (!win)  {
        throw(GemException("Unable to create window"));
      }
      // create the device context
      dc = GetDC(win);
      if (!dc)  {
        throw(GemException("GEM: Unable to create device context"));
      }

      // set the pixel format for the window
      if (!bSetupPixelFormat(dc, buffer))  {
        throw(GemException("Unable to set window pixel format"));
      }

      // create the OpenGL context
      context = wglCreateContext(dc);
      if (!context)  {
        throw(GemException("Unable to create OpenGL context"));
      }

      // do we share display lists?
      if (sharedContext) wglShareLists(sharedContext, context);

      // make the context the current rendering context
      if (!wglMakeCurrent(dc, context))   {
        throw(GemException("Unable to make OpenGL context current"));
      }
    }

    void destroy(void) {
      if (context) {
        wglDeleteContext(context);
      }
      if (win) {
        if (dc) {
          ReleaseDC(win, dc);
        }

        s_winmap.erase(win);
        DestroyWindow(win);
      }

      dc  = NULL;
      win = NULL;
      context=NULL;
    }


    static bool bSetupPixelFormat(HDC hdc, int buffer) {
      PIXELFORMATDESCRIPTOR pfd;
    
      // clean out the descriptor
      memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
      if (buffer == 2)
        pfd.dwFlags = pfd.dwFlags | PFD_DOUBLEBUFFER;
      pfd.dwLayerMask = PFD_MAIN_PLANE;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = 24;
      pfd.cRedBits = 8;
      pfd.cBlueBits = 8;
      pfd.cGreenBits = 8;
      pfd.cDepthBits = 16;
      pfd.cAccumBits = 0;
      pfd.cStencilBits = 8;
      
      int pixelformat;
      if ( (pixelformat = ChoosePixelFormat(hdc, &pfd)) == 0 )
        {
            throw(GemException("ChoosePixelFormat failed"));
        }
      if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
        {
            throw(GemException("SetPixelFormat failed"));
          return(false);
        }
      return(true);
    }

  ////////////////////////////////////////////////////////
  // MainWndProc
  //
  /////////////////////////////////////////////////////////
    public:
  static LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    gemw32window*obj=s_winmap[hWnd];
    if(obj)
      return obj->event(uMsg, wParam, lParam);
    
    return(DefWindowProc (hWnd, uMsg, wParam, lParam));
  }
};

HGLRC gemw32window::Window::sharedContext=NULL;
std::map<HWND, gemw32window*>gemw32window::Window::s_winmap;

CPPEXTERN_NEW(gemw32window);
gemw32window::gemw32window(void) :
  m_topmost(false),
  m_win(NULL)
{
  if(!initGemWin())
    throw(GemException("could not initialize window infrastructure"));
}
gemw32window::~gemw32window(void) {
    destroyMess();
}

/////////////////////////////////////////////////////////
// createGemWindow
//
/////////////////////////////////////////////////////////
bool gemw32window:: create(void)
{
  unsigned int w = m_width;
  unsigned int h = m_height;
  int x = m_xoffset;
  int y = m_yoffset;

  static bool firstTime = true;
    
  // Register the frame class
  HINSTANCE hInstance = GetModuleHandle(NULL);
  if (!hInstance)  {
    error("GEM: Unable to get module instance");
    return false;
  }
  if (firstTime)  {
    WNDCLASS wndclass;
    wndclass.style         = 0;
    wndclass.lpfnWndProc   = (WNDPROC)Window::MainWndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hCursor       = LoadCursor(NULL, IDC_CROSS);
    wndclass.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = "GEM";
    
    if (!RegisterClass(&wndclass) )  {
      error("GEM: Unable to register window class");
      return false;
    }
    firstTime = false;
  }

  if (NULL==Window::sharedContext) {
    static Window*s_sharedWindow=NULL;
    try {
        unsigned int w0=0, h0=0;
        int x0=0, y0=0;
        s_sharedWindow=new Window(NULL, hInstance, 2, false, false, "GEM",
                     x0, y0,
                     w0, h0);
        Window::sharedContext=s_sharedWindow->context;
  } catch (GemException&) {
      Window::sharedContext=NULL;
  }
  }
  m_win=NULL;
  try {
    m_win=new Window(this, hInstance, m_buffer, m_fullscreen>0, m_border, m_title,
                     x, y, w, h);
  } catch (GemException&x) {
    error("%s", x.what());
    return false;
  }
  // show and update main window
  if (m_fullscreen){
    ShowWindow(m_win->win,SW_SHOW);				// Show The Window
    SetForegroundWindow(m_win->win);				// Slightly Higher Priority
    SetFocus(m_win->win);
  } else  {
    ShowWindow(m_win->win, SW_SHOWNORMAL);
  }
    
  UpdateWindow(m_win->win);
  dimension(w, h);
  position(x, y);
  return createGemWindow();
}
void gemw32window:: createMess(std::string s) {
    if(m_win) {
        error("window already made");
        return;
    }
    if(!create()) {
        destroyMess();
        return;
    }
    topmostMess(m_topmost);
}

/////////////////////////////////////////////////////////
// destroyGemWindow
//
/////////////////////////////////////////////////////////
void gemw32window:: destroy(void)
{
  if (m_fullscreen)
    ChangeDisplaySettings(NULL,0);	// Switch Back To The Desktop

  if(m_win)
    delete m_win;
  m_win=NULL;

  destroyGemWindow();
}

/////////////////////////////////////////////////////////
// switch cursor on/off
//
/////////////////////////////////////////////////////////
void gemw32window::cursorMess(bool state)
{
    ShowCursor(state);
}

void gemw32window::fullscreenMess(int state) {
    m_fullscreen=state;
    if(!m_win)return;
    unsigned int w = m_width;
    unsigned int h = m_height;
    int x = m_xoffset;
    int y = m_yoffset;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!hInstance)  {
        error("GEM: Unable to get module instance");
        return;
    }
    Window*tmpwin=NULL;
    try {
        tmpwin=new Window(this, hInstance, m_buffer, m_fullscreen>0, m_border, m_title,
            x, y,
            w, h);
    } catch (GemException&x) {
        error("unable to toggle fullscreen mode: %s", x.what());
    }
    if(tmpwin) {
        delete m_win;
        m_win=tmpwin;
        // show and update main window
        if (m_fullscreen){
            ShowWindow(m_win->win,SW_SHOW);				// Show The Window
            SetForegroundWindow(m_win->win);				// Slightly Higher Priority
            SetFocus(m_win->win);
        } else  {
            ShowWindow(m_win->win, SW_SHOWNORMAL);
        }
        UpdateWindow(m_win->win);
        dimension(w, h);
        position(x, y);
    }
}

/////////////////////////////////////////////////////////
// set topmost position on/off
//
/////////////////////////////////////////////////////////
void gemw32window::topmostMess(bool state)
{
  m_topmost=state;

  if(m_win)
    SetWindowPos(m_win->win, (state?HWND_TOPMOST:HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); 
}
/////////////////////////////////////////////////////////
// dimensionsMess
//
/////////////////////////////////////////////////////////
void gemw32window :: dimensionsMess(unsigned int width, unsigned int height)
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

  move();
}
void gemw32window :: offsetMess(int x, int y) {
    GemWindow::offsetMess(x, y);
    move();
}

void gemw32window::move(void) {
    if(m_win) {
        DWORD style, exStyle;
        RECT newSize = Window::getRealRect(m_xoffset, m_yoffset, m_width, m_height, 
                          m_border, m_fullscreen>0, 
                          style, exStyle);

        //MoveWindow(m_win->win, m_xoffset, m_yoffset, m_width, m_height, true);
        MoveWindow(m_win->win, newSize.left,
                               newSize.top,
                               newSize.right - newSize.left,
                               newSize.bottom - newSize.top,
                               true);

    }
}

void gemw32window:: titleMess(std::string s) {
    m_title=s;
    if(m_win)
        SetWindowText(m_win->win, s.c_str());
}

void gemw32window::swapBuffers(void)
{
  SwapBuffers(m_win->dc);
}

bool gemw32window::makeCurrent(void)
{
  if (!m_win)return false;
  wglMakeCurrent(m_win->dc, m_win->context); 
  return true;
}

void gemw32window::dispatch(void)
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
}
LONG WINAPI gemw32window::event(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    
	static RECT rcClient;
	static int ctrlKeyDown = 0;

	// assume that we handle the message
  long lRet = 0;

  switch (uMsg)
    {
      // mouse motion
    case WM_MOUSEMOVE:
      motion(LOWORD(lParam), HIWORD(lParam));
      break;

      // left button up/down
    case WM_LBUTTONUP: case WM_LBUTTONDOWN:
      button(0, (uMsg==WM_LBUTTONDOWN));
      motion(LOWORD(lParam), HIWORD(lParam));
      break;
      // middle button up/down
    case WM_MBUTTONUP: case WM_MBUTTONDOWN:
      button(1, (uMsg==WM_MBUTTONDOWN));
      motion(LOWORD(lParam), HIWORD(lParam));
      break;
      // middle button up/down
    case WM_RBUTTONUP: case WM_RBUTTONDOWN:
      button(2, (uMsg==WM_RBUTTONDOWN));
      motion(LOWORD(lParam), HIWORD(lParam));
      break;

      // keyboard action
    case WM_KEYUP: case WM_KEYDOWN:
        key((char*)&wParam, (int)wParam, (uMsg==WM_KEYDOWN));
      break;
      // resize event
    case WM_SIZE:
      m_width=LOWORD(lParam);
      m_height=HIWORD(lParam);
      dimension(m_width, m_height);
      //GetClientRect(m_win->win, &rcClient);
      break;
    case WM_MOVE:
      m_xoffset=LOWORD(lParam);
      m_yoffset=HIWORD(lParam);
      position(m_xoffset, m_yoffset);
      break;

      // we want to override these messages
      // and not do anything (rather let the user react and programmatically destroy)
    case WM_DESTROY:
    case WM_CLOSE: do {
        std::vector<t_atom>al;
        t_atom a;
        SETSYMBOL(&a, gensym("window"));
        al.push_back(a);
        SETSYMBOL(&a, gensym("destroy"));
        al.push_back(a);
        info(al);
      } while(0);
      break;
    case WM_CREATE:
      {
      }
      break;
      
      // pass all unhandled messages to DefWindowProc
    default:
      lRet = DefWindowProc (m_win->win, uMsg, wParam, lParam);
      break;
    }
  return(lRet);
}



/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemw32window :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG1(classPtr, "topmost", topmostMess, bool);
}
#endif /* WIN32 */
