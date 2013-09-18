////////////////////////////////////////////////////////
//
// videoDS - Graphics Environment for Multimedia
//
// daniel@bogusfront.org
// zmoelnig@iem.at
//
// Implementation file 
//
//    Copyright (c) 2003 Daniel Heckenberg.
//    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "videoDS.h"
using namespace gem::plugins;
#include "Gem/RTE.h"
#include "plugins/PluginFactory.h"

#ifdef HAVE_DIRECTSHOW
REGISTER_VIDEOFACTORY("DS", videoDS);

# include <memory>
# include <Dvdmedia.h>
# include <streams.h>
# include <atlbase.h>
# include "DSgrabber.h"

# include <strsafe.h>
# include <comdef.h>

#define COMRELEASE(x) { if (x) x->Release(); x = NULL; }
//#define REGISTER_FILTERGRAPH 1

// Utility functions
void SetupCaptureDevice(ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase);
void dialogSource      (ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase);
void dialogFormat      (ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase);
void dialogDisplay     (ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase);
void dialogCrossbar    (ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase);

HRESULT FindCaptureDevice(int device, IBaseFilter ** ppSrcFilter);
HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond);
void GetBitmapInfoHdr(AM_MEDIA_TYPE* pmt, BITMAPINFOHEADER** ppbmih);
HRESULT GetPin(IBaseFilter *, PIN_DIRECTION, IPin **);
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
HRESULT SetAviOptions(IBaseFilter *ppf, InterleavingMode INTERLEAVE_MODE);
void RemoveGraphFromRot(DWORD pdwRegister);

/////////////////////////////////////////////////////////
//
// videoDS
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
videoDS :: videoDS(void)
  : videoBase("directshow", 0),
m_readIdx (0), m_lastreadIdx (0),
m_writeIdx(0), m_lastwriteIdx(0),
m_format(GL_BGR_EXT),
#ifdef USE_RECORDING
m_recording(false),
#endif

m_pGB(NULL),
m_pMC(NULL),
m_pME(NULL),
m_pMF(NULL),
m_pMS(NULL),
m_pMP(NULL),
SampleFilter(NULL),
NullFilter(NULL),
FileFilter(NULL),
SampleGrabber(NULL),
#ifdef DIRECTSHOW_LOGGING
LogFileHandle(NULL),
#endif
m_pCDbase(NULL),
m_pCG(NULL),
m_GraphRegister(0)
{
    // Initialize COM
    if(FAILED(CoInitialize(NULL))) {
        throw("could not initialise COM.");
    }

    m_width=720;
    m_height=576;

    // Initialize the input buffers
    for (int i = 0; i <= 2; i++) {
        m_pixBlockBuf[i].image.xsize=m_width;
        m_pixBlockBuf[i].image.ysize=m_height;
        m_pixBlockBuf[i].image.setCsizeByFormat(GL_RGBA);
        m_pixBlockBuf[i].image.reallocate();

        m_pixBlockBuf[i].newimage = 0;
        m_nPixDataSize[i] = 
            m_pixBlockBuf[i].image.xsize*
            m_pixBlockBuf[i].image.ysize*
            m_pixBlockBuf[i].image.csize;
    }

    m_image.image.xsize=m_width;
    m_image.image.ysize=m_height;
    m_image.image.setCsizeByFormat(GL_RGBA);
    m_image.image.reallocate();

#ifdef USE_RECORDING
    memset(m_filename, 0, MAXPDSTRING);
#endif

    provide("dv");
    provide("iidc");
    provide("analog");
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoDS :: ~videoDS(void)
{
    // Clean up the movie
    close();

    for (int i = 0; i <= 2; i++) {
        m_pixBlockBuf[i].image.allocate(0);
        m_pixBlockBuf[i].newimage = 0;
        m_nPixDataSize[i] = 0;
    }

    // Finished with COM
    CoUninitialize();
}

/////////////////////////////////////////////////////////
// open message
//
/////////////////////////////////////////////////////////
bool videoDS :: openDevice(gem::Properties&props)
{
    HRESULT			hr;
    AM_MEDIA_TYPE	MediaType;
    int device = m_devicenum;

    do  {
        // Get the interface for DirectShow's GraphBuilder
        if (FAILED(hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&m_pGB)))  {
            error("Could not get DShow GraphBuilder, hr 0x%X", hr);
            break;
        }

#ifdef DIRECTSHOW_LOGGING
        OFSTRUCT	OFStruct;

        LogFileHandle	= OpenFile("DirectShow.log", &OFStruct, OF_CREATE);

        if (LogFileHandle != NULL) {
            m_pGB->SetLogFile(LogFileHandle);
        }
#endif

        // Get the interface for DirectShow's CaptureGraphBuilder2 which allows the use of capture devices instead of file sources
        if (	FAILED(hr = (CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&m_pCG)))
            ||	FAILED(hr = m_pCG->SetFiltergraph(m_pGB))){
                error("Could not get DShow GraphBuilder, hr 0x%X", hr);
                break;
            }

            // Create the capture device.
            if (FAILED(hr = FindCaptureDevice(device, &m_pCDbase))) {
                error("Could not open device: %d\n", device);
                break;
            }

            // Create the SampleGrabber filter
            hr	= CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                IID_IBaseFilter, (void**)&SampleFilter);

            if (hr != S_OK || NULL == SampleFilter) {
                error("Unable to create SampleFilter interface %d", hr);
                return false;
            }

            // Query for the SampleGrabber interface in the SampleGrabber filter
            // Needed to grab the buffer using GetCurrentBuffer()
            hr	= SampleFilter->QueryInterface(IID_ISampleGrabber, (void **)&SampleGrabber);

            if (hr != S_OK || NULL == SampleGrabber) {
                error("Unable to create SampleGrabber interface %d", hr);
                return false;
            }

            // Set the media type that the SampleGrabber wants.
            // MEDIATYPE_Video selects only video and not interleaved audio and video
            // MEDIASUBTYPE_RGB24 is the colorspace and format to deliver frames
            // MediaType.formattype is GUID_NULL since it is handled later to get file info
            memset(&MediaType, 0, sizeof(AM_MEDIA_TYPE));
            MediaType.majortype		= MEDIATYPE_Video;
            MediaType.subtype		= MEDIASUBTYPE_RGB24;
            MediaType.formattype	= GUID_NULL;
            hr				= SampleGrabber->SetMediaType(&MediaType);

            // Set the SampleGrabber to return continuous frames
            hr	= SampleGrabber->SetOneShot(FALSE);

            if (hr != S_OK) {
                error("Unable to setup sample grabber %d", hr);
                return false;
            }

            // Set the SampleGrabber to copy the data to a buffer. This only set to FALSE when a 
            // callback is used.
            hr	= SampleGrabber->SetBufferSamples(TRUE);

            if (hr != S_OK) {
                error("Unable to setup sample grabber %d", hr);
                return false;
            }

            // Create the Null Renderer
            hr	= CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
                IID_IBaseFilter, (void**)&NullFilter);

            if (hr != S_OK || NULL == NullFilter) {
                error("Unable to create NullFilter interface %d", hr);
                return false;
            }

            // add the filters to the graph
            if (FAILED(hr = m_pGB->AddFilter(m_pCDbase, L"Capture Device")) || 
                FAILED(hr = m_pGB->AddFilter(SampleFilter, L"Sample Grabber")) ||
                FAILED(hr = m_pGB->AddFilter(NullFilter, L"Null Renderer"))) {
                    error("Could not add the filters to the graph, hr 0x%X", hr);
                    break;
                }

                // Automatically connect the Device filter to the NullFilter through the SampleFilter.
                // Additional filters may be added.
                // Try Interleaved Audio and Video first for DV input
                if (FAILED(hr = m_pCG->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved, 
                    m_pCDbase, SampleFilter, NullFilter))) {
                        //try Video only for devices with no audio
                        if (FAILED(hr = m_pCG->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, 
                            m_pCDbase, SampleFilter, NullFilter))) {
                                error("Unable to connect to SampleGrabber.");
                                return false;
                            }
                    }

                    // QueryInterface for DirectShow interfaces
                    if (FAILED(hr = (m_pGB->QueryInterface(IID_IMediaFilter, (void **)&m_pMF)))) {
                        error("Could not get media filter interface, hr 0x%X", hr);
                        break;
                    }

                    //MediaControl is used for Run, Stop, Pause and running state queries
                    if (FAILED(hr = (m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC)))) {
                        error("Could not get media control interface, hr 0x%X", hr);
                        break;
                    }

                    //not used right now
                    if (FAILED(hr = (m_pGB->QueryInterface(IID_IMediaEvent, (void **)&m_pME))))	{
                        error("Could not get media event interface, hr 0x%X", hr);
                        break;
                    }

                    //MediaSeeking for the end of a clip.  not really used here
                    if (FAILED(hr = (m_pGB->QueryInterface(IID_IMediaSeeking, (void **)&m_pMS)))){
                        error("Could not get media seeking interface, hr 0x%X", hr);
                        break;
                    }

                    //for the position of a clip.  not really used for device capture
                    if (FAILED(hr = (m_pGB->QueryInterface(IID_IMediaPosition, (void **)&m_pMP)))){
                        error("Could not get media position interface, hr 0x%X", hr);
                        break;
                    }

                    // Expose the filter graph so we can view it using GraphEdit
#ifdef REGISTER_FILTERGRAPH
                    if (FAILED(hr = AddGraphToRot(m_pGB, &m_GraphRegister))){
                        error("failed to register filter graph with ROT!  hr=0x%X", hr);
                        m_GraphRegister = 0;
                    }
#endif
                    // THIS KILLS FILE WRITING!! May improve latency on video preview/playback.
                    // Turn off the reference clock. 
                    //	if (FAILED(hr = m_pMF->SetSyncSource(NULL)))
                    //	{
                    //		error("failed to turn off the reference clock  hr=0x%X", hr);
                    //		break;
                    //	}

                    // Indicate that the video is ready.
#if USE_RECORDING
                    // JMZ ???
                    stopCapture();
#endif
                    return true;
    } while (0);

    closeDevice();
    return false;
}

////////////////////////////////////////////////////////
// close message
//
/////////////////////////////////////////////////////////
void videoDS :: closeDevice(void)
{
#ifdef DIRECTSHOW_LOGGING
    m_pGB->SetLogFile(NULL);
    CloseHandle((HANDLE)LogFileHandle);
#endif
    // Release the DirectShow interfaces
    COMRELEASE(m_pGB);
    COMRELEASE(m_pMC);
    COMRELEASE(m_pME);
    COMRELEASE(m_pMF);
    COMRELEASE(m_pMS);
    COMRELEASE(m_pMP);
    COMRELEASE(SampleFilter);
    COMRELEASE(NullFilter);
    COMRELEASE(FileFilter);
    COMRELEASE(m_pCDbase);
    COMRELEASE(m_pCG);

#ifdef REGISTER_FILTERGRAPH
    if (m_GraphRegister) {	
        HRESULT hr;
        RemoveGraphFromRot(m_GraphRegister);
        m_GraphRegister = 0;
    }
#endif
}
std::vector<std::string>videoDS :: dialogs(void) {
    std::vector<std::string>result;
    result.push_back("source");
    result.push_back("format");
    result.push_back("display");
    result.push_back("crossbar");
    return result;
}

////////////////////////////////////////////////////////
// enumerate message
//
/////////////////////////////////////////////////////////
std::vector<std::string>videoDS :: enumerate(void)
{
    std::vector<std::string>result;

    HRESULT hr;
    IBaseFilter * pSrc = NULL;

    IMoniker* pMoniker =NULL;
    ULONG cFetched;

    ICreateDevEnum* pDevEnum =NULL;
    IEnumMoniker* pClassEnum = NULL;

    do {
        // Create the system device enumerator
        hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
            IID_ICreateDevEnum, (void ** ) &pDevEnum);
        if (FAILED(hr)) {
            error("Couldn't create system enumerator!");
            break;
        }

        // Create an enumerator for the video capture devices
        hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
        if (FAILED(hr)) {
            error("Couldn't create class enumerator!");
            break;
        }

        // If there are no enumerators for the requested type, then 
        // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
        if (pClassEnum == NULL) {
            error("No video capture devices found!");
            break;
        }

        // Use the first video capture device on the device list.
        // Note that if the Next() call succeeds but there are no monikers,
        // it will return S_FALSE (which is not a failure).  Therefore, we
        // check that the return code is S_OK instead of using SUCCEEDED() macro.
        int devIndex = 0;
        while (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched))) {
            IPropertyBag *pPropBag;
            if (SUCCEEDED(hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag))) {
                // To retrieve the friendly name of the filter, do the following:
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                if (SUCCEEDED(hr)) {
                    std::string s=_bstr_t(varName.bstrVal);
                    result.push_back(s);
                    //post("Dev %d: %S", devIndex, varName.bstrVal);
                }
                VariantClear(&varName);

                COMRELEASE(pPropBag);
            }
            COMRELEASE(pMoniker);
            devIndex++;
        }
    } while (0);

    // Copy the found filter pointer to the output parameter.
    // Do NOT Release() the reference, since it will still be used
    // by the calling function.
    COMRELEASE(pDevEnum);
    COMRELEASE(pClassEnum);

    return result;
}

////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock* videoDS :: getFrame(void)
{
    // Copy the buffer from the camera buffer to the texture buffer
    copyBuffer();

    m_readIdx = m_lastwriteIdx;
    if (m_nPixDataSize[m_readIdx]){
        m_image.newimage=m_pixBlockBuf[m_readIdx].newimage;
        m_image.image.xsize=m_pixBlockBuf[m_readIdx].image.xsize;
        m_image.image.ysize=m_pixBlockBuf[m_readIdx].image.ysize;
        switch (m_pixBlockBuf[m_readIdx].image.format){
        case GL_BGR_EXT:
        default:
            m_image.image.fromBGR(m_pixBlockBuf[m_readIdx].image.data);
        }
        m_image.newimage=true;

        return &m_image;
    }
    return NULL;
}

/* copies the image from DS into the current m_pixBlockBuf */
void videoDS :: copyBuffer(void)
{
    HRESULT	hr;
    long	SampleSize;	

    // Get the media type
    AM_MEDIA_TYPE	pmt;
    int readIdx = m_readIdx;

    if ((m_writeIdx = (m_lastwriteIdx + 1) % 3) == readIdx)
        m_writeIdx = (readIdx + 1) % 3;

    // Get the current buffer size from the SampleGrabber.
    if (SampleGrabber != NULL) {
        hr	= SampleGrabber->GetCurrentBuffer(&SampleSize, NULL);

        if (hr != S_OK) return;
    }		

    // Check for a format change.
    if (NULL == SampleGrabber || FAILED(hr = SampleGrabber->GetConnectedMediaType(&pmt))) {
        error("could not get sample media type.");
        close();
        return;
    }

    if (S_OK == hr) {
        BITMAPINFOHEADER* pbmih;
        GetBitmapInfoHdr(&pmt, &pbmih);
        m_width = pbmih->biWidth;
        m_height = pbmih->biHeight;
        m_format = GL_BGR_EXT;
        FreeMediaType(pmt);	// is this necessary?!	
    }

    m_pixBlockBuf[m_writeIdx].image.xsize = m_width;
    m_pixBlockBuf[m_writeIdx].image.ysize = m_height;
    m_pixBlockBuf[m_writeIdx].image.setCsizeByFormat(m_format);
    m_pixBlockBuf[m_writeIdx].image.reallocate();
    m_pixBlockBuf[m_writeIdx].image.reallocate(SampleSize);
    m_nPixDataSize[m_writeIdx] = SampleSize;

    // Get the current buffer from the SampleGrabber.
    if (SampleGrabber != NULL) {
        hr = SampleGrabber->GetCurrentBuffer(&SampleSize, 
            (long *)m_pixBlockBuf[m_readIdx].image.data);
        if (hr != S_OK) return;
        m_pixBlockBuf[m_writeIdx].newimage = 1;
    }		

    m_lastwriteIdx = m_writeIdx;
}


////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void videoDS :: releaseFrame(void)
{
    if (!m_haveVideo || !m_capturing)return;

    m_pixBlockBuf[m_readIdx].newimage = 0;
    m_lastreadIdx = m_readIdx;
}

////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoDS :: startTransfer(void)
{
    HRESULT hr;

    m_readIdx = 0;
    m_lastreadIdx = 0;
    m_writeIdx = 1;
    m_lastwriteIdx = 0;

    // Get the stream characteristics
    AM_MEDIA_TYPE mt;
    BITMAPINFOHEADER* pbmih;
    if (NULL == SampleGrabber || FAILED(hr = SampleGrabber->GetConnectedMediaType(&mt))) {
        error("Could not get connect media type, hr 0x%X", hr);
        return false;
    }
    GetBitmapInfoHdr(&mt, &pbmih);
    m_width = pbmih->biWidth;
    m_height = pbmih->biHeight;
    m_format = GL_BGR_EXT;

    //starts the graph rendering
    if (FAILED(hr = m_pMC->Run())) {
        error("Could not start graph playback, hr 0x%X", hr);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoDS :: stopTransfer(void)
{
    HRESULT hr;
    if (FAILED(hr = m_pMC->Stop())) {
        error("Could not stop graph playback, hr 0x%X", hr);
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////
// dvMess
//
/////////////////////////////////////////////////////////
bool videoDS :: setDimen(int x, int y, int leftmargin, int rightmargin,
                         int topmargin, int bottommargin)
{
    /*
    norm                       NTSC        PAL
    DVRESOLUTION_FULL	    720 x 480	720 x 576
    DVRESOLUTION_HALF	    360 x 240	360 x 288
    DVRESOLUTION_QUARTER    180 x 120	180 x 144
    DVRESOLUTION_DC	     88 x 60     88 x 72
    */

    // 5*yPAL = 6*yNTSC

    bool pal=true; // how to know this?

    float y_fac = pal?1.:(5./6.);
    int resolution=DVRESOLUTION_FULL;
    if     ((x<= 88) && (y<=( 72*y_fac)))resolution=DVRESOLUTION_DC;
    else if((x<=180) && (y<=(144*y_fac)))resolution=DVRESOLUTION_QUARTER;
    else if((x<=360) && (y<=(288*y_fac)))resolution=DVRESOLUTION_HALF;
    else                                 resolution=DVRESOLUTION_FULL;

    HRESULT hr = S_OK;

    if(stop()) {
        IIPDVDec	*pDV=NULL;
        if (SUCCEEDED(hr = (m_pCG->FindInterface(NULL, NULL, m_pCDbase, IID_IIPDVDec, (void **)&pDV)))) {
            hr = pDV->put_IPDisplay(resolution);

            if (FAILED(hr)) {
                error("Could not set decoder resolution.");
            }
        }
        if(pDV)
            pDV->Release();

        start();      
    }
return true;
}

////////////////////////////////////////////////////////
// colorspace message
//
/////////////////////////////////////////////////////////
bool videoDS :: setColor(int format)
{
    if(format)m_image.image.setCsizeByFormat(format);
    return true;
}

////////////////////////////////////////////////////
// dialogMess
//
/////////////////////////////////////////////////////////
bool videoDS :: dialog(std::vector<std::string>dlg)
{
    HRESULT hr;  
    if (!m_haveVideo) return true;

    bool running=stop();

    if(dlg.empty()) {
        SetupCaptureDevice(m_pCG, m_pCDbase);
    } else {
        int i;
        for(i=0; i<dlg.size(); i++) {
            std::string type=dlg[i];

            if(type=="source")
                dialogSource(m_pCG, m_pCDbase);
            else if(type=="format")
                dialogFormat(m_pCG, m_pCDbase);
            else if(type=="display")
                dialogDisplay(m_pCG, m_pCDbase);
            else if(type=="crossbar")
                dialogCrossbar(m_pCG, m_pCDbase);
            else
                error ("dialog not known");
        }
    }

    if(running)start();
    return true;
}



#ifdef USE_RECORDING
void videoDS :: startCapture(void)
{
    HRESULT	hr;
    WCHAR	WideFileName[MAXPDSTRING];
    if (FALSE == m_recording && m_pCG != NULL && m_haveVideo) {
        // Convert filename to wide chars
        memset(&WideFileName, 0, MAXPDSTRING * 2);
        if (0 == MultiByteToWideChar(CP_ACP, 0, m_filename, strlen(m_filename), WideFileName, 
            MAXPDSTRING)) {
                error("Unable to capture to %s", m_filename);
                return;
            }
            // Set filename of output AVI. Returns pointer to a File Writer filter.
            if (FAILED(hr = m_pCG->SetOutputFileName(&MEDIASUBTYPE_Avi, WideFileName, 
                &FileFilter, NULL))) {
                    error("Unable to set output filename.");
                    return;
                }
                // Set AVI output option for interleaving.
                if (FAILED(hr = SetAviOptions(FileFilter, INTERLEAVE_NONE))) {
                    error("Unable to set avi options.");
                    return;
                }
                // Connect the Capture Device filter to the File Writer filter. Try using 
                //	MEDIATYPE_Interleaved first, else default to MEDIATYPE_Video.
                if (FAILED(hr = m_pCG->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,  
                    m_pCDbase, NULL, FileFilter))) {
                        if (FAILED(hr = m_pCG->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
                            m_pCDbase, NULL, FileFilter))) {
                                error("Unable to record to avi.");
                                return;
                            }
                    }
                    m_recording	= true;
    }
}
void videoDS :: stopCapture(void)
{
    HRESULT	RetVal;

    if (m_recording && m_haveVideo) {
        IBaseFilter	*Filter;

        // Remove the File Writer filter, if available.
        //This is probably where DS releases the written AVI file
        RetVal	= m_pGB->FindFilterByName(L"File Writer", &Filter);

        if (S_OK == RetVal) {
            m_pGB->RemoveFilter(Filter);
        }

        // Remove the AVI Mux filter, if available.
        RetVal	= m_pGB->FindFilterByName(L"Mux", &Filter);

        if (S_OK == RetVal) {
            m_pGB->RemoveFilter(Filter);
        }

        m_recording	= false;
    }
}
void videoDS :: fileMess(t_symbol *filename)
{
    _snprintf(m_filename, MAXPDSTRING, "%s", findFile(filename->s_name).c_str());
}
void videoDS :: recordMess(int state)
{
    if (NULL==m_filename || 0 == m_filename[0]) {
        error("No filename passed");
        return;
    }

    stopTransfer();
    if (state) 
        startCapture();
    else
        stopCapture();
    startTransfer();
}
#endif /* recording */












#if 0
"device" -> openMess
"open"   -> openMess
"close"  -> closeMess
"dv"     -> dvMess
"record" -> recordMess
"file"   -> fileMess
float    -> captureOnOff
#endif


// From Microsoft sample:
//
// Let's talk about UI for a minute.  There are many programmatic interfaces
// you can use to program a capture filter or related filter to capture the
// way you want it to.... eg:  IAMStreamConfig, IAMVideoCompression,
// IAMCrossbar, IAMTVTuner, IAMTVAudio, IAMAnalogVideoDecoder, IAMCameraControl,
// IAMVideoProcAmp, etc.
//
// But you probably want some UI to let the user play with all these settings.
// For new WDM-style capture devices, we offer some default UI you can use.
// The code below shows how to bring up all of the dialog boxes supported 
// by any capture filters.
//
// The following code shows you how you can bring up all of the
// dialogs supported by a particular object at once on a big page with lots
// of thumb tabs.  You do this by starting with an interface on the object that
// you want, and using ISpecifyPropertyPages to get the whole list, and
// OleCreatePropertyFrame to bring them all up.  This way you will get custom
// property pages a filter has, too, that are not one of the standard pages that
// you know about.  There are at least 9 objects that may have property pages.
// Your app already has 2 of the object pointers, the video capture filter and
// the audio capture filter (let's call them pVCap and pACap)
// 1.  The video capture filter - pVCap
// 2.  The video capture filter's capture pin - get this by calling
//     FindInterface(&PIN_CATEGORY_CAPTURE, pVCap, IID_IPin, &pX);
// 3.  The video capture filter's preview pin - get this by calling
//     FindInterface(&PIN_CATEGORY_PREVIEW, pVCap, IID_IPin, &pX);
// 4.  The audio capture filter - pACap
// 5.  The audio capture filter's capture pin - get this by calling
//     FindInterface(&PIN_CATEGORY_CAPTURE, pACap, IID_IPin, &pX);
// 6.  The crossbar connected to the video capture filter - get this by calling
//     FindInterface(NULL, pVCap, IID_IAMCrossbar, &pX);
// 7.  There is a possible second crossbar to control audio - get this by 
//     looking upstream of the first crossbar like this:
//     FindInterface(&LOOK_UPSTREAM_ONLY, pX, IID_IAMCrossbar, &pX2);
// 8.  The TV Tuner connected to the video capture filter - get this by calling
//     FindInterface(NULL, pVCap, IID_IAMTVTuner, &pX);
// 9.  The TV Audio connected to the audio capture filter - get this by calling
//     FindInterface(NULL, pACap, IID_IAMTVAudio, &pX);
// 10. We have a helper class, CCrossbar, which makes the crossbar issue less
//     confusing.  In fact, although not supported here, there may be more than
//     two crossbars, arranged in many different ways.  An application may not
//     wish to have separate dialogs for each crossbar, but instead hide the
//     complexity and simply offer the user a list of inputs that can be chosen.
//     This list represents all the unique inputs from all the crossbars.
//     The crossbar helper class does this and offers that list as #10.  It is
//     expected that an application will either provide the crossbar dialogs
//     above (#6 and #7) OR provide the input list (this #10), but not both.
//     That would be confusing because if you select an input using dialog 6 or
//     7 the input list here in #10 won't know about your choice.
//
// Your last choice for UI is to make your own pages, and use the results of 
// your custom page to call the interfaces programmatically.


void SetupCaptureDevice(ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase) 
{
    HRESULT hr;

    // Check for old style VFW dialogs
    IAMVfwCaptureDialogs *pDlg;
    if (SUCCEEDED(hr = (pCG->FindInterface(NULL, NULL, pCDbase, IID_IAMVfwCaptureDialogs, (void **)&pDlg)))) {
        if (S_OK == (pDlg->HasDialog(VfwCaptureDialog_Source)))
            if (FAILED(hr = pDlg->ShowDialog(VfwCaptureDialog_Source, NULL)))
                error("Could not show VFW Capture Source Dialog");
        if (S_OK == (pDlg->HasDialog(VfwCaptureDialog_Format)))
            if (FAILED(hr = pDlg->ShowDialog(VfwCaptureDialog_Format, NULL)))
                error("Could not show VFW Capture Format Dialog");
        if (S_OK == (pDlg->HasDialog(VfwCaptureDialog_Display)))
            if (FAILED(hr = pDlg->ShowDialog(VfwCaptureDialog_Display, NULL)))
                error("Could not show VFW Capture Display Dialog");
        pDlg->Release();
    }

    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    // 1. the video capture filter itself
    hr = pCDbase->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
    if(hr == S_OK) {
        hr = pSpec->GetPages(&cauuid);
        if (hr == S_OK && cauuid.cElems > 0) {
            hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                (IUnknown **)&pCDbase, cauuid.cElems,
                (GUID *)cauuid.pElems, 0, 0, NULL);
            CoTaskMemFree(cauuid.pElems);
        }
        pSpec->Release();
    }


    // 2.  The video capture capture pin
    IAMStreamConfig *pSC;
    hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved,
        pCDbase, IID_IAMStreamConfig, (void **)&pSC);
    if(hr != S_OK)
        hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video, pCDbase,
        IID_IAMStreamConfig, (void **)&pSC);

    if(hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if(hr == S_OK && cauuid.cElems > 0) {
                hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                    (IUnknown **)&pSC, cauuid.cElems,
                    (GUID *)cauuid.pElems, 0, 0, NULL);
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pSC->Release();
    }

    // 4 & 5.  The video crossbar, and a possible second crossbar
    IAMCrossbar *pX, *pX2;
    IBaseFilter *pXF;
    hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved, pCDbase,
        IID_IAMCrossbar, (void **)&pX);
    if(hr != S_OK)
        hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video, pCDbase,
        IID_IAMCrossbar, (void **)&pX);

    if(hr == S_OK) {
        hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
        if(hr == S_OK) {
            hr = pX->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
            if(hr == S_OK) {
                hr = pSpec->GetPages(&cauuid);
                if(hr == S_OK && cauuid.cElems > 0) {
                    hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                        (IUnknown **)&pX, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);
                    CoTaskMemFree(cauuid.pElems);
                }
                pSpec->Release();
            }
            hr = pCG->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pXF,
                IID_IAMCrossbar, (void **)&pX2);
            if(hr == S_OK) {
                hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK) {
                    hr = pSpec->GetPages(&cauuid);
                    if(hr == S_OK && cauuid.cElems > 0) {
                        hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                            (IUnknown **)&pX2, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                    }
                    pSpec->Release();
                }
                pX2->Release();
            }
            pXF->Release();
        }
        pX->Release();
    }
}

void dialogSource(ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase) {
    HRESULT hr;
    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    IAMVfwCaptureDialogs *pDlg = NULL;
    hr = pCG->FindInterface(NULL, NULL, pCDbase, IID_IAMVfwCaptureDialogs, (void **)&pDlg);

    if (pDlg) {
        if (S_OK == (pDlg->HasDialog(VfwCaptureDialog_Source))) {
            if (FAILED(hr = pDlg->ShowDialog(VfwCaptureDialog_Source, NULL))) {
                error("Could not show VFW Capture Source Dialog");
            }
        }
    } else {
        hr = pCDbase->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
                hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                    (IUnknown **)&pCDbase, cauuid.cElems,
                    (GUID *)cauuid.pElems, 0, 0, NULL);
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
    }
}

void dialogFormat(ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase) {
    HRESULT hr;
    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    IAMVfwCaptureDialogs *pDlg = NULL;
    hr = pCG->FindInterface(NULL, NULL, pCDbase, IID_IAMVfwCaptureDialogs, (void **)&pDlg);

    if (pDlg)	{
        if (S_OK == (pDlg->HasDialog(VfwCaptureDialog_Format)))
            if (FAILED(hr = pDlg->ShowDialog(VfwCaptureDialog_Format, NULL))) {
                error("Could not show VFW Capture Format Dialog");
            }
    } else {
        IAMStreamConfig *pSC;
        hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Interleaved,
            pCDbase, IID_IAMStreamConfig, (void **)&pSC);
        if(hr != S_OK)
            hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, pCDbase,
            IID_IAMStreamConfig, (void **)&pSC);
        if(hr == S_OK) {
            hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
            if(hr == S_OK) {
                hr = pSpec->GetPages(&cauuid);
                if(hr == S_OK && cauuid.cElems > 0) {
                    hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                        (IUnknown **)&pSC, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);
                    CoTaskMemFree(cauuid.pElems);
                }
                pSpec->Release();
            }
            pSC->Release();
        }
    }
}


void dialogDisplay(ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase) {
    HRESULT hr;
    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    IAMVfwCaptureDialogs *pDlg = NULL;
    hr = pCG->FindInterface(NULL, NULL, pCDbase, IID_IAMVfwCaptureDialogs, (void **)&pDlg);

    if (pDlg) {
        if (S_OK == (pDlg->HasDialog(VfwCaptureDialog_Display))) {
            if (FAILED((hr = pDlg->ShowDialog(VfwCaptureDialog_Display, NULL)))) {
                error("Could not show VFW Capture Display Dialog");
            }
        } else
            error("No display dialog for this device");
    }
}

void dialogCrossbar(ICaptureGraphBuilder2* pCG, IBaseFilter * pCDbase) {
    HRESULT hr;
    ISpecifyPropertyPages *pSpec=NULL;
    CAUUID cauuid;
    IAMCrossbar *pX=NULL, *pX2=NULL;
    IBaseFilter *pXF=NULL;

    hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved, pCDbase,
        IID_IAMCrossbar, (void **)&pX);
    if(hr != S_OK)
        hr = pCG->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video, pCDbase,
        IID_IAMCrossbar, (void **)&pX);

    if(hr == S_OK) {
        hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
        if(hr == S_OK) {
            hr = pX->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
            if(hr == S_OK) {
                hr = pSpec->GetPages(&cauuid);
                if(hr == S_OK && cauuid.cElems > 0) {
                    hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                        (IUnknown **)&pX, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);
                    CoTaskMemFree(cauuid.pElems);
                }
                pSpec->Release();
            }
            hr = pCG->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pXF,
                IID_IAMCrossbar, (void **)&pX2);
            if(hr == S_OK) {
                hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK) {
                    hr = pSpec->GetPages(&cauuid);
                    if(hr == S_OK && cauuid.cElems > 0) {
                        hr = OleCreatePropertyFrame(NULL, 30, 30, NULL, 1,
                            (IUnknown **)&pX2, cauuid.cElems,
                            (GUID *)cauuid.pElems, 0, 0, NULL);
                        CoTaskMemFree(cauuid.pElems);
                    }
                    pSpec->Release();
                }
                pX2->Release();
            }
            pXF->Release();
        }
        pX->Release();
    }
}


HRESULT
FindCaptureDevice(int device, IBaseFilter ** ppSrcFilter)
{
    HRESULT hr;
    IBaseFilter * pSrc = NULL;

    IMoniker* pMoniker =NULL;
    ULONG cFetched;

    ICreateDevEnum* pDevEnum =NULL;
    IEnumMoniker* pClassEnum = NULL;

    do {
        // Create the system device enumerator
        hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
            IID_ICreateDevEnum, (void ** ) &pDevEnum);
        if (FAILED(hr)) {
            error("Couldn't create system enumerator!");
            break;
        }

        // Create an enumerator for the video capture devices

        hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
        if (FAILED(hr)){
            error("Couldn't create class enumerator!");
            break;
        }

        // If there are no enumerators for the requested type, then
        // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
        if (pClassEnum == NULL) {
            error("No video capture devices found!");
            hr = E_FAIL;
            break;
        }

        // Use the first video capture device on the device list.
        // Note that if the Next() call succeeds but there are no monikers,
        // it will return S_FALSE (which is not a failure).  Therefore, we
        // check that the return code is S_OK instead of using SUCCEEDED() macro.
        int devIndex = 0;
        while(		(S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
            &&	(devIndex <= device)) {
                if (devIndex == device) {
                    // Bind Moniker to a filter object
                    hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
                    if (FAILED(hr)) {
                        error("Couldn't bind moniker to filter object!");
                    }
                }
                COMRELEASE(pMoniker);
                devIndex++;
            }
    } while (0);

    // Copy the found filter pointer to the output parameter.
    // Do NOT Release() the reference, since it will still be used
    // by the calling function.
    COMRELEASE(pDevEnum);
    COMRELEASE(pClassEnum);

    *ppSrcFilter = pSrc;

    return hr;
}

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond)
{
    IPin *pOut = NULL, *pIn = NULL;
    HRESULT hr = GetPin(pFirst, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) return hr;
    hr = GetPin(pSecond, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) {
        pOut->Release();
        return E_FAIL;
    }
    hr = pGraph->Connect(pOut, pIn);
    pIn->Release();
    pOut->Release();
    return hr;
}

void GetBitmapInfoHdr(AM_MEDIA_TYPE* pmt, BITMAPINFOHEADER** ppbmih) {
    *ppbmih = NULL;

    if (IsEqualGUID(pmt->formattype, FORMAT_VideoInfo) ||
        IsEqualGUID(pmt->formattype, FORMAT_MPEGVideo)) {

            VIDEOINFOHEADER * pVideoFormat = (VIDEOINFOHEADER *) pmt->pbFormat;
            *ppbmih = &(((VIDEOINFOHEADER *) pmt->pbFormat)->bmiHeader);
        } else if (	IsEqualGUID(pmt->formattype, FORMAT_MPEG2Video) ||
            IsEqualGUID(pmt->formattype, FORMAT_VideoInfo2)) {
                *ppbmih = &(((VIDEOINFOHEADER2 *) pmt->pbFormat)->bmiHeader);
            } else {
                error("Unknown media format");
                return;
            }
}

HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
    IEnumPins  *pEnum;
    IPin       *pPin;
    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK) {
        PIN_DIRECTION PinDirThis;

        pPin->QueryDirection(&PinDirThis);
        if (PinDir == PinDirThis) {
            pEnum->Release();
            *ppPin = pPin;
            return S_OK;
        }
        pPin->Release();
    }
    pEnum->Release();
    return E_FAIL;
}


HRESULT SetAviOptions(IBaseFilter *ppf, InterleavingMode INTERLEAVE_MODE)
{
    HRESULT hr;
    CComPtr<IConfigAviMux>        pMux           = NULL;
    CComPtr<IConfigInterleaving>  pInterleaving  = NULL;

    ASSERT(ppf);
    if (!ppf)
        return E_POINTER;

    // QI for interface AVI Muxer
    if (FAILED(hr = ppf->QueryInterface(IID_IConfigAviMux, reinterpret_cast<PVOID *>(&pMux)))) {
        error("IConfigAviMux failed.");
    }

    if (FAILED(hr = pMux->SetOutputCompatibilityIndex(TRUE))) {
        error("SetOutputCompatibilityIndex failed.");
    }

    // QI for interface Interleaving
    if (FAILED(hr = ppf->QueryInterface(IID_IConfigInterleaving, reinterpret_cast<PVOID *>(&pInterleaving)))) {
        error("IConfigInterleaving failed.");
    }

    // put the interleaving mode (full, none, half)
    if (FAILED(pInterleaving->put_Mode(INTERLEAVE_MODE))) {
        error("put_Mode failed.");
    }

    return hr;
}

HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }

    WCHAR wsz[128];
    StringCchPrintfW(wsz, 128, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph,
        GetCurrentProcessId());

    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}

void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}
#else /* !HAVE_DIRECTSHOW */
videoDS ::  videoDS() : videoBase("") {}
videoDS :: ~videoDS() {}
#endif
