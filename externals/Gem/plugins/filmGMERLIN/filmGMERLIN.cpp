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

#ifdef HAVE_LIBGMERLIN_AVDEC
# define HAVE_GMERLIN
#endif


#ifdef HAVE_GMERLIN
#include "filmGMERLIN.h"
#include "plugins/PluginFactory.h"
#include "Gem/RTE.h"

using namespace gem::plugins;

REGISTER_FILMFACTORY("gmerlin", filmGMERLIN);

/////////////////////////////////////////////////////////
//
// filmGMERLIN
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

filmGMERLIN :: filmGMERLIN(void) : filmBase(),
                                   m_file(NULL),
                                   m_opt(NULL),
                                   m_seekable(false),
                                   m_gformat(NULL),
                                   m_finalformat(new gavl_video_format_t[1]),
                                   m_track(0),
                                   m_stream(0),
                                   m_gframe(NULL),
                                   m_finalframe(NULL),
                                   m_gconverter(NULL),
                                   m_fps_num(1), m_fps_denum(1),
                                   m_next_timestamp(0),
#ifdef USE_FRAMETABLE
                                   m_frametable(NULL),
#endif
                                   m_lastFrame(0),
                                   m_doConvert(false)
{
  m_gconverter=gavl_video_converter_create ();
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
filmGMERLIN :: ~filmGMERLIN()
{
  close();
  if(m_gconverter)gavl_video_converter_destroy(m_gconverter);m_gconverter=NULL;
}

void filmGMERLIN :: close(void)
{
  if(m_file)bgav_close(m_file);m_file=NULL;
#ifdef USE_FRAMETABLE
  if(m_frametable)gavl_frame_table_destroy(m_frametable); m_frametable=NULL;
#endif
  /* LATER: free frame buffers */

}

/////////////////////////////////////////////////////////
// logging
//
/////////////////////////////////////////////////////////
void filmGMERLIN::log(bgav_log_level_t level, const char *log_domain, const char *message)
{
  switch(level) {
  case BGAV_LOG_DEBUG:
    logpost(NULL, 6, "[pix_film:%s] %s", log_domain, message);
    break;
  case BGAV_LOG_INFO:
    logpost(NULL, 5, "[pix_film:%s] %s", log_domain, message);
    break;
  case BGAV_LOG_WARNING:
    post("[pix_film:%s] %s", log_domain, message);
    break;
  case BGAV_LOG_ERROR:
    error("[pix_film:%s!] %s", log_domain, message);
    break;
  default:break;
  }
}
void filmGMERLIN::log_callback (void *data, bgav_log_level_t level, const char *log_domain, const char *message)
{
  ((filmGMERLIN*)(data))->log(level, log_domain, message);
}

/////////////////////////////////////////////////////////
// really open the file !
//
/////////////////////////////////////////////////////////
bool filmGMERLIN :: open(const std::string sfilename, const gem::Properties&wantProps)
{
  close();

  m_track=0;

  m_file = bgav_create();
  if(!m_file) return false;
  m_opt = bgav_get_options(m_file);
  if(!m_opt) return false;

  /*
    bgav_options_set_connect_timeout(m_opt,   connect_timeout);
    bgav_options_set_read_timeout(m_opt,      read_timeout);
    bgav_options_set_network_bandwidth(m_opt, network_bandwidth);
  */
  bgav_options_set_seek_subtitles(m_opt, 0);
  bgav_options_set_sample_accurate(m_opt, 1);

  bgav_options_set_log_callback(m_opt,
                                log_callback,
                                this);

  const char*filename=sfilename.c_str();

  if(!strncmp(filename, "vcd://", 6))
    {
      if(!bgav_open_vcd(m_file, filename + 5))
        {
          //error("Could not open VCD Device %s",  filename + 5);
          return false;
        }
    }
  else if(!strncmp(filename, "dvd://", 6))
    {
      if(!bgav_open_dvd(m_file, filename + 5))
        {
          //error("Could not open DVD Device %s", filename + 5);
          return false;
        }
    }
  else if(!strncmp(filename, "dvb://", 6))
    {
      if(!bgav_open_dvb(m_file, filename + 6))
        {
          //error("Could not open DVB Device %s", filename + 6);
          return false;
        }
    }
  else {
    if(!bgav_open(m_file, filename)) {
      //error("Could not open file %s", filename);
      close();

      return false;
    }
  }
  if(bgav_is_redirector(m_file))
    {
      int i=0;
      int num_urls=bgav_redirector_get_num_urls(m_file);
      post("Found redirector:");
      for(i = 0; i < num_urls; i++)
        {
          post("#%d: '%s' -> %s", i, bgav_redirector_get_name(m_file, i), bgav_redirector_get_url(m_file, i));
        }
      for(i = 0; i < num_urls; i++) {
        filename=(char*)bgav_redirector_get_url(m_file, i);
        close();
        if (open(filename, wantProps)) {
          return true;
        }
      }
      /* couldn't open a redirected stream */
      return false;
    }

  /*
   * ok, we have been able to open the "file"
   * now get some information from it...
   */
  m_numTracks = bgav_num_tracks(m_file);
  // LATER: check whether this track has a video-stream...
  int numvstreams=bgav_num_video_streams (m_file, m_track);
  if(numvstreams) {
    bgav_select_track(m_file, m_track);
  } else {
    post("track %d does not contain a video-stream: skipping");
  }

  m_seekable=bgav_can_seek_sample(m_file);

  bgav_set_video_stream(m_file, m_stream, BGAV_STREAM_DECODE);
  if(!bgav_start(m_file)) {
    close();
    return false;
  }
  m_next_timestamp=bgav_video_start_time(m_file, m_track);

  m_gformat = (gavl_video_format_t*)bgav_get_video_format (m_file, m_stream);
  m_gframe = gavl_video_frame_create_nopad(m_gformat);

  m_finalformat->frame_width = m_gformat->frame_width;
  m_finalformat->frame_height = m_gformat->frame_height;
  m_finalformat->image_width = m_gformat->image_width;
  m_finalformat->image_height = m_gformat->image_height;
  m_finalformat->pixel_width = m_gformat->pixel_width;
  m_finalformat->pixel_height = m_gformat->pixel_height;
  m_finalformat->frame_duration = m_gformat->frame_duration;
  m_finalformat->timescale = m_gformat->timescale;

  m_finalformat->pixelformat=GAVL_RGBA_32;

  m_finalframe = gavl_video_frame_create_nopad(m_finalformat);
  m_doConvert= (gavl_video_converter_init (m_gconverter, m_gformat, m_finalformat)>0);
  m_image.image.xsize=m_gformat->frame_width;
  m_image.image.ysize=m_gformat->frame_height;
  m_image.image.setCsizeByFormat(GL_RGBA);
  m_image.image.notowned=true;
  m_image.image.upsidedown=true;
  m_image.newfilm=true;

  if(m_gformat->frame_duration) {
    m_fps = m_gformat->timescale / m_gformat->frame_duration;
  } else {
    m_fps = m_gformat->timescale;
  }

  m_fps_num=m_gformat->timescale;
  m_fps_denum=m_gformat->frame_duration;

  m_numFrames=-1;
#ifdef USE_FRAMETABLE
  m_frametable=bgav_get_frame_table(m_file, m_track);
  if(m_frametable)
    m_numFrames=gavl_frame_table_num_frames (m_frametable);
#endif

  gavl_time_t dur=bgav_get_duration (m_file, m_track);
  if(m_numFrames<0)
    if(dur!=GAVL_TIME_UNDEFINED)
      m_numFrames = gavl_time_to_frames(m_fps_num, 
					m_fps_denum, 
					dur);

  return true;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock* filmGMERLIN :: getFrame(){
  if(!m_file)return NULL;
  bgav_read_video(m_file, m_gframe, m_stream);
  if(m_doConvert) {
    gavl_video_convert (m_gconverter, m_gframe, m_finalframe);
    m_image.image.data=m_finalframe->planes[0];
  } else {
    m_image.image.data=m_gframe->planes[0];
  }
  m_image.newimage=true;
  m_image.image.upsidedown=true;

  m_next_timestamp=m_gframe->timestamp+m_gframe->duration;
  return &m_image;
}


/////////////////////////////////////////////////////////
// changeFrame
//
/////////////////////////////////////////////////////////
film::errCode filmGMERLIN :: changeImage(int imgNum, int trackNum){
  if(trackNum<0) {
    /* just automatically proceed to the next frame: this might speed up things for linear decoding */
    return film::SUCCESS;
  }
      
  if(!m_file)return film::FAILURE;

#if 0
  // LATER implement track-switching
  // this really shares a lot of code with open() so it should go into a separate function
  if(trackNum) {
    if(m_numTracks>trackNum || trackNum<0) {
      post("selected invalid track %d of %d", trackNum, m_numTracks);
    } else {
      int numvstreams=bgav_num_video_streams (m_file, m_track);
      post("track %d contains %d video streams", m_track, numvstreams);
      if(numvstreams) {
        bgav_select_track(m_file, m_track);
#ifdef USE_FRAMETABLE
        if(m_frametable) {
          gavl_frame_table_destroy(m_frametable);
          m_frametable=bgav_get_frame_table(m_file, m_track);
        }
#endif
      } else {
        post("track %d does not contain a video-stream: skipping");
      }
    }
  }
#endif



  if(imgNum>=m_numFrames || imgNum<0)return film::FAILURE;
  if(imgNum>0)m_curFrame=imgNum;

  if(bgav_can_seek(m_file)) {
#ifdef USE_FRAMETABLE
    if(m_frametable) {
      //  no assumptions about fixed framerate
      int64_t seekpos = gavl_frame_table_frame_to_time(m_frametable, imgNum, NULL);
      bgav_seek_video(m_file, m_track, seekpos);
      return film::SUCCESS;
    } 
#else
    if(0) {
#warning no frametable with outdated version of gmerlin-avdecoder
    }
#endif
    else {
      //  assuming fixed framerate
      /*
        Plaum: "Relying on a constant framerate is not good."
        m_fps_denum and m_fps_num are set only once!
      */
      int64_t seekposOrg = imgNum*m_fps_denum;
      int64_t seekpos = seekposOrg;

#if 0
      // LATER: set a minimum frame offset
      // keep in mind that m_fps_denum could be 1!
      // so it's better to calculate the difference in milliseconds and compare
      int64_t diff=m_next_timestamp-seekpos;
#define TIMESTAMP_OFFSET_MAX 5
      if(diff<TIMESTAMP_OFFSET_MAX && diff>(TIMESTAMP_OFFSET_MAX * -1)) {
        // hey we are already there...
        return film::SUCCESS;
      }
#endif



      bgav_seek_scaled(m_file, &seekpos, m_fps_num);
      if(seekposOrg == seekpos)
        return film::SUCCESS;
      /* never mind: always return success... */
      return film::SUCCESS;
    }
  }

  return film::FAILURE;
}
#endif // GMERLIN
