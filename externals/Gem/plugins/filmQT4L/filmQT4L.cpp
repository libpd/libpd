////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file 
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_LIBQUICKTIME

#include "filmQT4L.h"
#include "plugins/PluginFactory.h"
#include "Gem/Properties.h"
#include "Gem/RTE.h"

using namespace gem::plugins;

#include <colormodels.h>

REGISTER_FILMFACTORY("quicktime4linux", filmQT4L);

/////////////////////////////////////////////////////////
//
// filmQT4L
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

filmQT4L :: filmQT4L(void) : filmBase(),
			     m_quickfile(NULL),
			     m_qtformat(0),
			     m_lastFrame(0)
{
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
filmQT4L :: ~filmQT4L()
{
  close();
}

void filmQT4L :: close(void)
{
  if(m_quickfile)quicktime_close(m_quickfile);
  m_quickfile=0;
}

/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////
bool filmQT4L :: open(const std::string filename, const gem::Properties&wantProps)
{
  int wantedFormat=GL_RGBA;
  double d;
  unsigned int format=0;
  if(wantProps.get("format", d)) {
    format=d;
  }
  switch(format){
  default:
    break;
  case GL_RGBA:
  case GL_YCBCR_422_GEM:
  case GL_LUMINANCE:
    m_wantedFormat=format;
    break;
  }

  char*cfilename=const_cast<char*>(filename.c_str());
  if (quicktime_check_sig(cfilename)){ /* ok, this is quicktime */
    if (!(m_quickfile = quicktime_open(filename.c_str(), 1, 0))){
//      post("filmQT4L: Unable to open file: %s", filename.c_str());
      return false;
    }
    m_curFrame = -1;
	
    // Get the number of tracks
    m_numTracks = quicktime_video_tracks(m_quickfile);
    // Get the length of the movie (on track current track)
    m_numFrames = quicktime_video_length(m_quickfile, m_curTrack) - 1;
    // Get the frame-rate
    m_fps = quicktime_frame_rate(m_quickfile, m_curTrack);
    // Get the video dimensions
    m_image.image.xsize = quicktime_video_width (m_quickfile, m_curTrack);
    m_image.image.ysize = quicktime_video_height(m_quickfile, m_curTrack);
    if (!quicktime_supported_video(m_quickfile, m_curTrack)){
      //    char *codec = quicktime_video_compressor(m_quickfile, m_curTrack);
      //    post("filmQT4L: unsupported CODEC '%s'!", codec);
      quicktime_close(m_quickfile);
      m_quickfile=0;
      return false;
    }
    m_image.image.setCsizeByFormat(wantedFormat);
    m_image.image.reallocate();

    m_qtimage.xsize=m_image.image.xsize;
    m_qtimage.ysize=m_image.image.ysize;
    m_qtimage.setCsizeByFormat(GL_RGB);
    m_qtimage.reallocate();

    m_newfilm = true;
   return true;
  }
  goto unsupported;
 unsupported:
  close();
  return false;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock* filmQT4L :: getFrame(){
  int i=m_image.image.ysize;
  if (m_lastFrame==m_curFrame && 
      m_image.image.format==m_wantedFormat) 
  {
	  m_image.newimage=0; 
	  return &m_image;
  }

  m_image.image.setCsizeByFormat(m_wantedFormat);
  m_image.image.reallocate();

  pixBlock* pimage = 0;
  unsigned char **rows = new unsigned char*[m_image.image.ysize];
  while(i--)rows[i]=m_qtimage.data+m_qtimage.xsize*m_qtimage.csize*(m_qtimage.ysize-i-1);

  m_lastFrame=m_curFrame;

  if (quicktime_decode_video(m_quickfile, rows, m_curTrack)) {
    post("filmQT4L:: couldn't decode video !");
  }else {
    m_image.image.convertFrom(&m_qtimage);
    m_image.newimage=1; m_image.image.upsidedown=false;
    pimage = &m_image;
    if(m_newfilm)m_image.newfilm=1; 
	m_newfilm=false;
  }
  delete[] rows;
  return pimage;
}

film::errCode filmQT4L :: changeImage(int imgNum, int trackNum){
  if(imgNum>m_numFrames || imgNum<0)return film::FAILURE;
  if  (imgNum>0)m_curFrame=imgNum;
  if(trackNum>0)m_curTrack=trackNum;

  int i=-1;
#ifdef HAVE_QUICKTIME_SEEK_VIDEO
  lqt_seek_video  	(m_quickfile, m_curTrack, m_curFrame, m_curTrack);
#else
  if ((i=quicktime_set_video_position(m_quickfile, m_curFrame, m_curTrack))){  }
#endif
  return film::SUCCESS;
}
#endif // QT4L
