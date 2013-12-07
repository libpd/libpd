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

#include "videoHALCON.h"
#include "plugins/PluginFactory.h"

#include <sstream>
using namespace gem::plugins;

#include "Gem/RTE.h"


#include "Gem/Files.h"
#include "Gem/Dylib.h"

#if 0
# define debug ::post
#else
# define debug
#endif

/////////////////////////////////////////////////////////
//
// videoHALCON
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
#ifdef HAVE_HALCON

REGISTER_VIDEOFACTORY("halcon", videoHALCON);

static std::vector<std::string>s_backends;
static std::vector<std::string>getBackends(void) {
  if(s_backends.size()>0)return s_backends;
#ifdef _WIN32
  std::string path = gem::files::expandEnv("%HALCONROOT%/bin/%HALCONARCH%/hAcq", true);
#else
  std::string path = gem::files::expandEnv("$HALCONROOT/lib/$HALCONARCH/hAcq", true);
#endif
  std::string pattern = path+std::string("*") +GemDylib::getDefaultExtension();

  std::vector<std::string>listing=gem::files::getFilenameListing(pattern);
  ////std::cerr << "pattern: '"<<pattern<<"' got "<<listing.size()<<" results " << std::endl;
  int i=0;
  for(i=0; i<listing.size(); i++) {
    std::string needle="hAcq";
    const size_t found = listing[i].rfind(needle);
    ////std::cerr << "checking: '"<<listing[i]<<"' found " << found << std::endl;

    if(std::string::npos != found) {
      const size_t start=found+needle.length();
      const size_t stop =listing[i].rfind(GemDylib::getDefaultExtension()) - start;
      std::string backend=listing[i].substr(start, stop);

	  ////std::cerr << "checking: '"<<listing[i]<<"' -> " << backend << std::endl;

      try {
        Halcon::HTuple Information;
        Halcon::HTuple ValueList;
        if(H_MSG_TRUE==info_framegrabber(backend.c_str(),
                                         "revision",
                                         &Information,
                                         &ValueList)) {
          s_backends.push_back(backend);
        }
      } catch (Halcon::HException &except) {
        //error("%s", except.message);
      }
    }
  }
  return s_backends;
}



// exception handler
static void MyHalconExceptionHandler(const Halcon::HException& except)
{
  // the exception handler is needed in order to prevent halcon from crashing
  // we just pass on the exception to upstream...
  throw except;	
}

videoHALCON :: videoHALCON() : videoBase("halcon"),
                               m_grabber(NULL)
{
  m_width=0;
  m_height=0;

  Halcon::HException::InstallHHandler(&MyHalconExceptionHandler);
  m_backends=getBackends();
  if(m_backends.size()>0) {
    int i=0;

    for(i=0; i<m_backends.size(); i++) {
      provide(m_backends[i]);
    }
  }
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoHALCON :: ~videoHALCON()
{
  close();
}


////////////////////////////////////////////////////////
// frame grabber
//
/////////////////////////////////////////////////////////
bool videoHALCON :: grabFrame() {
  Halcon::HImage img;

  if(NULL==m_grabber)
    return false;

  try {
    img=m_grabber->GrabImageAsync(-1);
  } catch (Halcon::HException& except) {
    error("Halcon::GrabImage exception: '%s'", except.message);
    return false;
  }
  Halcon::HTuple typ, W, H, pR, pG, pB;
  long r, g, b,  h, w;

  try {
    r = img.GetImagePointer3(&pG, &pB, &typ, &W, &H);
  } catch (Halcon::HException& except) {
    error("Halcon::GetImagePointer exception: '%s'", except.message);
    return false;
  }

  try {
#if 0
#define GETTUPLE(x, y) { try {x=y[0]; } catch (Halcon::HException& except) { error("HTuple exception @ %d: '%s'", __LINE__, except.message); } } while(0)
    GETTUPLE(g, pG);
    GETTUPLE(b, pB);
    GETTUPLE(w, W);
    GETTUPLE(h, H);
#else
    g=pG[0];
    b=pB[0];

    w=W[0];
    h=H[0];
#endif

    const unsigned char* ptrR=(const unsigned char*)r;
    const unsigned char* ptrG=(const unsigned char*)g;
    const unsigned char* ptrB=(const unsigned char*)b;
    //post("image[%dx%d]: %x %x %x --> %x %x %x", w, h, r, g, b, ptrR, ptrG, ptrB);
    lock();
    m_image.image.xsize=w;
    m_image.image.ysize=h;
    m_image.image.setCsizeByFormat(GL_RGBA);
    m_image.image.reallocate();
    long row, col;
    unsigned char*data=m_image.image.data;
    for(row=0; row<h; row++) {
      for(col=0; col<w; col++) {
        //        post("[%d,%d]", row, col);
        data[chRed  ]=*ptrR++;
        data[chGreen]=*ptrG++;
        data[chBlue ]=*ptrB++;
        data[chAlpha]=255;
        data+=4;
      }
    }

    m_image.newimage=true;
    m_image.image.upsidedown=true;
    unlock();

  } catch (Halcon::HException& except) {
    logpost(NULL, 5, "Halcon::HTuple exception: '%s'", except.message);
  }
  return true;
}


/**
 * device name parser
 */
static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while(std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}
static std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  return split(s, delim, elems);
}

static std::string parsedevicename(std::string devicename, std::string&cameratype, std::string&device) {
  std::string name;
  if(devicename.empty())return name;

  std::vector<std::string> parsed = split(devicename, ':');
  switch(parsed.size()) {
  default:
    logpost(NULL, 5, "could not parse '%s'", devicename.c_str());
    return name;
  case 3:
    if(parsed[2].size()>0)
      device=parsed[2];
  case 2:
    if(parsed[1].size()>0)
      cameratype=parsed[1];
  case 1:
    name=parsed[0];
  }
  logpost(NULL, 5, "HALCON: name  ='%s'", name.c_str());
  logpost(NULL, 5, "HALCON: camera='%s'", cameratype.c_str());
  logpost(NULL, 5, "HALCON: device='%s'", device.c_str());
  return name;
}

/////////////////////////////////////////////////////////
// openDevice
//
/////////////////////////////////////////////////////////

static void printtuple(Halcon::HTuple t) {
  int i=0;
  for(i=0; i< t.Num(); i++) {

    Halcon::HCtrlVal v=t[i];
    //std::cerr<<"["<<i<<"]: ";
    switch(v.ValType()) {
    case Halcon::LongVal:
      //std::cerr << v.L();
      break;
    case Halcon::DoubleVal:
      //std::cerr << v.D();
      break;
    case Halcon::StringVal:
      //std::cerr << v.S();
      break;
    case Halcon::UndefVal:
      //std::cerr << "<undef>";
    }
    //std::cerr << std::endl;
  }
}

static void printinfo(std::string name, std::string value) {
  try {
    Halcon::HTuple Information;
    Halcon::HTuple ValueList;

    //std::cerr << "getting info for "<<name<<"."<<value<<std::endl;

    Herror err=info_framegrabber(name.c_str(),
                                 value.c_str(),
                                 &Information,
                                 &ValueList);

    //std::cerr << "got info for "<<name<<"."<<value<<":"<<std::endl;
    printtuple(Information);
    //std::cerr << "got values: " << std::endl;
    printtuple(ValueList);
    //std::cerr << std::endl;
  }  catch (Halcon::HException &except) {
    error("%s", except.message);
  }
}


static void getparam(Halcon::HFramegrabber*grabber, std::string name) {
  try {
    Halcon::HTuple result=grabber->GetFramegrabberParam(name.c_str());
    //std::cerr << "got parm for "<<name<<std::endl;
    printtuple(result);
  }  catch (Halcon::HException &except) {
    error("%s", except.message);
  }
}

bool videoHALCON :: openDevice(gem::Properties&props)
{
  m_backendname.clear();
  if(m_grabber)closeDevice();

  double d=0;
  int w=0, h=0, p=0;

  if(props.get("width", d))
    w=d;
  if(props.get("height", d))
    h=d;
  if(props.get("channel", d))
    p=d;
  const int width=(w>0) ?w:0;
  const int height=(h>0)?h:0;
  const int port=(p>0)?p:-1;
  m_width=width;
  m_height=height;

  /* m_devicename has to provide:
   *    backendid
   *    cameratype
   *    device
   *
   * originally i though about using ":" as s delimiter,
   *  e.g. "GigEVision::001234567890" would use the GigE-device @ "00:12:34:56:78:90" with "default" cameratype
   * however, ":" is already internally used by e.g. the "1394IIDC" backend, that uses "format:mode:fps" as cameratype
   *
   * either use another delimiter, or find some escaping mechanism (e.g. '1394IIDC:"0:4:5":0x0814436102632378'
   *
   * another idea would be to get an idea about which <driver> was selected in [pix_video] and use that as the <backendid>
   * for this to work, we would have to provide a list of valid backends (e.g. dynamically query what is installed)
   * i don't think this is currently possible with halcon
   *
   */
  std::string cameratype="default";
  std::string device="default";

  std::string name;

  if(m_devicename.empty()) {
    if(m_device2backend.size()>0) {
      std::map<std::string, std::string>::iterator it( m_device2backend.begin() );
      std::advance( it, m_devicenum );
      if(it != m_device2backend.end()) {  
        device=it->first;
        name=it->second;
      }
    }
  } else {
    std::map<std::string,  std::string>::iterator it = m_device2backend.find(m_devicename);
    if(it != m_device2backend.end()) {
      device=it->first;
      name=it->second;
    } else {
      name=parsedevicename(m_devicename, cameratype, device);
    }
  }

  if(name.empty()) {
    return false;
  }

  try {
    m_grabber = new Halcon::HFramegrabber(
                                          name.c_str(), /* const HTuple &Name, */
                                          1, 1, /* const HTuple &HorizontalResolution = 1, const HTuple &VerticalResolution = 1, */
                                          width, height, /* const HTuple &ImageWidth = 0,           const HTuple &ImageHeight = 0, */
                                          0, 0, /* const HTuple &StartRow = 0,             const HTuple &StartColumn = 0, */
                                          "default", /* const HTuple &Field = "default", */
                                          8, /* const HTuple &BitsPerChannel = -1,  */
                                          "rgb", /* const HTuple &ColorSpace = "default", */
                                          -1, /* const HTuple &Gain = -1, */
                                          "default", /* const HTuple &ExternalTrigger = "default", */
                                          cameratype.c_str(), /* const HTuple &CameraType = "default", */
                                          device.c_str(), /* const HTuple &Device = "default", */
                                          port /* const HTuple &Port = -1, */
                                          /* const HTuple &LineIn = -1 */
                                          );
  } catch (Halcon::HException &except) {
    error("%s", except.message);
    m_grabber=NULL;
    return false;
  }

#if 0
  printinfo(name, "parameters");
  printinfo(name, "parameters_readonly");
  printinfo(name, "parameters_writeonly");

  printinfo(name, "port");

  getparam(m_grabber, "color_space_values");
  getparam(m_grabber, "revision");
#endif

  m_backendname = name;
  return true;
}
/////////////////////////////////////////////////////////
// closeDevice
//
/////////////////////////////////////////////////////////
void videoHALCON :: closeDevice() {
  if(m_grabber)delete m_grabber;
  m_grabber=NULL;
  m_backendname.clear();
}


/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoHALCON :: startTransfer()
{
  if(NULL!=m_grabber)
    m_grabber->GrabImageStart(-1);
  return true;
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoHALCON :: stopTransfer()
{
  return true;
}

std::vector<std::string> videoHALCON::enumerate() {
  std::vector<std::string> result;

  std::vector<std::string> backends;
  m_device2backend.clear();

  if(m_backendname.empty()) {
    backends=m_backends;
  } else {
    backends.push_back(m_backendname);
  }

  int i=0;

  for(i=0; i<backends.size(); i++) {
    try {
      Halcon::HTuple Information;
      Halcon::HTuple ValueList;

      if(H_MSG_TRUE != info_framegrabber(backends[i].c_str(),
                                         "device",
                                         &Information,
                                         &ValueList))
        continue;

      int j=0;
      for(j=0; j<ValueList.Num(); j++) {
        Halcon::HCtrlVal v=ValueList[j];
        std::string device;
        switch(v.ValType()) {
        case Halcon::StringVal:
          device=v.S();

          // some backends seem to be a bit buggy and report instructions what you should give rather than enumerate devices
          if("LPS36"==backends[i] && "<ip_address>"==device) {
            // e.g. LPS-36 gives us "<ip_address>" (probably it does return a valid IP-address if we had such a device, dunno)
          } else {
            m_device2backend[device]=backends[i];
            result.push_back(device);
          }
          break;
        default:
          break;
        }
      }
    }  catch (Halcon::HException &except) {
      // ...
    }
  }

  return result;
}

bool videoHALCON::enumProperties(gem::Properties&readable,
                                 gem::Properties&writeable) {
  int i=0;
  gem::any typeval;(void*)0;

  readable.clear();
  m_readable.clear();

  writeable.clear();
  m_writeable.clear();

  try {
    Halcon::HTuple Information;
    Halcon::HTuple ValueList;
    Herror err=info_framegrabber(m_backendname.c_str(),
                                 "parameters",
                                 &Information,
                                 &ValueList);
    if(ValueList.Num()>0) {
      for(i=0; i< ValueList.Num(); i++) {
        Halcon::HCtrlVal v=ValueList[i];
        if(v.ValType() == Halcon::StringVal) {
          readable.set(v.S(), typeval);
          m_readable[v.S()]=ValueList;

          writeable.set(v.S(), typeval);
          m_writeable[v.S()]=ValueList;
        }
      }
    }
  }  catch (Halcon::HException &except) {
    error("%s", except.message);
  }

  try {
    Halcon::HTuple Information;
    Halcon::HTuple ValueList;
    Herror err=info_framegrabber(m_backendname.c_str(),
                                 "parameters_readonly",
                                 &Information,
                                 &ValueList);
    if(ValueList.Num()>0) {
      for(i=0; i< ValueList.Num(); i++) {
        Halcon::HCtrlVal v=ValueList[i];
        if(v.ValType() == Halcon::StringVal) {
          readable.set(v.S(), typeval);
          m_readable[v.S()]=ValueList;

          writeable.erase(v.S());
          m_writeable.erase(v.S());
        } else {
          //std::cerr << "unknown ro" <<std::endl;
        }
      }
    }
  }  catch (Halcon::HException &except) {
    error("%s", except.message);
  }

  try {
    Halcon::HTuple Information;
    Halcon::HTuple ValueList;
    Herror err=info_framegrabber(m_backendname.c_str(),
                                 "parameters_writeonly",
                                 &Information,
                                 &ValueList);
    if(ValueList.Num()>0) {
      for(i=0; i< ValueList.Num(); i++) {
        Halcon::HCtrlVal v=ValueList[i];
        if(v.ValType() == Halcon::StringVal) {
          writeable.set(v.S(), typeval);
          m_writeable[v.S()]=ValueList;

          readable.erase(v.S());
          m_readable.erase(v.S());
        } else {
          //std::cerr << "unknown wo" <<std::endl;
        }
      }
    }
  }  catch (Halcon::HException &except) {
    error("%s", except.message);
  }

  return true;
}

void videoHALCON::setProperties(gem::Properties&props) {
  if(NULL==m_grabber)return;
  std::vector<std::string>keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    std::string s;
    double d;

    if(m_writeable.find(key) != m_writeable.end()) {
      try {
        const Halcon::HTuple Param=key.c_str();
        Halcon::HTuple Value;
        switch(props.type(key)) {
        case gem::Properties::STRING:
          if(props.get(key, s)) {
            try {
              m_grabber->SetFramegrabberParam(Param, s.c_str());
            } catch (Halcon::HException& except) {
              error("Halcon::SetFramegrabberParam(%s) exception: '%s'", key.c_str(), except.message);
            }
          }
          break;
        case gem::Properties::DOUBLE:
          if(props.get(key, d)) {
            try {
              m_grabber->SetFramegrabberParam(Param, d);
            } catch (Halcon::HException& except) {
              try {
                long l=d;
                m_grabber->SetFramegrabberParam(Param, l);
              } catch (Halcon::HException& except) {
                error("Halcon::SetFramegrabberParam(%s) exception: '%s'", key.c_str(), except.message);
              }
            }
          }
          break;
        default:
          error("Halcon::SetFramegrabberParam(%s): invalid type", key.c_str());
          break;
        }
      } catch (Halcon::HException& except) {
        error("Halcon::SetFramegrabberParam(%s) exception: '%s'", key.c_str(), except.message);
      }
    } else {
      if("width" == key) {
        double d=0;
        if(props.get(key, d)) {
          try {
            m_grabber->SetFramegrabberParam("image_width", d);
          } catch (Halcon::HException& except) {
            try {
              long l=d;
              m_grabber->SetFramegrabberParam("image_width", l);
            } catch (Halcon::HException& except) {
              error("Halcon::SetFramegrabberParam(width) exception: '%s'", except.message);
            }
          }
          m_width=d;
        }
      } else if ("height" == key) {
        double d=0;
        if(props.get(key, d)) {
          try {
            m_grabber->SetFramegrabberParam("image_height", d);
          } catch (Halcon::HException& except) {
            try {
              long l=d;
              m_grabber->SetFramegrabberParam("image_height", l);
            } catch (Halcon::HException& except) {
              error("Halcon::SetFramegrabberParam(height) exception: '%s'", except.message);
            }
          }
          m_height=d;
        }
      }

    }
  }
}

void videoHALCON::getProperties(gem::Properties&props) {
  if(NULL==m_grabber)return;
  std::vector<std::string>keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    if(m_readable.find(key) != m_readable.end()) {
      try {
        Halcon::HTuple hresult=m_grabber->GetFramegrabberParam(key.c_str());
        gem::any nonetype;
        int j=0;
        for(j=0; j< hresult.Num(); j++) {
          Halcon::HCtrlVal v=hresult[j];
          switch(v.ValType()) {
          case Halcon::LongVal:
            props.set(key, static_cast<double>(v.L()));
            break;
          case Halcon::DoubleVal:
            break;
          case Halcon::StringVal:
            props.set(key, std::string(v.S()));
            break;
          case Halcon::UndefVal:
            props.set(key, nonetype);
          default:
            break;
          }
        }
      } catch (Halcon::HException& except) {
        error("Halcon::GetFramegrabberParam exception: '%s'", except.message);
      }
    }
  }
}
#else
videoHALCON :: videoHALCON() : videoBase("")
{ }
videoHALCON :: ~videoHALCON()
{ }
#endif /* HAVE_HALCON */
