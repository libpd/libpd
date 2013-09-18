/*
 *  videoDarwin.cpp
 *  gem_darwin
 *
 *  Created by James Tittle on Fri Jul 12 2002.
 *  Copyright (c) 2002-2005 James Tittle & Chris Clepper
 *  Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
 *
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined __APPLE__  && defined __x86_64
#warning unconditionally disabling QuickTime/Carbon on OSX/64bit
// with OSX10.6, apple has removed loads of Carbon functionality (in 64bit mode)
// LATER make this a real check in configure
# undef HAVE_QUICKTIME
# undef HAVE_CARBON
#endif

#if defined HAVE_CARBON && defined HAVE_QUICKTIME
# define HAVE_VIDEODARWIN
#endif

#ifdef HAVE_VIDEODARWIN
#include "videoDarwin.h"
#include "Gem/RTE.h"
#include "plugins/PluginFactory.h"

using namespace gem::plugins;

#include <unistd.h> // needed for Unix file open() type functions
#include <stdio.h>
#include <fcntl.h> /* JMZ thinks that _this_ is needed for open() */

#define DEFAULT_WIDTH        320
#define DEFAULT_HEIGHT        240

static std::string pascal2str(const Str255 pstr) {
  const unsigned char*cstr=static_cast<const unsigned char*>(pstr);
  const char*str=(const char*)(cstr+1);
  const size_t length=cstr[0];
  return std::string(str, length);
}



REGISTER_VIDEOFACTORY("Darwin", videoDarwin);

videoDarwin :: videoDarwin() 
  : videoBase("darwin", 0),
    m_newFrame(false),
    m_srcGWorld(NULL),
    m_quality(channelPlayNormal),
    m_colorspace(GL_YCBCR_422_GEM),
    m_inputDevice(0)   //set to the first input device
{
  m_width= DEFAULT_WIDTH;
  m_height=DEFAULT_HEIGHT;

  m_image.image.xsize = 800;
  m_image.image.ysize = 600;
  m_image.image.setCsizeByFormat(GL_BGRA_EXT);
  m_image.image.allocate();

  //initSeqGrabber();
  provide("dv");
  provide("iidc");
  provide("analog");
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoDarwin :: ~videoDarwin()
{
  close();
  if (m_vc) {
    if (::SGDisposeChannel(m_sg, m_vc)) {
      error ("Unable to dispose a video channel");
    }
    m_vc = NULL;
  }
  if (m_sg) {
    if (::CloseComponent(m_sg)) {
      error("Unable to dispose a sequence grabber component");
    }
    m_sg = NULL;
    if (m_srcGWorld) {
      ::DisposeGWorld(m_srcGWorld);
      m_srcGWorld = NULL;
    }
  }
}
bool videoDarwin :: openDevice(gem::Properties&props) {
  applyProperties(props);
  bool success=initSeqGrabber();
  if(NULL==m_sg)success=false;
  if(success)
    applyProperties(props);
  else
    destroySeqGrabber();
  
  return success;
}
void videoDarwin :: closeDevice(void) {
  destroySeqGrabber();
}

////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
bool videoDarwin :: grabFrame()
{
  OSErr    err;

  err = SGIdle(m_sg);

  if (err != noErr){
    error("SGIdle failed with error %d",err);
    m_haveVideo = 0;
    //    return false;
  } else {
    //this doesn't do anything so far
    //VDCompressDone(m_vdig,frameCount,data,size,similar,time);
    //err = SGGrabFrameComplete(m_vc,frameCount,done);
    //if (err != noErr) error("SGGrabCompressComplete failed with error %d",err);
    //post("SGGrabFramecomplete done %d framecount = %d",done[0],frameCount);

    m_haveVideo = 1;
    m_newFrame = true;
  }
  if (!m_haveVideo)
    {
      post("no video yet");
      return true;
    }
  m_image.newimage = m_newFrame;
  m_newFrame = false;

  return true;
}

////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoDarwin :: startTransfer()
{
  OSErr    err = noErr;

  SGStartPreview(m_sg);
  m_haveVideo = true;
  m_image.newimage = true;
  return true;
}

////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoDarwin :: stopTransfer()
{
  OSErr    err = noErr;

  //might need SGPause or SGStop here
  err = SGStop(m_sg);
  if (err != noErr)error("SGStop failed with error %d",err);
  return true;
}

bool videoDarwin :: initSeqGrabber()
{
  OSErr anErr;
  Rect srcRect = {0,0, m_height, m_width};
  SGDeviceList devices;
  short        deviceCount = 0;

  m_sg = OpenDefaultComponent(SeqGrabComponentType, 0);
  if(m_sg==NULL){
    error("could not open default component");
    return false;
  }
  anErr = SGInitialize(m_sg);
  if(anErr!=noErr){
    error("could not initialize SG error %d",anErr);
    return false;
  }

  anErr = SGSetDataRef(m_sg, 0, 0, seqGrabDontMakeMovie);
  if (anErr != noErr){
    error("dataref failed with error %d",anErr);
  }

  anErr = SGNewChannel(m_sg, VideoMediaType, &m_vc);
  if(anErr!=noErr){
    error("could not make new SG channnel error %d",anErr);
    return false;
  }

  enumerate();
  anErr = SGGetChannelDeviceList(m_vc, sgDeviceListIncludeInputs, &devices);
  if(anErr!=noErr){
    error("could not get SG channnel Device List");
  }else{
    deviceCount = (*devices)->count;
    m_inputDevice = (*devices)->selectedIndex;
  }

  /* device selection */
  if(m_devicenum>=0)
    m_inputDevice=m_devicenum;
  else if (!m_devicename.empty()) {
    int i;
    const int maxcount=(deviceCount<m_devices.size()?deviceCount:m_devices.size());
    for(i=0; i<maxcount; i++) {
      if(m_devicename==m_devices[i]) {
        m_inputDevice=i;
        break;
      }
    }
  }

  //this call sets the input device
  if (m_inputDevice >= 0 && m_inputDevice < deviceCount) {//check that the device is not out of bounds
    std::string devname=pascal2str((*devices)->entry[m_inputDevice].name);
    logpost(NULL, 3, "SGSetChannelDevice trying[%d] %s", m_inputDevice, devname.c_str());
  }
  anErr = SGSetChannelDevice(m_vc, (*devices)->entry[m_inputDevice].name);
  if(anErr!=noErr) error("SGSetChannelDevice returned error %d",anErr);

  anErr = SGSetChannelDeviceInput(m_vc,m_inputDeviceChannel);
  if(anErr!=noErr) error("SGSetChannelDeviceInput returned error %d",anErr);

  //grab the VDIG info from the SGChannel
  m_vdig = SGGetVideoDigitizerComponent(m_vc);
  VideoDigitizerError vdigErr = VDGetDigitizerInfo(m_vdig,&m_vdigInfo); //not sure if this is useful

  Str255    vdigName;
  memset(vdigName,0,255);
  vdigErr = VDGetInputName(m_vdig,m_inputDevice,vdigName);
  logpost(NULL, 3, "vdigName is %s",pascal2str(vdigName).c_str());

  Rect vdRect;
  vdigErr = VDGetDigitizerRect(m_vdig,&vdRect);
  logpost(NULL, 3, "digitizer rect is top %d bottom %d left %d right %d",vdRect.top,vdRect.bottom,vdRect.left,vdRect.right);

  vdigErr = VDGetActiveSrcRect(m_vdig,0,&vdRect);
  logpost(NULL, 3, "active src rect is top %d bottom %d left %d right %d",vdRect.top,vdRect.bottom,vdRect.left,vdRect.right);

  anErr = SGSetChannelBounds(m_vc, &srcRect);
  if(anErr!=noErr){
    error("could not set SG ChannelBounds ");
  }

  anErr = SGSetVideoRect(m_vc, &srcRect);
  if(anErr!=noErr){
    error("could not set SG Rect ");
  }

  anErr = SGSetChannelUsage(m_vc, seqGrabPreview);
  if(anErr!=noErr){
    error("could not set SG ChannelUsage ");
  }
  SGSetChannelPlayFlags(m_vc, m_quality);
  OSType pixelFormat=0;
  m_image.image.xsize = m_width;
  m_image.image.ysize = m_height;

  if (m_colorspace==GL_BGRA_EXT){
    m_image.image.setCsizeByFormat(GL_RGBA_GEM);
    m_rowBytes = m_width*4;
    pixelFormat=k32ARGBPixelFormat;
    logpost(NULL, 3, "using RGB");
  } else {
    m_image.image.setCsizeByFormat(GL_YCBCR_422_APPLE);
    m_rowBytes = m_width*2;
    pixelFormat=k422YpCbCr8PixelFormat;
    logpost(NULL, 3, "using YUV");
  }
  m_image.image.reallocate();
  anErr = QTNewGWorldFromPtr (&m_srcGWorld,
                              pixelFormat,
                              &srcRect,
                              NULL,
                              NULL,
                              0,
                              m_image.image.data,
                              m_rowBytes);

  if (anErr!= noErr) {
    error("%d error at QTNewGWorldFromPtr", anErr);
    return false;
  }
  if (NULL == m_srcGWorld) {
    error("could not allocate off screen");
    return false;
  }
  SGSetGWorld(m_sg,(CGrafPtr)m_srcGWorld, NULL);
  SGStartPreview(m_sg); //moved to starttransfer?
  m_haveVideo = true;
  return true;
}

void videoDarwin :: setupCapture()
{
  if(stop()) {
    SGSetChannelUsage(m_vc, 0);
    SGSetChannelUsage(m_vc, seqGrabPreview);
    SGUpdate(m_sg,0);
    start();
  }
}

void videoDarwin :: destroySeqGrabber()
{
  if (m_vc) {
    if (::SGDisposeChannel(m_sg, m_vc)) {
      error ("Unable to dispose a video channel");
    }
    m_vc = NULL;
  }
  if (m_sg) {
    if (::CloseComponent(m_sg)) {
      error("Unable to dispose a sequence grabber component");
    }
    m_sg = NULL;
    if (m_srcGWorld) {
      ::DisposeGWorld(m_srcGWorld);
      m_srcGWorld = NULL;
    }
  }
}

void videoDarwin :: resetSeqGrabber()
{
  OSErr anErr;
  logpost(NULL, 3, "starting reset");

  destroySeqGrabber();
  initSeqGrabber();
}

/////////////////////////////////////////////////////////
// colorspaceMess
//
/////////////////////////////////////////////////////////
bool videoDarwin :: setColor(int format)
{
  m_colorspace = format;
  if(stop()) {
    resetSeqGrabber();
    start();
  }
}


/////////////////////////////////////////////////////////
// dialog
//
/////////////////////////////////////////////////////////
bool videoDarwin :: dialog(std::vector<std::string>dlg)
{
  Rect    newActiveVideoRect;
  Rect    curBounds, curVideoRect, newVideoRect;
  ComponentResult    err;

  // Get our current state - do i need this???
  err = SGGetChannelBounds (m_vc, &curBounds);
  err = SGGetVideoRect (m_vc, &curVideoRect);

  // Pause
  err = SGPause (m_sg, true);

  // Do the dialog thang
  err = SGSettingsDialog( m_sg, m_vc, 0, nil, 0, nil, 0);

  // What happened?
  err = SGGetVideoRect (m_vc, &newVideoRect);
  err = SGGetSrcVideoBounds (m_vc, &newActiveVideoRect);

  err = SGPause (m_sg, false);

  return true;
}

std::vector<std::string>videoDarwin :: dialogs(void) {
  std::vector<std::string>result;
  return result;
}

bool videoDarwin::enumProperties(gem::Properties&readable,
                                 gem::Properties&writeable) {
  bool iidc=false;
  gem::any typ;

  readable.clear();
  writeable.clear();

  typ=1.;
  if(m_vdig) {
    ComponentDescription    desc;
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);

    iidc=vdSubtypeIIDC == desc.componentSubType;
  }
#define SETPROP(key, value) typ=value; readable.set(key, typ); writeable.set(key, typ)
#define SETRPROP(key, value) typ=value; readable.set(key, typ)
#define SETWPROP(key, value) typ=value; writeable.set(key, typ)
  SETPROP("Hue", 1);
  SETPROP("Sharpness", 1);
  SETPROP("Saturation", 1);
  SETPROP("Brightness", 1);
  if(!iidc) {
    SETPROP("Contrast", 1);
    SETPROP("KeyColor", 1);
    //SETPROP("ClipState", 1);
    //SETPROP("ClipRng", 1);
    //SETPROP("PLLFilterType", 1);
    SETWPROP("MasterBlendLevel", 1);
    //SETPROP("PlayThroughOnOff", 1);
    //SETPROP("FieldPreference", 1);
    SETPROP("BlackLevelValue", 1);
    SETPROP("WhiteLevelValue", 1);
    //SETPROP("Input", 1);
    //SETPROP("InputStandard", 1);
  } else {
    SETWPROP("Gain", 1);
    SETWPROP("Iris", 1);
    SETWPROP("Shutter", 1);
    SETWPROP("Exposure", 1);
    SETWPROP("WhiteBalanceU", 1);
    SETWPROP("WhiteBalanceV", 1);
    SETWPROP("Gamma", 1);
    SETWPROP("Temperature", 1);
    SETWPROP("Zoom", 1);
    SETWPROP("Focus", 1);
    SETWPROP("Pan", 1);
    SETWPROP("Tilt", 1);
    SETWPROP("OpticalFilter", 1);
    SETWPROP("EdgeEnhancement", 1);
  }
  return true;
}

bool videoDarwin::setIIDCProperty(OSType specifier, double value) {
  QTAtomContainer         atomContainer;
  QTAtom                  featureAtom;
  VDIIDCFeatureSettings   settings;
  ComponentDescription    desc;
  ComponentResult         result = paramErr;
    
  //IIDC stuff
  result = VDIIDCGetFeaturesForSpecifier(m_vdig, specifier, &atomContainer);
  if (noErr != result) {
    return false;
  }
    
  featureAtom = QTFindChildByIndex(atomContainer, kParentAtomIsContainer,
                                   vdIIDCAtomTypeFeature, 1, NULL);
  if (0 == featureAtom) return false;//error("featureAtom vdIIDCFeatureSaturation not found");
    
  result = QTCopyAtomDataToPtr(atomContainer,
                               QTFindChildByID(atomContainer, featureAtom,
                                               vdIIDCAtomTypeFeatureSettings,
                                               vdIIDCAtomIDFeatureSettings, NULL),
                               true, sizeof(settings), &settings, NULL);

  settings.state.flags = (vdIIDCFeatureFlagOn |
                          vdIIDCFeatureFlagManual |
                          vdIIDCFeatureFlagRawControl);
    
  settings.state.value = value;
    
  result = QTSetAtomData(atomContainer,
                         QTFindChildByID(atomContainer, featureAtom,
                                         vdIIDCAtomTypeFeatureSettings,
                                         vdIIDCAtomIDFeatureSettings, NULL),
                         sizeof(settings), &settings);
    
  result = VDIIDCSetFeatures(m_vdig, atomContainer);
    
  return true;
}
inline unsigned short d2us(double x)
{ return (unsigned short)(65535.*((x > 1.f) ? 1.f : ( (x < 0.f) ? 0.f : x))); }
inline double us2d(unsigned short x)
{ static double factor=1./65535.; return (x*factor); }

bool videoDarwin::applyProperties(gem::Properties&props) {
  bool restart=false;

  bool iidc=false;
  if(m_vdig) {
    ComponentDescription    desc;
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);
    iidc=(vdSubtypeIIDC == desc.componentSubType);
  }

  std::vector<std::string>keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    double value_d=0.;
    unsigned short value_us=0;
    std::string key=keys[i];
    if("width"==key) {
      if(props.get(key, value_d)) {
        unsigned int width=value_d;
        if(m_width!=width)
          restart=true;
        m_width=width;
      }
    } else if("height"==key) {
      if(props.get(key, value_d)) {
        unsigned int height=value_d;
        if(m_height!=height)
          restart=true;
        m_height=height;
      }
    } else if("channel"==key) {
      if(props.get("channel", value_d)) {
        unsigned int channel=value_d;
        if(channel!=m_inputDeviceChannel)
          restart=true;
        m_inputDeviceChannel=channel;
      }
    } else if("quality"==key) {
      if(props.get(key, value_d)) {
        unsigned int quality=value_d;
        bool doit=false;
        switch (quality){
        case 0:
          m_quality=channelPlayNormal;
          doit=true;
          break;
        case 1:
          m_quality=channelPlayHighQuality;
          doit=true;
          break;
        case 2:
          m_quality=channelPlayFast;
          doit=true;
          break;
        case 3:
          m_quality=channelPlayAllData;
          doit=true;
          break;
        default:
          break;
        }
        if(doit&&m_vc)
          SGSetChannelPlayFlags(m_vc, m_quality); 
      }
#define PROPSET_IIDC_VD(NAME) \
      } else if (#NAME == key && props.get(key, value_d) && m_vdig) {  \
      if(iidc){setIIDCProperty(vdIIDCFeature ## NAME, value_d);}        \
      else {value_us = d2us(value_d);                                   \
        VDSet ## NAME (m_vdig,&value_us); } value_d=0
#define PROPSET_VD(NAME)                                                \
        } else if (#NAME == key && props.get(key, value_d) && m_vdig) { \
      if(!iidc) {value_us = d2us(value_d);                            \
        VDSet ## NAME (m_vdig,&value_us); } value_d=0
#define PROPSET_IIDC(NAME)                                              \
        } else if (#NAME == key && props.get(key, value_d) && iidc) {  \
      setIIDCProperty(vdIIDCFeature ## NAME, value_d); value_d=0

        PROPSET_IIDC_VD(Hue);
        PROPSET_IIDC_VD(Sharpness);
        PROPSET_VD(Contrast);
        //PROPSET_VD(KeyColor);
        //PROPSET_VD(ClipState);
        //PROPSET_VD(ClipRng);
        //PROPSET_VD(PLLFilterType);
        PROPSET_VD(MasterBlendLevel);
        //PROPSET_VD(PlayThroughOnOff);
        //PROPSET_VD(FieldPreference);
        PROPSET_VD(BlackLevelValue);
        PROPSET_VD(WhiteLevelValue);
        //PROPSET_VD(Input);
        //PROPSET_VD(InputStandard);
        PROPSET_IIDC_VD(Saturation);
        PROPSET_IIDC_VD(Brightness);
        PROPSET_IIDC(Gain);
        PROPSET_IIDC(Iris);
        PROPSET_IIDC(Shutter);
        PROPSET_IIDC(Exposure);
        PROPSET_IIDC(WhiteBalanceU);
        PROPSET_IIDC(WhiteBalanceV);
        PROPSET_IIDC(Gamma);
        PROPSET_IIDC(Temperature);
        PROPSET_IIDC(Zoom);
        PROPSET_IIDC(Focus);
        PROPSET_IIDC(Pan);
        PROPSET_IIDC(Tilt);
        PROPSET_IIDC(OpticalFilter);
        PROPSET_IIDC(EdgeEnhancement);
    }    
  }
  return restart;
}
void videoDarwin::setProperties(gem::Properties&props) {
  bool restart=applyProperties(props);
  if(restart) {
    if(stop()) {
      resetSeqGrabber();
      start();
    }
  }
}


void videoDarwin::getProperties(gem::Properties&props) {
  std::vector<std::string>keys=props.keys();
  bool iidc=false;
  if(m_vdig) {
    ComponentDescription    desc;
    GetComponentInfo((Component)m_vdig, &desc, NULL, NULL, NULL);
    iidc=(vdSubtypeIIDC == desc.componentSubType);
  }

  int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    double value_d=0.;
    unsigned short value_us=0;
    if(0) {
#define PROPGET_VD(NAME)                                                \
      } else if (#NAME == key && m_vdig && !iidc) {                              \
      if(0==VDGet ## NAME (m_vdig,&value_us)) {props.set(key, us2d(value_us)); } value_d=0     
    PROPGET_VD(Hue);
    PROPGET_VD(Sharpness);
    PROPGET_VD(Saturation);
    PROPGET_VD(Brightness);
    PROPGET_VD(Contrast);
    //PROPGET_VD(KeyColor);
    //PROPGET_VD(ClipState);
    //PROPGET_VD(ClipRng);
    //PROPGET_VD(PLLFilterType);
    //PROPGET_VD(PlayThroughOnOff);
    //PROPGET_VD(FieldPreference);
    PROPGET_VD(BlackLevelValue);
    PROPGET_VD(WhiteLevelValue);
    //PROPGET_VD(Input);
    //PROPGET_VD(InputStandard);
    }
  }
}

std::vector<std::string> videoDarwin::enumerate() {
  std::vector<std::string> result;
  OSErr anErr;
  SGDeviceList    devices;

  anErr = SGGetChannelDeviceList(m_vc, sgDeviceListIncludeInputs, &devices);
  if(anErr!=noErr){
    error("could not get SG channnel Device List");
  }else{
    short deviceCount = (*devices)->count;
    short deviceIndex = (*devices)->selectedIndex;
    short inputIndex;
    logpost(NULL, 3, "SG channnel Device List count %d index %d",deviceCount,deviceIndex);
    int i;
    m_devices.clear();
    for (i = 0; i < deviceCount; i++){
      m_devices.push_back(pascal2str((*devices)->entry[i].name));
      logpost(NULL, 3, "SG channnel Device List[%d]  %s", i, m_devices[i].c_str());
    }
    SGGetChannelDeviceAndInputNames(m_vc, NULL, NULL, &inputIndex);

    bool showInputsAsDevices = ((*devices)->entry[deviceIndex].flags) & sgDeviceNameFlagShowInputsAsDevices;

    SGDeviceInputList theSGInputList = ((SGDeviceName *)(&((*devices)->entry[deviceIndex])))->inputs; //fugly

    //we should have device names in big ass undocumented structs
    //walk through the list
    for (i = 0; i < inputIndex; i++){
      std::string input=pascal2str((*theSGInputList)->entry[i].name);
      logpost(NULL, 3, "SG channnel Input Device List %d %s",
           i, input.c_str());
    }
  }

  result=m_devices;
  return result;
}
#endif // HAVE_VIDEODARWIN
