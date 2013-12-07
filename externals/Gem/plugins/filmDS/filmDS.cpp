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
#if defined(_WIN32) && defined(HAVE_DIRECTSHOW)

#include "filmDS.h"
#include "plugins/PluginFactory.h"
#include "Gem/RTE.h"

using namespace gem::plugins;

REGISTER_FILMFACTORY("DirectShow", filmDS);

#include <atlbase.h>
#include <atlconv.h>
#include <streams.h>
#include <dvdmedia.h>
#define REGISTER_FILTERGRAPH 1

#include <strsafe.h>

HRESULT filmGetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin);
HRESULT filmConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond);
HRESULT filmAddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) ;
void filmRemoveGraphFromRot(DWORD pdwRegister);
/////////////////////////////////////////////////////////
//
// filmDS
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

filmDS :: filmDS(void) : filmBase() {
  HRESULT RetVal;
  m_reqFrame = 1;
  m_frame = NULL;
  m_csize=0;
  m_xsize=0;
  m_ysize=0;
  FilterGraph=NULL;
  VideoFilter=NULL;
  SampleFilter=NULL;
  NullFilter=NULL;
  SampleGrabber=NULL;
  MediaControl=NULL;
  MediaSeeking=NULL;
  MediaPosition=NULL;

  CoInitialize(NULL);

  // Create the base object of a filter graph
  RetVal	= CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                                   IID_IGraphBuilder, (void **)&FilterGraph);

  if (RetVal != S_OK || NULL == FilterGraph)
    {
      error("Unable to create FilterGraph interface %d", RetVal);
		
      return;
    }
	
  // Get the IMediaControl interface for Run, Stop, Pause and keeps control states
  RetVal	= FilterGraph->QueryInterface(IID_IMediaControl, (void **)&MediaControl);

  if (RetVal != S_OK || NULL == MediaControl)
    {
      error("Unable to create MediaControl interface %d", RetVal);
		
      return;
    }
	
  // Get the IMediaSeeking interface for rewinding video at loop point 
  // and set time format to frames 
  RetVal	= FilterGraph->QueryInterface(IID_IMediaSeeking, (void **)&MediaSeeking);

  if (RetVal != S_OK || NULL == MediaSeeking)
    {
      error("Unable to create MediaSeeking interface %d", RetVal);
		
      return;
    }
	
  // Get the IMediaPosition interface for getting the current position of the clip
  RetVal	= FilterGraph->QueryInterface(IID_IMediaPosition, (void **)&MediaPosition);

  if (RetVal != S_OK || NULL == MediaPosition)
    {
      error("Unable to create MediaPosition interface %d", RetVal);
		
      return;
    }
}

////////////////////////////////////////////////////////
// Destructor
//
////////////////////////////////////////////////////////
filmDS :: ~filmDS()
{
  close();
	
  // Release IMediaControl interface
  if (MediaControl != NULL)
    {
      MediaControl->Release();
		
      MediaControl	= NULL;
    }
	
  // Release IMediaSeeking interface
  if (MediaSeeking != NULL)
    {
      MediaSeeking->Release();
		
      MediaSeeking	= NULL;
    }
	
  // Release IMediaPosition interface
  if (MediaPosition != NULL)
    {
      MediaPosition->Release();
		
      MediaPosition	= NULL;
    }
	
  // Release base FilterGraph
  if (FilterGraph != NULL)
    {
      FilterGraph->Release();
		
      FilterGraph	= NULL;
    }
	
  // Release COM
  CoUninitialize();
}


void filmDS :: close(void)
{
  
  // Stop the video. Filters cannot be remove until video is stopped
  if (MediaControl != NULL)
    {
      MediaControl->Stop();
    }

  // Release ISampleGrabber interface
  if (SampleGrabber != NULL)
    {
      SampleGrabber->Release();
		
      SampleGrabber	= NULL;
    }
	
  // Remove and release SampleFilter (IBaseFilter) interface
  if (SampleFilter != NULL)
    {
      FilterGraph->RemoveFilter(SampleFilter);
      SampleFilter->Release();
		
      SampleFilter	= NULL;
    }

  // Remove and release VideoFilter (IBaseFilter) interface
  if (VideoFilter != NULL)
    {
      FilterGraph->RemoveFilter(VideoFilter);
      VideoFilter->Release();
		
      VideoFilter		= NULL;
    }

  // Remove and release NullFilter (IBaseFilter) interface
  if (NullFilter != NULL)
    {
      FilterGraph->RemoveFilter(NullFilter);
      NullFilter->Release();
		
      NullFilter		= NULL;
    }

  // Delete the graphics buffer
  if (m_frame != NULL)
    {
      delete [] m_frame;

      m_frame	= NULL;
    }

#ifdef REGISTER_FILTERGRAPH
  if (m_GraphRegister)
    {	
      HRESULT hr;

      filmRemoveGraphFromRot(m_GraphRegister);
      m_GraphRegister = 0;
    }
#endif

}

/////////////////////////////////////////////////////////
// open the file
//
/////////////////////////////////////////////////////////
bool filmDS :: open(const std::string filename, const gem::Properties&wantProps)
{
  WCHAR			WideFileName[MAXPDSTRING];
  HRESULT			RetVal;
  AM_MEDIA_TYPE	MediaType;
  BOOL			bFrameTime	= TRUE;
  GUID			Guid;
		
  logpost(NULL, 5, "Trying DirectShow");

  // Convert c-string to Wide string.
  memset(&WideFileName, 0, MAXPDSTRING * 2);
	
  if (0 == MultiByteToWideChar(CP_ACP, 0, filename.c_str(), filename.length(), WideFileName, 
                               MAXPDSTRING))
    {
      error("filmDS: Unable to load %s", filename.c_str());
		
      return false;
    }

  // Add a file source filter to the filter graph.
  RetVal	= FilterGraph->AddSourceFilter(WideFileName, L"SOURCE", &VideoFilter);
	
  if (RetVal != S_OK || NULL == VideoFilter)
    {
      error("filmDS: Unable to render %s", filename.c_str());
		
      return false;
    }

  // Create an instance of the sample grabber filter. The filter allows frames to be
  // buffered from a video source.  
  RetVal	= CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                                   IID_IBaseFilter, (void**)&SampleFilter);
    
  if (RetVal != S_OK || NULL == SampleFilter)
    {
      error("Unable to create SampleFilter interface %d", RetVal);
		
      return false;
    }

  // Add sample grabber filter to the filter graph.
  RetVal	= FilterGraph->AddFilter(SampleFilter, L"Sample Grabber");

  if (RetVal != S_OK)
    {
      error("Unable to add SampleFilter %d", RetVal);
		
      return false;
    }

  // Find an interface to the SampleGrabber from the SampleGrabber filter. The 
  // SampleGrabber allows frames to be grabbed from the filter. SetBufferSamples(TRUE)
  // tells the SampleGrabber to buffer the frames. SetOneShot(FALSE) tells the 
  // SampleGrabber to continuously grab frames.  has GetCurrentBuffer() method
  RetVal	= SampleFilter->QueryInterface(IID_ISampleGrabber, (void **)&SampleGrabber);

  if (RetVal != S_OK || NULL == SampleGrabber)
    {
      error("Unable to create SampleGrabber interface %d", RetVal);
		
      return false;
    }

  // Set the media type that the SampleGrabber wants.
  // MEDIATYPE_Video selects only video and not interleaved audio and video
  // MEDIASUBTYPE_RGB24 is the colorspace and format to deliver frames
  // MediaType.formattype is GUID_NULLsince it is handled later to get file info
  memset(&MediaType, 0, sizeof(AM_MEDIA_TYPE));
  MediaType.majortype		= MEDIATYPE_Video;
  MediaType.subtype		= MEDIASUBTYPE_RGB32;
  //MediaType.subtype		= MEDIASUBTYPE_RGB24;
  //MediaType.subtype		= MEDIASUBTYPE_UYVY;
  MediaType.formattype	= GUID_NULL;
  RetVal					= SampleGrabber->SetMediaType(&MediaType);

  // Set the SampleGrabber to return continuous frames
  RetVal	= SampleGrabber->SetOneShot(FALSE);
	
  if (RetVal != S_OK)
    {
      error("Unable to setup sample grabber %d", RetVal);
		
      return false;
    }

  // Set the SampleGrabber to copy the data to a buffer. This only set to FALSE when a 
  // callback is used.
  RetVal	= SampleGrabber->SetBufferSamples(TRUE);
	
  if (RetVal != S_OK)
    {
      error("Unable to setup sample grabber %d", RetVal);
		
      return false;
    }
 
  // Create the Null Renderer interface. The Null Renderer is used to disable rendering of a 
  // video stream to a window.
  RetVal	= CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
                                   IID_IBaseFilter, (void**)&NullFilter);
    
  if (RetVal != S_OK || NULL == NullFilter)
    {
      error("Unable to create NullFilter interface %d", RetVal);
		
      return false;
    }

  // Add the Null Renderer filter to the FilterGraph
  RetVal	= FilterGraph->AddFilter(NullFilter, L"NullRenderer");

  if (RetVal != S_OK)
    {
      error("Unable to add NullFilter %d", RetVal);
		
      return false;
    }

  // DS filter chain is FileSource -> SampleGrabber -> NullRenderer
  // DS can put any neeeded filters in the chain for format or colorspace conversion
  // decompression or other transforms

  // Connect the SampleFilter to the VideoFilter
  RetVal	= filmConnectFilters(FilterGraph, VideoFilter, SampleFilter);

  if (RetVal != S_OK)
    {
      error("Unable to connect filters %d", RetVal);
		
      return false;
    }

  // Connect the NullFilter to the SampleFilter
  RetVal	= filmConnectFilters(FilterGraph, SampleFilter, NullFilter);

  if (RetVal != S_OK)
    {
      error("Unable to connect filters %d", RetVal);
		
      return false;
    }
	
  // Set the time format to frames
  Guid	= TIME_FORMAT_FRAME;
	
  RetVal	= MediaSeeking->SetTimeFormat(&Guid);
	
  if (RetVal != S_OK)
    {
      // If frame time format not available, default to 100 nanosecond increments.
      bFrameTime	= FALSE;
		
      Guid	= TIME_FORMAT_MEDIA_TIME;
	
      RetVal	= MediaSeeking->SetTimeFormat(&Guid);
	
      if (RetVal != S_OK)
        {
          error("Unable to set video time format %d", RetVal);

          return false;
        }
    }
	
  // Get the duration of the video. Format will be in previously set time format. This is 
  // compatible with the value returned from GetCurrentPosition
  RetVal	= MediaSeeking->GetDuration(&m_Duration);
	
  if (RetVal != S_OK)
    {
      error("Unable to get video duration %d", RetVal);

      return false;
    }
	
  // Set the number of frames based on the time format used.
  if (TRUE == bFrameTime)
    {
      m_numFrames	= m_Duration;
    }
	
  else
    {
      LONGLONG	OutFormat;
      GUID		OutGuid;
				
      OutGuid	= TIME_FORMAT_FRAME;
      Guid	= TIME_FORMAT_MEDIA_TIME; 
		
      //converts from 100 nanosecond format to number of frames
      MediaSeeking->ConvertTimeFormat(&OutFormat, &OutGuid, m_Duration, &Guid);
		
      m_numFrames	= OutFormat;
    }
	
  // Get the format of the connected media.
  RetVal	= SampleGrabber->GetConnectedMediaType(&MediaType);
	
  if (RetVal != S_OK)
    {
      error("Unable to get media type %d", RetVal);
		
      return false;
    }

  // The SampleGrabber will only return video of the the 'FORMAT_VideoInfo' type.
  if (FORMAT_VideoInfo == MediaType.formattype && MediaType.pbFormat != NULL)
    {
      // Format returned is specific to the formattype.
      VIDEOINFOHEADER	*VideoInfo	= (VIDEOINFOHEADER *)MediaType.pbFormat;
		
      // Get size of the image from the BitmapInfoHeader returned in the VIDEOINFOHEADER.
      m_xsize		= VideoInfo->bmiHeader.biWidth;
      m_ysize		= VideoInfo->bmiHeader.biHeight;
      //m_csize		= 3;
      m_csize		= 4;
    }
	
  else
    {
      error("Invalid media type returned %s", filename.c_str());

      return false;
    }
	
  // Allocate video buffer if valid sizes returned.
  if (m_xsize > 0 && m_ysize > 0 && m_csize > 0)
    {
      if (m_frame != NULL)
        {
          delete [] m_frame;
        }
		
      m_frame		= new BYTE[m_xsize * m_ysize * m_csize];
		
      if (NULL == m_frame)
        {
          error("Unable to allocate memory for the video buffer %s", filename.c_str());

          return false;
        }
    }

  // Release the MediaType.pbFormat data
  FreeMediaType(MediaType);
	
  IBaseFilter	*DVFilter;
	
  // If DV video is used, set the quality to 720 x 480.
  RetVal	= FilterGraph->FindFilterByName(L"DV Video Decoder", &DVFilter);
	
  if (S_OK == RetVal && DVFilter != NULL)
    {
      IIPDVDec	*IPDVDec;
		
      // Find the IIPDVDec interface
      RetVal	= DVFilter->QueryInterface(IID_IIPDVDec, (void **)&IPDVDec);
		
      if (S_OK == RetVal && IPDVDec != NULL)
        {
          // Set the property to DVRESOLUTION_FULL
          IPDVDec->put_IPDisplay(DVRESOLUTION_FULL);
		
          // Release the interface
          IPDVDec->Release();
        }

      // Release the interface
      DVFilter->Release();
    }
	 
  // Setup the pixBlock data based on the media type.
  // this is a guess at the fast past for pixels on Windows
  m_image.image.xsize	= m_xsize;
  m_image.image.ysize	= m_ysize;
  m_image.image.csize	= m_csize;
  if (m_csize == 3) m_image.image.format	= GL_BGR_EXT;
  if (m_csize == 4) m_image.image.format	= GL_BGRA; 
  m_image.image.type	= GL_UNSIGNED_BYTE;

  // Start the video stream
  RetVal	= MediaControl->Run();
	
  if (RetVal != S_OK && RetVal != S_FALSE)
    {
      error("Unable to start video %d", RetVal);
		
      return false;
    } 
	
  // Wait for the video to begin playing.
  while (TRUE)
    {
      OAFilterState	FilterState;
		
      // Get the state and ensure it's not in an intermediate state
      RetVal	= MediaControl->GetState(0, &FilterState);

      if (RetVal != S_OK && RetVal != VFW_S_STATE_INTERMEDIATE)
        {
          error("Unable to run video %d", RetVal);
		
          return false;
        }

      // Ensure the video is running
      else if (RetVal == S_OK && State_Running == FilterState)
        {
          break;
        }
    }
	
  // Sets the tex coords
  //	prepareTexture();

  // Set the last frame to -1 so it will show the first frame.
  m_LastFrame	= -1;
	
  //	m_haveMovie	= TRUE;	  

#ifdef REGISTER_FILTERGRAPH
  if (FAILED(RetVal = filmAddGraphToRot(FilterGraph, &m_GraphRegister))){
    error("filmDS: failed to register filter graph with ROT!  hr=0x%X", RetVal);
    m_GraphRegister = 0;
  }
#endif
  return true;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock* filmDS :: getFrame(){
  long			frameSize	= m_ysize * m_xsize * m_csize;
  HRESULT			RetVal;
  OAFilterState	State;	
 // LONGLONG	CurrentPosition;
 // LONGLONG	Current	= 0;
	
  // Initially set the image as unchanged
  m_image.newimage	= FALSE;
	
  // If the MediaControl interface is unavailable return.
  if (NULL == MediaControl)
    {
      return 0;
    }
	
  // Ensure the video is running
  RetVal	= MediaControl->GetState(0, &State);

 if (m_auto > 0.f){

	 //if the video is paused then start it running again
	 if (State != State_Running) 
	 {
		 RetVal	= MediaControl->Run();
		RetVal	= MediaControl->GetState(0, &State);
	 }

	 //set the rate of the clip
	 RetVal	= MediaSeeking->SetRate(m_auto);
  
  
  if (SampleGrabber != NULL && State == State_Running)
    {
      // Get the current position of the video
      if (MediaSeeking != NULL)
        {
          LONGLONG	CurrentPosition;

          RetVal	= MediaSeeking->GetCurrentPosition(&CurrentPosition);

          if (S_OK == RetVal)
            {
              // If the current position is >= the duration, reset the position to the 
              // beginning
              if (CurrentPosition >= m_Duration)
                {
                  
				LONGLONG	Current	= 0;
                  // Set the start position to 0, do not change the end position.
                  RetVal	= MediaSeeking->SetPositions(&Current, 
                                                             AM_SEEKING_AbsolutePositioning | AM_SEEKING_NoFlush, 
                                                             NULL, AM_SEEKING_NoPositioning);

                  m_image.newimage	= TRUE;
                }
			
              // Indicate the the image has changed.
              else if (CurrentPosition > m_LastFrame)
                {
                  m_image.newimage	= TRUE;
                }
            }
        }

      // If the video image has changed, copy it to the pixBlock buffer.
      if (TRUE == m_image.newimage)
        {
          RetVal	= SampleGrabber->GetCurrentBuffer(&frameSize, (long *)m_frame);

          if (RetVal != S_OK)
            {
              m_image.image.data	= NULL;
            }

          else
            {
              m_image.image.data	= m_frame;
              //m_image.image.fromBGR(m_frame);
            }
        }
    }

  }else{ 
	  
	    LONGLONG frameSeek;

		frameSeek = (LONGLONG) m_reqFrame;
		
		if (State == State_Running) RetVal	= MediaControl->Pause();

		//check if the playback is 'Paused' and don't keep asking for the same frame
		if (m_reqFrame == m_LastFrame)
		{
			m_image.newimage	= FALSE;
			return &m_image;
		}


		RetVal	= MediaSeeking->SetPositions(&frameSeek, 
                                                AM_SEEKING_AbsolutePositioning, 
                                                NULL, AM_SEEKING_NoPositioning);
		
		if (RetVal != S_OK)
		{
			post("filmDS: SetPositions failed");
		}

		RetVal	= SampleGrabber->GetCurrentBuffer(&frameSize, (long *)m_frame);

		if (RetVal != S_OK)
            {
              m_image.image.data	= NULL;
			  post("filmDS: GetCurrentBuffer failed");
            }

        else
            {
              m_image.image.data	= m_frame;
			  m_image.newimage	= TRUE;
              //m_image.image.fromBGR(m_frame);
			  m_LastFrame = m_reqFrame;
            }

  }

  return &m_image;
}

film::errCode filmDS :: changeImage(int imgNum, int trackNum){

	m_reqFrame = imgNum;

	if (m_reqFrame > m_Duration) return film::FAILURE;
  
  return film::SUCCESS;
}

HRESULT filmGetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
  IEnumPins  *pEnum;
  IPin       *pPin;
	
  // Enumerate the pins on the filter
  pFilter->EnumPins(&pEnum);
  
  if (NULL == pEnum)
    {
      return	E_FAIL;
    }

  // Get the next pin. Needs to be called initially to get the first pin.
  while (pEnum->Next(1, &pPin, 0) == S_OK)
    {
      PIN_DIRECTION	PinDirThis;
		
      // Get the direction of a pin
      pPin->QueryDirection(&PinDirThis);
		
      // Check if pin is the same type of pin requested. Will only return the first pin
      // of a certain direction.
      if (PinDir == PinDirThis)
        {
          // Release the interface
          pEnum->Release();
			
          // Return the pin, since it's the same direction as requested.
          *ppPin	= pPin;

          return	S_OK;
        }
		
      // Release the pin, since it's not the correct direction.
      pPin->Release();
    }
	
	
  // Release the interface
  pEnum->Release();
	
  return	E_FAIL;
}

HRESULT filmConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond)
{
  IPin	*pOut	= NULL;
  IPin	*pIn	= NULL;

  // Find the first output pin on the first filter
  HRESULT	RetVal	= filmGetPin(pFirst, PINDIR_OUTPUT, &pOut);
	
  if (RetVal != S_OK)
    {
      return	RetVal;
    }

  if (NULL == pOut)
    {
      return	E_FAIL;
    }

  // Find the first input pin on the second filter
  RetVal	= filmGetPin(pSecond, PINDIR_INPUT, &pIn);

  if (RetVal != S_OK)
    {
      return	RetVal;
    }

  if (NULL == pIn)
    {
      return	E_FAIL;
    }

  if (RetVal != S_OK) 
    {
      pOut->Release();
		
      return	E_FAIL;
    }
	
  // Attempt to connect the two pins.
  RetVal	= pGraph->Connect(pOut, pIn);

  // A filter having audio and video will return a VFW_S_PARTIAL_RENDER when attempting
  // to connect to a filter only having video (ie. the SampleGrabber filter)
  if (VFW_S_PARTIAL_RENDER == RetVal)
    {
      return	S_OK;
    }

  // Release the pins
  pIn->Release();
  pOut->Release();
	
  return	RetVal;
}

HRESULT filmAddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
  IMoniker * pMoniker;
  IRunningObjectTable *pROT;
  if (FAILED(GetRunningObjectTable(0, &pROT))) 
    {
      return E_FAIL;
    }

  WCHAR wsz[128];
  StringCchPrintfW(wsz, 128, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, 
                   GetCurrentProcessId());

  HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
  if (SUCCEEDED(hr)) 
    {
      hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
      pMoniker->Release();
    }

  pROT->Release();
  return hr;
}

void filmRemoveGraphFromRot(DWORD pdwRegister)
{
  IRunningObjectTable *pROT;

  if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
      pROT->Revoke(pdwRegister);
      pROT->Release();
    }
}
#endif
