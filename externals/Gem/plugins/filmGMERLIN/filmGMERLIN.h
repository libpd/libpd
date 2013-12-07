/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load a digital video (quicktime4linux) for linux

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.


-----------------------------------------------------------------*/
#ifndef _INCLUDE_GEMPLUGIN__FILMGMERLIN_FILMGMERLIN_H_
#define _INCLUDE_GEMPLUGIN__FILMGMERLIN_FILMGMERLIN_H_
#include "plugins/filmBase.h"
#include <stdio.h>

# ifdef __cplusplus
extern "C" {
# endif
# include <gmerlin/avdec.h>

# include <gmerlin/bgav_version.h>
# if BGAV_BUILD >= BGAV_MAKE_BUILD(1,0,2)
#  define USE_FRAMETABLE
# else
#  undef USE_FRAMETABLE
# endif



# ifdef __cplusplus
}
# endif

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  filmGMERLIN
  
  Loads in a film
  
  KEYWORDS
  pix
  
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
class GEM_EXPORT filmGMERLIN : public filmBase {
 public:

  //////////
  // Constructor
  filmGMERLIN(void);

  //////////
  // Destructor
  virtual ~filmGMERLIN(void);

  //////////
  // open a movie up
  virtual bool open(const std::string filename, const gem::Properties&);
  //////////
  // close the movie file
  virtual void close(void);

  //////////
  // get the next frame
  virtual pixBlock* getFrame(void);

  //////////
  // set the next frame to read;
  virtual errCode changeImage(int imgNum, int trackNum = -1);

  //-----------------------------------
  // GROUP:	Movie data
  //-----------------------------------
 protected:
  bgav_t*   	 m_file;
  bgav_options_t * m_opt;
  bool           m_seekable; /* the track can be seeked */
  gavl_video_format_t*m_gformat,*m_finalformat;
  int m_track, m_stream;
  gavl_video_frame_t*m_gframe,*m_finalframe;
  gavl_video_converter_s*m_gconverter;

  int m_fps_num, m_fps_denum;

  int64_t m_next_timestamp;
#ifdef USE_FRAMETABLE
  gavl_frame_table_t *m_frametable;
#endif

  static void log_callback(void *data, bgav_log_level_t level, const char *log_domain, const char *message);
  virtual void log(bgav_log_level_t level, const char *log_domain, const char *message);

  int m_lastFrame;

 private:
  // whether we need to convert to use it in Gem
  bool m_doConvert;
};};};

#endif	// for header file
