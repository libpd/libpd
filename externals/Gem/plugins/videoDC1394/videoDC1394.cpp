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

#include "videoDC1394.h"
using namespace gem::plugins;

#include "Gem/RTE.h"
#include "Gem/Exception.h"
#include "plugins/PluginFactory.h"

#ifdef HAVE_LIBDC1394
#include <inttypes.h>

REGISTER_VIDEOFACTORY("dc1394", videoDC1394);
/////////////////////////////////////////////////////////
//
// videoDC1394
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
videoDC1394 :: videoDC1394() : videoBase("dc1394"),
                               m_dccamera(NULL),
                               m_dcframe(NULL),
                               m_dc(NULL)
{
  m_dc = dc1394_new(); /* Initialize libdc1394 */
  if(!m_dc) throw(GemException("unable to initialize DC1394"));

  m_frame.xsize=1600;
  m_frame.ysize=1200;
  m_frame.setCsizeByFormat(GL_RGBA);
  m_frame.allocate();

  provide("iidc");
}

////////////////////////////////////////////////////////
// Destructor
//
////////////////////////////////////////////////////////
videoDC1394 :: ~videoDC1394(){
  close();

  if(m_dccamera)dc1394_camera_free (m_dccamera);m_dccamera=NULL;
  if(m_dc)dc1394_free(m_dc);m_dc=NULL;
}
////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
bool videoDC1394 :: grabFrame()
{
  dc1394video_frame_t*frame, *colframe;
  dc1394error_t err=dc1394_capture_dequeue(m_dccamera, DC1394_CAPTURE_POLICY_POLL, &frame);/* Capture */
  if((DC1394_SUCCESS!=err)||(NULL==frame)) {
    usleep(10);
    return true;
  }

  /* do something with the frame */
  colframe=( dc1394video_frame_t*)calloc(1,sizeof(dc1394video_frame_t));
  colframe->color_coding=DC1394_COLOR_CODING_RGB8;
  dc1394_convert_frames(frame, colframe);

  m_frame.xsize=frame->size[0];
  m_frame.ysize=frame->size[1];
  m_frame.setCsizeByFormat(GL_RGBA);
  m_frame.fromRGB(colframe->image);

  lock();
  m_image.image.convertFrom(&m_frame, m_reqFormat);
  m_image.newimage=true;
  m_image.image.upsidedown=true;
  unlock();

  free(colframe->image);
  free(colframe);

  /* Release the buffer */
  err=dc1394_capture_enqueue(m_dccamera, frame);
  if(DC1394_SUCCESS!=err) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////
// openDevice
//
/////////////////////////////////////////////////////////
static std::string guid2string(uint64_t guid, int unit=-1) {
  std::string result;
  char buf[64];
  uint32_t value[2];

  value[0]= guid & 0xffffffff;
  value[1]= (guid >>32) & 0xffffffff;

  snprintf(buf, 64, "0x%08x%08x", value[0], value[1]);
  buf[63]=0;
  result=buf;

  if(unit>=0) {
    snprintf(buf, 64, "%d", unit);
    buf[63]=0;
    result+=":";
    result+=buf;
  }

  return result;
}


bool videoDC1394 :: openDevice(gem::Properties&props){
  dc1394error_t err;
  dc1394camera_list_t *list=NULL;

  err=dc1394_camera_enumerate (m_dc, &list); /* Find cameras */
  if(DC1394_SUCCESS!=err) {
    error("videoDC1394: %s: failed to enumerate", dc1394_error_get_string(err));
    return false;
  }
  if (list->num < 1) {
    error("videoDC1394: no cameras found");
    dc1394_camera_free_list (list);
    return false;
  }
  int devicenum=-1;
  if(m_devicenum>=0)devicenum=m_devicenum;
  else if (!m_devicename.empty()) {
    int i=0;
    for(i=0; i<list->num; i++) {
      // find camera based on its GUID
      std::string name=guid2string(list->ids[i].guid);
      if(guid2string(list->ids[i].guid)==m_devicename){
        devicenum=i;
        break;
      }
      if(guid2string(list->ids[i].guid, list->ids[i].unit)==m_devicename){
        devicenum=i;
        break;
      }
    }
  }
  if(devicenum<0) {
    return false;
  }

  if (devicenum < list->num) {
    /* Work with first camera */
    m_dccamera = dc1394_camera_new_unit(m_dc,
                                        list->ids[devicenum].guid,
                                        list->ids[devicenum].unit);
  } else {
    m_dccamera=NULL;
    error("videoDC1394: only found %d cameras but requested #%d!", list->num, devicenum);
  }
  dc1394_camera_free_list (list);

  if(!m_dccamera) {
    error("videoDC1394: could not access camera!");
    return false;
  }

  logpost(NULL, 5, "videoDC1394: using camera with GUID %s", guid2string(m_dccamera->guid, m_dccamera->unit).c_str());

  setProperties(props);

  if(gem::Properties::UNSET==props.type("mode")){
    /* check supported video modes */
    dc1394video_modes_t video_modes;
    dc1394video_mode_t  video_mode;
    dc1394color_coding_t coding;

    err=dc1394_video_get_supported_modes(m_dccamera,&video_modes);
    if(DC1394_SUCCESS!=err) {
      error("can't get video modes");
      closeDevice();
      return false;
    }
    int mode=-1;
    double d;
    if(props.get("channel", d)) // this used to be 'channel' rather than 'isochannel'
      mode=d;

    logpost(NULL, 5, "trying mode %d", mode);

    if(mode>=0) {
      if(mode>=video_modes.num) {
	error("requested channel %d/%d out of bounds", mode, video_modes.num);
	mode=-1;
      }
    }

    int i;
    for (i=video_modes.num-1;i>=0;i--) {
      unsigned int w=0, h=0;
      if(DC1394_SUCCESS==dc1394_get_image_size_from_video_mode(m_dccamera, video_modes.modes[i], &w, &h)) {
	logpost(NULL, 5, "videomode[%02d/%d]=%dx%d", i, video_modes.num, w, h);
      } else logpost(NULL, 5, "videomode %d refused dimen: %d", i, video_modes.modes[i]);

      dc1394_get_color_coding_from_video_mode(m_dccamera,video_modes.modes[i], &coding);
      dc1394bool_t iscolor=DC1394_FALSE;
      if(DC1394_SUCCESS==dc1394_is_color(coding, &iscolor)) {
	logpost(NULL, 5, "videomode[%02d/%d] %d is%scolor", i, video_modes.num, coding, (iscolor?" ":" NOT "));
      }

      if(mode<0) {  // find a mode matching the user's needs
	if(m_width==w && m_height==h) {
	  // what about color?
	  mode=i;
	  break;
	}
      }
    }

    if(mode<0) {
      // select highest res mode:
      for (i=video_modes.num-1;i>=0;i--) {
	if (!dc1394_is_video_mode_scalable(video_modes.modes[i])) {
	  dc1394_get_color_coding_from_video_mode(m_dccamera,video_modes.modes[i], &coding);

	  video_mode=video_modes.modes[i];
	  break;
	}
      }
      if (i < 0) {
	error("Could not get a valid mode");
	closeDevice();
	return false;
      }
    } else {
      logpost(NULL, 5, "using mode %d", mode);
      video_mode=video_modes.modes[mode];
    }

    if(1) {
      unsigned int w=0, h=0;
      if(DC1394_SUCCESS==dc1394_get_image_size_from_video_mode(m_dccamera, video_mode, &w, &h)) {
      logpost(NULL, 5, "videomode[%d]=%dx%d", video_mode, w, h);
      }
      dc1394_get_color_coding_from_video_mode(m_dccamera,video_mode, &coding);
      dc1394bool_t iscolor=DC1394_FALSE;
      if(DC1394_SUCCESS==dc1394_is_color(coding, &iscolor)) {
	logpost(NULL, 5, "videomode %d is%scolor", coding, (iscolor?" ":" NOT "));
      }
    }

    err=dc1394_video_set_mode(m_dccamera, video_mode);
    if(DC1394_SUCCESS!=err) {
      error("unable to set specified mode, using default");
    }
  }

  if(gem::Properties::UNSET==props.type("operationmode")){
    // try to set highest possible operation mode
    // FIXME this should be done via properties
    int operation_mode=DC1394_OPERATION_MODE_MAX;
    while(operation_mode>=DC1394_OPERATION_MODE_MIN) {
      err=dc1394_video_set_operation_mode(m_dccamera, (dc1394operation_mode_t)operation_mode);
      if(DC1394_SUCCESS==err)
	break;
      logpost(NULL, 5, "failed to set operation mode to %d", operation_mode);
      operation_mode--;
    }
    if(DC1394_SUCCESS!=err) {
      error("unable to set operation mode...continuing anyhow");
    }
  }

  if(gem::Properties::UNSET==props.type("speed")){
    // FIXME this should be done via properties
    dc1394speed_t orgspeed;
    dc1394_video_get_iso_speed(m_dccamera, &orgspeed);

    int speed=DC1394_ISO_SPEED_MAX;
    while(speed>=DC1394_ISO_SPEED_MIN) {
      err=dc1394_video_set_iso_speed(m_dccamera, (dc1394speed_t)speed);
      if(DC1394_SUCCESS==err)
	break;
      logpost(NULL, 5, "failed to set ISO speed to %d", 100*(1<<speed));

      speed--;
    }
    if(DC1394_SUCCESS!=err) {
      error("unable to set ISO speed...trying to set to original (%d)", 100*(1<<orgspeed));
      dc1394_video_get_iso_speed(m_dccamera, &orgspeed);
    }
  }

  if(gem::Properties::UNSET==props.type("framerate")){
    // get highest framerate
    dc1394framerates_t framerates;
    dc1394framerate_t framerate;
    dc1394video_mode_t  video_mode;

    err=dc1394_video_get_mode(m_dccamera, &video_mode);

    err=dc1394_video_get_supported_framerates(m_dccamera,video_mode,&framerates);
    if(DC1394_SUCCESS==err) {
      framerate=framerates.framerates[framerates.num-1];
      err=dc1394_video_set_framerate(m_dccamera, framerate);
      float fr=0;
      dc1394_framerate_as_float(framerate, &fr);
      logpost(NULL, 5, "DC1394: set framerate to %g", fr);
    }
  }

  err=dc1394_capture_setup(m_dccamera,
                           4,  /* 4 DMA buffers */
                           DC1394_CAPTURE_FLAGS_DEFAULT);     /* Setup capture */
  if(DC1394_SUCCESS!=err) {
    error("videoDC1394: %s: failed to enumerate", dc1394_error_get_string(err));
    return false;
  }

  logpost(NULL, 5, "DC1394: Successfully opened...");
  return true;
}
////////////////////////////////////////////////////////
// closeDevice
//
/////////////////////////////////////////////////////////
void videoDC1394 :: closeDevice(void){
  if(m_dccamera) {
    dc1394error_t err=dc1394_capture_stop(m_dccamera);  /* Stop capture */
  }
}

////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoDC1394 :: startTransfer()
{
  /* Start transmission */
  dc1394error_t err=dc1394_video_set_transmission(m_dccamera, DC1394_ON);
  if(DC1394_SUCCESS!=err) {
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoDC1394 :: stopTransfer()
{
  /* Stop transmission */
  dc1394error_t err=dc1394_video_set_transmission(m_dccamera, DC1394_OFF);
  if(DC1394_SUCCESS!=err) {
    error("unable to stop transmission");
  }
  return true;
}

std::vector<std::string>videoDC1394 :: enumerate(){
  std::vector<std::string>result;

  dc1394camera_list_t *list=NULL;
  dc1394error_t err=dc1394_camera_enumerate (m_dc, &list); /* Find cameras */
  if(DC1394_SUCCESS!=err) {
    return result;
  }

  int i=0;
  for(i=0; i<list->num; i++) {
    //    post("IIDC#%02d: %"PRIx64"\t%x\t%s", i, list->ids[i].guid, list->ids[i].unit, buf);
    result.push_back(guid2string(list->ids[i].guid, list->ids[i].unit));
  }
  return result;
}

bool videoDC1394 :: setColor(int format){
  if (format<=0)return false;
  m_reqFormat=format;
  return true;
}

bool videoDC1394::enumProperties(gem::Properties&readable,
			      gem::Properties&writeable) {
  /*
    framerate
    mode
    operationmode
    isospeed
    isochannel
    transmission
    oneshot
    multishot
  */

  gem::any type;
  std::string key;

  readable.clear();
  writeable.clear();

  key="framerate"; type=0;
  readable .set(key, type);
  writeable.set(key, type);


  key="mode"; type=std::string("");
  readable .set(key, type);
  writeable.set(key, type);

  key="operationmode"; type=std::string("");
  readable .set(key, type);
  writeable.set(key, type);

  key="speed"; type=0;
  readable .set(key, type);
  writeable.set(key, type);

  key="channel"; type=0;
  readable .set(key, type);
  writeable.set(key, type);

  key="transmission"; type=0;
  readable .set(key, type);
  writeable.set(key, type);

  key="oneshot"; type=0;
  readable .set(key, type);
  writeable.set(key, type);

  key="multishot"; type=0;
  readable .set(key, type);
  writeable.set(key, type);
  
  dc1394featureset_t feature_set;
  dc1394error_t err;
  
  err = dc1394_feature_get_all(m_dccamera, &feature_set);
  dc1394bool_t is_readable;
  if ( err ) return false;
    
  for ( int i = 0 ; i < DC1394_FEATURE_NUM ; i++ ) {
	  if ( feature_set.feature[i].available ) {
		  // TODO remove space in feature name (eg. "Trigger Delay")
		  key = dc1394_feature_get_string(feature_set.feature[i].id);
		  type=0; // what is this ?
		  dc1394_feature_is_readable(m_dccamera, feature_set.feature[i].id, &is_readable );
		  if ( is_readable ) readable.set(key,type);
		  writeable.set(key,type);
		}
  }
  
  return true;
}
void videoDC1394::getProperties(gem::Properties&props) {
  if(!m_dccamera)return;
  std::vector<std::string>keys=props.keys();
  double value;
  std::string svalue;

  int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    dc1394error_t err=DC1394_SUCCESS;
#define DC1394_TRYGET(x)					    \
      if(DC1394_SUCCESS!=(err=dc1394_video_get_##x)) {			    \
	error("videoDC1394: getting '%s' failed with '%s'", \
	      key.c_str(),				    \
	      dc1394_error_get_string(err));		    \
	continue;					    \
      }

    if("framerate" == key) {
      dc1394framerate_t framerate;
      //      if(dc1394_video_get_framerate(m_dccamera, &framerate))continue;
      DC1394_TRYGET(framerate(m_dccamera, &framerate));
      switch(framerate) {
      case DC1394_FRAMERATE_1_875: value=  1.875; break;
      case DC1394_FRAMERATE_3_75 : value=  3.75 ; break;
      case DC1394_FRAMERATE_7_5  : value=  7.5  ; break;
      case DC1394_FRAMERATE_15   : value= 15    ; break;
      case DC1394_FRAMERATE_30   : value= 30    ; break;
      case DC1394_FRAMERATE_60   : value= 60    ; break;
      case DC1394_FRAMERATE_120  : value=120    ; break;
      case DC1394_FRAMERATE_240  : value=240    ; break;
      default:
	continue;
      }
      props.set(key, value);
    } else if("mode" == key) {
      dc1394video_mode_t mode;
      DC1394_TRYGET(mode(m_dccamera, &mode));
      switch(mode) {
      case DC1394_VIDEO_MODE_160x120_YUV444: svalue="160x120_YUV444"; break;
      case DC1394_VIDEO_MODE_320x240_YUV422: svalue="320x240_YUV422"; break;
      case DC1394_VIDEO_MODE_640x480_YUV411: svalue="640x480_YUV411"; break;
      case DC1394_VIDEO_MODE_640x480_YUV422: svalue="640x480_YUV422"; break;
      case DC1394_VIDEO_MODE_640x480_RGB8: svalue="640x480_RGB8"; break;
      case DC1394_VIDEO_MODE_640x480_MONO8: svalue="640x480_MONO8"; break;
      case DC1394_VIDEO_MODE_640x480_MONO16: svalue="640x480_MONO16"; break;
      case DC1394_VIDEO_MODE_800x600_YUV422: svalue="800x600_YUV422"; break;
      case DC1394_VIDEO_MODE_800x600_RGB8: svalue="800x600_RGB8"; break;
      case DC1394_VIDEO_MODE_800x600_MONO8: svalue="800x600_MONO8"; break;
      case DC1394_VIDEO_MODE_1024x768_YUV422: svalue="1024x768_YUV422"; break;
      case DC1394_VIDEO_MODE_1024x768_RGB8: svalue="1024x768_RGB8"; break;
      case DC1394_VIDEO_MODE_1024x768_MONO8: svalue="1024x768_MONO8"; break;
      case DC1394_VIDEO_MODE_800x600_MONO16: svalue="800x600_MONO16"; break;
      case DC1394_VIDEO_MODE_1024x768_MONO16: svalue="1024x768_MONO16"; break;
      case DC1394_VIDEO_MODE_1280x960_YUV422: svalue="1280x960_YUV422"; break;
      case DC1394_VIDEO_MODE_1280x960_RGB8: svalue="1280x960_RGB8"; break;
      case DC1394_VIDEO_MODE_1280x960_MONO8: svalue="1280x960_MONO8"; break;
      case DC1394_VIDEO_MODE_1600x1200_YUV422: svalue="1600x1200_YUV422"; break;
      case DC1394_VIDEO_MODE_1600x1200_RGB8: svalue="1600x1200_RGB8"; break;
      case DC1394_VIDEO_MODE_1600x1200_MONO8: svalue="1600x1200_MONO8"; break;
      case DC1394_VIDEO_MODE_1280x960_MONO16: svalue="1280x960_MONO16"; break;
      case DC1394_VIDEO_MODE_1600x1200_MONO16: svalue="1600x1200_MONO16"; break;
      case DC1394_VIDEO_MODE_EXIF: svalue="EXIF"; break;
      case DC1394_VIDEO_MODE_FORMAT7_0: svalue="FORMAT7_0"; break;
      case DC1394_VIDEO_MODE_FORMAT7_1: svalue="FORMAT7_1"; break;
      case DC1394_VIDEO_MODE_FORMAT7_2: svalue="FORMAT7_2"; break;
      case DC1394_VIDEO_MODE_FORMAT7_3: svalue="FORMAT7_3"; break;
      case DC1394_VIDEO_MODE_FORMAT7_4: svalue="FORMAT7_4"; break;
      case DC1394_VIDEO_MODE_FORMAT7_5: svalue="FORMAT7_5"; break;
      case DC1394_VIDEO_MODE_FORMAT7_6: svalue="FORMAT7_6"; break;
      case DC1394_VIDEO_MODE_FORMAT7_7: svalue="FORMAT7_7"; break;
      default:continue;
      }
      props.set(key, svalue);
    } else if("operationmode" == key) {
      dc1394operation_mode_t mode;
      DC1394_TRYGET(operation_mode(m_dccamera, &mode));
      switch(mode) {
      case DC1394_OPERATION_MODE_LEGACY: svalue="1394a"; break;
      case DC1394_OPERATION_MODE_1394B : svalue="1394B"; break;
      default: continue;
      }
      props.set(key, svalue);
    } else if("speed" == key) {
      dc1394speed_t speed;
      DC1394_TRYGET(iso_speed(m_dccamera, &speed));
      value=100*(1<<speed);
      props.set(key, value);
    } else if("channel" == key) {
      uint32_t channel;
      DC1394_TRYGET(iso_channel(m_dccamera, &channel));
      value=channel;
      props.set(key, value);
    } else if("transmission" == key) {
      dc1394switch_t is_on;
      DC1394_TRYGET(transmission(m_dccamera, &is_on));
      value=((DC1394_ON==is_on)?1:0);
      props.set(key, value);
    } else if("oneshot" == key) {
      dc1394bool_t is_on;
      DC1394_TRYGET(one_shot(m_dccamera, &is_on));
      value=((DC1394_TRUE==is_on)?1:0);
      props.set(key, value);
    } else if("multishot" == key) {
      dc1394bool_t is_on;
      uint32_t   numFrames;
      DC1394_TRYGET(multi_shot(m_dccamera, &is_on, &numFrames));
      value=((DC1394_TRUE==is_on)?numFrames:-1);
      props.set(key, value);
    } else 
    {
		dc1394featureset_t feature_set;
		dc1394error_t err;
		err = dc1394_feature_get_all(m_dccamera, &feature_set);
		dc1394bool_t is_readable;
		// if ( err ) return false;
		
		uint32_t dc1394_value;
		for ( int i = 0 ; i < DC1394_FEATURE_NUM ; i++ ) {

			if (feature_set.feature[i].available && key == dc1394_feature_get_string(feature_set.feature[i].id)) {
				dc1394_feature_get_value(m_dccamera, feature_set.feature[i].id, &dc1394_value);
				value = dc1394_value;
				props.set(key, value);
			}
		}    
	}
  }
}
void videoDC1394::setProperties(gem::Properties&props) {
  if(!m_dccamera)return;

  std::vector<std::string>keys=props.keys();
  double value;
  std::string svalue;

  int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    dc1394error_t err=DC1394_SUCCESS;

    if("framerate" == key) {
      if(props.get(key, value)) {
	double d0=value/1.875;
	unsigned short base=d0;
	int r=DC1394_FRAMERATE_MIN;
	while(base>>=1)
	  r=r+1;
	if(r>DC1394_FRAMERATE_MAX)r=DC1394_FRAMERATE_MAX;
	dc1394framerate_t rate=(dc1394framerate_t)r;
	err=dc1394_video_set_framerate(m_dccamera, rate);
      }

    } else if("mode" == key) {
      dc1394video_mode_t mode;
      if(props.get(key, svalue)) {
	if("160x120_YUV444"==svalue) { mode=DC1394_VIDEO_MODE_160x120_YUV444;
	} else if("320x240_YUV422"==svalue) { mode=DC1394_VIDEO_MODE_320x240_YUV422;
	} else if("640x480_YUV411"==svalue) { mode=DC1394_VIDEO_MODE_640x480_YUV411;
	} else if("640x480_YUV422"==svalue) { mode=DC1394_VIDEO_MODE_640x480_YUV422;
	} else if("640x480_RGB8"==svalue) { mode=DC1394_VIDEO_MODE_640x480_RGB8;
	} else if("640x480_MONO8"==svalue) { mode=DC1394_VIDEO_MODE_640x480_MONO8;
	} else if("640x480_MONO16"==svalue) { mode=DC1394_VIDEO_MODE_640x480_MONO16;
	} else if("800x600_YUV422"==svalue) { mode=DC1394_VIDEO_MODE_800x600_YUV422;
	} else if("800x600_RGB8"==svalue) { mode=DC1394_VIDEO_MODE_800x600_RGB8;
	} else if("800x600_MONO8"==svalue) { mode=DC1394_VIDEO_MODE_800x600_MONO8;
	} else if("1024x768_YUV422"==svalue) { mode=DC1394_VIDEO_MODE_1024x768_YUV422;
	} else if("1024x768_RGB8"==svalue) { mode=DC1394_VIDEO_MODE_1024x768_RGB8;
	} else if("1024x768_MONO8"==svalue) { mode=DC1394_VIDEO_MODE_1024x768_MONO8;
	} else if("800x600_MONO16"==svalue) { mode=DC1394_VIDEO_MODE_800x600_MONO16;
	} else if("1024x768_MONO16"==svalue) { mode=DC1394_VIDEO_MODE_1024x768_MONO16;
	} else if("1280x960_YUV422"==svalue) { mode=DC1394_VIDEO_MODE_1280x960_YUV422;
	} else if("1280x960_RGB8"==svalue) { mode=DC1394_VIDEO_MODE_1280x960_RGB8;
	} else if("1280x960_MONO8"==svalue) { mode=DC1394_VIDEO_MODE_1280x960_MONO8;
	} else if("1600x1200_YUV422"==svalue) { mode=DC1394_VIDEO_MODE_1600x1200_YUV422;
	} else if("1600x1200_RGB8"==svalue) { mode=DC1394_VIDEO_MODE_1600x1200_RGB8;
	} else if("1600x1200_MONO8"==svalue) { mode=DC1394_VIDEO_MODE_1600x1200_MONO8;
	} else if("1280x960_MONO16"==svalue) { mode=DC1394_VIDEO_MODE_1280x960_MONO16;
	} else if("1600x1200_MONO16"==svalue) { mode=DC1394_VIDEO_MODE_1600x1200_MONO16;
	} else if("EXIF"==svalue) { mode=DC1394_VIDEO_MODE_EXIF;
	} else if("FORMAT7_0"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_0;
	} else if("FORMAT7_1"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_1;
	} else if("FORMAT7_2"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_2;
	} else if("FORMAT7_3"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_3;
	} else if("FORMAT7_4"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_4;
	} else if("FORMAT7_5"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_5;
	} else if("FORMAT7_6"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_6;
	} else if("FORMAT7_7"==svalue) { mode=DC1394_VIDEO_MODE_FORMAT7_7;
	} else continue;
      } else if (props.get(key, value)) {
	int m=value;
	if(m<DC1394_VIDEO_MODE_MIN || m>DC1394_VIDEO_MODE_MAX) continue;
	mode=(dc1394video_mode_t)m;
      } else continue;

      err=dc1394_video_set_mode(m_dccamera, mode);

    } else if("operationmode" == key) {
      dc1394operation_mode_t mode;
      if(props.get(key, svalue)) {
	if("1394a"==svalue || "1394A"==svalue || "legacy"==svalue)
	  mode=DC1394_OPERATION_MODE_LEGACY;
	else if("1394b"==svalue || "1394B"==svalue)
	  mode=DC1394_OPERATION_MODE_1394B;
	else continue;
      } else if (props.get(key, value)) {
	if(value<=480)
	  mode=DC1394_OPERATION_MODE_LEGACY;
	else
	  mode=DC1394_OPERATION_MODE_1394B;
      } else continue;

      err=dc1394_video_set_operation_mode(m_dccamera, mode);

    } else if("speed" == key) {
      dc1394speed_t speed=DC1394_ISO_SPEED_MIN;

      if (props.get(key, value) && value>0) {
	int s;
	for(s=DC1394_ISO_SPEED_MIN; s<DC1394_ISO_SPEED_MAX; s++) {
	  if(value>=(100*(1<<s)))
	    speed=(dc1394speed_t)s;
	}
	err=dc1394_video_set_iso_speed(m_dccamera, speed);
      }

    } else if("channel" == key) {
      if (props.get(key, value) && value>=0) {
	uint32_t channel=value;
	err=dc1394_video_set_iso_channel(m_dccamera, channel);
      }

    } else if("transmission" == key) {
      if (props.get(key, value)) {
	bool b=value;
	dc1394switch_t pwr=(b?DC1394_ON:DC1394_OFF);
	err=dc1394_video_set_transmission(m_dccamera, pwr);
      }

    } else if("oneshot" == key) {
      if (props.get(key, value)) {
	bool b=value;
	dc1394switch_t pwr=(b?DC1394_ON:DC1394_OFF);
	err=dc1394_video_set_one_shot(m_dccamera, pwr);
      }
    } else if("multishot" == key) {
      if (props.get(key, value)) {
	dc1394switch_t pwr=DC1394_OFF;
	uint32_t   numFrames=0;
	if(value>0) {
	  pwr=DC1394_ON;
	  numFrames=value;
	}
	err=dc1394_video_set_multi_shot(m_dccamera, numFrames, pwr);
      }
    }
    else {
		dc1394featureset_t feature_set;
		dc1394error_t err;
		err = dc1394_feature_get_all(m_dccamera, &feature_set);
		dc1394bool_t is_readable;
		// if ( err ) return false;
		
		uint32_t dc1394_value;
		for ( int i = 0 ; i < DC1394_FEATURE_NUM ; i++ ) {

			if (feature_set.feature[i].available && key == dc1394_feature_get_string(feature_set.feature[i].id)) {
				props.get(key, value);
				dc1394_value = value;
				dc1394_feature_set_value(m_dccamera, feature_set.feature[i].id, dc1394_value);
				// TODO : limit to min / max value for each parametter
			}
		}    
	}
    if(DC1394_SUCCESS!=err) {
      error("videoDC1394: setting '%s' failed with '%s'",
	    key.c_str(),
	    dc1394_error_get_string(err));
    }
  }
}


#else
videoDC1394 :: videoDC1394() : videoBase("")
{}
videoDC1394 :: ~videoDC1394()
{}

#endif // HAVE_LIBDC1394
