////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "LICENSE.txt"
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined HAVE_LIBPYLON
# define HAVE_PYLON
#endif

#ifdef HAVE_PYLON

#include "videoPYLON.h"
#include "plugins/PluginFactory.h"

#include <sstream>
using namespace gem::plugins;

#define NUM_BUFFERS 8

#include "Gem/RTE.h"
#include "Gem/Exception.h"

#if 0
# define debug ::post
#else
# define debug
#endif

using namespace Basler_GigECameraParams;
using namespace Basler_GigEStreamParams;


#include "CameraProperties.h"
#include "StreamGrabberProperties.h"

// Constructor allocates the image buffer

class videoPYLON::CGrabBuffer {
  static int buffercount;
public:

  CGrabBuffer(const size_t ImageSize)
    : m_pBuffer(NULL)  {
    buffercount++;
    m_pBuffer = new uint8_t[ ImageSize ];
    if (NULL == m_pBuffer)
      {
        GenICam::GenericException e("Not enough memory to allocate image buffer", __FILE__, __LINE__);
        throw e;
      }
  }

  // Freeing the memory
  ~CGrabBuffer() {
    if (NULL != m_pBuffer)
      delete[] m_pBuffer;
    buffercount--;
  }
  uint8_t* GetBufferPointer(void) { return m_pBuffer; }
  Pylon::StreamBufferHandle GetBufferHandle(void) { return m_hBuffer; }
  void SetBufferHandle(Pylon::StreamBufferHandle hBuffer) { m_hBuffer = hBuffer; };
  
protected:
  uint8_t *m_pBuffer;
  Pylon::StreamBufferHandle m_hBuffer;

};
int videoPYLON::CGrabBuffer::buffercount=0;


struct videoPYLON::Converter {
#ifdef HAVE_LIBPYLONUTILITY
  Pylon::CPixelFormatConverter*converter;
#endif
  imageStruct image;
  struct Pylon::SImageFormat inFormat;
  struct Pylon::SOutputImageFormat outFormat;

  Converter(void)
#ifdef HAVE_LIBPYLONUTILITY
    : converter(NULL)
#endif
  {
    
  }
  ~Converter(void) {
    destroyConverter();
  }

  void destroyConverter(void) {
#ifdef HAVE_LIBPYLONUTILITY
    if(converter) {
      delete converter;
      converter=NULL;
    }
#endif
  }

  void makeConverter(const struct Pylon::SImageFormat&format) {
    destroyConverter();
    
    using namespace Pylon;
    
    bool rgba_out=false;
    bool need_converter=true;
    
#ifdef HAVE_LIBPYLONUTILITY
    switch(format.PixelFormat) {
    case PixelType_Mono8:
    case PixelType_Mono8signed:
      /* no converter needed */
      need_converter=false;
      rgba_out=false;
      break;

    case PixelType_RGBA8packed:
      /* no converter needed */
      need_converter=false;
      rgba_out=true;
      break;

    case PixelType_Mono10:
    case PixelType_Mono12:
    case PixelType_Mono16:
      converter= new	Pylon::CPixelFormatConverterGamma ();
      // converter= new	Pylon::CPixelFormatConverterTruncate ();
      rgba_out=false;
      break;

    case PixelType_Mono10packed:
    case PixelType_Mono12packed:
      converter= new	Pylon::CPixelFormatConverterGammaPacked ();
      // converter= new	Pylon::CPixelFormatConverterTruncatePacked ();
      rgba_out=false;
      break;
    
    case PixelType_BayerGR8:
    case PixelType_BayerRG8:
    case PixelType_BayerGB8:
    case PixelType_BayerBG8:
    case PixelType_BayerGR10:
    case PixelType_BayerRG10:
    case PixelType_BayerGB10:
    case PixelType_BayerBG10:
    case PixelType_BayerGR12:
    case PixelType_BayerRG12:
    case PixelType_BayerGB12:
    case PixelType_BayerBG12:
    case PixelType_BayerGB12Packed:
    case PixelType_BayerGR12Packed:
    case PixelType_BayerRG12Packed:
    case PixelType_BayerBG12Packed:
    case PixelType_BayerGR16:
    case PixelType_BayerRG16:
    case PixelType_BayerGB16:
    case PixelType_BayerBG16:
      converter= new	Pylon::CPixelFormatConverterBayer ();
      rgba_out=true;
      break;

    case PixelType_RGB8planar:
    case PixelType_RGB10planar:
    case PixelType_RGB12planar:
    case PixelType_RGB16planar:

    case PixelType_RGB8packed:
    case PixelType_BGR8packed:
    case PixelType_BGRA8packed:
    case PixelType_RGB10packed:
    case PixelType_BGR10packed:
    case PixelType_RGB12packed:
    case PixelType_BGR12packed:
    case PixelType_BGR10V1packed:
    case PixelType_BGR10V2packed:
      converter= new	Pylon::CPixelFormatConverterRGB ();
      rgba_out=true;
      break;

    case PixelType_YUV422_YUYV_Packed:
      converter= new	Pylon::CPixelFormatConverterYUV422YUYV();
      rgba_out=true;
      break;

    case PixelType_YUV422packed:
      converter= new	Pylon::CPixelFormatConverterYUV422();
      rgba_out=true;
      break;

    case PixelType_YUV411packed:
    case PixelType_YUV444packed:
    case PixelType_RGB12V1packed:
      // ?
      break;
    }

    if(NULL==converter && need_converter) {
      error("PYLON: could not find a converter for given colorspace");
    }
    if(converter)
      converter->Init(format);
#endif
    if(rgba_out) {
      image.setCsizeByFormat(GL_RGBA);
    } else {
      image.setCsizeByFormat(GL_LUMINANCE);
    }
  }


  static struct Pylon::SImageFormat getInFormat(const Pylon::GrabResult&Result) {
    using namespace Pylon;
    struct SImageFormat imageFormat;
    const enum PixelType pixelType=Result.GetPixelType();
    imageFormat.Width =Result.GetSizeX();
    imageFormat.Height=Result.GetSizeY();
    imageFormat.PixelFormat=pixelType;
    imageFormat.LinePitch=IsPacked(pixelType)?
      0:
      (7+Result.GetSizeX()*BitPerPixel(pixelType)) >>3;
    
    return imageFormat;
  }

  static struct Pylon::SOutputImageFormat getOutFormat(const imageStruct&img) {
    using namespace Pylon;
    struct SOutputImageFormat imageFormat;
    
    switch(img.format) {
    case GL_RGBA:
      imageFormat.LinePitch=img.xsize*img.csize;
      imageFormat.PixelFormat=Pylon::PixelType_RGBA8packed;
      break;
    case GL_LUMINANCE:
    default:
      imageFormat.LinePitch=img.xsize*img.csize;
      imageFormat.PixelFormat=Pylon::PixelType_Mono8;
      break;
    }
    return imageFormat;
  }

  bool convertFrom(Pylon::GrabResult Result) {
    const struct Pylon::SImageFormat format=getInFormat(Result);
    bool init=false;

#ifdef HAVE_LIBPYLONUTILITY
    if(!converter)init=true;
    if(converter && !converter->IsInitialized())init=true;
#endif

    if(format!=inFormat)init=true;
    
    if(init) {
      makeConverter(format);
      inFormat=format;      
    }


    image.xsize=inFormat.Width;
    image.ysize=inFormat.Height;
    image.reallocate();
    
    if(0) {; 
#ifdef HAVE_LIBPYLONUTILITY
    } else if(converter) {
      const struct Pylon::SOutputImageFormat oformat=getOutFormat(image);
      converter->Convert(image.data,
                         image.xsize*image.ysize*image.csize,
                         Result.Buffer(),
                         Result.GetPayloadSize(),
                         format,
                         oformat);
#endif
    } else {
      if(image.format==GL_RGBA)
        image.fromRGBA(reinterpret_cast<unsigned char*>(Result.Buffer()));
      else
        image.fromGray(reinterpret_cast<unsigned char*>(Result.Buffer()));
    }
    return true;
  }

  bool convertTo(imageStruct&img) {
    img.convertFrom(&image);
  }
};

/////////////////////////////////////////////////////////
//
// videoPYLON
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

REGISTER_VIDEOFACTORY("pylon", videoPYLON);

videoPYLON :: videoPYLON() : videoBase("pylon")
                           , m_factory(NULL)
                           , m_camera(NULL)
                           , m_grabber(NULL)
                           , m_converter(new Converter())
                           , m_numBuffers(NUM_BUFFERS)
{
  m_width=0;
  m_height=0;

  try {
    m_factory = &Pylon::CTlFactory::GetInstance();
    Pylon::TlInfoList_t tli;
    int count= m_factory->EnumerateTls (tli);
    int i=0;
    for(i=0; i<count; i++) {
      std::string s=tli[i].GetFriendlyName().c_str();
      if("GigE"==s)
        provide("gige");
      provide(s);
    }

  } catch (GenICam::GenericException &e) {
    // Error handling
    throw(GemException(e.GetDescription()));
    return;
  }
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoPYLON :: ~videoPYLON()
{
  close();
}

////////////////////////////////////////////////////////
// frame grabber
//
/////////////////////////////////////////////////////////
bool videoPYLON :: grabFrame() {
  if(NULL==m_camera || NULL==m_grabber)
    return false;

  struct Pylon::SImageFormat imageFormat;
  struct Pylon::SOutputImageFormat outImageFormat;

  if(m_grabber->GetWaitObject().Wait(3000)) {
    Pylon::GrabResult Result;
    m_grabber->RetrieveResult(Result);
    switch(Result.Status()) {
    case Pylon::Grabbed: {
      m_converter->convertFrom(Result);

      lock();
      m_converter->convertTo(m_image.image);
      m_image.image.upsidedown=true;
      m_image.newimage=true;
      unlock();

      m_grabber->QueueBuffer(Result.Handle(), NULL);

    }
      break;
    case Pylon::Failed:
      //std::cerr << "PYLON: grab failed and ";
    default:
      //std::cerr << "PYLON: grab returned "<<Result.Status()<<std::endl;
      break;
    }
  } else {
    // timeout
    //std::cerr << "PYLON: grab timed out"<<std::endl;
    m_grabber->CancelGrab();
    return false;
  }

  return true;
}


/////////////////////////////////////////////////////////
// openDevice
//
/////////////////////////////////////////////////////////

bool videoPYLON :: openDevice(gem::Properties&props)
{
  double d;
  uint32_t channel=0;
  if(props.get("channel", d))
    channel=d;

  if(m_camera)closeDevice();
  if(NULL==m_factory)return false;

  Pylon::IPylonDevice *device = NULL;

  try {
    if(m_devicename.empty()) {
      if(m_id2device.empty())
        enumerate();
      std::map<std::string, Pylon::CDeviceInfo>::iterator it=m_id2device.begin();
      if(m_devicenum>=0) {
        std::advance( it, m_devicenum );
      } 
      if(it != m_id2device.end()) {  
        device = m_factory->CreateDevice(it->second);
      }
    } else {
      std::map<std::string, Pylon::CDeviceInfo>::iterator it=m_id2device.find(m_devicename);
      if(it!=m_id2device.end())
        device = m_factory->CreateDevice(it->second);
      else
        device = m_factory->CreateDevice(Pylon::String_t(m_devicename.c_str()));
    }
  } catch (GenICam::GenericException &e) {
    //std::cerr << e.GetDescription() << std::endl;
    return false;
  }

  if(device==NULL)
    return false;

  try {
    m_camera=new Pylon::CBaslerGigECamera (device);
    m_camera->Open();
    uint32_t maxchannel=m_camera->GetNumStreamGrabberChannels();
    if(channel>maxchannel)channel=maxchannel;

    m_grabber=new Pylon::CBaslerGigEStreamGrabber(m_camera->GetStreamGrabber(channel));
  } catch (GenICam::GenericException &e) {
    //std::cerr << e.GetDescription() << std::endl;
    close();
    return false;
  }
  return true;
}
/////////////////////////////////////////////////////////
// closeDevice
//
/////////////////////////////////////////////////////////
void videoPYLON :: closeDevice() {
  if(m_camera){
    m_camera->Close();
    delete m_camera;
  }
  m_camera=NULL;

  if(m_grabber)delete m_grabber;
  m_grabber=NULL;
}


/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoPYLON :: startTransfer()
{
  if(NULL==m_camera)
    return false;

  if(NULL==m_grabber)
    return false;

  try {
    m_grabber->Open();

    // Set the image format and AOI
    //    m_camera->PixelFormat.SetValue(Basler_GigECameraParams::PixelFormat_Mono8Signed);
    //    m_camera->OffsetX.SetValue(0);
    //    m_camera->OffsetY.SetValue(0);

    //    m_camera->Width.SetValue(m_camera->Width.GetMax());
    //    m_camera->Height.SetValue(m_camera->Height.GetMax());

    // Set the camera to continuous frame mode
    /*
      m_camera->TriggerSelector.SetValue(TriggerSelector_AcquisitionStart);
      m_camera->TriggerMode.SetValue(TriggerMode_Off);
      m_camera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
    */
    //    m_camera->ExposureMode.SetValue(ExposureMode_Timed);
    //    m_camera->ExposureTimeRaw.SetValue(100);

    const size_t ImageSize = (size_t)(m_camera->PayloadSize.GetValue());
    m_grabber->MaxBufferSize.SetValue(ImageSize);
    m_grabber->MaxNumBuffer.SetValue(m_numBuffers);
    m_grabber->PrepareGrab();

    uint32_t i;
    for (i = 0; i < m_numBuffers; ++i) {
      CGrabBuffer *pGrabBuffer = new CGrabBuffer(ImageSize);
      pGrabBuffer->SetBufferHandle(m_grabber->RegisterBuffer(
                                                             pGrabBuffer->GetBufferPointer(),
                                                             ImageSize));

      // Put the grab buffer object into the buffer list
      m_buffers.push_back(pGrabBuffer);
    }
    std::vector<CGrabBuffer*>::const_iterator x;
    for (x=m_buffers.begin(); x != m_buffers.end(); ++x) {
      // Put buffer into the grab queue for grabbing
      m_grabber->QueueBuffer((*x)->GetBufferHandle(), NULL);
    }

    m_camera->AcquisitionStart.Execute();
  } catch (GenICam::GenericException &e) {
    //std::cerr << e.GetDescription() << std::endl;
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoPYLON :: stopTransfer()
{
  if(m_camera) {
    // Stop acquisition
    try {
      m_camera->AcquisitionStop.Execute();
    } catch (GenICam::GenericException &e) {
      //std::cerr << e.GetDescription() << std::endl;
    }
  }
  if(m_grabber) {
    try {
      m_grabber->CancelGrab();
      // Get all buffers back
      Pylon::GrabResult r;
      while(m_grabber->RetrieveResult(r)){;}

      std::vector<CGrabBuffer*>::iterator it;
      for (it = m_buffers.begin(); it != m_buffers.end(); it++) {
        m_grabber->DeregisterBuffer((*it)->GetBufferHandle());
        delete *it;
        *it = NULL;
      }
      m_buffers.clear();

      m_grabber->FinishGrab();
      m_grabber->Close();
    } catch (GenICam::GenericException &e) {
      //std::cerr << e.GetDescription() << std::endl;
    }
  }
  return true;
}

std::vector<std::string> videoPYLON::enumerate() {
  m_id2device.clear();
  std::vector<std::string> result;
  if(NULL==m_factory)return result;

  Pylon::DeviceInfoList_t devices;
  if (0 == m_factory->EnumerateDevices(devices))  {
    std::cout << "could not enumerate" << std::endl;
    return result;
  }
  if(devices.empty() )
    return result;

  int i=0;
  for(i=0; i<devices.size(); i++) {
    std::string name;
    Pylon::CDeviceInfo&device=devices[i];
    bool added=false;
#if 1
#define SHOWNAME(x) //std::cerr << x<<"["<<i<<"]: "<<name<<std::endl;
#else
#define SHOWNAME(x)
#endif

    name=device.GetUserDefinedName();
    if(!name.empty()) { m_id2device[name]=device; SHOWNAME("user")}
    if(!added && !name.empty()) {  result.push_back(name);  added=true;  }

    try {
      /* darn, this doesn't seem to work..., it would be great though */
      Pylon::CBaslerGigEDeviceInfo&gdevice=dynamic_cast< Pylon::CBaslerGigEDeviceInfo&>(device);

      name=gdevice.GetAddress ();
      if(!name.empty()) { m_id2device[name]=device; SHOWNAME("address")}
      if(!added && !name.empty()) {  result.push_back(name);  added=true;  }

    } catch (const std::bad_cast& e) {
      //      //std::cerr << i<<" not a GigE&device"<<std::endl;
    }

    name=device.GetFullName();
    if(!name.empty()) { m_id2device[name]=device; SHOWNAME("full")}
    if(!added && !name.empty()) {  result.push_back(name);  added=true;  }

    name=device.GetSerialNumber();
    if(!name.empty()) { m_id2device[name]=device; SHOWNAME("serial")}
    if(!added && !name.empty()) {  result.push_back(name);  added=true;  }

    name=device.GetFriendlyName();
    if(!name.empty()) { m_id2device[name]=device; SHOWNAME("friendly")}
    if(!added && !name.empty()) {  result.push_back(name);  added=true;  }
  }

  return result;
}


bool videoPYLON::enumProperties(gem::Properties&readable,
                                gem::Properties&writeable) {

  int i=0;
  gem::Properties props;
  std::vector<std::string>keys;
  gem::any type;

  readable.clear();
  writeable.clear();


  props=gem::pylon::streamgrabberproperties::getKeys();
  keys=props.keys();
  if(m_grabber) {
    GenApi::INodeMap*nodes=m_grabber->GetNodeMap();

    for(i=0; i<keys.size(); i++) {
      GenApi::INode *node=nodes->GetNode(keys[i].c_str());
      if(node) {
        switch(node->GetAccessMode()) {
        case GenApi::RO: case GenApi::RW:
          readable.set(keys[i], props.get(keys[i]));
        default:
          break;
        }
      }
    }
  } else {
#if 0
    for(i=0; i<keys.size(); i++)
      readable.set(keys[i], type);
#endif
  }

  props=gem::pylon::streamgrabberproperties::setKeys();
  keys=props.keys();
  if(m_grabber) {
    GenApi::INodeMap*nodes=m_grabber->GetNodeMap();

    for(i=0; i<keys.size(); i++) {
      GenApi::INode *node=nodes->GetNode(keys[i].c_str());
      if(node) {
        switch(node->GetAccessMode()) {
        case GenApi::WO: case GenApi::RW:
          writeable.set(keys[i], props.get(keys[i]));
        default:
          break;
        }
      }
    }
  } else {
#if 0
    for(i=0; i<keys.size(); i++)
      writeable.set(keys[i], type);
#endif
  }

  props=gem::pylon::cameraproperties::getKeys();
  keys=props.keys();
  if(m_camera) {
    GenApi::INodeMap*nodes=m_camera->GetNodeMap();

    for(i=0; i<keys.size(); i++) {
      GenApi::INode *node=nodes->GetNode(keys[i].c_str());
      if(node) {
        switch(node->GetAccessMode()) {
        case GenApi::RO: case GenApi::RW:
          readable.set(keys[i], props.get(keys[i]));
        default:
          break;
        }
      }
    }
  } else {
#if 0
    for(i=0; i<keys.size(); i++)
      readable.set(keys[i], type);
#endif
  }

  props=gem::pylon::cameraproperties::setKeys();
  keys=props.keys();

  if(m_camera) {
    GenApi::INodeMap*nodes=m_camera->GetNodeMap();

    for(i=0; i<keys.size(); i++) {
      GenApi::INode *node=nodes->GetNode(keys[i].c_str());
      if(node) {
        switch(node->GetAccessMode()) {
        case GenApi::WO: case GenApi::RW:
          writeable.set(keys[i], props.get(keys[i]));
        default:
          break;
        }
      }
    }
  } else {
#if 0
    for(i=0; i<keys.size(); i++)
      writeable.set(keys[i], type);
#endif
  }

#if 0
  if(m_camera) {
    try {
      const Pylon::CDeviceInfo & di=m_camera->GetDeviceInfo();
      Pylon::StringList_t names;
      di.GetPropertyNames (names);

      int i=0;
      for(i=0; i<names.size(); i++) {
        std::string key=names[i].c_str();
        //        //std::cerr << "property#"<<i<<": "<<names[i]<<std::endl;
        writeable.set(key, type);
        readable.set (key, type);
      }
    } catch (GenICam::GenericException &e) {
      //std::cerr << e.GetDescription() << std::endl;
      return false;
    }
  }
#endif
  return false;
}
void videoPYLON::setProperties(gem::Properties&props) {
  std::vector<std::string>keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    const std::string key=keys[i];
    bool didit=false;
    if(m_grabber) {
      try {
        didit=gem::pylon::streamgrabberproperties::set(m_grabber, key, props);
      } catch (GenICam::GenericException &e) {
        error("videoPYLON: [%s] %s", key.c_str(), e.GetDescription());
        ////std::cerr << e.GetDescription() << std::endl;
        didit=false;
      }
    }

    if(!didit && m_camera)
      try {
        didit=gem::pylon::cameraproperties::set(m_camera, key, props);
      } catch (GenICam::GenericException &e) {
        error("videoPYLON: [%s] %s", key.c_str(), e.GetDescription());
        ////std::cerr << e.GetDescription() << std::endl;
        didit=false;
      }

  }
}

void videoPYLON::getProperties(gem::Properties&props) {
  std::vector<std::string>keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    const std::string key=keys[i];
    props.erase(key);
    gem::any result;
    if(m_grabber) {
      try {
        gem::pylon::streamgrabberproperties::get(m_grabber, key, result);
      } catch (GenICam::GenericException &e) {result.reset(); }
    }

    if(result.empty() && m_camera)
      try {
        gem::pylon::cameraproperties::get(m_camera, key, result);
      } catch (GenICam::GenericException &e) {result.reset(); }

    if(result.empty())
      continue;
    props.set(key, result);
  }
}

#endif /* HAVE_PYLON */
