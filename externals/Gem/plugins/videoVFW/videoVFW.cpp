////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file 
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "videoVFW.h"
#include "plugins/PluginFactory.h"
using namespace gem::plugins;
#include "Gem/RTE.h"

#ifdef HAVE_VFW_H
REGISTER_VIDEOFACTORY("VFW", videoVFW);
/////////////////////////////////////////////////////////
//
// videoVFW
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
videoVFW :: videoVFW()
  : videoBase("vfw", 0), 
    m_hWndC(NULL)
{
  provide("dv");
  provide("iidc");
  provide("analog");
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoVFW :: ~videoVFW()
{
  close();
}


/////////////////////////////////////////////////////////
// openDevice
//
/////////////////////////////////////////////////////////
bool videoVFW :: openDevice(gem::Properties&props)
{
  char driverName[256];
  char driverDesc[256];
  if (capGetDriverDescription(0, driverName, 256, driverDesc, 256))
    post("videoVFW: driver '%s'", driverName);


  double d;
  if (props.get("width", d))
    m_width=d;

  if (props.get("height", d))
    m_height=d;

  if(m_hWndC)closeDevice();
  
  // Connect to the daemon
  m_hWndC = capCreateCaptureWindow ((LPSTR) "GEM video",	// window name if pop-up 
				    0,				// window style (not visible)
				    0, 0, m_width, m_height,// window position and dimensions
				    GetDesktopWindow(), 0);
  if (!m_hWndC) {
    error("Unable to create capture window");
    return false;
  } 

  if (!capDriverConnect(m_hWndC, 0)) {
    error("Unable to connect to video driver");
    closeDevice();
    return false;
  }

  CAPTUREPARMS params;
  if (!capCaptureGetSetup(m_hWndC, &params, sizeof(CAPTUREPARMS))) {
    error("Unable to get capture parameters");
    closeDevice();
    return false;
  }

  params.fYield = TRUE;
  params.fCaptureAudio = FALSE;
  params.wPercentDropForError = 100;
  params.fLimitEnabled = FALSE;
  params.AVStreamMaster = AVSTREAMMASTER_NONE;
  params.fStepCaptureAt2x = FALSE;
  params.fAbortLeftMouse = FALSE;
  params.fAbortRightMouse = FALSE;
  if (!capCaptureSetSetup(m_hWndC, &params, sizeof(CAPTUREPARMS)))
    {
      error("Unable to set capture parameters");
      closeDevice();
      return false;
    }

  if (!capSetCallbackOnVideoStream(m_hWndC, videoVFW::videoFrameCallback))
    {
      error("Unable to set frame callback");
      closeDevice();
      return false;
    }
  if (!capSetUserData(m_hWndC, this))
    {
      error("Unable to set user data");
      closeDevice();
      return false;
    }
  DWORD formSize = capGetVideoFormat(m_hWndC, NULL, 0);
  BITMAPINFO *videoFormat = (BITMAPINFO *)(new char[formSize]);
  if (!capGetVideoFormat(m_hWndC, videoFormat, formSize))
    {
      error("Unable to get video format");
      closeDevice();
      return false;
    }

  videoFormat->bmiHeader.biWidth = m_width;
  videoFormat->bmiHeader.biHeight = m_height;
  videoFormat->bmiHeader.biBitCount = 24;
  videoFormat->bmiHeader.biCompression = BI_RGB;
  videoFormat->bmiHeader.biClrUsed = 0;
  videoFormat->bmiHeader.biClrImportant = 0;
  videoFormat->bmiHeader.biSizeImage = 0;
  if (!capSetVideoFormat(m_hWndC, videoFormat, formSize)) {
      error("Unable to set video format");
      delete videoFormat;
      closeDevice();
      return false;
    }
  if (!capGetVideoFormat(m_hWndC, videoFormat, formSize)) {
    error("Unable to get video format");
  }
  m_width=static_cast<int>(videoFormat->bmiHeader.biWidth);
  m_height=static_cast<int>(videoFormat->bmiHeader.biHeight);

  logpost(NULL, 5, "Connected with %dx%d @ %d",
          m_width, m_height,
          static_cast<int>(videoFormat->bmiHeader.biBitCount));

  delete videoFormat;

  m_image.image.xsize = m_width;
  m_image.image.ysize = m_height;
  m_image.image.setCsizeByFormat(GL_RGBA);

  m_image.image.reallocate();
  m_image.image.setBlack();

  return true;
}

/////////////////////////////////////////////////////////
// closeDevice
//
/////////////////////////////////////////////////////////
void videoVFW :: closeDevice()
{
  if (m_hWndC) {
    capDriverDisconnect(m_hWndC);
    DestroyWindow(m_hWndC);
  }
  m_hWndC=NULL;
}


/////////////////////////////////////////////////////////
// videoFrame callback
//
/////////////////////////////////////////////////////////
void videoVFW :: videoFrame(LPVIDEOHDR lpVHdr)
{
  int count = lpVHdr->dwBytesUsed;
  // notice that it is times 3 for the color!
  // incoming data is BGR
  const int dataSize = m_image.image.xsize * m_image.image.ysize * 3;

  if (count < dataSize)
    {
      return;
    }

  lock();
  m_image.image.fromBGR(lpVHdr->lpData);
  m_image.newimage = true;
  unlock();
}
void videoVFW :: videoFrameCallback(HWND hWnd, LPVIDEOHDR lpVHdr)
{
  videoVFW*ptr = reinterpret_cast<videoVFW*>(capGetUserData(hWnd));
  ptr->videoFrame(lpVHdr);
}


/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
bool videoVFW :: grabFrame()
{
  return true;
}

/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoVFW :: startTransfer()
{
  bool result= capCaptureSequenceNoFile(m_hWndC);
  m_image.newfilm=result;

  return result;
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoVFW :: stopTransfer()
{
  capCaptureStop(m_hWndC);
  return true;
}

/////////////////////////////////////////////////////////
// csMess
//
/////////////////////////////////////////////////////////
bool videoVFW :: setColor(int format)
{
  if(format)m_image.image.setCsizeByFormat(format);
  return true;
}



bool videoVFW :: enumProperties(gem::Properties&readable, gem::Properties&writeable) {
  readable.clear();
  writeable.clear();

  gem::any type=0;

  writeable.set("width", type);
  writeable.set("height", type);


  return true;
}

void videoVFW :: setProperties(gem::Properties&props) {
  double d;
  bool dorestart=false;

  if (props.get("width", d)) {
    m_width=d;
    dorestart=true;
  }

  if (props.get("height", d)) {
    m_height=d;
    dorestart=true;
  }

  if(dorestart && m_hWndC)
    reset();
}
void videoVFW :: getProperties(gem::Properties&props) {
}

#else
videoVFW ::  videoVFW() : videoBase("") { }
videoVFW :: ~videoVFW() { }
#endif
