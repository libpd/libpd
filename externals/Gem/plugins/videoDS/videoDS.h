/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Copyright (c) 2003 Daniel Heckenberg
Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE_GEMPLUGIN__VIDEODS_VIDEODS_H_
#define _INCLUDE_GEMPLUGIN__VIDEODS_VIDEODS_H_

#include "plugins/videoBase.h"

#ifdef HAVE_DIRECTSHOW
# include <dshow.h>
# include <qedit.h>
#endif

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_video
    
  captures a video on Apple machines
    
  KEYWORDS
  pix
    
  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
 class GEM_EXPORT videoDS : public videoBase {
  public:
    //////////
    // Constructor
    videoDS(void);
    	    	
    //////////
    // Destructor
    virtual ~videoDS(void);

#ifdef HAVE_DIRECTSHOW
    ////////
    // open the video-device
    virtual bool           openDevice(gem::Properties&props);
    // and close the video-device
    virtual void          closeDevice(void);
    
    //////////
    // Start up the video device
    // [out] int - returns 0 if bad
    virtual bool	    	startTransfer(void);
    //////////
    // Stop the video device
    // [out] int - returns 0 if bad
    virtual bool	   	stopTransfer(void);

    //////////
    // get the next frame
    virtual pixBlock* getFrame(void);
    virtual void releaseFrame(void);

    //////////
    // Set the video dimensions
    virtual bool setDimen(int x, int y, int leftmargin, int rightmargin, int topmargin, int bottommargin);
    virtual bool setColor(int d);
    virtual bool dialog(std::vector<std::string>);
    virtual std::vector<std::string>dialogs(void);
    virtual std::vector<std::string>videoDS :: enumerate(void);
    
  protected:
    //-----------------------------------
    // GROUP:	Video data
    //-----------------------------------

    //////////
    // The pixBlocks for the captured and rendered image
    void copyBuffer(void);
    pixBlock    m_pixBlockBuf[3];
    int		m_nPixDataSize[3];

    // Index to read latest image.
    int		m_readIdx;
    int		m_lastreadIdx;
    int		m_writeIdx;
    int		m_lastwriteIdx;

    int		m_format;
#ifdef USE_RECORDING
    bool	m_recording;
    char        m_filename[MAXPDSTRING];
#endif

    // DirectShow Interfaces that we may need
    IGraphBuilder*	m_pGB;
    IMediaControl*	m_pMC;
    IMediaEvent*      	m_pME;
    IMediaFilter*	m_pMF;
    IMediaSeeking*	m_pMS;
    IMediaPosition*	m_pMP;
    IBaseFilter		*SampleFilter;		// Sample filter
    IBaseFilter		*NullFilter;		// Null render base Filter for video
    IBaseFilter		*FileFilter;		// File filter for writing video
    ISampleGrabber	*SampleGrabber;		// Sample grabber
#ifdef DIRECTSHOW_LOGGING
    HFILE		  LogFileHandle;
#endif

    IBaseFilter*	  m_pCDbase;
    ICaptureGraphBuilder2*m_pCG;

    unsigned long	  m_GraphRegister;
#endif /*HAVE_DIRECTSHOW */
  }; 
};};


#endif	// for header file
