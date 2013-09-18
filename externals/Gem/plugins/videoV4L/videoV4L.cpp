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
/*
  this is an attempt at a Linux version of pix_video by Miller Puckette.
  Anyone conversant in c++ will probably howl at this.  I'm uncertain of
  several things.
    
  First, the #includes I threw in pix_video.h may not all be necessary; I
  notice that far fewer are needed for the other OSes.
    
  Second, shouldn't the os-dependent state variables be "private"?  I
  followed the lead of the other os-dependent state variables.  Also,
  I think the indentation is goofy but perhaps there's some reason for it.

  Third, I probably shouldn't be using sprintf to generate filenames; I
  don't know the "modern" c++ way to do this.
    
  Fourth, I don't know why some state variables 
  show up as "arguments" in the pix_video :: pix_video().
     
  This code is written with the "bttv" device in mind, which memory mapes
  images up to 24 bits per pixel.  So we request the whole 24 and don't
  settle for anything of lower quality (nor do we offer anything of higher
  quality; it seems that Gem is limited to 32 bits per pixel including
  alpha.)  We take all video images to be opaque by setting the alpha
  channel to 255.

*/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined HAVE_LIBV4L1 || defined HAVE_LINUX_VIDEODEV_H
# define HAVE_VIDEO4LINUX
#endif

#ifdef HAVE_VIDEO4LINUX

#include "videoV4L.h"
#include "plugins/PluginFactory.h"
using namespace gem::plugins;

#include "Gem/RTE.h"
#include "Gem/Files.h"

#ifndef HAVE_LIBV4L1
# define v4l1_open ::open
# define v4l1_close ::close
# define v4l1_dup ::dup
# define v4l1_ioctl ::ioctl
# define v4l1_read ::read
# define v4l1_mmap ::mmap
# define v4l1_munmap ::munmap
#endif /* libv4l-1 */

#if 0
# define debug ::post
#else
# define debug
#endif

/////////////////////////////////////////////////////////
//
// videoV4L
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
REGISTER_VIDEOFACTORY("v4l", videoV4L);


videoV4L :: videoV4L() : videoBase("v4l")
                       ,
                         tvfd(0),
                         frame(0),
                         videobuf(NULL), 
                         mytopmargin(0), mybottommargin(0), 
                         myleftmargin(0), myrightmargin(0),
                         m_gotFormat(0),m_colorConvert(false),
                         m_norm(VIDEO_MODE_AUTO),
                         m_channel(V4L_COMPOSITEIN),
                         errorcount(0)
			 
{
  if (!m_width)m_width=64;
  if (!m_height)m_height=64;

  m_capturing=false;

  m_devicenum=V4L_DEVICENO;

  provide("analog");
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoV4L :: ~videoV4L()
{
  close();
}


////////////////////////////////////////////////////////
// frame grabber
//
/////////////////////////////////////////////////////////
bool videoV4L :: grabFrame() {
  bool noerror=true;

  frame++;
  frame%=V4L_NBUF;

  vmmap[frame].width = m_image.image.xsize + myleftmargin + myrightmargin;
  vmmap[frame].height = m_image.image.ysize + mytopmargin + mybottommargin;
  
  /* syncing */
  if (v4l1_ioctl(tvfd, VIDIOCSYNC, &vmmap[frame].frame) < 0)
    {
      perror("v4l: VIDIOCSYNC");
      noerror=false;
    }

  /* capturing */
  if (v4l1_ioctl(tvfd, VIDIOCMCAPTURE, &vmmap[frame]) < 0)
    {
      if (errno == EAGAIN)
	error("v4l: can't sync (no v4l source?)");
      else 
	perror("v4l: VIDIOCMCAPTURE1");

      /* let's try again... */
      if (v4l1_ioctl(tvfd, VIDIOCMCAPTURE, &vmmap[frame]) < 0) {
	perror("v4l: VIDIOCMCAPTURE2");
	noerror=false;
      }
    }
  if(noerror){
    errorcount=0;
  } else {
    errorcount++;
    if(errorcount>1000) {
      error("v4L: %d capture errors in a row... I think I better stop now...", errorcount);
      return false;
    }
  }
  
  lock();
  if (m_colorConvert){
    m_image.image.notowned = false;
    switch(m_gotFormat){
    case VIDEO_PALETTE_YUV420P: m_image.image.fromYUV420P(videobuf + vmbuf.offsets[frame]); break;
    case VIDEO_PALETTE_RGB24:   m_image.image.fromBGR    (videobuf + vmbuf.offsets[frame]); break;
    case VIDEO_PALETTE_RGB32:   m_image.image.fromBGRA   (videobuf + vmbuf.offsets[frame]); break;
    case VIDEO_PALETTE_GREY:    m_image.image.fromGray   (videobuf + vmbuf.offsets[frame]); break;
    case VIDEO_PALETTE_YUV422:  m_image.image.fromYUV422 (videobuf + vmbuf.offsets[frame]); break;
      
    default: // ? what should we do ?
      m_image.image.data=videobuf + vmbuf.offsets[frame];
      m_image.image.notowned = true;
    }
  } else {
    m_image.image.data=videobuf + vmbuf.offsets[frame];
    m_image.image.notowned = true;
  }
  m_image.image.upsidedown=true;
  
  m_image.newimage = 1;
  unlock();
  return true;
}

/////////////////////////////////////////////////////////
// openDevice
//
/////////////////////////////////////////////////////////
bool videoV4L :: openDevice(gem::Properties&props)
{
  char buf[256];
  int i;

  if(!m_devicename.empty()){
    snprintf(buf,256,"%s", m_devicename.c_str());
    buf[255]=0;
  } else {
    if (m_devicenum<0){
      sprintf(buf, "/dev/video");
    } else {
      snprintf(buf, 256, "/dev/video%d", m_devicenum);
      buf[255]=0;
    }
  }
  
  if ((tvfd = v4l1_open(buf, O_RDWR)) < 0) {
    error("v4l: failed opening device: '%s'", buf);
    perror(buf);
    goto closit;
  }

  /* get picture information */
  if (v4l1_ioctl(tvfd, VIDIOCGPICT, &vpicture) < 0) {
    perror("v4l: VIDIOCGPICT");
    goto closit;
  }

  /* get capabilities */
  if (v4l1_ioctl(tvfd, VIDIOCGCAP, &vcap) < 0) {
    perror("v4l: VIDIOCGCAP");
    goto closit;
  }

  for (i = 0; i < vcap.channels; i++) {
    vchannel.channel = i;
    logpost(NULL, 6, "getting channel info for #%d", i);
    if (v4l1_ioctl(tvfd, VIDIOCGCHAN, &vchannel) < 0)  {
      perror("v4l: VIDIOCGCHAN");
      goto closit;
    }
  }

  setProperties(props);

  return true;

 closit:
  closeDevice();
  return false;
}
/////////////////////////////////////////////////////////
// closeDevice
//
/////////////////////////////////////////////////////////
void videoV4L :: closeDevice() {
  if (tvfd>=0) v4l1_close(tvfd);
  tvfd = -1;
}


/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoV4L :: startTransfer()
{
  if(tvfd<0)return false;
  int i;
  int width, height;


  errorcount=0;

  frame = 0;

#if 0
  /* get picture information */
  if (v4l1_ioctl(tvfd, VIDIOCGPICT, &vpicture) < 0) {
    perror("v4l: VIDIOCGPICT");
    return false;
  }

  /* hmm, what does this do? */
  for (i = 0; i < vcap.channels; i++) {
    vchannel.channel = i;
    if (v4l1_ioctl(tvfd, VIDIOCGCHAN, &vchannel) < 0) {
      perror("v4l: VDIOCGCHAN");
      return false;
    }
  }

  /* select a channel (takes effect in next VIDIOCSCHAN call */
  vchannel.channel = ((vcap.channels-1)<m_channel)?(vcap.channels-1):m_channel;

  /* hmm, what does this do? */
  if (v4l1_ioctl(tvfd, VIDIOCGCHAN, &vchannel) < 0) {
    perror("v4l: VDIOCGCHAN");
    return false;
  }

  /* select the video-norm */
  vchannel.norm = m_norm;

  /* apply video-channel and -norm */
  if (v4l1_ioctl(tvfd, VIDIOCSCHAN, &vchannel) < 0) {
    perror("v4l: VDIOCSCHAN");
    return false;
  }
#endif

  /* get mmap numbers */
  if (v4l1_ioctl(tvfd, VIDIOCGMBUF, &vmbuf) < 0)
    {
      perror("v4l: VIDIOCGMBUF");
      return false;
    }

  if (!(videobuf = (unsigned char *)
        v4l1_mmap(0, vmbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, tvfd, 0)))
    {
      perror("v4l: mmap");
      return false;
    }


  /* dimension settings
   *
   * what happened to the margins?
   */

  width = (m_width  > vcap.minwidth ) ? m_width        : vcap.minwidth;   
  width = (width    > vcap.maxwidth ) ? vcap.maxwidth  : width;
  height =(m_height > vcap.minheight) ? m_height       : vcap.minheight;
  height =(height   > vcap.maxheight) ? vcap.maxheight : height;

  for (i = 0; i < V4L_NBUF; i++)    {
    switch(m_reqFormat){
    case GL_LUMINANCE:
    	vmmap[i].format = VIDEO_PALETTE_GREY;
      break;
    case GL_RGBA:
    case GL_BGRA:
    	vmmap[i].format = VIDEO_PALETTE_RGB24;
      break;
    case GL_YCBCR_422_GEM:
        /* this is very unfortunate:
         * PALETTE_YUV422 obviously is something different than ours
         * although our yuv422 reads uyvy it is
         * not PALETTE_UYVY either !
         */
        vmmap[i].format = VIDEO_PALETTE_YUV420P;
      break;
    default:
    case GL_RGB:
    case GL_BGR:
    	vmmap[i].format = VIDEO_PALETTE_RGB24;
    }

    vmmap[i].width = width;
    vmmap[i].height = height;
    vmmap[i].frame  = i;
  }

  if (v4l1_ioctl(tvfd, VIDIOCMCAPTURE, &vmmap[frame]) < 0)    {
    for (i = 0; i < V4L_NBUF; i++)vmmap[i].format = vpicture.palette;
    if (v4l1_ioctl(tvfd, VIDIOCMCAPTURE, &vmmap[frame]) < 0)    {
      if (errno == EAGAIN)
        error("v4l: can't sync (no video source?)");
      else 
        perror("v4l: VIDIOCMCAPTURE");
    }
  }
  /* fill in image specifics for Gem pixel object.  Could we have
     just used RGB, I wonder? */
  m_image.image.xsize = vmmap[frame].width;
  m_image.image.ysize = vmmap[frame].height;
  m_image.image.setCsizeByFormat(m_reqFormat);
  m_image.image.reallocate();

  switch((m_gotFormat=vmmap[frame].format)){
  case VIDEO_PALETTE_GREY  : m_colorConvert=(m_reqFormat!=GL_LUMINANCE); break;
  case VIDEO_PALETTE_RGB24 : m_colorConvert=(m_reqFormat!=GL_BGR); break;
  case VIDEO_PALETTE_RGB32 : m_colorConvert=(m_reqFormat!=GL_BGRA); break;
  case VIDEO_PALETTE_YUV422: m_colorConvert=(m_reqFormat!=GL_YCBCR_422_GEM); break;
  default: m_colorConvert=true;
  }
  
#if 0
  myleftmargin = 0;
  myrightmargin = 0;
  mytopmargin = 0;
  mybottommargin = 0;
#endif

  m_haveVideo = 1;

  logpost(NULL, 5, "v4l::startTransfer opened video connection %X", tvfd);
  return true;
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoV4L :: stopTransfer()
{
  if(!m_capturing)return false;
  v4l1_munmap(videobuf, vmbuf.size);
  m_capturing=false;
  return true;
}

bool videoV4L :: setColor(int format)
{
  if (format<=0 || format==m_reqFormat)return -1;
  m_reqFormat=format;
  restartTransfer();
  return 0;
}

std::vector<std::string> videoV4L::enumerate() {
  std::vector<std::string> result;
  std::vector<std::string> glob, allglob;
  int i=0;
  glob=gem::files::getFilenameListing("/dev/video*");
  for(i=0; i<glob.size(); i++)
    allglob.push_back(glob[i]);

  glob=gem::files::getFilenameListing("/dev/v4l/video*");
  for(i=0; i<glob.size(); i++)
    allglob.push_back(glob[i]);

  for(i=0; i<allglob.size(); i++) {
    std::string dev=allglob[i];
    logpost(NULL, 6, "V4L: found possible device %s", dev.c_str());
    int fd=v4l1_open(dev.c_str(), O_RDONLY | O_NONBLOCK);
    logpost(NULL, 6, "V4L: v4l1_open returned %d", fd);
    if(fd<0)continue;
    if (ioctl(fd, VIDIOCGCAP, &vcap) >= 0)
    {
      if (vcap.type & VID_TYPE_CAPTURE) {
        result.push_back(dev);  
      } else {
        logpost(NULL, 5, "%s is v4l1 but cannot capture", dev.c_str());
      }
    } else {
      logpost(NULL, 5, "%s is no v4l1 device", dev.c_str());
    }

    v4l1_close(fd);
  }
  
  return result;
}

bool videoV4L::enumProperties(gem::Properties&readable,
			      gem::Properties&writeable) {
  int i=0;
  std::vector<std::string>keys;
  gem::any type;


  readable.clear();
  writeable.clear();

  keys.clear();
  keys.push_back("width");
  keys.push_back("height");

  keys.push_back("leftmargin");
  keys.push_back("rightmargin");
  keys.push_back("topmargin");
  keys.push_back("bottommargin");

  keys.push_back("channel");
  keys.push_back("frequency");

  type=0;
  for(i=0; i<keys.size(); i++) {
    readable .set(keys[i], type);
    writeable.set(keys[i], type);
  }
  keys.clear();

  keys.push_back("norm");

  type=std::string("");
  for(i=0; i<keys.size(); i++) {
    readable .set(keys[i], type);
    writeable.set(keys[i], type);
  }
  keys.clear();

}
void videoV4L::setProperties(gem::Properties&props) {
  std::vector<std::string>keys=props.keys();
  bool restart=false;
  bool do_s_chan=false, do_s_pict=false;
  int i=0;
  double d;
  std::string s;

  if(tvfd<0)return;

  if (v4l1_ioctl(tvfd, VIDIOCGCHAN, &vchannel) < 0) {
    perror("v4l: VDIOCGCHAN");
  }

  for(i=0; i<keys.size(); i++) {
    const std::string key=keys[i];
    if(0){;
#define RESTART_WITH_CHANGED_UINT(x) do { unsigned int ui=d; restart=(x!=ui); x=ui; } while(0)
    } else if (key=="width") {
      if(props.get(key, d)) {
	RESTART_WITH_CHANGED_UINT(m_width);
      }
    } else if (key=="height") {
      if(props.get(key, d)) {
	RESTART_WITH_CHANGED_UINT(m_height);
      }
    } else if (key=="leftmargin") {
      if(props.get(key, d)) {
	RESTART_WITH_CHANGED_UINT(myleftmargin);
      }
    } else if (key=="rightmargin") {
      if(props.get(key, d)) {
	RESTART_WITH_CHANGED_UINT(myrightmargin);
      }
    } else if (key=="topmargin") {
      if(props.get(key, d)) {
	RESTART_WITH_CHANGED_UINT(mytopmargin);
      }
    } else if (key=="bottommargin") {
      if(props.get(key, d)) {
	RESTART_WITH_CHANGED_UINT(mybottommargin);
      }
    } else if (key=="channel") {
      if(props.get(key, d)) {
	int channel=d;
	if(channel<0 || channel>(vcap.channels-1)) {
	  error("channel %d out of range [0..%d]", channel, vcap.channels-1);
	  continue;
	}
  m_channel=channel;
	vchannel.channel=channel;

	do_s_chan=true;
      }
    } else if (key=="frequency") {
      if(props.get(key, d)) {
	if (v4l1_ioctl(tvfd,VIDIOCGTUNER,&vtuner) < 0) {
	  error("pix_video[v4l]: error setting frequency -- no tuner");
	  continue;
	}
	unsigned long freq=d;
	if (v4l1_ioctl(tvfd,VIDIOCSFREQ,&freq) < 0) {
	  error("pix_video[v4l]: error setting frequency");
	}
      }
    } else if (key=="norm") {
      int i_norm=-1;
      if(props.get(key, s)) {
	if("PAL"==s || "pal"==s)
	  i_norm=VIDEO_MODE_PAL;
	else if("NTSC"==s || "ntsc"==s)
	  i_norm=VIDEO_MODE_NTSC;
	else if("SECAM"==s || "secam"==s)
	  i_norm=VIDEO_MODE_SECAM;
	else if("AUTO"==s || "auto"==s)
	  i_norm=VIDEO_MODE_AUTO;

	if(i_norm<0) {
	  error("unknown norm '%s'", s.c_str());
	} else {
    m_norm=i_norm;
	  vchannel.norm=i_norm;
	  do_s_chan=true;
	}
      } else if(props.get(key, d)) {
	i_norm=d;
	if(i_norm<0 || i_norm>VIDEO_MODE_AUTO) {
	  error("unknown norm %d", i_norm);
	} else {
	  vchannel.norm=i_norm;
	  do_s_chan=true;
	}
      }
    } else if (key=="Brightness") {
      if(props.get(key, d)) {
	vpicture.brightness=d;
	do_s_pict=true;
      }
    } else if (key=="Hue") {
      if(props.get(key, d)) {
	vpicture.hue=d;
	do_s_pict=true;
      }
    } else if (key=="Colour" || key=="Color") {
      if(props.get(key, d)) {
	vpicture.colour=d;
	do_s_pict=true;
      }
    } else if (key=="Contrast") {
      if(props.get(key, d)) {
	vpicture.contrast=d;
	do_s_pict=true;
      }
    } else if (key=="Whiteness") {
      if(props.get(key, d)) {
	vpicture.whiteness=d;
	do_s_pict=true;
      }


    } /* ifelse key */
  } // loop

  /* do compound settings */
  if(do_s_chan) {
    logpost(NULL, 6, "calling VIDIOCSCHAN");
    if (v4l1_ioctl(tvfd, VIDIOCSCHAN, &vchannel) < 0) {
      perror("v4l: VDIOCSCHAN");
    }
  }
  if(do_s_pict) {
    logpost(NULL, 6, "calling VIDIOCSPICT");
    if (v4l1_ioctl(tvfd, VIDIOCSPICT, &vpicture) < 0) {
      perror("v4l: VIDIOCSPICT");
    }
  }

  if(restart)
    restartTransfer();
}
void videoV4L::getProperties(gem::Properties&props) {
#define IOCTL_ONCE(x, y) if(!y##_done)if(v4l1_ioctl(tvfd, x, &y) < 0) {perror("v4l"#x"");} else y##_done=true

  bool vpicture_done=false, vchannel_done, vcap_done=false;



  std::vector<std::string>keys=props.keys();
  if(tvfd<0){
    props.clear();
    return;
  }
  int i;

  for(i=0; i<keys.size(); i++) {
    const std::string key=keys[i];

    if(key=="width") {

    } else if(key=="height") {

      /* picture data */
    } else if(key=="Brightness") {
      IOCTL_ONCE(VIDIOCGPICT, vpicture);
      props.set(key, vpicture.brightness);
    } else if(key=="Hue") {
      IOCTL_ONCE(VIDIOCGPICT, vpicture);
      props.set(key, vpicture.hue);
    } else if(key=="Colour" || key=="Color") {
      IOCTL_ONCE(VIDIOCGPICT, vpicture);
      props.set(key, vpicture.colour);
    } else if(key=="Contrast") {
      IOCTL_ONCE(VIDIOCGPICT, vpicture);
      props.set(key, vpicture.contrast);
    } else if(key=="Whiteness") {
      IOCTL_ONCE(VIDIOCGPICT, vpicture);
      props.set(key, vpicture.whiteness);

    } else if(key=="channels") {
      IOCTL_ONCE(VIDIOCGCAP, vcap);
      props.set(key, vcap.channels);


    } else if(key=="frequency") {
      unsigned long freq=0;
      if (v4l1_ioctl(tvfd,VIDIOCGFREQ,&freq) >= 0) {
	double d=freq;
	props.set(key, d);
      }

      /* channel data */
    } else if(key=="channel") {
      //      IOCTL_ONCE(VIDIOCGCHAN, vchannel);
      props.set(key, vchannel.channel);

    } else if(key=="norm") {
      IOCTL_ONCE(VIDIOCGCHAN, vchannel);
      switch(vchannel.norm) {
      case VIDEO_MODE_PAL:  
	props.set(key, std::string("PAL")); 
	break;
      case VIDEO_MODE_NTSC: 
	props.set(key, std::string("NTSC")); 
	break;
      case VIDEO_MODE_SECAM:
	props.set(key, std::string("SECAM")); 
	break;
      case VIDEO_MODE_AUTO: 
	props.set(key, std::string("AUTO")); 
	break;
      default:
	props.set(key, vchannel.norm);
      }
    } /* else if key */
  } /* key loop */
}
#endif /* HAVE_VIDEO4LINUX */
