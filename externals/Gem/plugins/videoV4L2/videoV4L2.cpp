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

#include "videoV4L2.h"
#include "plugins/PluginFactory.h"

using namespace gem::plugins;

#include "Gem/RTE.h"
#include "Gem/Files.h"

#ifndef HAVE_LIBV4L2
# define v4l2_open ::open
# define v4l2_close ::close
# define v4l2_dup ::dup
# define v4l2_ioctl ::ioctl
# define v4l2_read ::read
# define v4l2_mmap ::mmap
# define v4l2_munmap ::munmap
#endif /* libv4l-2 */


/* debugging helpers  */
#define debugPost
#define debugThread
#define debugIOCTL 

#if 0
# undef debugPost
# define debugPost ::startpost("%s:%s[%d]", __FILE__, __FUNCTION__, __LINE__); ::post
#endif

#if 0
# undef debugThread
# define debugThread ::startpost("%s:%s[%d]", __FILE__, __FUNCTION__, __LINE__); ::post
#endif

#if 0
# undef debugIOCTL 
# define debugIOCTL ::post
#endif


#define WIDTH_FLAG 1
#define HEIGHT_FLAG 2

/////////////////////////////////////////////////////////
//
// videoV4L2
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
#ifdef HAVE_VIDEO4LINUX2

#include <sys/stat.h>

REGISTER_VIDEOFACTORY("v4l2", videoV4L2);

videoV4L2 :: videoV4L2() : videoBase("v4l2", 0)
                                   , m_gotFormat(0), m_colorConvert(0),
                                     m_tvfd(0),
                                     m_buffers(NULL), m_nbuffers(0), 
                                     m_currentBuffer(NULL),
                                     m_frame(0), m_last_frame(0),
                                     m_maxwidth(844), m_minwidth(32),
                                     m_maxheight(650), m_minheight(32),
                                     m_thread_id(0), m_continue_thread(false), m_frame_ready(false),
                                     m_rendering(false),
                                     m_stopTransfer(false),
                                     m_frameSize(0)
{
  if (!m_width)m_width=320;
  if (!m_height)m_height=240;
  m_capturing=false;
  m_devicenum=V4L2_DEVICENO;

  provide("analog");
}
  
////////////////////////////////////////////////////////
// Destructor
//
////////////////////////////////////////////////////////
videoV4L2 :: ~videoV4L2()
{
  close();
}

static int xioctl(int                    fd,
                  int                    request,
                  void *                 arg)
{
  int r;
  const unsigned char req=(request&0xFF);

  debugIOCTL("V4L2: xioctl %d\n", req);
  do {
    r = v4l2_ioctl (fd, request, arg);
    debugIOCTL("V4L2: xioctl %d->%d\n", r, errno);
  }
  while (-1 == r && EINTR == errno);

  debugIOCTL("V4L2: xioctl done %d\n", r);
#if 0
  if(r!=0) {
    char buf[13];
    snprintf(buf, 13, "xioctl[%03d]:", req);
    buf[12]=0;
    perror(buf);
  }
#endif
  return r;
}

static int reqbufs(int fd, unsigned int numbufs) {
  struct v4l2_requestbuffers req;
  memset (&(req), 0, sizeof (req));

  req.count               = numbufs;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_MMAP;

  if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
    return -1;
  }

  return req.count;
}

int videoV4L2::init_mmap (void)
{
  const char *devname=(m_devicename.empty())?"device":m_devicename.c_str();
  int count = reqbufs(m_tvfd, V4L2_NBUF);

  if(count<0) {
    if (EINVAL == errno) {
      error("%s does not support memory mapping", devname);
      return 0;
    } else {
      perror("v4l2: VIDIOC_REQBUFS");
      return 0;
    }
  }

  if (count < V4L2_NBUF) {
    //error("Insufficient buffer memory on %s: %d", devname, count);
    //return(0);
  }

  m_buffers = (t_v4l2_buffer*)calloc (count, sizeof (*m_buffers));

  if (!m_buffers) {
    perror("v4l2: out of memory");
    return(0);
  }

  for (m_nbuffers = 0; m_nbuffers < count; ++m_nbuffers) {
    struct v4l2_buffer buf;

    memset (&(buf), 0, sizeof (buf));

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = m_nbuffers;
    debugPost("v4l2: buf.index==%d", buf.index);

    if (-1 == xioctl (m_tvfd, VIDIOC_QUERYBUF, &buf)){
      perror("v4l2: VIDIOC_QUERYBUF");
      return(0);
    }

    m_buffers[m_nbuffers].length = buf.length;
    m_buffers[m_nbuffers].start =
      v4l2_mmap (NULL /* start anywhere */,
                 buf.length,
                 PROT_READ | PROT_WRITE /* required */,
                 MAP_SHARED /* recommended */,
                 m_tvfd, buf.m.offset);

    if (MAP_FAILED == m_buffers[m_nbuffers].start){
      perror("v4l2: mmap");
      return 0;
    }
  }
  return 1;
}

/////////////////////////////////////////////////////////
// this is the work-horse
// a thread that does the capturing
//
/////////////////////////////////////////////////////////
void *videoV4L2::capturing_(void*you)
{
  videoV4L2 *me=(videoV4L2 *)you;
  return me->capturing();
}
void *videoV4L2 :: capturing(void)
{
  int errorcount=0;

  t_v4l2_buffer*buffers=m_buffers;
  void *currentBuffer=NULL;

  const __u32 expectedSize=m_frameSize;
  __u32 gotSize=0;


  struct v4l2_buffer buf;
  int nbuf=m_nbuffers;
  
  fd_set fds;
  struct timeval tv;
  int r;

  unsigned int i;


  m_capturing=true;

  debugThread("V4L2: memset");
  memset(&(buf), 0, sizeof (buf));
  
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  while(m_continue_thread){
    bool captureerror=false;
    FD_ZERO (&fds);
    FD_SET (m_tvfd, &fds);

    debugThread("V4L2: grab");

    m_frame++;
    m_frame%=nbuf;

    
    /* Timeout. */
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    r = select(0,0,0,0,&tv);
    debugThread("V4L2: waited...");


    if (-1 == r) {
      if (EINTR == errno)
        continue;
      perror("v4l2: select");//exit
    }

    memset(&(buf), 0, sizeof (buf));
    debugThread("V4L2: memset...");
  
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl (m_tvfd, VIDIOC_DQBUF, &buf)) {
      switch (errno) {
      case EAGAIN:
        perror("v4l2: VIDIOC_DQBUF: stopping capture thread!");
        m_stopTransfer=true;
        m_continue_thread=false;
      case EIO:
        /* Could ignore EIO, see spec. */
        /* fall through */
      default:
        captureerror=true;
        perror("v4l2: VIDIOC_DQBUF");
      }
    }

    debugThread("V4L2: grabbed %d", buf.index);

    gotSize=buf.bytesused;
    currentBuffer=buffers[buf.index].start;
    //process_image (m_buffers[buf.index].start);

    if (-1 == xioctl (m_tvfd, VIDIOC_QBUF, &buf)){
      perror("v4l2: VIDIOC_QBUF");
      captureerror=true;
    }

    debugThread("V4L2: dequeueued");
    
    if(expectedSize==gotSize) {
      m_frame_ready = 1;
      m_last_frame=m_frame;
      m_currentBuffer=currentBuffer;
    } else {
      post("oops, skipping incomplete capture %d of %d bytes", gotSize, expectedSize);
    }

    if(captureerror) {
      errorcount++;
      if(errorcount>1000) {
        error("v4L2: %d capture errors in a row... I think I better stop now...", errorcount);
        m_continue_thread=false;
        m_stopTransfer=true;
      }
    } else {
      errorcount=0;
    }
  }

  // stop capturing
  m_capturing=false;
  debugThread("V4L2: thread finished");
  return NULL;
}

//////////////////
// this reads the data that was captured by capturing() and returns it within a pixBlock
pixBlock *videoV4L2 :: getFrame(){
  if(!(m_haveVideo && m_capturing))return NULL;
  if(m_stopTransfer) {
    bool rendering=m_rendering;
    stopTransfer();
    m_rendering=rendering;
    return NULL;
  }
  //debugPost("v4l2: getting frame %d", m_frame_ready);
  m_image.newfilm=0;
  if (!m_frame_ready) m_image.newimage = 0;
  else {
    unsigned char*data=(unsigned char*)m_currentBuffer;
    if (m_colorConvert){
      m_image.image.notowned = false;
      switch(m_gotFormat){
#if 1
# define CONVERT(type) m_image.image.from##type (data)
#else
# define CONVERT(type) post("from " #type "!");m_image.image.from##type (data)
#endif
      case V4L2_PIX_FMT_RGB24: CONVERT(RGB   ); break;
      case V4L2_PIX_FMT_BGR32: CONVERT(BGRA  ); break;
      case V4L2_PIX_FMT_RGB32: CONVERT(ARGB  ); break;
      case V4L2_PIX_FMT_GREY : CONVERT(Gray  ); break;
      case V4L2_PIX_FMT_UYVY : CONVERT(YUV422); break;
      case V4L2_PIX_FMT_YUYV : CONVERT(YUY2  ); break;
      case V4L2_PIX_FMT_YUV420:CONVERT(YU12  ); break;


      default: // ? what should we do ?
        m_image.image.data=data;
        m_image.image.notowned = true;
      }
    } else {
      m_image.image.data=data;
      m_image.image.notowned = true;
    }
    m_image.image.upsidedown=true;
    
    m_image.newimage = 1;
    m_frame_ready = false;
  }
  return &m_image;
}

bool videoV4L2 :: openDevice(gem::Properties&props) {
  close();

  /* check the device */
  // if we don't have a devicename, create one
  std::string devname = m_devicename;


  if(devname.empty()) {
    devname="/dev/video";
    if(m_devicenum>=0) {
      char buf[255];
      snprintf(buf, 255, "%d", m_devicenum);
      buf[255]=0;
      devname+=buf;
    }
  }
  const char*dev_name=devname.c_str();

  // try to open the device
  debugPost("v4l2: device: %s", dev_name);
  
  m_tvfd = v4l2_open (dev_name, O_RDWR /* required */, 0);

  if (-1 == m_tvfd) {
    error("Cannot open '%s': %d, %s", dev_name, errno, strerror (errno));
    closeDevice(); return false;
  }

  struct stat st; 
  if (-1 == fstat (m_tvfd, &st)) {
    error("Cannot identify '%s': %d, %s", dev_name, errno, strerror (errno));
    closeDevice(); return false;
  }

  if (!S_ISCHR (st.st_mode)) {
    error("%s is no device", dev_name);
    closeDevice(); return false;
  }

  // by now, we have an open file-descriptor
  // check whether this is really a v4l2-device
  struct v4l2_capability cap;
  if (-1 == xioctl (m_tvfd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      error("%s is no V4L2 device",  dev_name);
      closeDevice(); return false;
    } else {
      perror("v4l2: VIDIOC_QUERYCAP");//exit
      closeDevice(); return false;
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    error("%s is no video capture device", dev_name);
    closeDevice(); return false;
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    error("%s does not support streaming i/o", dev_name);
    closeDevice(); return false;
  }

  logpost(NULL, 5, "v4l2: successfully opened %s", dev_name);

  setProperties(props);

  return true;
}
void videoV4L2 :: closeDevice() {
  logpost(NULL, 5, "v4l: closing device %d", m_tvfd);
  if (m_tvfd>=0) v4l2_close(m_tvfd);
  m_tvfd=-1;
}


/////////////////////////////////////////////////////////
// restartTransfer
//
/////////////////////////////////////////////////////////
bool videoV4L2 :: restartTransfer()
{
  bool rendering=m_rendering;
  debugPost("v4l2: restart transfer");
  if(m_capturing)stopTransfer();
  debugPost("v4l2: restart stopped");
  if (rendering)startTransfer();
  debugPost("v4l2: restart started");

  return true;
}

/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoV4L2 :: startTransfer()
{
  if(m_tvfd<0)return false;
  debugPost("v4l2: startTransfer: %d", m_capturing);
  if(m_capturing)stopTransfer(); // just in case we are already running!
  debugPost("v4l2: start transfer");
  m_stopTransfer=false;
  m_rendering=true;
  //  logpost(NULL, 5, "starting transfer");
  int i;

  __u32 pixelformat=0;
  struct v4l2_format fmt;

  enum v4l2_buf_type type;

  m_frame = 0;
  m_last_frame = 0;

  /* Select video format */
  memset (&(fmt), 0, sizeof (fmt));
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  /* initialize the format struct to what we currently have */
  if (-1 == xioctl (m_tvfd, VIDIOC_G_FMT, &fmt)){
    perror("v4l2: VIDIOC_G_FMT");//exit
  }

  switch(m_reqFormat){
  case GL_YCBCR_422_GEM: 
    pixelformat = V4L2_PIX_FMT_UYVY; 
    break;
  case GL_LUMINANCE: 
    pixelformat = V4L2_PIX_FMT_GREY; 
    break;
  case GL_RGB: 
    pixelformat = V4L2_PIX_FMT_RGB24; 
    break;
  default: 
    pixelformat = V4L2_PIX_FMT_RGB32; 
    m_reqFormat=GL_RGBA;
    break;
  }

  if(fmt.fmt.pix.pixelformat != pixelformat) {
    fmt.fmt.pix.pixelformat = pixelformat;
  
    logpost(NULL, 5, "v4l2: want 0x%X == '%c%c%c%c' ", m_reqFormat, 
            (char)(fmt.fmt.pix.pixelformat),
            (char)(fmt.fmt.pix.pixelformat>>8),
            (char)(fmt.fmt.pix.pixelformat>>16),
            (char)(fmt.fmt.pix.pixelformat>>24));

    if (-1 == xioctl (m_tvfd, VIDIOC_S_FMT, &fmt)){
      perror("v4l2: VIDIOC_S_FMT(fmt)");//exit
    }
  
    // query back what we have set
    /* in theory this should not be needed, 
     * as S_FMT is supposed to return the actual data
     * however, some buggy drivers seem to not do that, 
     * so we have to make sure...
     */
    if (-1 == xioctl (m_tvfd, VIDIOC_G_FMT, &fmt)){
      perror("v4l2: VIDIOC_G_FMT");//exit
    }
  }

  m_gotFormat=fmt.fmt.pix.pixelformat;
  switch(m_gotFormat){
  case V4L2_PIX_FMT_RGB32: debugPost("v4l2: ARGB");break;
  case V4L2_PIX_FMT_RGB24: debugPost("v4l2: RGB");break;
  case V4L2_PIX_FMT_UYVY:  debugPost("v4l2: YUV ");break;
  case V4L2_PIX_FMT_GREY:  debugPost("v4l2: gray");break;
  case V4L2_PIX_FMT_YUV420:debugPost("v4l2: YUV 4:2:0");break;
  default: 
    /* hmm, we don't know how to handle this 
     * let's try formats that should be always supported by libv4l2
     */
    switch(m_reqFormat){
    case GL_YCBCR_422_GEM: 
    case GL_LUMINANCE: 
      pixelformat = V4L2_PIX_FMT_YUV420; 
      break;
    default: 
      pixelformat = V4L2_PIX_FMT_RGB24; 
      break;
    }
    fmt.fmt.pix.pixelformat = pixelformat;

    if (-1 == xioctl (m_tvfd, VIDIOC_S_FMT, &fmt)){
      perror("v4l2: VIDIOC_S_FMT(fmt2)");
    }
    // query back what we have set
    if (-1 == xioctl (m_tvfd, VIDIOC_G_FMT, &fmt)){
      perror("v4l2: VIDIOC_G_FMT(fmt2)");
    }
    m_gotFormat=fmt.fmt.pix.pixelformat;
  }

  switch(m_gotFormat){
  case V4L2_PIX_FMT_RGB32: case V4L2_PIX_FMT_RGB24:
  case V4L2_PIX_FMT_UYVY: case V4L2_PIX_FMT_YUV420: case V4L2_PIX_FMT_YUYV: 
  case V4L2_PIX_FMT_GREY: 
    break;
  default: 
    error("unknown format '%c%c%c%c'",
          (char)(m_gotFormat),
          (char)(m_gotFormat>>8),
          (char)(m_gotFormat>>16),
          (char)(m_gotFormat>>24));
    /* we should really return here! */
  }

  logpost(NULL, 5, "v4l2: got '%c%c%c%c'", 
          (char)(m_gotFormat),
          (char)(m_gotFormat>>8),
          (char)(m_gotFormat>>16),
          (char)(m_gotFormat>>24));

  if(!init_mmap ())goto closit;

  for (i = 0; i < m_nbuffers; ++i) {
    struct v4l2_buffer buf;
    
    memset (&(buf), 0, sizeof (buf));
    
    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;
    
    if (-1 == xioctl (m_tvfd, VIDIOC_QBUF, &buf)){
      perror("v4l2: VIDIOC_QBUF");//exit
    }
  }

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl (m_tvfd, VIDIOC_STREAMON, &type)){
    perror("v4l2: VIDIOC_STREAMON");//exit
  }

  m_frameSize=fmt.fmt.pix.sizeimage;
  
  /* fill in image specifics for Gem pixel object.  Could we have
     just used RGB, I wonder? */
  m_image.image.xsize = fmt.fmt.pix.width;
  m_image.image.ysize = fmt.fmt.pix.height;
  m_image.image.setCsizeByFormat(m_reqFormat);
  m_image.image.reallocate();
  
  debugPost("v4l2: format: %c%c%c%c -> 0x%X", 
            (char)(m_gotFormat),
            (char)(m_gotFormat>>8),
            (char)(m_gotFormat>>16),
            (char)(m_gotFormat>>24),
            m_reqFormat);
  switch(m_gotFormat){
  case V4L2_PIX_FMT_GREY  : m_colorConvert=(m_reqFormat!=GL_LUMINANCE); break;
  case V4L2_PIX_FMT_RGB24 : m_colorConvert=(m_reqFormat!=GL_BGR); break;
  case V4L2_PIX_FMT_RGB32 : m_colorConvert=true; break;
  case V4L2_PIX_FMT_UYVY  : m_colorConvert=(m_reqFormat!=GL_YCBCR_422_GEM); break;
  case V4L2_PIX_FMT_YUV420: m_colorConvert=1; break;
  default: m_colorConvert=true;
  }
  
  debugPost("v4l2: colorconvert=%d", m_colorConvert);
  
  /* create thread */
  m_continue_thread = 1;
  m_frame_ready = 0;
  pthread_create(&m_thread_id, 0, capturing_, this);
  while(!m_capturing){
    usleep(10);
    debugPost("v4l2: waiting for thread to come up");
  }
  
  post("v4l2: GEM: pix_video: Opened video connection 0x%X", m_tvfd);
  
  return(1);
  
 closit:
  debugPost("v4l2: closing it!");
  stopTransfer();
  debugPost("v4l2: closed it");
  return(0);
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
bool videoV4L2 :: stopTransfer()
{
  debugPost("v4l2: stoptransfer");
  if(!m_capturing)return false;
  int i=0;
  /* close the v4l2 device and dealloc buffer */
  /* terminate thread if there is one */
  if(m_continue_thread){
    void *dummy;
    m_continue_thread = 0;
    pthread_join (m_thread_id, &dummy);
  }
  while(m_capturing){
    usleep(10);
    debugPost("v4l2: waiting for thread to finish");
  }

  // unmap the mmap
  debugPost("v4l2: unmapping %d buffers: %x", m_nbuffers, m_buffers);
  if(m_buffers){
    for (i = 0; i < m_nbuffers; ++i)
      if (-1 == v4l2_munmap (m_buffers[i].start, m_buffers[i].length)){
        // oops: couldn't unmap the memory
      }
    debugPost("v4l2: freeing buffers: %x", m_buffers);
    free (m_buffers);
  }
  m_buffers=NULL;
  debugPost("v4l2: freed");

  // stop streaming
  if(m_tvfd){
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl (m_tvfd, VIDIOC_STREAMOFF, &type)){
      perror("v4l2: VIDIOC_STREAMOFF");
    }
  }
  
  debugPost("v4l2: de-requesting buffers");
  reqbufs(m_tvfd, 0);

  m_frame_ready = 0;
  m_rendering=false;
  debugPost("v4l2: stoppedTransfer");
  return true;
}

bool videoV4L2 :: setColor(int format)
{
  if (format<=0 || format==m_reqFormat)return -1;
  m_reqFormat=format;
  restartTransfer();
  return true;
}

std::vector<std::string> videoV4L2::enumerate() {
  std::vector<std::string> result;
  std::vector<std::string> glob, allglob;
  int i=0;
  glob=gem::files::getFilenameListing("/dev/video*");
  for(i=0; i<glob.size(); i++)
    allglob.push_back(glob[i]);

  glob=gem::files::getFilenameListing("/dev/v4l/*");
  for(i=0; i<glob.size(); i++)
    allglob.push_back(glob[i]);

  for(i=0; i<allglob.size(); i++) {
    std::string dev=allglob[i];
    logpost(NULL, 6, "V4L2: found possible device %s", dev.c_str());
    int fd=v4l2_open(dev.c_str(), O_RDWR);
    logpost(NULL, 6, "V4L2: v4l2_open returned %d", fd);
    if(fd<0)continue;
    struct v4l2_capability cap;
    if (-1 != xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
      if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        result.push_back(dev);      
      } else logpost(NULL, 5, "%s is v4l2 but cannot capture", dev.c_str());
    } else {
      logpost(NULL, 5, "%s is no v4l2 device", dev.c_str());
    }
    v4l2_close(fd);
  }
  
  return result;
}

void videoV4L2::addProperties(struct v4l2_queryctrl queryctrl,
			  gem::Properties&readable,
			  gem::Properties&writeable) {
  const char* name=NULL;
  gem::any typ;

  if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    return;

  switch(queryctrl.type) {
  case V4L2_CTRL_TYPE_BUTTON:
    break;
  case V4L2_CTRL_TYPE_BOOLEAN:
    typ=1;
    break;
  case V4L2_CTRL_TYPE_MENU:
    typ=queryctrl.maximum;
    break;
  case V4L2_CTRL_TYPE_INTEGER:
    typ=queryctrl.maximum;
    break;
  case V4L2_CTRL_TYPE_INTEGER64:
    typ=0;
    break;
  default:
    return;
  }
  
  name=(const char*)(queryctrl.name);

  m_readprops[name]=queryctrl;
  readable.set(name, typ);

  if (!(queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY)) {
    m_writeprops[name]=queryctrl;
    writeable.set(name, typ);
  }
}

bool videoV4L2 :: enumProperties(gem::Properties&readable,
				 gem::Properties&writeable) {
  struct v4l2_queryctrl queryctrl;
  __u32 id=0;

  if(m_tvfd<0)
    return false;

  readable.clear();
  writeable.clear();

  m_readprops.clear();
  m_writeprops.clear();

  memset (&queryctrl, 0, sizeof (queryctrl));

  for (id = V4L2_CID_BASE;
       id < V4L2_CID_LASTP1;
       id++) {
    queryctrl.id = id;
    if (0 == xioctl (m_tvfd, VIDIOC_QUERYCTRL, &queryctrl)) {
      addProperties(queryctrl, readable, writeable);

    } else {
      if (errno == EINVAL)
	continue;
    }
  }

  for (id = V4L2_CID_PRIVATE_BASE;;
       id++) {
    queryctrl.id = id;
    if (0 == xioctl (m_tvfd, VIDIOC_QUERYCTRL, &queryctrl)) {
      addProperties(queryctrl, readable, writeable);
    } else {
      if (errno == EINVAL)
	break;
    }
  }
  return true;
}
void videoV4L2 :: getProperties(gem::Properties&props) {
  int getformat=0;

  if(m_tvfd<0) {
    props.clear();
    return;
  }
  std::vector<std::string>keys=props.keys();
  int i=0; 
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    std::map<std::string,  struct v4l2_queryctrl>::iterator it = m_readprops.find(key);
    if(it != m_readprops.end()) {
      struct v4l2_queryctrl qc=it->second;
      struct v4l2_control vc;
      memset (&vc, 0, sizeof (vc));
      vc.id=qc.id;
      vc.value=0;

      int err=xioctl(m_tvfd, VIDIOC_G_CTRL, &vc);
      if(0==err) {
	props.set(key, vc.value);
      } else {
	props.erase(key);
      }
    } else {
      if("norm" == key) {
	v4l2_std_id stdid = 0;
	if(0==xioctl(m_tvfd, VIDIOC_G_STD, &stdid)) {
	  std::string std;
	  switch(stdid) {
	  default:
	  case V4L2_STD_UNKNOWN: std="UNKNOWN"; break; 
	  case V4L2_STD_ALL: std="ALL"; break; 

	  case V4L2_STD_ATSC: std="ATSC"; break; 
	  case V4L2_STD_625_50: std="625_50"; break; 
	  case V4L2_STD_525_60: std="525_60"; break; 
	  case V4L2_STD_SECAM: std="SECAM"; break; 
	  case V4L2_STD_SECAM_DK: std="SECAM_DK"; break; 
	  case V4L2_STD_NTSC: std="NTSC"; break; 
	  case V4L2_STD_PAL: std="PAL"; break; 
	  case V4L2_STD_PAL_DK: std="PAL_DK"; break; 
	  case V4L2_STD_PAL_BG: std="PAL_BG"; break; 
	  case V4L2_STD_DK: std="DK"; break; 
	  case V4L2_STD_GH: std="GH"; break; 
	  case V4L2_STD_B: std="B"; break; 
	  case V4L2_STD_MN: std="MN"; break; 
	  case V4L2_STD_ATSC_16_VSB: std="ATSC_16_VSB"; break; 
	  case V4L2_STD_ATSC_8_VSB: std="ATSC_8_VSB"; break; 
	  case V4L2_STD_SECAM_LC: std="SECAM_LC"; break; 
	  case V4L2_STD_SECAM_L: std="SECAM_L"; break; 
	  case V4L2_STD_SECAM_K1: std="SECAM_K1"; break; 
	  case V4L2_STD_SECAM_K: std="SECAM_K"; break; 
	  case V4L2_STD_SECAM_H: std="SECAM_H"; break; 
	  case V4L2_STD_SECAM_G: std="SECAM_G"; break; 
	  case V4L2_STD_SECAM_D: std="SECAM_D"; break; 
	  case V4L2_STD_SECAM_B: std="SECAM_B"; break; 
	  case V4L2_STD_NTSC_M_KR: std="NTSC_M_KR"; break; 
	  case V4L2_STD_NTSC_443: std="NTSC_443"; break; 
	  case V4L2_STD_NTSC_M_JP: std="NTSC_M_JP"; break; 
	  case V4L2_STD_NTSC_M: std="NTSC_M"; break; 
	  case V4L2_STD_PAL_60: std="PAL_60"; break; 
	  case V4L2_STD_PAL_Nc: std="PAL_Nc"; break; 
	  case V4L2_STD_PAL_N: std="PAL_N"; break; 
	  case V4L2_STD_PAL_M: std="PAL_M"; break; 
	  case V4L2_STD_PAL_K: std="PAL_K"; break; 
	  case V4L2_STD_PAL_D1: std="PAL_D1"; break; 
	  case V4L2_STD_PAL_D: std="PAL_D"; break; 
	  case V4L2_STD_PAL_I: std="PAL_I"; break; 
	  case V4L2_STD_PAL_H: std="PAL_H"; break; 
	  case V4L2_STD_PAL_G: std="PAL_G"; break; 
	  case V4L2_STD_PAL_B1: std="PAL_B1"; break; 
	  case V4L2_STD_PAL_B: std="PAL_B"; break; 
	  }
	  props.set("norm", std);
	}
      } else if("channel" == key) {
	int channel=0;
	if(0==xioctl(m_tvfd, VIDIOC_G_INPUT, &channel))
	  props.set("channel", channel);
      } else if("frequency" == key) {
	struct v4l2_frequency freq;
	memset (&(freq), 0, sizeof (freq));
	freq.tuner=0; /* FIXXME: this should be a bit more intelligent... */

	if(0==xioctl(m_tvfd, VIDIOC_G_FREQUENCY, &freq)) {
	  props.set("frequency", freq.frequency);
	}
      } else if("width" == key) {
	getformat|=WIDTH_FLAG;
      } else if("height" == key) {
	getformat|=HEIGHT_FLAG;
      } else {
      }
    }
  }

  if(getformat) {
    struct v4l2_format fmt;
    memset (&(fmt), 0, sizeof (fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl (m_tvfd, VIDIOC_G_FMT, &fmt)) {
      if(getformat & WIDTH_FLAG)
	props.set("width", fmt.fmt.pix.width);
      if(getformat & HEIGHT_FLAG)
	props.set("height", fmt.fmt.pix.height);
    } else {
      perror("VIDIOC_G_FMT");
    }
  }
}
void videoV4L2 :: setProperties(gem::Properties&props) {
  if(m_tvfd<0) {
    return;
  }
  bool restart=false;

  bool setformat=false, setcropping=false;


  std::vector<std::string>keys=props.keys();
  int i=0; 
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    std::map<std::string,  struct v4l2_queryctrl>::iterator it = m_writeprops.find(key);
    if(it != m_writeprops.end()) {
      double d=0;
      struct v4l2_queryctrl qc=it->second;
      struct v4l2_control vc;
      memset (&vc, 0, sizeof (vc));
      vc.id=qc.id;
      if(props.get(key, d)) {
	vc.value=d;
      }

      int err=xioctl(m_tvfd, VIDIOC_S_CTRL, &vc);

      if(V4L2_CTRL_TYPE_BUTTON == qc.type) {
	props.erase(key);
      }
    } else {
      if("norm" == key) {
	v4l2_std_id stdid = V4L2_STD_UNKNOWN;
	std::string std;
	switch(props.type(key)) {
	  case gem::Properties::STRING:
	    if(props.get(key, std)) {
	      if("ALL" == std)  stdid=V4L2_STD_ALL;
	      else if("PAL_B" == std)  stdid=V4L2_STD_PAL_B;
	      else if("PAL_B1" == std)  stdid=V4L2_STD_PAL_B1;
	      else if("PAL_G" == std)  stdid=V4L2_STD_PAL_G;
	      else if("PAL_H" == std)  stdid=V4L2_STD_PAL_H;
	      else if("PAL_I" == std)  stdid=V4L2_STD_PAL_I;
	      else if("PAL_D" == std)  stdid=V4L2_STD_PAL_D;
	      else if("PAL_D1" == std)  stdid=V4L2_STD_PAL_D1;
	      else if("PAL_K" == std)  stdid=V4L2_STD_PAL_K;
	      else if("PAL_M" == std)  stdid=V4L2_STD_PAL_M;
	      else if("PAL_N" == std)  stdid=V4L2_STD_PAL_N;
	      else if("PAL_Nc" == std)  stdid=V4L2_STD_PAL_Nc;
	      else if("PAL_60" == std)  stdid=V4L2_STD_PAL_60;
	      else if("NTSC_M" == std)  stdid=V4L2_STD_NTSC_M;
	      else if("NTSC_M_JP" == std)  stdid=V4L2_STD_NTSC_M_JP;
	      else if("NTSC_443" == std)  stdid=V4L2_STD_NTSC_443;
	      else if("NTSC_M_KR" == std)  stdid=V4L2_STD_NTSC_M_KR;
	      else if("SECAM_B" == std)  stdid=V4L2_STD_SECAM_B;
	      else if("SECAM_D" == std)  stdid=V4L2_STD_SECAM_D;
	      else if("SECAM_G" == std)  stdid=V4L2_STD_SECAM_G;
	      else if("SECAM_H" == std)  stdid=V4L2_STD_SECAM_H;
	      else if("SECAM_K" == std)  stdid=V4L2_STD_SECAM_K;
	      else if("SECAM_K1" == std)  stdid=V4L2_STD_SECAM_K1;
	      else if("SECAM_L" == std)  stdid=V4L2_STD_SECAM_L;
	      else if("SECAM_LC" == std)  stdid=V4L2_STD_SECAM_LC;
	      else if("ATSC_8_VSB" == std)  stdid=V4L2_STD_ATSC_8_VSB;
	      else if("ATSC_16_VSB" == std)  stdid=V4L2_STD_ATSC_16_VSB;
	      else if("MN" == std)  stdid=V4L2_STD_MN;
	      else if("B" == std)  stdid=V4L2_STD_B;
	      else if("GH" == std)  stdid=V4L2_STD_GH;
	      else if("DK" == std)  stdid=V4L2_STD_DK;
	      else if("PAL_BG" == std)  stdid=V4L2_STD_PAL_BG;
	      else if("PAL_DK" == std)  stdid=V4L2_STD_PAL_DK;
	      else if("PAL" == std)  stdid=V4L2_STD_PAL;
	      else if("NTSC" == std)  stdid=V4L2_STD_NTSC;
	      else if("SECAM_DK" == std)  stdid=V4L2_STD_SECAM_DK;
	      else if("SECAM" == std)  stdid=V4L2_STD_SECAM;
	      else if("525_60" == std)  stdid=V4L2_STD_525_60;
	      else if("625_50" == std)  stdid=V4L2_STD_625_50;
	      else if("ATSC" == std)  stdid=V4L2_STD_ATSC;
	    }
	    break;
	default:
	  continue;
	}
	xioctl(m_tvfd, VIDIOC_S_STD, &stdid);

      } else if("channel" == key) {
	double ch;
	if(props.get("channel", ch)) {
	  int channel=ch;
	  xioctl(m_tvfd, VIDIOC_S_INPUT, &channel);
	}
      } else if("frequency" == key) {
      } else if("width" == key) {
        double d=0.;
        if(props.get("width", d)) {
          int i=d;
          if(m_width != i)
            setformat=true;
          m_width = i;
        }
      } else if("height" == key) {
        double d=0.;
        if(props.get("height", d)) {
          int i=d;
          if(m_height != i)
            setformat=true;
          m_height = i;
        }
      } else {
      }
    }
  }


  if(setformat || setcropping)
    restart=true;

  if(restart) {
    bool rendering=m_rendering;
    if(m_capturing)stopTransfer();

#if 0
    setcropping=true;
    if(setcropping) {
      struct v4l2_cropcap cropcap;
      cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if (-1 == xioctl (m_tvfd, VIDIOC_CROPCAP, &cropcap)) {
	/* Errors ignored. */
      }

      memset(&(cropcap), 0, sizeof (cropcap));
      cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if (0 == xioctl (m_tvfd, VIDIOC_CROPCAP, &cropcap)) {
	struct v4l2_crop crop;

	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; /* reset to default */

	if (-1 == xioctl (m_tvfd, VIDIOC_S_CROP, &crop)) {
	  perror("v4l2: vidioc_s_crop");
	  switch (errno) {
	  case EINVAL:
	    /* Cropping not supported. */
	    break;
	  default:
	    /* Errors ignored. */
	    break;
	  }
	}
      }
    } // cropping
#endif

    if(setformat) {
      struct v4l2_format fmt;
      memset (&(fmt), 0, sizeof (fmt));
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      
      if (0 == xioctl (m_tvfd, VIDIOC_G_FMT, &fmt)) {
	double d;
        debugPost("current format %dx%d", fmt.fmt.pix.width, fmt.fmt.pix.height);

	if(props.get("width", d))
	  fmt.fmt.pix.width=d;
	if(props.get("height", d))
	  fmt.fmt.pix.height=d;

        debugPost("want format %dx%d", fmt.fmt.pix.width, fmt.fmt.pix.height);
	if(0 != xioctl (m_tvfd, VIDIOC_S_FMT, &fmt)) {
	  perror("VIDIOC_S_FMT(dim)");
	}
        debugPost("new format %dx%d", fmt.fmt.pix.width, fmt.fmt.pix.height);
      }
    } // format

    if (rendering)startTransfer();
  }
}


#else
videoV4L2 ::  videoV4L2() : videoBase("") {}
videoV4L2 :: ~videoV4L2() {}
#endif /* HAVE_VIDEO4LINUX2 */
