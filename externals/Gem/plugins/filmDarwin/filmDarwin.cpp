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
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "filmDarwin.h"
#ifdef HAVE_CARBONQUICKTIME

#include "plugins/PluginFactory.h"
#include "Gem/RTE.h"
#include "Gem/Properties.h"

using namespace gem::plugins;

REGISTER_FILMFACTORY("Darwin", filmDarwin);

/////////////////////////////////////////////////////////
//
// filmDarwin
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

filmDarwin :: filmDarwin(void) : filmBase(false),
                                 m_movie(NULL),
                                 m_srcGWorld(NULL),
                                 m_movieTime(0),
                                 m_movieTrack(0),
                                 m_movieMedia(0),
                                 m_timeScale(0),
                                 durationf(0.)
{
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
filmDarwin :: ~filmDarwin()
{
  close();
}

void filmDarwin :: close(void)
{
  if(m_srcGWorld){
    ::DisposeMovie(m_movie);
    ::DisposeGWorld(m_srcGWorld);
    m_srcGWorld = NULL;
  }
}

/////////////////////////////////////////////////////////
// open the file
//
/////////////////////////////////////////////////////////
bool filmDarwin :: open(const std::string filename, const gem::Properties&wantProps)
{
  double d;
  if(wantProps.get("colorspace", d) && d>0) {
    m_wantedFormat=d;
  }
  FSSpec		theFSSpec;
  OSErr		err = noErr;
  FSRef		ref;
  Rect		m_srcRect;
  long		m_rowBytes;
  OSType		whichMediaType = VisualMediaCharacteristic;
  short		flags = nextTimeMediaSample + nextTimeEdgeOK;
  short	refnum = 0;
  OSType pixelformat=0;
  long hints;
  
  if(filename.empty()) {
    //post("filmDarwin: no filename passed");
    goto unsupported;
  } else { 
    err = ::FSPathMakeRef((const UInt8*)filename.c_str(), &ref, NULL);
    err = ::FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, &theFSSpec, NULL);
    if (err) {
      //error("filmDarwin: Unable to find file: %s", filename.c_str());
      goto unsupported;
    }
  }
  
  err = ::OpenMovieFile(&theFSSpec, &refnum, fsRdPerm);
  if (err) {
    //error("filmDarwin: Couldn't open the movie file: %#s (%d)", theFSSpec.name, err);
    if (refnum) ::CloseMovieFile(refnum);
    goto unsupported;
  }
  
  ::NewMovieFromFile(&m_movie, refnum, NULL, NULL, newMovieActive, NULL);
  if (refnum) ::CloseMovieFile(refnum);
  
  // m_curFrame = -1;
  m_numTracks = (int)GetMovieTrackCount(m_movie);
  post("filmDarwin:  m_numTracks = %d",m_numTracks);

  // Get the length of the movie
  long	movieDur, movieScale;
  movieDur = (long)GetMovieDuration(m_movie);
  movieScale = (long)GetMovieTimeScale(m_movie);
        
  post("Movie duration = %d timescale = %d timebase = %d",movieDur,
       movieScale,
       (long)GetMovieTimeBase(m_movie));
#if 1
  if (true) {
	Track movieTrack = GetMovieIndTrackType(m_movie,
                                    1,
                                    VideoMediaType,
                                    movieTrackMediaType);  //get first video track
	Media trackMedia = GetTrackMedia(movieTrack);
  	m_numFrames= GetMediaSampleCount(trackMedia);
  	durationf = static_cast<double>(movieDur)/static_cast<double>(m_numFrames);
  }
#else
  GetMovieNextInterestingTime( m_movie, flags, (TimeValue)1, &whichMediaType, 0, 
                               fixed1, NULL, &duration);
  m_numFrames = movieDur/duration;
#endif
  // Get the bounds for the movie
  ::GetMovieBox(m_movie, &m_srcRect);
  OffsetRect(&m_srcRect,  -m_srcRect.left,  -m_srcRect.top);
  SetMovieBox(m_movie, &m_srcRect);	
  m_image.image.xsize = m_srcRect.right - m_srcRect.left;
  m_image.image.ysize = m_srcRect.bottom - m_srcRect.top;
  post("rect rt:%d lt:%d", m_srcRect.right, m_srcRect.left);
  post("rect top:%d bottom:%d", m_srcRect.top, m_srcRect.bottom);
  post("movie size x:%d y:%d", m_image.image.xsize, m_image.image.ysize);

#if 1
  switch(m_wantedFormat) {
  case 0: // if no other format is requested, use YUV
  case GL_YCBCR_422_APPLE:
    m_image.image.format = m_wantedFormat;
    hints |= hintsHighQuality | hintsDeinterlaceFields;
    pixelformat=k422YpCbCr8CodecType;
    break;
  default:
    m_image.image.format = GL_BGRA_EXT;
    hints |= hintsHighQuality;
    pixelformat=k32ARGBPixelFormat;
    break;
  }
  m_image.image.setCsizeByFormat();
#else
#warning add YUV support

  m_image.image.csize = 4;
  m_image.image.format = GL_BGRA_EXT;
  m_image.image.type = GL_UNSIGNED_INT_8_8_8_8_REV;
  hints |= hintsHighQuality;
  pixelformat=k32ARGBPixelFormat;
#endif

  m_image.image.data = new unsigned char [m_image.image.xsize*m_image.image.ysize*m_image.image.csize]; 
  m_rowBytes = m_image.image.xsize * m_image.image.csize;
  SetMoviePlayHints(m_movie, hints, hints);
  err = QTNewGWorldFromPtr(	&m_srcGWorld, 
                            pixelformat,
                            &m_srcRect, 
                            NULL, 
                            NULL, 
                            0, 
                            m_image.image.data, 
                            m_rowBytes);
  if (err) {
    //error("filmDarwin: Couldn't make QTNewGWorldFromPtr %d", err);
    goto unsupported;
  }
  m_movieTime = 0;
  // *** set the graphics world for displaying the movie ***
  ::SetMovieGWorld(m_movie, m_srcGWorld, GetGWorldDevice(m_srcGWorld));
  ::MoviesTask(m_movie, 0);	// *** this does the actual drawing into the GWorld ***
  return true;
  goto unsupported;
 unsupported:
  //post("Darwin: unsupported!");
  return false;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock* filmDarwin :: getFrame(){
  CGrafPtr	 	savedPort;
  GDHandle     	savedDevice;
  Rect		m_srcRect;
  PixMapHandle	m_pixMap;
  Ptr			m_baseAddr;
    
  ::GetGWorld(&savedPort, &savedDevice);
  ::SetGWorld(m_srcGWorld, NULL);
  ::GetMovieBox(m_movie, &m_srcRect);
    
  m_pixMap = ::GetGWorldPixMap(m_srcGWorld);
  m_baseAddr = ::GetPixBaseAddr(m_pixMap);

  // get the next frame of the source movie
  short 	flags = nextTimeStep;
  OSType	whichMediaType = VisualMediaCharacteristic;
  TimeValue	duration;

  if (IsMovieDone(m_movie)) {
    GoToBeginningOfMovie(m_movie);
    return NULL;
  }
    
  
  //check for last frame to loop the clip
  if (m_curFrame >= m_numFrames){
    return NULL;
    m_curFrame = 0;
    m_movieTime = 0;
  }

  m_movieTime = static_cast<long>(static_cast<double>(m_curFrame) * durationf);

  m_movieTime-=9; //total hack!! subtract an arbitrary amount and have nextinterestingtime find the exact place
    
  //check for -1
  if (m_movieTime < 0) m_movieTime = 0;
       
  // if this is the first frame, include the frame we are currently on
  if (m_curFrame == 0) flags |= nextTimeEdgeOK;

  // if (m_auto) {
  if (1) {
    ::GetMovieNextInterestingTime(m_movie,
                                  flags,
                                  1,
                                  &whichMediaType,
                                  m_movieTime,
                                  0,
                                  &m_movieTime,
                                  // NULL);
                                  &duration);
    flags = 0;
    flags = nextTimeStep;
    // m_curFrame++;

  }else{
  }

  // set the time for the frame and give time to the movie toolbox	
  SetMovieTimeValue(m_movie, m_movieTime); 
  MoviesTask(m_movie, 0);	// *** this does the actual drawing into the GWorld ***

  //  m_image.image.data = (unsigned char *)m_baseAddr;
  m_image.newimage=1;
  return &m_image;
}

film::errCode filmDarwin :: changeImage(int imgNum, int trackNum){
  m_curFrame=imgNum;
  //  return 0;
  return film::SUCCESS;
}
#endif /* CARBONQUiCKTIME */
