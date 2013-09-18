////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) 2000-2004 Jamie Tittle
//    Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#ifdef __APPLE__
#include "Gem/GemGL.h"
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <AGL/agl.h>

#include "gemmacwindow.h"
#include "Gem/Exception.h"
#include "RTE/MessageCallbacks.h"

#define DEBUGPOST ::startpost("%s:%d[%s] ", __FILE__, __LINE__, __FUNCTION__ ), ::post

#define PIXEL_SIZE	32		// 16 or 32
#define DEPTH_SIZE	16

struct structGLInfo // storage for setup info
{
  SInt16 width;		// input: width of drawable (screen width in full screen mode), return: actual width allocated
  SInt16 height;		// input: height of drawable (screen height in full screen mode), return: actual height allocated
  UInt32 pixelDepth;		// input: requested pixel depth
  Boolean fDepthMust;		// input: pixel depth must be set (if false then current depth will be used if able)
  Boolean fFullscreen;	// input: use DSp to get fullscreen? (or find full screen renderer)
  // if fFullscreen, will search for full screen renderers first then use DSp for others
  //  unless a device is specified, in which case we will try there first
  Boolean fAcceleratedMust; 	// input: must renderer be accelerated?
  GLint aglAttributes[64]; 	// input: pixel format attributes always required (reset to what was actually allocated)
  SInt32 VRAM;		// input: minimum VRAM; output: actual (if successful otherwise input)
  SInt32 textureRAM;		// input: amount of texture RAM required on card; output: same (used in allcoation to ensure enough texture
  AGLPixelFormat	fmt;	// input: none; output pixel format...
  SInt32 freq;		// input: frequency request for display; output: actual
};
typedef struct structGLInfo structGLInfo;
typedef struct structGLInfo * pstructGLInfo;

// structure for creating a context from a window
struct structGLWindowInfo // storage for setup info
{
  Boolean fAcceleratedMust; 	// input: must renderer be accelerated?
  GLint aglAttributes[64]; 	// input: pixel format attributes always required (reset to what was actually allocated)
  SInt32 VRAM;		// input: minimum VRAM; output: actual (if successful otherwise input)
  SInt32 textureRAM;		// input: amount of texture RAM required on card; output: same (used in allcoation to ensure enough texture
  AGLPixelFormat	fmt;	// input: none; output pixel format...
  Boolean fDraggable;		// input: is window going to be dragable, 
  //        if so renderer check (accel, VRAM, textureRAM) will look at all renderers vice just the current one
  //        if window is not dragable renderer check will either check the single device or short 
  //            circuit to software if window spans multiple devices 
  // software renderer is consider to have unlimited VRAM, unlimited textureRAM and to not be accelerated
};
typedef struct structGLWindowInfo structGLWindowInfo;
typedef struct structGLWindowInfo * pstructGLWindowInfo;

// the master context associated with no window and shared by all other contexts
AGLContext masterContext = NULL;

// globals (internal/private) -----------------------------------------------

const RGBColor		rgbBlack = { 0x0000, 0x0000, 0x0000 };

const short kWindowType = kWindowDocumentProc;

// GL stuff
Boolean gfHasPackedPixels = false;
structGLWindowInfo	glWInfo;
AGLDrawable 		gaglDraw = NULL;
AGLContext			gaglContext = 0;
GLuint				gFontList = 0;
char				gInfoString [512] = "";

EventHandlerUPP		gEvtHandler;
EventHandlerRef		ref;
ProcessSerialNumber	psn = {0, kCurrentProcess };
static pascal OSStatus evtHandler(EventHandlerCallRef myHandler, EventRef event, void* userData);

AGLContext SetupAGLFullScreen (GDHandle display, short * pWidth, short * pHeight);

// Prototypes
// -------------------------------------------------------------------------------------------------------------
// Takes device # and geometry request and tries to build best context and drawable
// 	If requested device does not work, will start at first device and walk down devices 
//	 looking for first one that satisfies requirments
//  Devices are numbered in order that DMGetFirstScreenDevice/DMGetNextScreenDevice returns, 
//	 fullscreen devices are numbered after this, but they will be searched first if fFullscreen == true,
//	 they will not be searched in the non-fullscreen case

// Inputs: 	*pnumDevice: -1: main device, 0: any device, other #: attempt that device first, then any device
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglDraw, *paglContext and *pdspContext as allocated
//			*pnumDevice to device number in list that was used 
//			*pcontextInfo:  allocated parameters

// If fail to build context: paglDraw, paglContext and pdspContext will be NULL
// If fatal error: will return error and paglDraw, paglContext and pdspContext will be NULL
// Note: Errors can be generated internally when a specific device fails, this is normal and these
//		  will not be returned is a subsequent device succeeds

OSStatus BuildGL (AGLDrawable* paglDraw, AGLContext* paglContext,
                  short* pnumDevice, pstructGLInfo pcontextInfo, AGLContext aglShareContext);
                                  
static OSStatus BuildGLonDevice (AGLDrawable* paglDraw, AGLContext* paglContext, 
                                 GDHandle hGD, pstructGLInfo pcontextInfo, AGLContext aglShareContext);

static OSStatus BuildDrawable (AGLDrawable* paglDraw, GDHandle hGD, pstructGLInfo pcontextInfo);

static OSStatus BuildGLContext (AGLDrawable* paglDraw, AGLContext* paglContext, GDHandle hGD, 
                                pstructGLInfo pcontextInfo, AGLContext aglShareContext);
static void DumpCurrent (AGLDrawable* paglDraw, AGLContext* paglContext, pstructGLInfo pcontextInfo);
static Boolean CheckRenderer (GDHandle hGD, long *VRAM, long *textureRAM, GLint* pDepthSizeSupport, Boolean fAccelMust);
static Boolean CheckAllDeviceRenderers (long* pVRAM, long* pTextureRAM, GLint* pDepthSizeSupport, Boolean fAccelMust);
static Boolean CheckWindowExtents (GDHandle hGD, short width, short height);

// Destroys drawable and context
// Ouputs: *paglDraw, *paglContext and *pdspContext should be 0 on exit

OSStatus DestroyGL (AGLDrawable* paglDraw, AGLContext* paglContext, pstructGLInfo pcontextInfo);

// same as above except that it takes a window as input and attempts to build requested conext on that
OSStatus BuildGLFromWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo, AGLContext aglShareContext);

// same as above but destorys a context that was associated with an existing window, window is left intacted
OSStatus DestroyGLFromWindow (AGLContext* paglContext, pstructGLWindowInfo pcontextInfo);

// Handle reporting of agl errors, error code is passed through
OSStatus aglReportError (void);

// Runtime check to see if we are running on Mac OS X
// Inputs:  None
// Returns: 0 if < Mac OS X or version number of Mac OS X (10.0 for GM)
UInt32 CheckMacOSX (void);

short FindGDHandleFromRect (Rect * pRect, GDHandle * phgdOnThisDevice);

short FindGDHandleFromWindow (WindowPtr pWindow, GDHandle * phgdOnThisDevice);



/////////////////////////////////////////////////////////

GEM_EXTERN void gemAbortRendering();

#pragma mark -----functions-----
//-----------------------------------------------------------------------------------------------------------------------
// SetupAGLFullScreen, for 2nd monitor
//-----------------------------------------------------------------------------------------------------------------------
AGLContext SetupAGLFullScreen (GDHandle display, short * pWidth, short * pHeight)
{
  GLint		attrib[64];
  AGLPixelFormat 	fmt;
  AGLContext     	ctx;
	
  // different possible pixel format choices for different renderers 
  // basics requirements are RGBA and double buffer
  // OpenGL will select acclerated context if available

  short i = 0;
  attrib [i++] = AGL_RGBA; // red green blue and alpha
  attrib [i++] = AGL_DOUBLEBUFFER; // double buffered
  attrib [i++] = AGL_ACCELERATED; // HWA pixel format only
  attrib [i++] = AGL_FULLSCREEN;
  attrib [i++] = AGL_PIXEL_SIZE;
  attrib [i++] = 32;
  attrib [i++] = AGL_DEPTH_SIZE;
  attrib [i++] = 16;
  attrib [i++] = AGL_NONE;	

  fmt = aglChoosePixelFormat(&display, 1, attrib); // this may fail if looking for acclerated across multiple monitors
  if (NULL == fmt) 
    {
      error("Could not find valid pixel format");
      return NULL;
    }

  ctx = aglCreateContext (fmt, NULL); // Create an AGL context
  if (NULL == ctx)
    {
      error ("Could not create context");
      return NULL;
    }
        
  ::aglEnable( ctx, AGL_FS_CAPTURE_SINGLE );

  //if (!aglSetFullScreen (ctx, *pWidth, *pHeight, 60, 0))
  if (!aglSetFullScreen (ctx, *pWidth, *pHeight, 0, 0))
    {
      error ("SetFullScreen failed");
      return NULL;
    }

  if (!aglSetCurrentContext (ctx)) // make the context the current context
    {
      error ("SetCurrentContext failed");
      aglSetDrawable (ctx, NULL); // turn off full screen
      return NULL;
    }

  aglDestroyPixelFormat(fmt); // pixel format is no longer needed

  return ctx;
}

//-----------------------------------------------------------------------------------------------------------------------

// BuildGLFromWindow

// Takes window in the form of an AGLDrawable and geometry request and tries to build best context

// Inputs: 	aglDraw: a valid AGLDrawable (i.e., a WindowPtr)
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglContext as allocated
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglContext will be NULL
// if error: will return error and paglContext will be NULL
/*
  OSStatus BuildGLFromWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo, AGLContext aglShareContext)
  {
  if (!pWindow){
  return paramErr;
  }
  return BuildGLonWindow (pWindow, paglContext, pcontextInfo, aglShareContext);
  }
*/
// --------------------------------------------------------------------------

// BuildGLonDrawable

// Takes a drawable and tries to build on it

// Inputs: 	aglDraw: a valid AGLDrawable
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglContext as allocated
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglContext will be NULL
// if error: will return error and paglContext will be NULL
//static
OSStatus BuildGLFromWindow (WindowPtr pWindow, AGLContext* paglContext, pstructGLWindowInfo pcontextInfo, AGLContext aglShareContext)
{
  GDHandle hGD = NULL;
  GrafPtr cgrafSave = NULL;
  short numDevices;
  GLint depthSizeSupport;
  OSStatus err = noErr;
  logpost(NULL, 6,"MAC: BuildGLonWindow entered");
  if (!pWindow || !pcontextInfo)
    {
      logpost(NULL, 6,"MAC: BuildGLonWindow: no drawable");
      return paramErr;
    }
	
  GetPort (&cgrafSave);
  SetPortWindowPort(pWindow);

  // check renderer VRAM and acceleration
  numDevices = FindGDHandleFromWindow (pWindow, &hGD);
  if (!pcontextInfo->fDraggable) 	// if numDevices > 1 then we will only be using the software renderer otherwise check only window device
    {
      logpost(NULL, 6,"MAC: BuildGLonWindow: fDraggable= false");
      if ((numDevices > 1) || (numDevices == 0)) // this window spans mulitple devices thus will be software only
        {
          logpost(NULL, 6,"MAC: BuildGLonWindow: numDevices>1 || numDevices ==0");
          // software renderer
          // infinite VRAM, infinite textureRAM, not accelerated
          if (pcontextInfo->fAcceleratedMust)
            {
              logpost(NULL, 6,"MAC: BuildGLonWindow: trying to accelerate window that spans multiple devices");
              //   return err;


            }
        } 
      else // not draggable on single device
        {
          logpost(NULL, 6,"MAC: BuildGLonWindow: not draggable on single device");
          if (!CheckRenderer (hGD, &(pcontextInfo->VRAM), &(pcontextInfo->textureRAM), &depthSizeSupport, pcontextInfo->fAcceleratedMust))
            {
              logpost(NULL, 6,"MAC: BuildGLonWindow: Renderer check failed 1");
              return err;
            }
        }
    }
  // else draggable so must check all for support (each device should have at least one renderer that meets the requirements)
  else if (!CheckAllDeviceRenderers (&(pcontextInfo->VRAM), &(pcontextInfo->textureRAM), &depthSizeSupport, pcontextInfo->fAcceleratedMust))
    {
      logpost(NULL, 6,"MAC: BuildGLonWindow: Renderer check failed 2");
      return err;
    }
	
  // do agl
  if (reinterpret_cast<Ptr>(kUnresolvedCFragSymbolAddress) == reinterpret_cast<Ptr>(aglChoosePixelFormat)) // check for existance of OpenGL
    {
      logpost(NULL, 6,"MAC: BuildGLonWindow: OpenGL not installed");
      return NULL;
    }	
  // we successfully passed the renderer check

  if ((!pcontextInfo->fDraggable && (numDevices == 1))){  // not draggable on a single device
    pcontextInfo->fmt = aglChoosePixelFormat (&hGD, 1, pcontextInfo->aglAttributes); // get an appropriate pixel format
    logpost(NULL, 6,"MAC: BuildGLonWindow (!pcontextInfo->fDraggable && (numDevices == 1))");
  } else {
    pcontextInfo->fmt = aglChoosePixelFormat (NULL, 0, pcontextInfo->aglAttributes); // get an appropriate pixel format
    logpost(NULL, 6,"MAC: BuildGLonWindow else");
  }
  aglReportError ();
  if (NULL == pcontextInfo->fmt) 
    {
      logpost(NULL, 6,"MAC: BuildGLonWindow: Could not find valid pixel format");
      return NULL;
    }

  *paglContext = aglCreateContext (pcontextInfo->fmt, aglShareContext); // Create an AGL context
         
  if (AGL_BAD_MATCH == aglGetError()){
    logpost(NULL, 6,"MAC: BuildGLonWindow: AGL_BAD_MATCH");
    *paglContext = aglCreateContext (pcontextInfo->fmt, 0); // unable to share context, create without sharing
  }
  aglReportError ();
  if (NULL == *paglContext) 
    {
      logpost(NULL, 6,"MAC: BuildGLonWindow: Unable to create AGL context");
      return NULL;
    }
	
  if (!aglSetDrawable (*paglContext, GetWindowPort (pWindow))){ // attach the CGrafPtr to the context
    error("MAC: BuildGLonWindow: Unable to attach the CGrafPtr to the context");
    return aglReportError ();
  }
	
  if(!aglSetCurrentContext (*paglContext)){ // make the context the current context
    error("MAC: BuildGLonWindow: Unable to make the context the current context");
    return aglReportError ();
  }
  logpost(NULL, 6,"MAC: BuildGLonWindow exit");
  return err;
}

// --------------------------------------------------------------------------

// BuildGL

// Takes device and geometry request and tries to build best context and drawable
// if device does not work will walk down devices looking for first one that satisfies requirments

// Inputs: 	*pnumDevice: 0 any device, # attempt that device first, then any device
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglDraw, *paglContext and *pdspContext as allocated
//			*pnumDevice to device number in list that was used 
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglDraw, paglContext and pdspContext will be NULL
// if error: will return error and paglDraw, paglContext and pdspContext will be NULL

OSStatus BuildGL (AGLDrawable* paglDraw, AGLContext* paglContext,
                  short* pnumDevice, pstructGLInfo pcontextInfo, AGLContext aglShareContext)
{
  OSStatus err = noErr;
  GDHandle hGD = NULL;
  structGLInfo contextInfoSave;
	
  // clear
  *paglDraw = NULL;
  *paglContext = 0;
  contextInfoSave = *pcontextInfo; // save info to reset on failures
	
  //find main device
  if (*pnumDevice == -1)
    {
      GDHandle hDevice; // check number of screens
      hGD = GetMainDevice ();
      if (NULL != hGD)
        {
          err = BuildGLonDevice (paglDraw, paglContext, hGD, pcontextInfo, aglShareContext);
          // find device number
          *pnumDevice = 0;
          hDevice = DMGetFirstScreenDevice (true);
          do
            {
              if (hDevice == hGD)
                break;
              hDevice = DMGetNextScreenDevice (hDevice, true);
              (*pnumDevice)++;
            }
          while (hDevice);
          if (!hDevice)
            error("WinCreateMac: main device match not found");
        }
    }

  if ((err != noErr) || (*paglContext == 0))
    {
      err = noErr;
      DumpCurrent (paglDraw, paglContext, pcontextInfo); // dump what ever partial solution we might have
      *pcontextInfo = contextInfoSave; // restore info
      //find target device and check this first is one exists
      if (*pnumDevice)
        {
          short i;
          hGD = DMGetFirstScreenDevice (true);
          for (i = 0; i < *pnumDevice; i++)
            {
              GDHandle hGDNext = DMGetNextScreenDevice (hGD, true);
              if (NULL == hGDNext) // ensure we did not run out of devices
                break; // if no more devices drop out
              else
                hGD = hGDNext; // otherwise continue
            }
          *pnumDevice = i; // record device we actually got
          err = BuildGLonDevice (paglDraw, paglContext, /*pdspContext,*/ hGD, pcontextInfo, aglShareContext);
        }
    }
	
  // while we have not allocated a context or there were errors
  if ((err != noErr) || (*paglContext == 0))
    {
      err = noErr;
      DumpCurrent (paglDraw, paglContext, pcontextInfo); // dump what ever partial solution we might have
      *pcontextInfo = contextInfoSave; // restore info
      // now look through the devices in order
      hGD = DMGetFirstScreenDevice (true);	
      *pnumDevice = -1;
      do 
        {
          (*pnumDevice)++;
          err = BuildGLonDevice (paglDraw, paglContext, hGD, pcontextInfo, aglShareContext);
          if ((err != noErr) || (*paglDraw == NULL) || (*paglContext == 0))	// reset hGD only if we are not done
            {
              hGD = DMGetNextScreenDevice (hGD, true);
              DumpCurrent (paglDraw, paglContext, pcontextInfo); // dump what ever partial solution we might have
              *pcontextInfo = contextInfoSave; // restore info
            }
        }
      while (((err != noErr) || (*paglContext == 0)) && hGD);
    }
  return err;
}

// --------------------------------------------------------------------------

// DestroyGL

// Destroys drawable and context
// Ouputs: *paglDraw, *paglContext and *pdspContext should be 0 on exit

OSStatus DestroyGL (AGLDrawable* paglDraw, AGLContext* paglContext, pstructGLInfo pcontextInfo)
{
  if ((!paglContext) || (!*paglContext))
    return paramErr; // not a valid context
  glFinish ();
  DumpCurrent (paglDraw, paglContext, pcontextInfo);
  return noErr;
}

// --------------------------------------------------------------------------

// DestroyGLFromWindow

// Destroys context that waas allocated with BuildGLFromWindow
// Ouputs: *paglContext should be NULL on exit

OSStatus DestroyGLFromWindow (AGLContext* paglContext, pstructGLWindowInfo pcontextInfo)
{
  OSStatus err;
	
  if ((!paglContext) || (!*paglContext))
    return paramErr; // not a valid context
  glFinish ();
  aglSetCurrentContext (NULL);
  err = aglReportError ();
  aglSetDrawable (*paglContext, NULL);
  err = aglReportError ();
  aglDestroyContext (*paglContext);
  err = aglReportError ();
  *paglContext = NULL;

  if (pcontextInfo->fmt)
    {
      aglDestroyPixelFormat (pcontextInfo->fmt); // pixel format is no longer valid
      err = aglReportError ();
    }
  pcontextInfo->fmt = 0;
	
  return err;
}

//---------------------------------------------------------------------------
// BuildGLonDevice

// Takes device single device and tries to build on it

// Inputs: 	hGD: GDHandle to device to look at
//			*pcontextInfo: request and requirements for cotext and drawable

// Outputs: *paglDraw, *paglContext and *pdspContext as allocated
//			*pcontextInfo:  allocated parameters

// if fail to allocate: paglDraw, paglContext and pdspContext will be NULL
// if error: will return error and paglDraw, paglContext and pdspContext will be NULL
// Note: *paglDraw and *pdspContext can be null is aglFullScreen is used

static OSStatus BuildGLonDevice (AGLDrawable* paglDraw, AGLContext* paglContext, 
                                 GDHandle hGD, pstructGLInfo pcontextInfo, AGLContext aglShareContext)
{
  GLint depthSizeSupport;
  OSStatus err = noErr;
  Boolean fCheckRenderer = false;
  logpost(NULL, 5, "WinCreateMac:BuildGLonDevice: no fullscreen");
  {
    if (pcontextInfo->pixelDepth == 0)	// default
      {
        pcontextInfo->pixelDepth = (**(**hGD).gdPMap).pixelSize;
        if (16 > pcontextInfo->pixelDepth)
          pcontextInfo->pixelDepth = 16;
      }
    if (pcontextInfo->fDepthMust && (static_cast<int>(pcontextInfo->pixelDepth) != (**(**hGD).gdPMap).pixelSize))	// device depth must match and does not
      {
        error("Pixel Depth does not match device in windowed mode.");
        return err;
      }
    // copy back the curretn depth
    pcontextInfo->pixelDepth = (**(**hGD).gdPMap).pixelSize;
    if (!CheckWindowExtents (hGD, pcontextInfo->width, pcontextInfo->height))
      {
        error("Window will not fit on device in windowed mode.");
        return err;
      }
  }
	
  // if we have not already checked the renderer, check for VRAM and accelerated
  if (!fCheckRenderer)
    if (!CheckRenderer (hGD, &(pcontextInfo->VRAM), &(pcontextInfo->textureRAM), &depthSizeSupport, pcontextInfo->fAcceleratedMust))
      {
        error("Renderer check failed");
        return err;
      }
	
  // do agl
  // need to send device #'s through this
  err = BuildGLContext (paglDraw, paglContext, hGD, pcontextInfo, aglShareContext);

  return err;
}
// --------------------------------------------------------------------------

// DumpCurrent

// Kills currently allocated context
// does not care about being pretty (assumes display is likely faded)

// Inputs: 	paglDraw, paglContext, pdspContext: things to be destroyed

void DumpCurrent (AGLDrawable* paglDraw, AGLContext* paglContext, pstructGLInfo pcontextInfo)
{
  if (*paglContext)
    {
      aglSetCurrentContext (NULL);
      aglReportError ();
      aglSetDrawable (*paglContext, NULL);
      aglReportError ();
      aglDestroyContext (*paglContext);
      aglReportError ();
      *paglContext = NULL;
    }
	
  if (pcontextInfo->fmt)
    {
      aglDestroyPixelFormat (pcontextInfo->fmt); // pixel format is no longer needed
      aglReportError ();
    }
  pcontextInfo->fmt = 0;

  if (*paglDraw && !(pcontextInfo->fFullscreen && CheckMacOSX ())) // do not destory a window on DSp if in Mac OS X
    // since there is no window built in X
    DisposeWindow (GetWindowFromPort (*paglDraw));

  *paglDraw = NULL;
}
// CheckRenderer

// looks at renderer attributes it has at least the VRAM is accelerated

// Inputs: 	hGD: GDHandle to device to look at
//			pVRAM: pointer to VRAM in bytes required; out is actual VRAM if a renderer was found, otherwise it is the input parameter
//			pTextureRAM:  pointer to texture RAM in bytes required; out is same (implementation assume VRAM returned by card is total so we add texture and VRAM)
//			fAccelMust: do we check for acceleration

// Returns: true if renderer for the requested device complies, false otherwise

static Boolean CheckRenderer (GDHandle hGD, long* pVRAM, long* pTextureRAM, GLint* pDepthSizeSupport, Boolean fAccelMust)
{
  AGLRendererInfo info, head_info;
  GLint inum;
  GLint dAccel = 0;
  GLint dVRAM = 0, dMaxVRAM = 0;
  Boolean canAccel = false, found = false;
  head_info = aglQueryRendererInfo(&hGD, 1);
  aglReportError ();
  if(!head_info)
    {
      error("aglQueryRendererInfo error");
      return false;
    }
  else
    {
      info = head_info;
      inum = 0;
      // see if we have an accelerated renderer, if so ignore non-accelerated ones
      // this prevents returning info on software renderer when actually we'll get the hardware one
      while (info)
        {	
          aglDescribeRenderer(info, AGL_ACCELERATED, &dAccel);
          aglReportError ();
          if (dAccel)
            canAccel = true;
          info = aglNextRendererInfo(info);
          aglReportError ();
          inum++;
        }
			
      info = head_info;
      inum = 0;
      while (info)
        {
          aglDescribeRenderer (info, AGL_ACCELERATED, &dAccel);
          aglReportError ();
          // if we can accel then we will choose the accelerated renderer 
          // how about compliant renderers???
          if ((canAccel && dAccel) || (!canAccel && (!fAccelMust || dAccel)))
            {
              aglDescribeRenderer (info, AGL_VIDEO_MEMORY, &dVRAM);	// we assume that VRAM returned is total thus add texture and VRAM required
              aglReportError ();
              if (dVRAM >= (*pVRAM + *pTextureRAM))
                {
                  if (dVRAM >= dMaxVRAM) // find card with max VRAM
                    {
                      aglDescribeRenderer (info, AGL_DEPTH_MODES, pDepthSizeSupport);	// which depth buffer modes are supported
                      aglReportError ();
                      dMaxVRAM = dVRAM; // store max
                      found = true;
                    }
                }
            }
          info = aglNextRendererInfo(info);
          aglReportError ();
          inum++;
        }
    }
  aglDestroyRendererInfo(head_info);
  if (found) // if we found a card that has enough VRAM and meets the accel criteria
    {
      *pVRAM = dMaxVRAM; // return VRAM
      return true;
    }
  // VRAM will remain to same as it did when sent in
  return false;
}

//-----------------------------------------------------------------------------------------------------------------------

// CheckAllDeviceRenderers 

// looks at renderer attributes and each device must have at least one renderer that fits the profile

// Inputs: 	pVRAM: pointer to VRAM in bytes required; out is actual min VRAM of all renderers found, otherwise it is the input parameter
//			pTextureRAM:  pointer to texture RAM in bytes required; out is same (implementation assume VRAM returned by card is total so we add texture and VRAM)
//			fAccelMust: do we check fro acceleration

// Returns: true if any renderer for on each device complies (not necessarily the same renderer), false otherwise

static Boolean CheckAllDeviceRenderers (long* pVRAM, long* pTextureRAM, GLint* pDepthSizeSupport, Boolean fAccelMust)
{
  AGLRendererInfo info, head_info;
  GLint inum;
  GLint dAccel = 0;
  GLint dVRAM = 0, dMaxVRAM = 0;
  Boolean canAccel = false, found = false, goodCheck = true; // can the renderer accelerate, did we find a valid renderer for the device, are we still successfully on all the devices looked at
  long MinVRAM = 0x8FFFFFFF; // max long
  GDHandle hGD = GetDeviceList (); // get the first screen
  while (hGD && goodCheck)
    {
      head_info = aglQueryRendererInfo(&hGD, 1);
      aglReportError ();
      if(!head_info)
        {
          error("aglQueryRendererInfo error");
          return false;
        }
      else
        {
          info = head_info;
          inum = 0;
          // see if we have an accelerated renderer, if so ignore non-accelerated ones
          // this prevents returning info on software renderer when actually we'll get the hardware one
          while (info)
            {
              aglDescribeRenderer(info, AGL_ACCELERATED, &dAccel);
              aglReportError ();
              if (dAccel)
                canAccel = true;
              info = aglNextRendererInfo(info);
              aglReportError ();
              inum++;
            }
                            
          info = head_info;
          inum = 0;
          while (info)
            {	
              aglDescribeRenderer(info, AGL_ACCELERATED, &dAccel);
              aglReportError ();
              // if we can accel then we will choose the accelerated renderer 
              // how about compliant renderers???
              if ((canAccel && dAccel) || (!canAccel && (!fAccelMust || dAccel)))
                {
                  aglDescribeRenderer(info, AGL_VIDEO_MEMORY, &dVRAM);	// we assume that VRAM returned is total thus add texture and VRAM required
                  aglReportError ();
                  if (dVRAM >= (*pVRAM + *pTextureRAM))
                    {
                      if (dVRAM >= dMaxVRAM) // find card with max VRAM
                        {
                          aglDescribeRenderer(info, AGL_DEPTH_MODES, pDepthSizeSupport);	// which depth buffer modes are supported
                          aglReportError ();
                          dMaxVRAM = dVRAM; // store max
                          found = true;
                        }
                    }
                }
              info = aglNextRendererInfo(info);
              aglReportError ();
              inum++;
            }
        }
      aglDestroyRendererInfo(head_info);
      if (found) // if we found a card that has enough VRAM and meets the accel criteria
        {
          if (MinVRAM > dMaxVRAM)
            MinVRAM = dMaxVRAM; // return VRAM
        }
      else
        goodCheck = false; // one device failed thus entire requirement fails
      hGD = GetNextDevice (hGD); // get next device
    } // while
  if (goodCheck) // we check all devices and each was good
    {
      *pVRAM = MinVRAM; // return VRAM
      return true;
    }
  return false; //at least one device failed to have mins
}
//-----------------------------------------------------------------------------------------------------------------------

// CheckWindowExtents

// checks to see window fits on screen completely

// Inputs: 	hGD: GDHandle to device to look at
//			width/height: requested width and height of window

// Returns: true if window and borders fit, false otherwise

static Boolean CheckWindowExtents (GDHandle hGD, short width, short height)
{
  Rect strucRect, rectWin = {0, 0, 1, 1};
  short deviceHeight = static_cast<short> ((**hGD).gdRect.bottom - (**hGD).gdRect.top - GetMBarHeight ());	
  short deviceWidth = static_cast<short> ((**hGD).gdRect.right - (**hGD).gdRect.left);
  short windowWidthExtra, windowHeightExtra;
  // build window (not visible)
  // FIXXME weird cast to "\p"
  WindowPtr pWindow = NewCWindow (NULL, &rectWin, (const unsigned char*)"\p", true, kWindowType, reinterpret_cast<WindowPtr>(-1), 0, 0);
	
  GetWindowBounds (pWindow, kWindowStructureRgn, &strucRect);

  windowWidthExtra = static_cast<short> ((strucRect.right - strucRect.left) - 1);
  windowHeightExtra = static_cast<short> ((strucRect.bottom - strucRect.top) - 1);
  DisposeWindow (pWindow);
  if ((width + windowWidthExtra <= deviceWidth) &&
      (height + windowHeightExtra <= deviceHeight))
    return true;
  return false;
}
// --------------------------------------------------------------------------

// BuildGLContext

// Builds OpenGL context

// Inputs: 	hGD: GDHandle to device to look at
//			pcontextInfo: request and requirements for context and drawable

// Outputs: paglContext as allocated
//			pcontextInfo:  allocated parameters

// if fail to allocate: paglContext will be NULL
// if error: will return error paglContext will be NULL

static OSStatus BuildGLContext (AGLDrawable* paglDraw, AGLContext* paglContext,
                                GDHandle hGD, pstructGLInfo pcontextInfo, AGLContext aglShareContext)
{
  OSStatus err = noErr;

  if (reinterpret_cast<Ptr>(kUnresolvedCFragSymbolAddress) == reinterpret_cast<Ptr>(aglChoosePixelFormat)) // check for existance of OpenGL
    {
      error("OpenGL not installed");
      return noErr;
    }	
	
  // DSp has problems on Mac OS X with DSp version less than 1.99 so use agl full screen
  if ((pcontextInfo->fFullscreen) && (CheckMacOSX ()) ) // need to set pixel format for full screen
    {
      short i = 0;
      while (pcontextInfo->aglAttributes[i++] != AGL_NONE) {}
      i--; // point to AGL_NONE
      pcontextInfo->aglAttributes [i++] = AGL_FULLSCREEN;
      pcontextInfo->aglAttributes [i++] = AGL_PIXEL_SIZE;
      pcontextInfo->aglAttributes [i++] = static_cast<SInt32>(pcontextInfo->pixelDepth);
      pcontextInfo->aglAttributes [i++] = AGL_NONE;
    }

  pcontextInfo->fmt = aglChoosePixelFormat (&hGD, 1, pcontextInfo->aglAttributes); // get an appropriate pixel format
  aglReportError ();
  if (NULL == pcontextInfo->fmt) 
    {
      error("Could not find valid pixel format");
      return noErr;
    }

  // using a default method of sharing all the contexts enables texture sharing across these contexts by default
  *paglContext = aglCreateContext (pcontextInfo->fmt, aglShareContext);		
  // Create an AGL context
  if (AGL_BAD_MATCH == aglGetError())
    *paglContext = aglCreateContext (pcontextInfo->fmt, 0); // unable to share context, create without sharing
  aglReportError ();
  if (NULL == *paglContext) 
    {
      error("Could not create context");
      return paramErr;
    }
  if (aglShareContext == NULL)
    aglShareContext = *paglContext;
	
  // set our drawable
  // not Mac OS X fullscreen:  this is for three cases 1) Mac OS 9 windowed 2) Mac OS X windowed 3) Mac OS 9 fullscreen (as you need to build a window on top of DSp for GL to work correctly
  {
    // build window as late as possible
    err = BuildDrawable (paglDraw, hGD, pcontextInfo);
    if (err != noErr)
      {
        error("Could not build drawable");
        return err;
      }
    if (!aglSetDrawable (*paglContext, *paglDraw))		// attach the CGrafPtr to the context
      return aglReportError ();
  }
  if(!aglSetCurrentContext (*paglContext))			// make the context the current context
    return aglReportError ();
		
  // set swap interval to sync with vbl
  /*
    GLint	swapinterval = 1;
    if (!aglSetInteger(*paglContext, AGL_SWAP_INTERVAL, &swapinterval))
    return aglReportError();
  */
  return err;
}
/////////////////////////////////////////////////////////////////////////////
// CheckMacOSX

// Runtime check to see if we are running on Mac OS X

// Inputs:  None

// Returns: 0 if < Mac OS X or version number of Mac OS X (10.0 for GM)

UInt32 CheckMacOSX (void)
{
  UInt32 response;
    
  if ((Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x01000))
    return response;
  else
    return 0;
}
// --------------------------------------------------------------------------

// GetWindowDevice

// Inputs:	a valid WindowPtr

// Outputs:	the GDHandle that that window is mostly on

// returns the number of devices that the windows content touches

short FindGDHandleFromWindow (WindowPtr pWindow, GDHandle * phgdOnThisDevice)
{
  GrafPtr pgpSave;
  Rect rectWind, rectSect;
  long greatestArea, sectArea;
  short numDevices = 0;
  GDHandle hgdNthDevice;
	
  if (!pWindow || !phgdOnThisDevice)
    return NULL;
		
  *phgdOnThisDevice = NULL;
	
  GetPort (&pgpSave);
  SetPortWindowPort (pWindow);

  GetWindowPortBounds (pWindow, &rectWind);

  LocalToGlobal (reinterpret_cast<Point*>(& rectWind.top));	// convert to global coordinates
  LocalToGlobal (reinterpret_cast<Point*>(& rectWind.bottom));
  hgdNthDevice = GetDeviceList ();
  greatestArea = 0;
  // check window against all gdRects in gDevice list and remember 
  //  which gdRect contains largest area of window}
  while (hgdNthDevice)
    {
      if (TestDeviceAttribute (hgdNthDevice, screenDevice))
        if (TestDeviceAttribute (hgdNthDevice, screenActive))
          {
            // The SectRect routine calculates the intersection 
            //  of the window rectangle and this gDevice 
            //  rectangle and returns TRUE if the rectangles intersect, 
            //  FALSE if they don't.
            SectRect (&rectWind, &(**hgdNthDevice).gdRect, &rectSect);
            // determine which screen holds greatest window area
            //  first, calculate area of rectangle on current device
            sectArea = static_cast<long> (rectSect.right - rectSect.left) * (rectSect.bottom - rectSect.top);
            if (sectArea > 0)
              numDevices++;
            if (sectArea > greatestArea)
              {
                greatestArea = sectArea; // set greatest area so far
                *phgdOnThisDevice = hgdNthDevice; // set zoom device
              }
            hgdNthDevice = GetNextDevice(hgdNthDevice);
          }
    }
	
  SetPort (pgpSave);
  return numDevices;
}
// --------------------------------------------------------------------------

// BuildDrawable

// Builds window to be used as drawable

// Inputs: 	hGD: GDHandle to device to look at
//			pcontextInfo: request and requirements for cotext and drawable

// Outputs: paglDraw as allocated
//			pcontextInfo:  allocated parameters

// if fail to allocate: paglDraw will be NULL
// if error: will return error paglDraw will be NULL

static OSStatus BuildDrawable (AGLDrawable* paglDraw, GDHandle hGD, pstructGLInfo pcontextInfo)
{
  Rect rectWin;
  RGBColor rgbSave;
  GrafPtr pGrafSave;
  OSStatus err = noErr;
	
  // center window in our context's gdevice
  rectWin.top  = static_cast<short> ((**hGD).gdRect.top + ((**hGD).gdRect.bottom - (**hGD).gdRect.top) / 2); // v center
  rectWin.top  -= pcontextInfo->height / 2;
  rectWin.left  = static_cast<short> ((**hGD).gdRect.left + ((**hGD).gdRect.right - (**hGD).gdRect.left) / 2);	// h center
  rectWin.left  -= pcontextInfo->width / 2;
  rectWin.right = static_cast<short> (rectWin.left + pcontextInfo->width);
  rectWin.bottom = static_cast<short> (rectWin.top + pcontextInfo->height);
	
  if (pcontextInfo->fFullscreen)
    // FIXXME weird cast to "\p"
    *paglDraw = GetWindowPort (NewCWindow (NULL, &rectWin, (const unsigned char*)"\p", 0, plainDBox,   reinterpret_cast<WindowPtr>(-1), 0, 0));
  else
    *paglDraw = GetWindowPort (NewCWindow (NULL, &rectWin, (const unsigned char*)"\p", 0, kWindowType, reinterpret_cast<WindowPtr>(-1), 0, 0));
  ShowWindow (GetWindowFromPort (*paglDraw));

  GetPort (&pGrafSave);
  SetPort (static_cast<GrafPtr>(*paglDraw));
  GetForeColor (&rgbSave);
  RGBForeColor (&rgbBlack);

  GetWindowBounds (GetWindowFromPort (*paglDraw), kWindowContentRgn, &rectWin);

  PaintRect (&rectWin);
  RGBForeColor (&rgbSave); // ensure color is reset for proper blitting
  SetPort (pGrafSave);
  return err;
}
//---------------------------------------------------------------------------

// if error dump agl errors to debugger string, return error

OSStatus aglReportError (void)
{
  GLenum err = aglGetError();
  if (AGL_NO_ERROR != err)
    error((char *)aglErrorString(err));
  // ensure we are returning an OSStatus noErr if no error condition
  if (err == AGL_NO_ERROR)
    return noErr;
  else
    return (OSStatus) err;
}
static pascal OSStatus evtHandler (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
  gemmacwindow*me=reinterpret_cast<gemmacwindow*>(userData);
  return me->eventHandler(event);
}

bool gemmacwindow::init(void) {
  // Check QuickTime installed
  long	QDfeature;
  if (OSErr err = ::Gestalt(gestaltQuickTime, &QDfeature)) {
    ::error("GEM: QuickTime is not installed : %d", err);
    return false;
  } else {
    if (OSErr err = ::EnterMovies()) {
      ::error("GEM: Couldn't initialize QuickTime : %d", err);
      return false;
    }
  }
  // check existence of OpenGL libraries
  if (reinterpret_cast<Ptr>(kUnresolvedCFragSymbolAddress) == reinterpret_cast<Ptr>(aglChoosePixelFormat)) {
    ::error("GEM: OpenGL is not installed");
    return false;
  }
  // This is to create a "master context" on Gem initialization, with
  //  the hope that we can then share it with later context's created
  //  when opening new rendering windows, and thereby share resources
  //  - no window will be directly associate with this context!
  //  - should remove the need for GemMan::HaveValidContext()
  GLint attrib[] = {AGL_RGBA, AGL_DOUBLEBUFFER, AGL_NO_RECOVERY, AGL_NONE};
  
  //  GDHandle display = GetMainDevice();
  //  AGLPixelFormat aglPixFmt = aglChoosePixelFormat( &display, 1, attrib );
  AGLPixelFormat aglPixFmt = aglChoosePixelFormat( NULL, 0, attrib );
  GLenum err = aglGetError();
  if (AGL_NO_ERROR != err)
    ::error((char *)aglErrorString(err));
  masterContext = aglCreateContext( aglPixFmt, NULL );
  err = aglGetError();
  if (AGL_NO_ERROR != err)
    ::error((char *)aglErrorString(err));
  aglSetCurrentContext( masterContext);
  
  //  AGL_MACRO_DECLARE_VARIABLES()

  aglDestroyPixelFormat( aglPixFmt );

  return true;
}

#if 0
GEM_EXTERN void initWin_sharedContext(WindowInfo &info, WindowHints &hints)
{
  m_info->context = masterContext; /* info */
  m_shared = masterContext;        /* hints */
}
#endif


struct gemmacwindow::Info {
  Info(void) : 
    pWind(NULL), 
    context(NULL), 
    offscreen(NULL), 
    pixelSize(32),
    pixMap(NULL), 
    rowBytes(0), 
    baseAddr(NULL),
    fontList(0)
  {}
  ~Info(void) {
  }
  WindowPtr		pWind;		// GEM window reference for gemwin
  AGLContext		context;	// OpenGL context
  GWorldPtr		offscreen;	// Macintosh offscreen buffer
  long		pixelSize;	//
  Rect		r;		//
  PixMapHandle	pixMap;		// PixMap Handle
  long		rowBytes;	// 
  void 		*baseAddr;	// 
  short		fontList;	// Font
  EventHandlerRef ehr;
};


CPPEXTERN_NEW(gemmacwindow);
gemmacwindow::gemmacwindow(void) :

    m_actuallyDisplay(true),
    m_info(new gemmacwindow::Info())
{
  if(!init())
    throw(GemException("could not initialize window infrastructure"));

  if(m_yoffset==0)
    m_yoffset=50;

}
gemmacwindow::~gemmacwindow(void) {
    destroyMess();
}
bool gemmacwindow::makeCurrent(void) {
  /* m_ was nfo. */
  ::aglSetDrawable( m_info->context, GetWindowPort(m_info->pWind) );
  ::aglSetCurrentContext(m_info->context);

  return true;
}

void gemmacwindow::swapBuffers(void)
{
  ::aglSwapBuffers(m_info->context);
}

bool gemmacwindow::create(void) {
  OSStatus	err;
  GDHandle hGD;
  short numDevices = 0;
  short whichDevice = 1; // number of device to try (0 = 1st device, 1 = 2nd/external device)
  logpost(NULL, 6,"MAC: createGemWindow()");
  EventTypeSpec	list[] = {		//{ kEventClassWindow, kEventWindowActivated },
    //{ kEventClassWindow, kEventWindowGetClickActivation },
    { kEventClassWindow, kEventWindowClosed },
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseMoved },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseWheelMoved },
    { kEventClassKeyboard, kEventRawKeyDown },
    { kEventClassKeyboard, kEventRawKeyUp } };
  int windowType=0, windowFlags=0;
  // If m_border != 0, then make a window with a titlebar
  //   otherwise, don't draw a titlebar
  if (m_border){
    windowType = kDocumentWindowClass;
    windowFlags = kWindowStandardHandlerAttribute | kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute;
  }else{
    windowType = kPlainWindowClass;
    windowFlags = kWindowStandardHandlerAttribute;
  }

  short i =0;
	
  // Build GL context and window or fullscreen

  if (!m_actuallyDisplay){
    return true;
  }
  // look for connected graphics devices
  hGD = DMGetFirstScreenDevice (true); // check number of screens

  //////////////////////////////////////////////////////////////////
  //check for a fullscreen request and then do the 10.3 workaround
  //
  // NOTE - this does NOT set the m_height and width so the viewport is still the default
  // maybe it's better to have a standard viewport so screen resizing doesn't change geometry settings?
  //
  if (m_fullscreen) {
    ////////////////////////////////////////////
    // a new CGL method of determining the number of attached displays and their coords
    CGDisplayCount maxDisplays = 32;
    CGDirectDisplayID activeDspys[32];
    CGDisplayErr disperror;
    CGDisplayCount newDspyCnt = 0;
    CGRect displayRect;
        
    disperror = CGGetActiveDisplayList(maxDisplays, activeDspys, &newDspyCnt);
    if (disperror) {
      error("GemWinCreateMac: CGGetActiveDisplayList returned error %d", disperror);
    }
    logpost(NULL, 5, "GemWinCreateMac: newDspyCnt %d", newDspyCnt);

    for (i=0; i < static_cast<int>(newDspyCnt); i++){
      CGRect displayRect = CGDisplayBounds (activeDspys[i]);
      logpost(NULL, 5, "GemWinCreateMac: display %d dimen %dx%d+%d+%d", i, 
           static_cast<long>(displayRect.size.width), 
           static_cast<long>(displayRect.size.height),
           static_cast<long>(displayRect.origin.x),
           static_cast<long>(displayRect.origin.y));
    }
        
    logpost(NULL, 5, "GemWinCreateMac: attempting fullscreen on display %d",m_fullscreen-1);
    if (m_fullscreen-1 > static_cast<int>(newDspyCnt)){
      logpost(NULL, 5, "GemWinCreateMac: display %d does not exist",m_fullscreen-1);
      return false;
    }

    //the device should be the first one so this will find the next one until it gets the user requested device
    //i think this should work
    whichDevice = m_fullscreen;
    do  {
      numDevices++;
      hGD = DMGetNextScreenDevice (hGD, true);
    }
    while (hGD);

    //grab the coords of the requested display
    displayRect = CGDisplayBounds (activeDspys[m_fullscreen-1]);

    //set a rect to cover the entire selected display
    SetRect(&m_info->r,
            static_cast<long>(displayRect.origin.x),
            static_cast<long>(displayRect.origin.y),
            (static_cast<long>(displayRect.size.width)  + static_cast<long>(displayRect.origin.x)),
            (static_cast<long>(displayRect.size.height) + static_cast<long>(displayRect.origin.y))
            );
DEBUGPOST("");
    //this winodw has no attributes like a title bar etc
    err = CreateNewWindow ( kDocumentWindowClass,
                            kWindowNoAttributes,
                            &m_info->r,
                            &m_info->pWind );
DEBUGPOST("");
    if (err) {
      error("GemWinCreateMac: Fullscreen CreateNewWindow err = %d",err);
      return false;
    }
  } else{ //go the usual windowed way
    ///////////////////////////////////////////
    //find the right device
    //might be redundant now with the new fullscreen stuff??
    do {
      numDevices++;
      hGD = DMGetNextScreenDevice (hGD, true);
    } while (hGD);
    logpost(NULL, 5, "GemwinMac: dimen %dx%d",m_width,m_height);
    // show and update main window
DEBUGPOST("info=%x", m_info);
    // this should put the title bar below the menu bar
    if (m_yoffset < 50){
      m_yoffset+=50; 
    }
        
    SetRect(&m_info->r, 
            static_cast<short>(m_xoffset), 
            static_cast<short>(m_yoffset),
            static_cast<short>(m_width + m_xoffset),
            static_cast<short>(m_height + m_yoffset));
DEBUGPOST("");
    err = CreateNewWindow ( windowType,
                            windowFlags,
                            &m_info->r,
                            &m_info->pWind );
DEBUGPOST("");
    if (err) {
      error("err = %d",err);
      return false;
    }
DEBUGPOST("");
    //this takes whatever input the user sets with the gemwin hints 'title' message
    CFStringRef tempTitle = CFStringCreateWithCString(NULL, m_title.c_str(), kCFStringEncodingASCII);		
    SetWindowTitleWithCFString ( m_info->pWind, tempTitle );
    CFRelease( tempTitle );
DEBUGPOST("");
    gaglDraw = GetWindowPort( m_info->pWind );
        
  }//end of conditional for fullscreen vs windowed
    DEBUGPOST("");
  ProcessSerialNumber psn;
  GetCurrentProcess(&psn);
  TransformProcessType(&psn, kProcessTransformToForegroundApplication);
		
  gEvtHandler = NewEventHandlerUPP( evtHandler );
  InstallEventHandler( GetApplicationEventTarget(), gEvtHandler,
                       GetEventTypeCount( list ), list,
                       this, &m_info->ehr );

  glWInfo.fAcceleratedMust = true; 		// must renderer be accelerated?
  glWInfo.VRAM = 0 * 1048576;			// minimum VRAM (if not zero this is always required)
  glWInfo.textureRAM = 0 * 1048576;		// minimum texture RAM (if not zero this is always required)
  glWInfo.fDraggable = false; 		// desired vertical refresh frquency in Hz (0 = any)
  glWInfo.fmt = 0;				// output pixel format
		
  i = 0;
  glWInfo.aglAttributes [i++] = AGL_RGBA;
  glWInfo.aglAttributes [i++] = AGL_PIXEL_SIZE;
  glWInfo.aglAttributes [i++] = 32;
  glWInfo.aglAttributes [i++] = AGL_DEPTH_SIZE; // 0, 16, 24, or 32
  glWInfo.aglAttributes [i++] = 24;
  glWInfo.aglAttributes [i++] = AGL_STENCIL_SIZE; // 0 or 8
  glWInfo.aglAttributes [i++] = 8;

  if (m_buffer == 2){
    glWInfo.aglAttributes [i++] = AGL_DOUBLEBUFFER;
  }
  //going to try for some FSAA here
  if (m_fsaa){
    glWInfo.aglAttributes [i++] = AGL_SAMPLE_BUFFERS_ARB;
    glWInfo.aglAttributes [i++] = 1;
    glWInfo.aglAttributes [i++] = AGL_SAMPLES_ARB;
    glWInfo.aglAttributes [i++] = m_fsaa;
  }
    
  glWInfo.aglAttributes [i++] = AGL_ACCELERATED;
  glWInfo.aglAttributes [i++] = AGL_NO_RECOVERY; 	// should be used whenever packed pixels is used to 
  glWInfo.aglAttributes [i++] = AGL_NONE;
//  BuildGLFromWindow ( m_info->pWind, &m_info->context, &glWInfo, m_shared);
  BuildGLFromWindow ( m_info->pWind, &m_info->context, &glWInfo, m_info->context);
  //		AGL_MACRO_DECLARE_VARIABLES()
  // }// end of window creation on main device - this is the old fullscreen code
    
  if (!m_info->context){
    error("MAC:  no m_info->context");
    return false;
  }
    
  SetFrontProcess( &psn );
    
  ShowWindow ( m_info->pWind );
  // um, this may be overkill?
  SelectWindow( m_info->pWind );
  err = ActivateWindow( m_info->pWind, true );
  if(err)
    error("GemWindow Activate err = %d", err );

  logpost(NULL, 6,"createGemWindow() finished");
  logpost(NULL, 6,"hints: actuallyDisplay = %d",m_actuallyDisplay);
  logpost(NULL, 6,"hints: border = %d",m_border);
  logpost(NULL, 6,"hints: width = %d",m_width);
  logpost(NULL, 6,"hints: height = %d",m_height);
  logpost(NULL, 6,"hints: x_offset = %d",m_xoffset);
  logpost(NULL, 6,"hints: y_offset = %d", m_yoffset);
  logpost(NULL, 6,"hints: fullscreen = %d", m_fullscreen);
  logpost(NULL, 6,"hints: title = %s",m_title.c_str());
  logpost(NULL, 6,"hints: shared = %d",m_info->context);//m_shared);
  logpost(NULL, 6,"hints: fsaa = %d",m_fsaa);
  hGD = NULL;
  return createGemWindow();
}


void gemmacwindow::destroy(void) {
  logpost(NULL, 6,"destroyGemWindow()");
  if (m_info->offscreen) {
    if (m_info->context) {
      ::aglSetCurrentContext(NULL);
      ::aglSetDrawable(m_info->context, NULL);
      ::aglDestroyContext(m_info->context);
      m_info->context  = NULL;
    }
    RemoveEventHandler( m_info->ehr );
    ::UnlockPixels(m_info->pixMap);
    ::DisposeGWorld(m_info->offscreen);
    m_info->offscreen = NULL;
    ::DisposeWindow( ::GetWindowFromPort(gaglDraw) );
    return;
  }
  if (m_info->context)
    {
      ::aglSetCurrentContext(NULL);
      ::aglSetDrawable(m_info->context, NULL);
      ::aglDestroyContext(m_info->context);
      m_info->context  = NULL;
      logpost(NULL, 6,"destroy context done");
    }

  if (m_info->pWind){
    logpost(NULL, 5, "destroyGemWindow() DisposeWindow");  
    ::DisposeWindow( m_info->pWind );
    logpost(NULL, 6,"destroyGemWindow() finished");
  }else error("no m_info->pWind to destroy!!");

  return destroyGemWindow();
}
void gemmacwindow::dispatch(void) {
  EventRef	theEvent;
  EventTargetRef theTarget;
    
  theTarget = GetEventDispatcherTarget();
  // TODO:
  //   this only gets one event per frame, so there's gotta be a better way, right?
  ReceiveNextEvent( 0, NULL, kEventDurationNoWait, true, &theEvent );
  {
    SendEventToEventTarget( theEvent, theTarget);
    ReleaseEvent( theEvent );
  }
}

OSStatus gemmacwindow::eventHandler (EventRef event)
{
  OSStatus result = eventNotHandledErr;
  UInt32 evtClass = GetEventClass (event);
  UInt32 kind = GetEventKind (event);
  WindowRef			winRef;
  UInt32				keyCode=0;
  char				macKeyCode[2];
  Point				location;
  EventMouseButton	buttonid = 0;
  //MouseWheelAxis	axis = 0;
  UInt32				modifiers = 0;
  long				wheelDelta = 0;

  if (eventNotHandledErr == result)
    {
      switch (evtClass) {
      case kEventClassApplication:
        switch (kind)
          {
          case kEventAppActivated:
            GetEventParameter( event, kEventParamWindowRef, typeWindowRef, NULL, sizeof(WindowRef), NULL, &winRef);
            SelectWindow(winRef);
            result = noErr;
            break;
          }
        break;
      case kEventClassWindow:
        switch (kind)
          {
          case kEventWindowClosed:
            info("window", "destroy");
            break;
          }
        break;
      case kEventClassKeyboard:
        switch (kind)
          {
          case kEventRawKeyDown:
          case kEventRawKeyUp:
            GetEventParameter( event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode);
            GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &macKeyCode[0]);
            macKeyCode[1]='\0';
            key((char *)&macKeyCode, keyCode, (kEventRawKeyDown==kind));
            result = noErr;
            break;
          }
        break;
      case kEventClassMouse:
        winRef = m_info->pWind;
        switch (kind)
          {
          case kEventMouseMoved:
            GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 
                              NULL, sizeof(Point), NULL, &location);
            QDGlobalToLocalPoint( GetWindowPort( winRef ), &location );
            motion(static_cast<int>(location.h), 
                   static_cast<int>(location.v)
                   );
            result = noErr;
            break;
                        
          case kEventMouseDragged:
            GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 
                              NULL, sizeof(Point), NULL, &location);
            QDGlobalToLocalPoint( GetWindowPort( winRef ), &location );
            motion(static_cast<int>(location.h), 
                   static_cast<int>(location.v)
                   );
            result = noErr;
            break;

          case kEventMouseDown:
          case kEventMouseUp:
            GetEventParameter(event, kEventParamMouseButton, typeMouseButton, 
                              NULL, sizeof(EventMouseButton), NULL, &buttonid);
            GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 
                              NULL, sizeof(Point), NULL, &location);
            QDGlobalToLocalPoint( GetWindowPort( winRef ), &location );
            // mac-button: 1-Left; 2-Right; 3-Middle
            // gem-button: 0-Left; 2-Right; 1-Middle
            switch(buttonid) {
            case (2):  // right
              buttonid=2; 
              break;
            case (3): // middle
              buttonid=1;
              break;
            default:   // all others
              buttonid-=1;
              break;
            }
            button(buttonid, (kEventMouseDown==kind));
            motion(static_cast<int>(location.h), 
                   static_cast<int>(location.v) 
                   );

            if(kEventMouseDown==kind) {
              /* FIXME: don't ignore modifier keys */
              GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 
                                NULL, sizeof(UInt32), NULL, &modifiers);
            }
            result = noErr;
            break;
          case kEventMouseWheelMoved:
            /* FIXME: how to handle scroll wheel? */
            GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, 
                              NULL, sizeof(long), NULL, &wheelDelta);
            //GetEventParameter(event, kEventParamMouseWheelAxis, typeMouseWheelAxis, 
            //                        NULL, sizeof(long), NULL, &axis);
            //triggerWheelEvent( axis, wheelDelta );
            result = noErr;
            break;
          }
        break;
      }
    }
  return result;
}



/////////////////////////////////////////////////////////
// Messages
//
/////////////////////////////////////////////////////////
void gemmacwindow :: createMess(std::string s) {
  if(!create()) {
    destroyMess();
    return;
  }
  cursorMess(m_cursor);
  dispatch();
}
void gemmacwindow :: destroyMess(void) {
  if(makeCurrent()) {
    destroy();
  } else {
    error("unable to destroy current window");
  }
}

void gemmacwindow :: dimensionsMess(unsigned int width, unsigned int height)
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
void gemmacwindow :: cursorMess(bool state)
{
  if (state)
    ShowCursor();
  else
    HideCursor();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemmacwindow :: obj_setupCallback(t_class *classPtr)
{
}



#endif
