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

#include "videoUNICAP.h"
#include "plugins/PluginFactory.h"

using namespace gem::plugins;

// for debugging...
#include "Gem/RTE.h"

/* debugging helpers  */
#define debugPost
#define debugThread

#if 0
# undef debugPost 
# define debugPost ::startpost("%s:%s[%d]", __FILE__, __FUNCTION__, __LINE__); ::post
#endif

#if 0
# undef debugThread
# define debugThread ::startpost("%s:%s[%d]", __FILE__, __FUNCTION__, __LINE__); ::post
#endif


#ifdef FOURCC
# undef FOURCC
#endif


#define FOURCC(a,b,c,d) (unsigned int)((((unsigned int)d)<<24)+(((unsigned int)c)<<16)+(((unsigned int)b)<<8)+a) 
/*
  #define FOURCC(a) (unsigned int) ( 
  ((((unsigned char*)a)[0])<<24)+  \
  ((((unsigned char*)a)[1])<<16)+  \
  ((((unsigned char*)a)[2])<< 8)+  \
  ((((unsigned char*)a)[3])<< 0))
*/

#ifdef HAVE_UNICAP

static void post_fmt(unicap_format_t*fmt) {
  if(!fmt)return;
  debugPost("format %dx%d+%d+%d '%s' -> %d (%d[%d]/%d)",
            fmt->size.width, fmt->size.height,
            fmt->size.x, fmt->size.y,
            fmt->identifier,
            fmt->buffer_size,
            fmt->buffer_type,
            fmt->system_buffer_count,
            fmt->buffer_types);
}

/////////////////////////////////////////////////////////
//
// videoUNICAP
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

#include <sys/stat.h>

REGISTER_VIDEOFACTORY("unicap", videoUNICAP);

videoUNICAP :: videoUNICAP() : videoBase("unicap", 0)
                             , m_handle(NULL)
{
  m_width=0; m_height=0;
  provide("analog");
  enumerate();
}
  
////////////////////////////////////////////////////////
// Destructor
//
////////////////////////////////////////////////////////
videoUNICAP :: ~videoUNICAP()
{
  close();
}

//////////////////
// this reads the data that was captured by capturing() and returns it within a pixBlock
pixBlock *videoUNICAP :: getFrame(){

  mutex.lock();
  return &m_image;
}

void videoUNICAP::releaseFrame() {
  mutex.unlock();
  videoBase::releaseFrame();
}

bool videoUNICAP :: openDevice(gem::Properties&props) {
  std::vector<unsigned int>ds;
  if(m_devicename.empty()) {
    ds.push_back(m_devicenum);
  } else {
    ds=m_name2devices[m_devicename];
  }
  unicap_status_t status = -1;
  int i=0;
  for(i=0; i<ds.size(); i++) {
    unicap_device_t device;
    unsigned int d=ds[i];
    status = unicap_enumerate_devices (NULL, &device, d); // (1)
    if(SUCCESS(status)) {
      status = unicap_open (&m_handle, &device);  
      if(SUCCESS(status)) {
        continue;
      }
    }
  }
  if(!SUCCESS(status))
    return false;

  setProperties(props);
  return true;
}
void videoUNICAP :: closeDevice() {
  if(m_handle) {
    unicap_close(m_handle);
    m_handle=NULL;
  }
}

void videoUNICAP::newFrameCB (unicap_event_t event, 
                              unicap_handle_t handle, 
                              unicap_data_buffer_t * buffer, 
                              void *usr_data)
{
  videoUNICAP*v=(videoUNICAP*)usr_data;
  debugPost("newFrameCB");
  if(v && UNICAP_EVENT_NEW_FRAME==event)
    v->newFrame(handle, buffer);
}

typedef enum {
  FIRST=0,
  RGB, 
  RGBA, 
  BGR,
  BGRA,
  RGB16,
  ABGR,
  ARGB,
  GREY,
  GREY16,
  UYVY,
  YUY2,
  YVYU,
  YV12,
  YU12,
  
  ILLEGAL
} fourcc_t;


void videoUNICAP::newFrame (unicap_handle_t handle, 
                            unicap_data_buffer_t * buffer) {
  unicap_format_t*fmt=&(buffer->format);
  post_fmt(fmt);
  fourcc_t format=ILLEGAL;

  switch(fmt->fourcc) {
  case (FOURCC('R', 'G', 'B', '2')):    format=RGB;    break;
  case (FOURCC('R', 'G', 'B', ' ')):    format=RGB;    break;
  case (FOURCC('R', 'G', 'B', 'A')):    format=RGBA;   break;
  case (FOURCC('B', 'G', 'R', ' ')):    format=BGR;    break;
  case (FOURCC('B', 'G', 'R', 'A')):    format=BGRA;   break;
  case (FOURCC('A', 'B', 'G', 'R')):    format=ABGR;   break;
  case (FOURCC('A', 'R', 'G', 'B')):    format=ARGB;   break;

  case (FOURCC('Y', '8', '0', '0')):
  case (FOURCC('Y', ' ', ' ', ' ')):
  case (FOURCC('G', 'R', 'E', 'Y')):    format=GREY;    break;
  case (FOURCC('Y', '1', '6', ' ')):    format=GREY16;  break;

  case (FOURCC('U', 'Y', 'N', 'V')):
  case (FOURCC('Y', '4', '2', '2')):
  case (FOURCC('H', 'D', 'Y', 'C')): // ?
  case (FOURCC('U', 'Y', 'V', 'Y')):    format=UYVY;    break;

  case (FOURCC('Y', 'U', 'Y', 'V')):
  case (FOURCC('Y', 'U', 'N', 'V')):
  case (FOURCC('Y', 'U', 'Y', '2')):    format=YUY2;    break;

  case (FOURCC('Y', 'V', 'Y', 'U')):    format=YVYU;    break;

  default:
    logpost(NULL, 5, "unsupported format '%s'", fmt->identifier);
    break;
  }

  unsigned char*data=buffer->data;

  lock();
  m_image.image.xsize=fmt->size.width;
  m_image.image.ysize=fmt->size.height;

  m_image.image.upsidedown=true;

  m_image.image.reallocate(); // actually this is also done in from*()
  switch (format) {
  case RGB: m_image.image.fromRGB(data); break;
  case RGBA: m_image.image.fromRGBA(data); break;
  case BGR: m_image.image.fromBGR(data); break;
  case BGRA: m_image.image.fromBGRA(data); break;
  case RGB16: m_image.image.fromRGB16(data); break;
  case ABGR: m_image.image.fromABGR(data); break;
  case ARGB: m_image.image.fromARGB(data); break;
  case GREY: m_image.image.fromGray(data); break;
  case GREY16: m_image.image.fromGray((short*)data); break;
  case UYVY: m_image.image.fromUYVY(data); break;
  case YUY2: m_image.image.fromYUY2(data); break;
  case YVYU: m_image.image.fromYVYU(data); break;
  case YV12: m_image.image.fromYV12(data); break;
  case YU12: m_image.image.fromYU12(data); break;
  default:
    logpost(NULL, 5, "cannot convert from given format");
    break;
  }
  m_image.newimage=1;

  unlock();
}


/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoUNICAP :: startTransfer()
{
  unicap_status_t status = 0;
  unicap_format_t format;
  defaultFormat();
  if (!SUCCESS (unicap_get_format (m_handle, &format))) {
    logpost(NULL, 5, "failed to query format");
    return false;
  }
  debugPost("got format: %x", status);
  post_fmt(&format);
  format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM; 
  if (!SUCCESS (unicap_set_format (m_handle, &format))) {
    logpost(NULL, 5, "failed to set format (sysbuf)");
    
    return false;
  }
  debugPost("set format %x", status);
  status=unicap_register_callback (m_handle, 
                                   UNICAP_EVENT_NEW_FRAME, 
                                   (unicap_callback_t) newFrameCB,
                                   (void *) this);
  debugPost("registered callback: %x", status);
  if(!SUCCESS(status))
    return false;
  status=unicap_start_capture (m_handle);
  debugPost("start capture: %x", status);
  if(!SUCCESS(status))
    return false;
  return true;
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoUNICAP :: stopTransfer()
{
  unicap_stop_capture (m_handle);        // (3)

  return true;
}

bool videoUNICAP :: setColor(int format)
{
  if (format<=0 || format==m_reqFormat)return -1;
  m_reqFormat=format;
  restartTransfer();
  return true;
}

std::vector<std::string> videoUNICAP::enumerate() {
  std::vector<std::string> result;
  int devcount=0;
  unicap_status_t status = 0;
  int i=0;

  status = unicap_reenumerate_devices(&devcount);
  if(!SUCCESS(status))
    return result;

  m_devices.clear();
  m_name2devices.clear();


  for(i=0; i<devcount; i++) {
    unicap_device_t device;
    status = unicap_enumerate_devices (NULL, &device, i); // (1)


    if(SUCCESS(status)) {
      const unsigned int cur=m_devices.size();
#if 0
      post("ID='%s'\tmodel='%s'\tvendor='%s'\tdevice='%s'\tCPI='%s'",
           device.identifier,
           device.model_name,
           device.vendor_name,
           device.device,
           device.cpi_layer);
#endif   
      m_devices.push_back(device);

      m_name2devices[device.identifier ].push_back(cur);
      m_name2devices[device.model_name ].push_back(cur);
      m_name2devices[device.vendor_name].push_back(cur);
      if(device.device[0])
        m_name2devices[device.device].push_back(cur);

      /* hmm, think about this .... */
      std::string cpi=device.cpi_layer;
      cpi=cpi.substr(cpi.rfind("/")+1);
      cpi=cpi.substr(0, cpi.find("."));

      provide(cpi);

      result.push_back(device.identifier);
    }
  }

  return result;
}

bool videoUNICAP :: defaultFormat() {
  if(!m_handle)return false;
  int count=0;
  unicap_status_t status= unicap_reenumerate_formats(m_handle, &count );
  if(!SUCCESS(status))
    return false;

  if(count==0)return true;

  unicap_format_t format;


  status=unicap_enumerate_formats( m_handle, NULL, &format, 0 );

  debugPost("got %d formats", count);

  post_fmt(&format);

#if 0
  if(count==1) {
    
    status= unicap_set_format(m_handle, &format);
    return SUCCESS(status);
  }
#endif
  

  return true;
}


bool videoUNICAP :: enumProperties(gem::Properties&readable,
                                   gem::Properties&writeable) {
  readable.clear();
  writeable.clear();

  if(m_handle) {
    int count=0;
    unicap_status_t status= unicap_reenumerate_properties(m_handle, &count );
    if(!SUCCESS(status))
      return false;

    int id=0;
    for(id=0; id<count; id++) {
      unicap_property_t prop;
      gem::any typ;

      status = unicap_enumerate_properties(m_handle, NULL, &prop, id);
      if(!SUCCESS(status))
        continue;

      debugPost("id='%s'\tcat='%s'\tunit='%s'\tflags=%d", 
                prop.identifier,
                prop.category,
                prop.unit,
                prop.flags);


      switch(prop.type) {
      case UNICAP_PROPERTY_TYPE_RANGE: 
        debugPost("range %f-%f", prop.range.min, prop.range.min);
        typ=prop.range.max; 
        break;
      case UNICAP_PROPERTY_TYPE_VALUE_LIST: 
        debugPost("value_list %d", prop.value_list.value_count);
        typ=prop.value_list.value_count;
        break;
      case UNICAP_PROPERTY_TYPE_MENU: 
        debugPost("menu '%s' of %d", prop.menu_item, prop.menu.menu_item_count);
        typ=std::string(prop.menu_item);//prop.menu.menu_item_count;
        break;
      case UNICAP_PROPERTY_TYPE_FLAGS: 
        debugPost("flags");
        break;
      default:
        debugPost("unknown");
        // ?
        break;
      }

      readable.set(prop.identifier, typ);
      if(!(prop.flags & UNICAP_FLAGS_READ_ONLY))
        writeable.set(prop.identifier, typ);

#warning check UNICAP_FLAGS_ON_OFF & UNICAP_FLAGS_ONE_PUSH
    }
  }
  return true;
}
void videoUNICAP :: getProperties(gem::Properties&props) {
  if(!m_handle)return;
  unicap_status_t status=0;

  std::vector<std::string> keys=props.keys();

  bool getwidth=false, getheight=false;
  int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    unicap_property_t prop;
    strncpy(prop.identifier, key.c_str(), 128);

    if("width"==key) {
      getwidth=true;
      continue;
    }
    if("height"==key) {
      getheight=true;
      continue;
    }

    status= unicap_get_property(m_handle, &prop );

    if(SUCCESS(status)) {
      switch(prop.type) {
      case UNICAP_PROPERTY_TYPE_VALUE_LIST: 
      case UNICAP_PROPERTY_TYPE_FLAGS: 
      case UNICAP_PROPERTY_TYPE_RANGE: 
        props.set(key, prop.value);
        break;
      case UNICAP_PROPERTY_TYPE_MENU:
        props.set(key, std::string(prop.menu_item));
        break;
      default:
        props.erase(key);
        // ?
        break;
      }
    }
  }


  if(getwidth||getheight) {
    unicap_format_t fmt;
    status=unicap_get_format(m_handle, &fmt);
    post("getting dimen");post_fmt(&fmt);
    if(SUCCESS(status)) {
      if(getwidth )props.set("width" , fmt.size.width);
      if(getheight)props.set("height", fmt.size.height);
    }
  }


}
void videoUNICAP :: setProperties(gem::Properties&props) {
  if(!m_handle)
    return;

  unicap_status_t status = 0;
  bool restart=false;

  unsigned int width=0, height=0;

  std::vector<std::string> keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];

    double d=0;
    std::string s;


    if("width"==key) {
      if(props.get(key, d)) {
        width=d;
        if(m_width!=width) {
          m_width=width;
          restart=true;
        }
      }
      continue;
    }
    if ("height"==key) {
      if(props.get(key, d)) {
        height=d;
        if(m_height!=height) {
          m_height=height;
          restart=true;
        }
      }
      continue;
    }

    unicap_property_t prop;
    strncpy(prop.identifier, key.c_str(), 128);
    status=unicap_get_property(m_handle, &prop );

    if(SUCCESS(status)) {
      switch(prop.type) {
      case UNICAP_PROPERTY_TYPE_VALUE_LIST: 
      case UNICAP_PROPERTY_TYPE_FLAGS: 
      case UNICAP_PROPERTY_TYPE_RANGE: 
        if(props.get(key, d)) {
          prop.value=d;
          status= unicap_set_property(m_handle, &prop );
        }
        break;
      case UNICAP_PROPERTY_TYPE_MENU:
        if(props.get(key, d)) {
          if(d>=0 && d < prop.menu.menu_item_count) {
            int i=d;
            post("directly setting menu-value to '%s' (please report if this works!)", prop.menu.menu_items[i]);
            prop.value=i;
            status= unicap_set_property(m_handle, &prop );
          }
        } else if (props.get(key, s)) {
          post("setting menu-value to '%s' (please report if this works!)", s.c_str());
          strncpy(prop.menu_item, s.c_str(), 128);
          status= unicap_set_property(m_handle, &prop );
        }
        break;
      default:
        // ?
        break;
      }

      if(!SUCCESS(status)) {
        logpost(NULL, 5, "could not set property '%s'", key.c_str());
#if 0
      } else {
        logpost(NULL, 5, "successfully set property '%s'", key.c_str());
#endif
      }

    }
  }

  while(restart) { restart=false;
    unicap_format_t fmt;
    status=unicap_get_format(m_handle, &fmt);
    post("setting dimen(%d)", status);post_fmt(&fmt);


    if(!SUCCESS(status))break;
    bool running=stop();
    debugPost("running=%d", running);

    if(m_width>0)
      fmt.size.width=m_width;
    if(m_height>0)
      fmt.size.height=m_height;

    status=unicap_set_format(m_handle, &fmt);

    if (running)start();
  }
}


#else
videoUNICAP ::  videoUNICAP() : videoBase("") {}
videoUNICAP :: ~videoUNICAP() {}
#endif /* HAVE_UNICAP */
