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

# include "GemWinCreate.h"

# include <stdlib.h>

# include "Gem/Event.h"
# include "Gem/GemGL.h"

#include "Gem/RTE.h"

GEM_EXTERN void gemAbortRendering();

/////////////////////////////////////////////////////////
// bSetupPixelFormat
//
/////////////////////////////////////////////////////////
BOOL bSetupPixelFormat(HDC hdc, const WindowHints &hints)
{
    PIXELFORMATDESCRIPTOR pfd;
	
	// clean out the descriptor
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    if (hints.buffer == 2)
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
        post("GEM: ChoosePixelFormat failed");
        return(FALSE);
    }
    if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
    {
        post("GEM: SetPixelFormat failed");
        return(FALSE);
    }
    return(TRUE);
}

/////////////////////////////////////////////////////////
// MainWndProc
//
/////////////////////////////////////////////////////////
LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rcClient;
	static int ctrlKeyDown = 0;

	// assume that we handle the message
    long lRet = 0;

    switch (uMsg)
    {
        // mouse motion
        case WM_MOUSEMOVE:
            triggerMotionEvent(LOWORD(lParam), HIWORD(lParam));
            break;

        // left button up
        case WM_LBUTTONUP:
            triggerButtonEvent(0, 0, LOWORD(lParam), HIWORD(lParam));
            break;

        // left button down
        case WM_LBUTTONDOWN:
            triggerButtonEvent(0, 1, LOWORD(lParam), HIWORD(lParam));
            break;

        // middle button up
        case WM_MBUTTONUP:
            triggerButtonEvent(1, 0, LOWORD(lParam), HIWORD(lParam));
            break;

        // middle button down
        case WM_MBUTTONDOWN:
            triggerButtonEvent(1, 1, LOWORD(lParam), HIWORD(lParam));
            break;

        // right button up
        case WM_RBUTTONUP:
            triggerButtonEvent(2, 0, LOWORD(lParam), HIWORD(lParam));
            break;

        // right button down
        case WM_RBUTTONDOWN:
            triggerButtonEvent(2, 1, LOWORD(lParam), HIWORD(lParam));
            break;
        // keyboard action
        case WM_KEYUP:
			if ((int)wParam == VK_CONTROL)
				ctrlKeyDown = 0;

            triggerKeyboardEvent((char*)&wParam, (int)wParam, 1);
            break;

            // keyboard action
    case WM_KEYDOWN:
			if ((int)wParam == VK_CONTROL)
				ctrlKeyDown = 1;
			else if (ctrlKeyDown && (int)wParam == 'R')
        gemAbortRendering();
			else
				triggerKeyboardEvent((char*)&wParam, (int)wParam, 0);
      break;
      
      // resize event
    case WM_SIZE:
      triggerResizeEvent(LOWORD(lParam), HIWORD(lParam));
      GetClientRect(hWnd, &rcClient);
      break;
      
      // we want to override these messages
      // and not do anything
    case WM_DESTROY:
    case WM_CLOSE:
      break;
    case WM_CREATE:
      {
      }
      break;
      
      // pass all unhandled messages to DefWindowProc
    default:
      lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
      break;
    }
    return(lRet);
}

/////////////////////////////////////////////////////////
// createGemWindow
//
/////////////////////////////////////////////////////////
GEM_EXTERN int createGemWindow(WindowInfo &info, WindowHints &hints)
{
  static int firstTime = 1;
    
  // Register the frame class
  HINSTANCE hInstance = GetModuleHandle(NULL);
  if (!hInstance)
    {
      error("GEM: Unable to get module instance");
      return(0);
    }
  if (firstTime)
    {
      WNDCLASS wndclass;
      wndclass.style         = 0;
      wndclass.lpfnWndProc   = (WNDPROC)MainWndProc;
      wndclass.cbClsExtra    = 0;
      wndclass.cbWndExtra    = 0;
      wndclass.hInstance     = hInstance;
      wndclass.hCursor       = LoadCursor(NULL, IDC_CROSS);
      wndclass.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
      wndclass.hbrBackground = NULL;
      wndclass.lpszMenuName  = NULL;
      wndclass.lpszClassName = "GEM";

      if (!RegisterClass(&wndclass) )
        {
	  error("GEM: Unable to register window class");
	  return(0);
        }
      firstTime = 0;
    }

  DWORD dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
  DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

  hints.real_w = hints.width;
  hints.real_h = hints.height;

  int x = hints.x_offset;
  int y = hints.y_offset;

  bool fullscreen=(hints.fullscreen!=0);
  if (fullscreen){
    DEVMODE dmScreenSettings;								// Device Mode
    
    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings)){
      error("GEM: couldn't get screen capabilities!");
    }
    int w = dmScreenSettings.dmPelsWidth;
    int h = dmScreenSettings.dmPelsHeight;
    
    x=y=0;

    memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
    dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
    dmScreenSettings.dmPelsWidth	= hints.width;			// Selected Screen Width
    dmScreenSettings.dmPelsHeight	= hints.height;			// Selected Screen Height
    dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
    dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
    // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
    if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
      dmScreenSettings.dmPelsWidth	= w;
      dmScreenSettings.dmPelsHeight	= h;
      if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
	error("GEM: couldn't switch to fullscreen");
	fullscreen=false;
      } else {
	hints.real_h=h;
	hints.real_w=w;
      }
    }
  }
  if (fullscreen){
    dwExStyle  = WS_EX_APPWINDOW;
    style     |= WS_POPUP;
  } else {
    dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    if (hints.border)
      style |= WS_OVERLAPPEDWINDOW;
    else
      style |= WS_POPUP;
  }

  info.fs = fullscreen;//hints.fullscreen;

  // Since Windows uses some of the window for the border, etc,
  //		we have to ask how big the window should really be
  RECT newSize;
  newSize.left = x;
  newSize.top = y;
  newSize.right = hints.real_w+x;
  newSize.bottom = hints.real_h+y;

  AdjustWindowRectEx(&newSize, style, FALSE, dwExStyle); // no menu

  if (newSize.left<0 && x>=0){
	  newSize.right-=newSize.left;
	  newSize.left=0;
  }
  if (newSize.top<0 && y>=0){
	  newSize.bottom-=newSize.top;
	  newSize.top=0;
  }

  // Create the window
  info.win = CreateWindowEx (
			     dwExStyle,
			     "GEM",
			     hints.title,
			     style,
			     newSize.left,
			     newSize.top,
			     newSize.right - newSize.left,
			     newSize.bottom - newSize.top,
			     NULL,
			     NULL,
			     hInstance,
			     NULL);

  if (!info.win)  {
      error("GEM: Unable to create window");
      return(0);
    }

  // create the device context
  info.dc = GetDC(info.win);
  if (!info.dc)  {
      error("GEM: Unable to create device context");
      destroyGemWindow(info);
      return(0);
    }

  // set the pixel format for the window
  if (!bSetupPixelFormat(info.dc, hints))  {
      error("GEM: Unable to set window pixel format");
      destroyGemWindow(info);
      return(0);
    }

  // create the OpenGL context
  info.context = wglCreateContext(info.dc);
  if (!info.context)  {
      error("GEM: Unable to create OpenGL context");
      destroyGemWindow(info);
      return(0);
    }

  // do we share display lists?
  if (hints.shared) wglShareLists(hints.shared, info.context);

  // make the context the current rendering context
  if (!wglMakeCurrent(info.dc, info.context))   {
      error("GEM: Unable to make OpenGL context current");
      destroyGemWindow(info);
      return(0);
    }

  if (!hints.actuallyDisplay) return(1);

  // show and update main window
  if (fullscreen){
    ShowWindow(info.win,SW_SHOW);				// Show The Window
    SetForegroundWindow(info.win);				// Slightly Higher Priority
    SetFocus(info.win);
  } else  ShowWindow(info.win, SW_SHOWNORMAL);

  UpdateWindow(info.win);

  return(1);
}

/////////////////////////////////////////////////////////
// destroyGemWindow
//
/////////////////////////////////////////////////////////
GEM_EXTERN void destroyGemWindow(WindowInfo &info)
{
  if (info.fs) ChangeDisplaySettings(NULL,0);	// Switch Back To The Desktop

  if (info.win) {
    if (info.dc) {
      if (info.context) {
	wglDeleteContext(info.context);
      }
      ReleaseDC(info.win, info.dc);
    }
    DestroyWindow(info.win);
  }
  info.dc  = NULL;
  info.win = NULL;
  info.dc  = NULL;
}

/////////////////////////////////////////////////////////
// switch cursor on/off
//
/////////////////////////////////////////////////////////
int cursorGemWindow(WindowInfo &info, int state)
{
  static int cursor_state = 1;
  state=!(!state);
  if (cursor_state != state){
    cursor_state=ShowCursor(state)+1;
  }

  return cursor_state;
}

/////////////////////////////////////////////////////////
// set topmost position on/off
//
/////////////////////////////////////////////////////////
int topmostGemWindow(WindowInfo &info, int state)
{
  static int topmost_state = 0;
  state=!(!state);
  if (state)
	SetWindowPos(info.win, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); 
  else
	SetWindowPos(info.win, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); 
  topmost_state = state;
  return topmost_state;
}


void gemWinSwapBuffers(WindowInfo&nfo)
{
  SwapBuffers(nfo.dc);
}

void gemWinMakeCurrent(WindowInfo&nfo) 
{
  if (!nfo.dc && !nfo.context)return; // do not crash ??
  wglMakeCurrent(nfo.dc, nfo.context); 
}

bool initGemWin(void) {
  return 1;
}


GEM_EXTERN void initWin_sharedContext(WindowInfo &info, WindowHints &hints)
{
  //  myHints.shared = constInfo.context;
  hints.shared = NULL;
}


GEM_EXTERN void dispatchGemWindowMessages(WindowInfo &win)
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
}

#endif /* WIN32 */
