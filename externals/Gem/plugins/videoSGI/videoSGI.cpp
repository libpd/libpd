////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "videoSGI.h"
#include "plugins/PluginFactory.h"
using namespace gem::plugins;

#include "Gem/RTE.h"

#ifdef HAVE_VL_VL_H
# include <unistd.h>
# include <dmedia/vl_vino.h>

REGISTER_VIDEOFACTORY("sgi", videoSGI);


/*
  from pix_indycam:

  "zoom" -> zoomMess
  "bright" -> brightMess
  "contrast" -> contrastMess
  "hue" -> hueMess
  "sat" -> satMess
*/


/////////////////////////////////////////////////////////
//
// video
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
videoSGI :: videoSGI() 
  : videoBase("sgi", 0),
    m_haveVideo(0), m_swap(1), m_colorSwap(0),
    m_svr(NULL), m_drn(NULL), m_src(NULL), m_path(NULL)
{
  provide("indy");
  provide("analog");
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoSGI :: ~videoSGI()
{
  stopTransfer();
  close();
  cleanPixBlock();
}

bool videoSGI :: openDevice()
{
  // Connect to the daemon
  if ( !(m_svr = vlOpenVideo("")) )
    {
    	error("GEM: videoSGI: Unable to open video");
    	goto cleanup;
    }
  // Set up a drain node in memory
  m_drn = vlGetNode(m_svr, VL_DRN, VL_MEM, VL_ANY);
    
  // Set up a source node on any video source
  m_src = vlGetNode(m_svr, VL_SRC, VL_VIDEO, VL_ANY);

  // Create a path using the first device that will support it
  m_path = vlCreatePath(m_svr, VL_ANY, m_src, m_drn); 

  // Set up the hardware for and define the usage of the path
  if ( (vlSetupPaths(m_svr, (VLPathList)&m_path, 1, VL_SHARE, VL_SHARE)) < 0 )
    {
    	error("GEM: videoSGI: Unable to setup video path");
    	goto cleanup;
    }

  // Set the packing to RGBA
  VLControlValue val;

  // first try to see if it can handle RGBA format

  // according to vl.h, this is really RGBA (OpenGL format)
  val.intVal = VL_PACKING_ABGR_8;
  if ( vlSetControl(m_svr, m_path, m_drn, VL_PACKING, &val) )
    {
    	// nope, so try ABGR, and set the color swapping
    	// according to vl.h, this is really ABGR (IrisGL format)
      val.intVal = VL_PACKING_RGBA_8;
      if ( vlSetControl(m_svr, m_path, m_drn, VL_PACKING, &val) )
        {
    	    post("GEM: videoSGI: Unable to set the video packing");
          goto cleanup;
        }
    	post("GEM: videoSGI: Video has to color swap (ABGR to RGBA)");
    	m_colorSwap = 1;
    }
    
  // Get the video size

  VLControlValue value;
  if(m_width>0) {
    value.xyVal.x = m_width;
    value.xyVal.y = m_height;
    if ( vlSetControl(m_svr, m_path, m_drn, VL_SIZE, &value) ) {
      vlGetControl(m_svr, m_path, m_drn, VL_SIZE, &value);
    
      post("GEM: videoSGI: dimen error: wanted %dx%d got %dx%d", m_width, m_height, value.xyVal.x, value.xyVal.y);
      m_width =value.xyVal.x;
      m_height=value.xyVal.y;
    }
  }
  vlGetControl(m_svr, m_path, m_drn, VL_SIZE, &value);
  m_width =value.xyVal.x;
  m_height=value.xyVal.y;

  m_pixBlock.image.xsize = m_width;
  m_pixBlock.image.ysize = m_height;
  m_pixBlock.image.reallocate();

  return true;
 cleanup:

  closeDevice();
  return false;
}

void videoSGI :: closeDevice() {
  m_src=NULL;
  m_drn=NULL;
  if(m_svr) {
    if(m_path)
      vlDestroyPath(m_svr, m_path);

    vlCloseVideo(m_svr);
  }

  m_path=NULL;
  m_svr=NULL;
}



////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock *videoSGI::getFrame(void) {
{
  if(!(m_haveVideo && m_capturing))return NULL;

  VLInfoPtr info = vlGetLatestValid(m_svr, m_buffer);
  while (!info)
    {
      sginap(1);
      info = vlGetLatestValid(m_svr, m_buffer);
    }

  // Get a pointer to the frame
  unsigned char *dataPtr = (unsigned char *)(vlGetActiveRegion(m_svr, m_buffer, info));
  m_pixBlock.image.fromABGR(dataPtr);

  // free the frame
  vlPutFree(m_svr, m_buffer);

  m_pixBlock.newimage = 1;
  return &m_pixBlock;

}


////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoSGI :: startTransfer()
{
  if(NULL=m_svr || NULL=m_path || NULL==m_drn || NULL=m_src)return false;
    
  // Create and register a buffer for 1 frame
  m_buffer = vlCreateBuffer(m_svr, m_path, m_drn, 1);
  if ( !m_buffer )
    {
    	error("GEM: videoSGI: Unable to allocate buffer");	
    	return false;
    }

  vlRegisterBuffer(m_svr, m_path, m_drn, m_buffer);
    
  // Begin the data transfer
  if ( vlBeginTransfer(m_svr, m_path, 0, NULL) )
    {
    	error("GEM: videoSGI: Unable to start video transfer");
      vlDeregisterBuffer(m_svr, m_path, m_drn, m_buffer);
      vlDestroyBuffer(m_svr, m_buffer);
    	return false;
    }
  return true;
}

////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoSGI :: stopTransfer()
{
  if ( !m_capturing ) return false;
  if(NULL=m_svr || NULL=m_path || NULL==m_drn || NULL=m_src)return false;

  // Clean up the buffer
  vlEndTransfer(m_svr, m_path);
  vlDeregisterBuffer(m_svr, m_path, m_drn, m_buffer);
  vlDestroyBuffer(m_svr, m_buffer);
  m_buffer=NULL;
    
  return true;
}

#if 0
////////////////////////////////////////////////////////
// offsetMess
//
/////////////////////////////////////////////////////////
void videoSGI :: offsetMess(int x, int y)
{
  if (!m_haveVideo)
    {
    	post("GEM: videoSGI: Connect to video first");
    	return;
    }
    
  // stop the transfer and destroy the buffer
  if ( !stopTransfer() ) 
    {
    	post("GEM: videoSGI: error stopping transfer");
    	return;
    }

  VLControlValue value;
  value.xyVal.x = x;
  value.xyVal.y = y;
  if ( vlSetControl(m_svr, m_path, m_drn, VL_OFFSET, &value) )
    {
    	post("GEM: videoSGI: offset error");
    	startTransfer();
    	return;
    }

  // start the transfer and rebuild the buffer
  if ( !startTransfer() ) 
    {
    	post("GEM: videoSGI: error starting transfer");
    	return;
    }
}


/////////////////////////////////////////////////////////
// zoomMess
//
/////////////////////////////////////////////////////////
void videoINDY :: zoomMess(int num, int denom)
{
    if (!m_haveVideo)
    {
    	error("Connect to video first");
    	return;
    }
    VLControlValue value;
    value.fractVal.numerator = num;
    value.fractVal.denominator = denom;
    if ( vlSetControl(m_svr, m_path, m_drn, VL_ZOOM, &value) )
    	error("zoom error");
}

/////////////////////////////////////////////////////////
// brightMess
//
/////////////////////////////////////////////////////////
void videoINDY :: brightMess(int val)
{
    if (!m_haveVideo)
    {
    	error("Connect to video first");
    	return;
    }
    VLControlValue value;
    value.intVal = val;
    if ( vlSetControl(m_svr, m_path, m_drn, VL_BRIGHTNESS, &value) )
    	error("problem setting brightness");
}

/////////////////////////////////////////////////////////
// contrastMess
//
/////////////////////////////////////////////////////////
void videoINDY :: contrastMess(int val)
{
    if (!m_haveVideo)
    {
    	error("Connect to video first");
    	return;
    }
    VLControlValue value;
    value.intVal = val;
    if ( vlSetControl(m_svr, m_path, m_drn, VL_CONTRAST, &value) )
    	error("problem setting contrast");
}

/////////////////////////////////////////////////////////
// hueMess
//
/////////////////////////////////////////////////////////
void videoINDY :: hueMess(int val)
{
    if (!m_haveVideo)
    {
    	error("Connect to video first");
    	return;
    }
    VLControlValue value;
    value.intVal = val;
    if ( vlSetControl(m_svr, m_path, m_drn, VL_HUE, &value) )
    	error("problem setting hue");
}

/////////////////////////////////////////////////////////
// satMess
//
/////////////////////////////////////////////////////////
void videoINDY :: satMess(int val)
{
    if (!m_haveVideo)
    {
    	error("Connect to video first");
    	return;
    }
    VLControlValue value;
    value.intVal = val;
    if ( vlSetControl(m_svr, m_path, m_src, VL_VINO_INDYCAM_SATURATION, &value) )
    	error("problem setting saturation");
}


#endif
///////////////////////////////////////////////////////
// dimenMess
//
////////////////////////////////////////////////////////
bool videoSGI :: setDimen(int x, int y, int leftmargin, int rightmargin, int topmargin, int bottommargin){
{
  bool result=true;
  m_width=x;
  m_height=y

  if(NULL=m_svr || NULL=m_path || NULL==m_drn || NULL=m_src)return false;
  if(!m_capturing)return false;
  
  stopTransfer();

  VLControlValue value;
  value.xyVal.x = m_width;
  value.xyVal.y = m_height;
  if ( vlSetControl(m_svr, m_path, m_drn, VL_SIZE, &value) ) {
    vlGetControl(m_svr, m_path, m_drn, VL_SIZE, &value);
    
    post("GEM: videoSGI: dimen error: wanted %dx%d got %dx%d", m_width, m_height, value.xyVal.x, value.xyVal.y);
    m_width =value.xyVal.x;
    m_height=value.xyVal.y;
    result=false;
  }
  m_pixBlock.image.xsize = m_width;
  m_pixBlock.image.ysize = m_height;
  m_pixBlock.image.reallocate();
  
  start();
  return result;
}
#else /* !HAVE_VL_VL_H */
videoSGI ::  videoSGI() : videoBase("") { }
videoSGI :: ~videoSGI() { }
#endif  /* !HAVE_VL_VL_H */
