////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "Gem/GemConfig.h"

#if defined __APPLE__ && !defined __x86_64__
// with OSX10.6, apple has removed loads of Carbon functionality (in 64bit mode)
// LATER make this a real check in configure
# define HAVE_CARBONQUICKTIME
#endif

#if !defined HAVE_CARBONQUICKTIME  && defined GEM_FILMBACKEND && GEM_FILMBACKEND == GEM_FILMBACKEND_Darwin
# undef GEM_FILMBACKEND
#endif

#if defined GEM_FILMBACKEND && GEM_FILMBACKEND == GEM_FILMBACKEND_Darwin

#define HELPSYMBOL "pix_film"
#include "pix_filmDarwin.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_filmDarwin, t_symbol *, A_DEFSYM);

/////////////////////////////////////////////////////////
//
// pix_filmDarwin
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

pix_filmDarwin :: pix_filmDarwin(t_symbol *filename) :
	pix_filmOS(filename), 
  m_srcGWorld(NULL), 
  m_hiquality(1),
  m_play(0),
  m_rate(1.0),
  prevTime(0), curTime(0),
  m_Task(0),
  m_volume(0.f),
  m_movie(NULL)
{
  m_colorspace = GL_YUV422_GEM;

  // make sure that there are some characters
  if (filename && filename->s_name && filename->s_name[0]) openMess(filename);
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_filmDarwin :: ~pix_filmDarwin()
{
  closeMess();
  deleteBuffer();

  outlet_free(m_outNumFrames);
  outlet_free(m_outEnd);
}

void pix_filmDarwin :: closeMess(void)
{
  switch (m_haveMovie) {
  case GEM_MOVIE_MOV:
  case GEM_MOVIE_AVI:
  case GEM_MOVIE_MPG:
    ::DisposeMovie(m_movie);
    ::DisposeGWorld(m_srcGWorld);
    m_srcGWorld = NULL;
    m_haveMovie = GEM_MOVIE_NONE;
    break;
  case GEM_MOVIE_NONE:
  default:
    break;
  }
}
/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////
void pix_filmDarwin :: realOpen(char *filename)
{
  FSSpec		theFSSpec;
  OSErr		err = noErr;
  FSRef		ref;
	
	Track		movieTrack, audioTrack;
	Media		trackMedia;
	
	long		sampleCount;

  long		m_rowBytes;
	
	MatrixRecord	matrix;
	
  if (!filename[0]) {
    error("no filename passed");
  } else {
    UInt8*filename8=reinterpret_cast<UInt8*>(filename);
    err = ::FSPathMakeRef(filename8, &ref, NULL);
    err = ::FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, &theFSSpec, NULL);

    if (err) {
      error("unable to find file: %s", filename);
      return;
    }
    m_haveMovie = GEM_MOVIE_MOV;
  }

  short	refnum = 0;
  err = ::OpenMovieFile(&theFSSpec, &refnum, fsRdPerm);
  if (err) {
    error("couldn't open the movie file: %#s (%d)", theFSSpec.name, err);
    if (refnum) ::CloseMovieFile(refnum);
    return;
  }

	

  ::NewMovieFromFile(&m_movie, refnum, NULL, NULL, newMovieActive, NULL);
  if (refnum) ::CloseMovieFile(refnum);

  m_reqFrame = 0;
  m_curFrame = -1;
  m_numTracks = static_cast<int>(GetMovieTrackCount(m_movie));
	
  movieTrack = GetMovieIndTrackType(m_movie,1,VideoMediaType,movieTrackMediaType);  //get first video track
	
  trackMedia = GetTrackMedia(movieTrack);
	
  sampleCount = GetMediaSampleCount(trackMedia);
	
  m_numFrames = sampleCount;
	
  audioTrack = GetMovieIndTrackType(m_movie,1,SoundMediaType,movieTrackMediaType);
	
  SetTrackEnabled(audioTrack, FALSE);
	
  // Get the length of the movie

  movieDur = static_cast<long>(GetMovieDuration(m_movie));
  movieScale = static_cast<long>(GetMovieTimeScale(m_movie));

	
  durationf = static_cast<double>(movieDur)/static_cast<double>(m_numFrames);

  // Get the bounds for the movie
  ::GetMovieBox(m_movie, &m_srcRect);
  OffsetRect(&m_srcRect,  -m_srcRect.left,  -m_srcRect.top);
  SetMovieBox(m_movie, &m_srcRect);
  m_xsize = m_srcRect.right - m_srcRect.left;
  m_ysize = m_srcRect.bottom - m_srcRect.top;

	//long	index;
	
	//special code for trapping HD formats which have pixel dimensions which are different from what QT reports
	//this is undocumented anywhere by Apple - thanks to Marc Van Olmen for helping sort this out
	
	ImageDescriptionHandle desc = NULL;
	
	desc = reinterpret_cast<ImageDescriptionHandle>(NewHandle(0));
	
	GetMediaSampleDescription(trackMedia,1,reinterpret_cast<SampleDescriptionHandle>(desc));
# ifdef kDVCPROHD720pCodecType
	//DVCPRO720p
	if ((*desc)->cType == kDVCPROHD720pCodecType){
	
		post("kDVCPROHD720pCodecType");
		
		m_xsize = 960;
		SetRect( &m_srcRect, 0, 0, m_xsize, m_ysize );
		SetMovieBox(m_movie, &m_srcRect);		
		ScaleMatrix(&matrix,FloatToFixed(0.75),FloatToFixed(1.),FloatToFixed(1.),FloatToFixed(1.));
	
		SetMovieMatrix(m_movie,&matrix);
	}

	
	//DVCPRO 1080i60
	if ((*desc)->cType == kDVCPROHD1080i60CodecType){
	
		post("kDVCPROHD1080i60CodecType");
		m_hiquality = 0;
		
		m_xsize = 1280;
		SetRect( &m_srcRect, 0, 0, m_xsize, m_ysize );
			
		ScaleMatrix(&matrix,FloatToFixed(2.f/3.f),FloatToFixed(1.),FloatToFixed(1.),FloatToFixed(1.));
		SetMovieBox(m_movie, &m_srcRect);
		SetMovieMatrix(m_movie,&matrix);
		
	}

# endif
	//DVCPRO 1080i
	
	//HDV
	
	//post("image description width %d heigh %d hRes %d vRes %d",(*desc)->width,(*desc)->height,Fix2Long((*desc)->hRes),Fix2Long((*desc)->vRes));
	
	// We will use a YUV GWorld/Texture to get the fastest performance
	// 16 bits per pixel for 4:2:2
	// RowBytes should be a multiple of 32 for GL_STORAGE_SHARED_APPLE to work
	// This means movie width for 16bits need to be a multiple of 16
	//   (and for rgba/32bits width needs to be a multiple of 32)
	// we pad out to that. The texture coords ensure we do not use the extra bytes.
	int bpp;
	if (m_colorspace == GL_RGBA_GEM)
		bpp = 32;
	else
		bpp = 16;

	UInt32 thePadOffset = m_xsize % bpp;
	if( thePadOffset != 0 )
    {
      m_xsize += (bpp - thePadOffset);
      SetRect( &m_srcRect, 0, 0, m_xsize, m_ysize );
    }

  if (m_colorspace == GL_RGBA_GEM){
    m_format = GL_RGBA_GEM;
    createBuffer();
    prepareTexture();
    m_rowBytes = m_xsize * 4;
    if (m_hiquality) SetMoviePlayHints(m_movie, hintsHighQuality, hintsHighQuality);
    err = QTNewGWorldFromPtr(	&m_srcGWorld,
                              k32ARGBPixelFormat,	// gives noErr
                              &m_srcRect,
                              NULL,
                              NULL,
                              0,
                              m_pixBlock.image.data,
                              m_rowBytes);

  }else{
    m_format=GL_YUV422_GEM;
    createBuffer();
    // prepareTexture();
    m_rowBytes = m_xsize * 2;
    if (m_hiquality) SetMoviePlayHints(m_movie, hintsHighQuality | hintsDeinterlaceFields, hintsHighQuality | hintsDeinterlaceFields);
    err = QTNewGWorldFromPtr(	&m_srcGWorld,
                              k422YpCbCr8CodecType,
                              &m_srcRect,
                              NULL,
                              NULL,
                              0,
                              m_pixBlock.image.data,
                              m_rowBytes);
  }
  if (err) {
    error("couldn't make QTNewGWorldFromPtr %d", err);
    m_haveMovie = 0;
    return;
  }

	

  /* movies task method */
  m_movieTime = GetMovieTime(m_movie,nil);
  playRate = GetMoviePreferredRate(m_movie);

  // *** set the graphics world for displaying the movie ***
  ::SetMovieGWorld(m_movie, m_srcGWorld, GetGWorldDevice(m_srcGWorld));

  if (m_auto) {
    SetMovieRate(m_movie,X2Fix(1.0));
    m_play = 1;
		
  }
  else {
    SetMovieRate(m_movie,X2Fix(0.0));
  }

	SetMovieVolume(m_movie,FloatToFixed(m_volume));
  ::MoviesTask(m_movie, 0);	// *** this does the actual drawing into the GWorld ***
  curTime = GetMovieTime(m_movie,NULL);
  prevTime = 0;
  newImage = 1;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_filmDarwin :: getFrame()
{
  short 	flags = nextTimeStep;
  OSType	whichMediaType = VisualMediaCharacteristic;
  if (!m_haveMovie) return;

  m_Task = 0;
  if (m_curFrame >= m_numFrames) m_curFrame = 0;

  // ***********************************
  //
  //what follows is some of the worst hack work i've ever done to get QT to 'work'
  //
  //the problem is that QT is very good a playing media if it manages everything itself internally.
  //however, that doesn't fit well with GEM because GEM has it's own internal tasking callbacks, so
  //in order to get the two to play nice, a bunch of ugly, shit code has to be done.   below is a way to
  //track the internal state of QT MoviesTask() and figure out which frame it is currently processing.
  //this avoids the frame being processed twice by the GEM render chain by managing the newImage flag
  //
  //note all of the crap to check for the direction of the playback and loop points.
  //
  // THERE MUST BE A BETTER WAY!!!!!!!!!!!!!!
  //
  // ************************************

  if (m_auto){
    //play the startmovie() way
    if (!m_play){
      SetMovieRate(m_movie,X2Fix(m_rate));
      m_play = 1;
      newImage = 0;
      return;
      //	post("curTime %d prevTime %d",curTime,prevTime);
      //	SetMovieVolume(m_movie, kFullVolume);
    }

    if (m_rate > 0.f) {
      if (IsMovieDone(m_movie)) {
        outlet_bang(m_outEnd);
        GoToBeginningOfMovie(m_movie);
        prevTime = 0;
        flags |= nextTimeEdgeOK;
        m_reqFrame = 0;
      }

      m_Task = 1;
      MoviesTask(m_movie, 0);	// *** this does the actual drawing into the GWorld ***
      curTime = GetMovieTime(m_movie,NULL);

      //check to see if the current position is past our next frame
      if (prevTime < curTime){
        //if (prevTime != curTime){
        newImage = 1;
        prevTime = curTime;
			
        //find next frame bounds using GetMovieNextIntertestingTime()
        GetMovieNextInterestingTime(m_movie,
                                    flags,
                                    1,
                                    &whichMediaType,
                                    curTime,
                                    0,
                                    &prevTime,
                                    //  NULL,
                                    nil);

      }
      else{
        //if it's still the same frame then don't process
        newImage = 0;
      }
    }
    else {

      if (GetMovieTime(m_movie,nil) <= 0) {
        GoToEndOfMovie(m_movie);
        prevTime = GetMovieTime(m_movie,NULL);
        curTime = prevTime;
        // get the frame prior to the last frame
        GetMovieNextInterestingTime(m_movie,
                                    flags,
                                    1,
                                    &whichMediaType,
                                    prevTime,
                                    -1,
                                    &prevTime,
                                    NULL);


      }else{

        m_Task = 1;
        MoviesTask(m_movie, 0);	// *** this does the actual drawing into the GWorld ***
        curTime = GetMovieTime(m_movie,NULL);

        if (prevTime >= curTime){
          newImage = 1;
          prevTime = curTime;
          //find next frame bounds using GetMovieNextIntertestingTime()
          GetMovieNextInterestingTime(m_movie,
                                      flags,
                                      1,
                                      &whichMediaType,
                                      prevTime,
                                      -1,
                                      &prevTime,
                                      NULL);

        }
        else{
          newImage = 0;
        }
      }
    }
    if (m_newFilm){
      newImage = 1;
      MoviesTask(m_movie, 0); // *** this does the actual drawing into the GWorld ***
      // curTime = GetMovieTime(m_movie,NULL);
    }

  }
  else
    {
      //play the manual way
      if (m_play) {
        SetMovieRate(m_movie,X2Fix(0.0));
        m_play = 0; //turn off play
        newImage = 0;
        m_movieTime = GetMovieTime(m_movie,NULL);
        //	SetMovieVolume(m_movie, kNoVolume);
        return;  //not sure about this
      }else{

        m_movieTime = m_reqFrame * duration;
        m_movieTime = static_cast<long>(static_cast<double>(m_reqFrame) * durationf);

        m_movieTime-=9; //total hack!! subtract an arbitrary amount and have nextinterestingtime find the exact place
        ::GetMovieNextInterestingTime(	m_movie,
                                        flags,
                                        1,
                                        &whichMediaType,
                                        m_movieTime,
                                        0,
                                        &m_movieTime,
                                        NULL);
      }
        
      SetMovieTimeValue(m_movie, m_movieTime);
      m_Task = 1;
      newImage = 1;
      MoviesTask(m_movie, 0);
      curTime = GetMovieTime(m_movie,NULL);
    }
  //I suppose if you roll your own YUV->ARGB it would go here?
}

void pix_filmDarwin :: postrender(GemState *state)
{
}

void pix_filmDarwin :: startRendering()
{
	//bit of a hack related to stopRendering()
	if (m_auto && m_haveMovie) SetMovieVolume(m_movie, static_cast<short>(m_volume * 255.f));
}

void pix_filmDarwin :: stopRendering()
{
	//bit of a hack to keep the sound from playing after rendering stops
	if (m_auto && m_haveMovie) SetMovieVolume(m_movie, kNoVolume);
}

void pix_filmDarwin :: LoadRam()
{
  TimeValue	length;
  OSErr err;
  if (m_haveMovie){
    m_movieTime = 0;
    length = GetMovieDuration(m_movie);
    err = LoadMovieIntoRam(m_movie,m_movieTime,length,keepInRam);
    if (err)
      {
        error("LoadMovieIntoRam failed miserably");
      }
  }else{
    error("no movie to load into RAM!");
  }
}

void pix_filmDarwin :: MovRate(float rate)
{
  m_rate = static_cast<float>(rate);
  if (m_auto && m_haveMovie) {
    SetMovieRate(m_movie,X2Fix(static_cast<double>(m_rate)));
  }
}

void pix_filmDarwin :: MovVolume(float volume)
{
  m_volume = static_cast<float>(volume);
  if (m_auto && m_haveMovie) {
    SetMovieVolume(m_movie,static_cast<short>(m_volume * 255.f));
  }
}


void pix_filmDarwin :: doDebug()
{
  post("---------- pix_filmDarwin doDebug start----------");
  post("m_numTracks = %d",m_numTracks);
  post("Movie duration = %d timescale = %d timebase = %d", movieDur, movieScale, reinterpret_cast<long>(GetMovieTimeBase(m_movie)));
  post("rect rt:%d lt:%d", m_srcRect.right, m_srcRect.left);
  post("rect top:%d bottom:%d", m_srcRect.top, m_srcRect.bottom);
  post("movie size x:%d y:%d", m_xsize, m_ysize);
  if (m_colorspace == GL_BGRA_EXT) post("color space ARGB");
  else  post("color space YUV");
  post("Preferred rate fixed: %d int: %d float %f", playRate, Fix2Long(playRate),static_cast<float>(Fix2X(playRate)));

  post("---------- pix_filmDarwin doDebug end----------");
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_filmDarwin :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_filmDarwin), gensym("pix_film"), A_DEFSYM, A_NULL);
  pix_filmOS::real_obj_setupCallback(classPtr);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_filmDarwin::openMessCallback),
                  gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_filmDarwin::ramCallback),
                  gensym("ram"),  A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_filmDarwin::hiqualityCallback),
                  gensym("hiquality"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_filmDarwin::rateCallback),
                  gensym("rate"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_filmDarwin::debugCallback),
                  gensym("debug"),  A_NULL);
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_filmDarwin::volumeCallback),
                  gensym("volume"), A_DEFFLOAT, A_NULL);

}

void pix_filmDarwin :: openMessCallback(void *data, t_symbol *filename)
{
  GetMyClass(data)->openMess(filename);
}

void pix_filmDarwin :: ramCallback(void *data)
{
  GetMyClass(data)->LoadRam();
}

void pix_filmDarwin :: hiqualityCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_hiquality=static_cast<int>(state);
}

void pix_filmDarwin :: rateCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->MovRate(state);
}

void pix_filmDarwin :: volumeCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->MovVolume(state);
}

void pix_filmDarwin :: debugCallback(void *data)
{
  GetMyClass(data)->doDebug();
}
#endif // GEM_FILMBACKEND_Darwin
