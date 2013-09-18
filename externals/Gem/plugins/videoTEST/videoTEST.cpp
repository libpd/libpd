#include "videoTEST.h"
#include "plugins/PluginFactory.h"
using namespace gem::plugins;

REGISTER_VIDEOFACTORY("test", videoTEST);

static double getRandom(void) {
  static unsigned int random_nextseed = 1489853723;
  random_nextseed = random_nextseed * 435898247 + 938284281;
  return random_nextseed * (1./4294967296.);;
}

videoTEST::videoTEST() :
  m_name(std::string("test")),
  m_open(false),
  m_type(0)
{
  m_pixBlock.image.xsize = 64;
  m_pixBlock.image.ysize = 64;
  m_pixBlock.image.setCsizeByFormat(GL_RGBA);
  m_pixBlock.image.reallocate();
}

videoTEST::~videoTEST(void) {
}

bool videoTEST::open(gem::Properties&props) {
  setProperties(props);
  return (m_open);
}

static void setNoise(unsigned char*data, unsigned int count) {
  unsigned int i=0;
  for(i=0; i<count;i++) {
    *data++=(255*getRandom());
    *data++=(255*getRandom());
    *data++=(255*getRandom());
    *data++=255;
  }
}
static void setRed(unsigned char*data, unsigned int count) {
  unsigned int i=0;
  for(i=0; i<count;i++) {
    data[chRed]=255;
    data[chGreen]=0;
    data[chBlue]=0;
    data[chAlpha]=255;
    data+=4;
  }
}
static void setGreen(unsigned char*data, unsigned int count) {
  unsigned int i=0;
  for(i=0; i<count;i++) {
    data[chRed]=0;
    data[chGreen]=255;
    data[chBlue]=0;
    data[chAlpha]=255;
    data+=4;
  }
}
static void setBlue(unsigned char*data, unsigned int count) {
  unsigned int i=0;
  for(i=0; i<count;i++) {
    data[chRed]=0;
    data[chGreen]=0;
    data[chBlue]=255;
    data[chAlpha]=255;
    data+=4;
  }
}

pixBlock*videoTEST::getFrame(void) {
  m_pixBlock.image.setCsizeByFormat(GL_RGBA);
  m_pixBlock.image.reallocate();
  const unsigned int count = m_pixBlock.image.xsize * m_pixBlock.image.ysize;
  unsigned int i=0;
  unsigned char*data=m_pixBlock.image.data;

  switch(m_type) {
  case  1: setRed(data, count); break;
  case  2: setGreen(data, count); break;
  case  3: setBlue(data, count); break;
  default: setNoise(data, count); break;
  }

  m_pixBlock.newimage = true;

  return &m_pixBlock;
}

std::vector<std::string>videoTEST::enumerate(void) {
  std::vector<std::string>result;
  result.push_back("test");
  return result;
}

bool videoTEST::setDevice(int ID) {
  m_open=(0==ID);
  return m_open;
}
bool videoTEST::setDevice(std::string device) {
  m_open=("test"==device);
  return m_open;
}
bool videoTEST::enumProperties(gem::Properties&readable,
			       gem::Properties&writeable) {
  readable.clear();
  writeable.clear();


  writeable.set("width", 64);  readable.set("width", 64);
  writeable.set("height", 64); readable.set("height", 64);

  writeable.set("type", std::string("noise"));
}
void videoTEST::setProperties(gem::Properties&props) {
  m_props=props;

  double d;
  if(props.get("width", d)) {
    if(d>0)
      m_pixBlock.image.xsize = d;
  }
  if(props.get("height", d)) {
    if(d>0)
      m_pixBlock.image.ysize = d;
  }
  std::string s;
  if(props.get("type", s)) {
    if("noise"==s)
      m_type=0;
    else if("red"==s)
      m_type=1;
    else if("green"==s)
      m_type=2;
    else if("blue"==s)
      m_type=3;
  }
}
void videoTEST::getProperties(gem::Properties&props) {
  std::vector<std::string>keys=props.keys();
  double d;
  int i;
  for(i=0; i<keys.size(); i++) {
    if("width"==keys[i]) {
      props.set(keys[i], m_pixBlock.image.xsize);
    }
    if("height"==keys[i]) {
      props.set(keys[i], m_pixBlock.image.ysize);
    }
  }
}

std::vector<std::string>videoTEST::dialogs(void) {
  std::vector<std::string>result;
  return result;
}
bool videoTEST::provides(const std::string name) {
  return (name==m_name);
}
std::vector<std::string>videoTEST::provides(void) {
  std::vector<std::string>result;
  result.push_back(m_name);
  return result;
}
const std::string videoTEST::getName(void) {
  return m_name;
}
