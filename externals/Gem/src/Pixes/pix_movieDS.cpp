#include "Gem/GemConfig.h"

#if defined GEM_FILMBACKEND && GEM_FILMBACKEND == GEM_FILMBACKEND_DS

#include <atlbase.h>
#include <atlconv.h>
#include <streams.h>
#include <dvdmedia.h>

#pragma comment(lib, "strmbase.lib")
#pragma comment(lib, "strmiids.lib")

#include "pix_movieDS.h"
#include "Gem/Manager.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_movieDS, t_symbol *, A_DEFSYM);

pix_movieDS :: pix_movieDS(t_symbol *filename) :
	m_haveMovie(FALSE),
	m_frame(NULL),
	m_csize(0),
	m_xsize(0),
	m_ysize(0),
	FilterGraph(NULL),
	VideoFilter(NULL),
	SampleFilter(NULL),
	NullFilter(NULL),
	SampleGrabber(NULL),
	MediaControl(NULL),
	MediaSeeking(NULL),
	MediaPosition(NULL)
{
	HRESULT	RetVal;
	
	m_dataSize[0]	= m_csize;
	m_dataSize[1]	= m_xsize;
	m_dataSize[2]	= m_ysize;
	
	// Initialize COM
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
	
	if (strlen(filename->s_name) > 0)
	{
		openMess(filename, GL_RGBA);
	}
}

pix_movieDS::~pix_movieDS(void)
{
	closeMess();
	
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

void pix_movieDS::deleteBuffer()
{
}

void pix_movieDS::createBuffer()
{
}

// cleanup DS 
void pix_movieDS::closeMess(void)
{
	// Mark movie as unavailable
	m_haveMovie	= FALSE;

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
}

/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////

void pix_movieDS::openMess(t_symbol *filename, int format)
{
//	if (filename == x_filename)
//	{
//		return;
//	}

	x_filename	= filename;

	if (format)
	{
		m_colorspace	= format;
	}

	char buf[MAXPDSTRING];
	canvas_makefilename(getCanvas(), filename->s_name, buf, MAXPDSTRING);
  
	// Clean up any open files
	closeMess();

	realOpen(buf);

	if (FALSE == m_haveMovie)
	{
		return;
	}

	t_atom	ap[3];

	SETFLOAT(ap, m_numFrames);
	SETFLOAT(ap+1, m_xsize);
	SETFLOAT(ap+2, m_ysize);

	m_newFilm	= 1;
}

/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////
void pix_movieDS::realOpen(char *filename)
{
	WCHAR			WideFileName[MAXPDSTRING];
	HRESULT			RetVal;
	AM_MEDIA_TYPE	MediaType;
	BOOL			bFrameTime	= TRUE;
	GUID			Guid;
			
	// Convert c-string to Wide string.
	memset(&WideFileName, 0, MAXPDSTRING * 2);
	
	if (0 == MultiByteToWideChar(CP_ACP, 0, filename, strlen(filename), WideFileName, 
		MAXPDSTRING))
	{
		error("Unable to load %s", filename);
		
		return;
	}

	// Add a file source filter to the filter graph.
	RetVal	= FilterGraph->AddSourceFilter(WideFileName, L"SOURCE", &VideoFilter);
	
	if (RetVal != S_OK || NULL == VideoFilter)
	{
		error("Unable to render %s", filename);
		
		return;
	}

	// Create an instance of the sample grabber filter. The filter allows frames to be
	// buffered from a video source.  
	RetVal	= CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&SampleFilter);
    
	if (RetVal != S_OK || NULL == SampleFilter)
	{
		error("Unable to create SampleFilter interface %d", RetVal);
		
		return;
	}

 	// Add sample grabber filter to the filter graph.
	RetVal	= FilterGraph->AddFilter(SampleFilter, L"Sample Grabber");

	if (RetVal != S_OK)
	{
		error("Unable to add SampleFilter %d", RetVal);
		
		return;
	}

	// Find an interface to the SampleGrabber from the SampleGrabber filter. The 
	// SampleGrabber allows frames to be grabbed from the filter. SetBufferSamples(TRUE)
	// tells the SampleGrabber to buffer the frames. SetOneShot(FALSE) tells the 
	// SampleGrabber to continuously grab frames.  has GetCurrentBuffer() method
	RetVal	= SampleFilter->QueryInterface(IID_ISampleGrabber, (void **)&SampleGrabber);

	if (RetVal != S_OK || NULL == SampleGrabber)
	{
		error("Unable to create SampleGrabber interface %d", RetVal);
		
		return;
	}

	// Set the media type that the SampleGrabber wants.
	// MEDIATYPE_Video selects only video and not interleaved audio and video
	// MEDIASUBTYPE_RGB24 is the colorspace and format to deliver frames
	// MediaType.formattype is GUID_NULLsince it is handled later to get file info
	memset(&MediaType, 0, sizeof(AM_MEDIA_TYPE));
	MediaType.majortype		= MEDIATYPE_Video;
	MediaType.subtype		= MEDIASUBTYPE_RGB24;
	MediaType.formattype	= GUID_NULL;
	RetVal					= SampleGrabber->SetMediaType(&MediaType);

	// Set the SampleGrabber to return continuous frames
	RetVal	= SampleGrabber->SetOneShot(FALSE);
	
	if (RetVal != S_OK)
	{
		error("Unable to setup sample grabber %d", RetVal);
		
		return;
	}

	// Set the SampleGrabber to copy the data to a buffer. This only set to FALSE when a 
	// callback is used.
	RetVal	= SampleGrabber->SetBufferSamples(TRUE);
	
	if (RetVal != S_OK)
	{
		error("Unable to setup sample grabber %d", RetVal);
		
		return;
	}

	// Create the Null Renderer interface. The Null Renderer is used to disable rendering of a 
	// video stream to a window.
	RetVal	= CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&NullFilter);
    
	if (RetVal != S_OK || NULL == NullFilter)
	{
		error("Unable to create NullFilter interface %d", RetVal);
		
		return;
	}

	// Add the Null Renderer filter to the FilterGraph
	RetVal	= FilterGraph->AddFilter(NullFilter, L"NullRenderer");

	if (RetVal != S_OK)
	{
		error("Unable to add NullFilter %d", RetVal);
		
		return;
	}

	// DS filter chain is FileSource -> SampleGrabber -> NullRenderer
	// DS can put any neeeded filters in the chain for format or colorspace conversion
	// decompression or other transforms

	// Connect the SampleFilter to the VideoFilter
	RetVal	= movieConnectFilters(FilterGraph, VideoFilter, SampleFilter);

	if (RetVal != S_OK)
	{
		error("Unable to connect filters %d", RetVal);
		
		return;
	}

	// Connect the NullFilter to the SampleFilter
	RetVal	= movieConnectFilters(FilterGraph, SampleFilter, NullFilter);

	if (RetVal != S_OK)
	{
		error("Unable to connect filters %d", RetVal);
		
		return;
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

			return;
		}
	}
	
	// Get the duration of the video. Format will be in previously set time format. This is 
	// compatible with the value returned from GetCurrentPosition
	RetVal	= MediaSeeking->GetDuration(&m_Duration);
	
	if (RetVal != S_OK)
	{
		error("Unable to get video duration %d", RetVal);

		return;
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
		
		return;
	}

	// The SampleGrabber will only return video of the the 'FORMAT_VideoInfo' type.
	if (FORMAT_VideoInfo == MediaType.formattype && MediaType.pbFormat != NULL)
	{
		// Format returned is specific to the formattype.
		VIDEOINFOHEADER	*VideoInfo	= (VIDEOINFOHEADER *)MediaType.pbFormat;
		
		// Get size of the image from the BitmapInfoHeader returned in the VIDEOINFOHEADER.
		m_xsize		= VideoInfo->bmiHeader.biWidth;
		m_ysize		= VideoInfo->bmiHeader.biHeight;
		m_csize		= 3;
	}
	
	else
	{
		error("Invalid media type returned %s", filename);

		return;
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
			error("Unable to allocate memory for the video buffer %s", filename);

			return;
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
	
	post("xsize %d ysize %d csize %",m_xsize, m_ysize, m_csize);

	// Setup the pixBlock data based on the media type.
	// this is a guess at the fast past for pixels on Windows
	m_pixBlock.image.xsize	= m_xsize;
	m_pixBlock.image.ysize	= m_ysize;
	m_pixBlock.image.csize	= m_csize;
	m_pixBlock.image.format	= GL_BGR_EXT;
	m_pixBlock.image.type	= GL_UNSIGNED_BYTE;

	// Start the video stream
	RetVal	= MediaControl->Run();
	
	if (RetVal != S_OK && RetVal != S_FALSE)
	{
		error("Unable to start video %d", RetVal);
		
		return;
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
		
			return;
		}

		// Ensure the video is running
		else if (RetVal == S_OK && State_Running == FilterState)
		{
			break;
		}
	}
	
	// Sets the tex coords
	prepareTexture();

	// Set the last frame to -1 so it will show the first frame.
	m_LastFrame	= -1;
	
	m_haveMovie	= TRUE;	  
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_movieDS::getFrame()
{	
	long			frameSize	= m_ysize * m_xsize * m_csize;
	HRESULT			RetVal;
	OAFilterState	State;	
	
	// Initially set the image as unchanged
	m_pixBlock.newimage	= FALSE;
	
	// If the MediaControl interface is unavailable return.
	if (NULL == MediaControl)
	{
		return;
	}
	
	// Ensure the video is running
	RetVal	= MediaControl->GetState(0, &State);

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

					m_pixBlock.newimage	= TRUE;
				}
			
				// Indicate the the image has changed.
				else if (CurrentPosition > m_LastFrame)
				{
					m_pixBlock.newimage	= TRUE;
				}
			}
		}

		// If the video image has changed, copy it to the pixBlock buffer.
		if (TRUE == m_pixBlock.newimage)
		{
			RetVal	= SampleGrabber->GetCurrentBuffer(&frameSize, (long *)m_frame);

			if (RetVal != S_OK)
			{
				m_pixBlock.image.data	= NULL;
			}

			else
			{
				m_pixBlock.image.data	= m_frame;
			}
		}
	}
}	

void pix_movieDS::render(GemState *state)
{
	if (!m_haveMovie)
	{
		return;
	}

	getFrame();
	texFrame(state,1);
}

/////////////////////////////////////////////////////////
// on opening a file, prepare for texturing
//
/////////////////////////////////////////////////////////
void pix_movieDS::prepareTexture()
{

	//needs to be right side up
	m_coords[3].s	= m_xsize;
	m_coords[3].t	= 0.f;
    
	m_coords[2].s	= 0.f;
	m_coords[2].t	= 0.f;
    
	m_coords[1].s	= 0.f;
	m_coords[1].t	= m_ysize;
    
	m_coords[0].s	= m_xsize;
	m_coords[0].t	= m_ysize;
}

/////////////////////////////////////////////////////////
// setUpTextureState
//
/////////////////////////////////////////////////////////
void pix_movieDS::setUpTextureState()
{
	glTexParameterf(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_PRIORITY, 0.0);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_movieDS::texFrame(GemState *state, int doit)
{
	state->texture		= 1;
	state->texCoords	= m_coords;
	state->numTexCoords	= 4;
  

//check to see if rectangle textures are fast path currently
	glEnable(GL_TEXTURE_RECTANGLE_EXT);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, m_textureObj);
 
	if (m_pixBlock.image.csize != m_dataSize[0] ||
		m_pixBlock.image.xsize != m_dataSize[1] ||
		m_pixBlock.image.ysize != m_dataSize[2])
	{
		m_dataSize[0]	= m_pixBlock.image.csize;
		m_dataSize[1]	= m_pixBlock.image.xsize;
		m_dataSize[2]	= m_pixBlock.image.ysize;
		
		glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
			GL_RGBA,
			m_pixBlock.image.xsize,
			m_pixBlock.image.ysize, 
			0,
			m_pixBlock.image.format,
			m_pixBlock.image.type,
			m_pixBlock.image.data);
            
		post("new rectangle texture size - glTexImage2D");
    }
/*
    if
     (m_newFilm ){
            glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
		     GL_RGBA,
		     m_pixBlock.image.xsize,
		     m_pixBlock.image.ysize, 0,
		     //m_pixBlock.image.format,
		     //m_pixBlock.image.type,
                     GL_YCBCR_422_APPLE,
                     GL_UNSIGNED_SHORT_8_8_REV_APPLE,
		     m_pixBlock.image.data);
	    post("new film");
            m_newFilm = 0; //just to be sure
      } 
*/
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
	    0, 0,							// position
	    m_pixBlock.image.xsize,			// the x size of the data
	    m_pixBlock.image.ysize,			// the y size of the data
		m_pixBlock.image.format,
		m_pixBlock.image.type,
		m_pixBlock.image.data);			// the data + header offset
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_movieDS::postrender(GemState *state)
{
	glDisable(GL_TEXTURE_RECTANGLE_EXT);
}

void pix_movieDS::MovRate(float rate)
{
	HRESULT	RetVal;

	// If rate is set to 0.0, pause the video. MediaControl will not accept a rate of 0.0
	//need to have a way to unpause the video.  probably set a flag?
	if (0.0f == rate)
	{
		if (MediaControl != NULL)
		{
			RetVal	= MediaControl->Pause();
		}
	}
	
	// Set the playback speed.
	else
	{
		if (MediaSeeking != NULL)
		{
			//might need MediaControl->Run() if paused?
			RetVal = MediaControl->Run();  //should un pause
			RetVal	= MediaSeeking->SetRate(rate);
		}
	}
}

/////////////////////////////////////////////////////////
// changeImage
//
/////////////////////////////////////////////////////////
void pix_movieDS::changeImage(int imgNum, int trackNum)
{
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_movieDS::obj_setupCallback(t_class *classPtr)
{
	class_addcreator(reinterpret_cast<t_newmethod>(create_pix_movieDS,gensym("pix_movieDS"),A_DEFSYM,A_NULL));
	
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDS::openMessCallback),
		gensym("open"), A_SYMBOL, A_NULL);
	
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDS::changeImageCallback),
		gensym("img_num"), A_GIMME, A_NULL);
	
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDS::autoCallback),
		gensym("auto"), A_DEFFLOAT, A_NULL);
	
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDS::rateCallback),
		gensym("rate"), A_DEFFLOAT, A_NULL);
}

void pix_movieDS::openMessCallback(void *data, t_symbol *filename)
{
    GetMyClass(data)->openMess(filename,0);
}

void pix_movieDS::changeImageCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
	//  GetMyClass(data)->changeImage((int)imgNum);
	GetMyClass(data)->changeImage((argc<1)?0:atom_getint(argv), (argc<2)?0:atom_getint(argv+1));
}

void pix_movieDS::autoCallback(void *data, t_floatarg state)
{
	GetMyClass(data)->m_auto=!(!(int)state);
}

void pix_movieDS::rateCallback(void *data, t_floatarg state)
{
	GetMyClass(data)->MovRate(state);
}

void pix_movieDS::rectangleCallback(void *data, t_floatarg state)
{
	GetMyClass(data)->m_rectangle=(int)state;
}

HRESULT movieGetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
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

HRESULT movieConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond)
{
	IPin	*pOut	= NULL;
	IPin	*pIn	= NULL;

	// Find the first output pin on the first filter
	HRESULT	RetVal	= movieGetPin(pFirst, PINDIR_OUTPUT, &pOut);
	
	if (RetVal != S_OK)
	{
		return	RetVal;
	}

	if (NULL == pOut)
	{
		return	E_FAIL;
	}

	// Find the first input pin on the second filter
	RetVal	= movieGetPin(pSecond, PINDIR_INPUT, &pIn);

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

#endif /* HAVE_DIRECTSHOW */
