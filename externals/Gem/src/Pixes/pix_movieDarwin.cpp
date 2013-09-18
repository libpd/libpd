////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com & cgc@humboldtblvd.com
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) 2002 James Tittle & Chris Clepper
//
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

// as long as there is no special help-file, fallback to the [pix_movie]
#define HELPSYMBOL "pix_movie"

#include "pix_movieDarwin.h"
#include "Gem/Manager.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_movieDarwin, t_symbol *, A_DEFSYM);

/////////////////////////////////////////////////////////
//
// pix_movie
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_movieDarwin :: pix_movieDarwin(t_symbol *filename) :
  m_oldTexCoords(NULL), m_oldNumCoords(0), m_oldTexture(0), m_oldImage(NULL),
  m_textureObj(0), m_xRatio(1.f), m_yRatio(1.f), upsidedown(true), m_textureType(GL_TEXTURE_RECTANGLE_EXT),
  x_filename(NULL), 
  m_frame(NULL), m_data(NULL), 
  m_haveMovie(0), m_auto(0), 
  m_numFrames(0), m_reqFrame(0), m_curFrame(0), m_track(0),
  m_xsize(0), m_ysize(0), m_csize(0), 
  m_film(false), m_newFilm(0), newImage(0), 
  m_colorspace(GL_YCBCR_422_GEM), m_format(0), m_rectangle(1),
  m_hiquality(1),
  m_play(0), m_rate(1.f)
{
  prevTime = 0;
  curTime = 0; 

  
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("img_num"));
  
  m_outNumFrames = outlet_new(this->x_obj, 0);
  m_outEnd       = outlet_new(this->x_obj, 0);
  m_outTex       = outlet_new(this->x_obj, 0);
  
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_movieDarwin :: ~pix_movieDarwin()
{
  // Clean up the movie
  closeMess();
  deleteBuffer();

  if(m_outNumFrames)
    outlet_free(m_outNumFrames);
  if(m_outEnd)
    outlet_free(m_outEnd);
  if(m_outTex)
    outlet_free(m_outTex);

  m_outNumFrames = NULL;
  m_outEnd       = NULL;
  m_outTex       = NULL;
}

void pix_movieDarwin :: deleteBuffer()
{
  if (m_data){
    delete [] m_data;
  }
  
  m_pixBlock.image.data=NULL;
  m_frame=m_data=NULL;
  m_pixBlock.image.xsize=m_pixBlock.image.ysize=m_pixBlock.image.csize=0;
}

void pix_movieDarwin :: createBuffer()
{
  const int neededXSize = m_xsize;
  const int neededYSize = m_ysize;
  int	oldx, oldy;
  
  oldx = 0;
  oldy = 0;

  if (neededXSize != oldx || neededYSize != oldy)
    {
      deleteBuffer();

      m_pixBlock.image.xsize = neededXSize;
      m_pixBlock.image.ysize = neededYSize;
      m_pixBlock.image.csize = m_csize;
      m_pixBlock.image.format= m_format;
  
      int dataSize = m_pixBlock.image.xsize * m_pixBlock.image.ysize * m_pixBlock.image.csize;

      m_data = new unsigned char[dataSize];

      m_pixBlock.image.data = m_data;
      m_frame =  m_data;

      m_pixBlock.image.csize = m_csize;
      m_pixBlock.image.format= m_format;

      oldx = m_pixBlock.image.xsize;
      oldy = m_pixBlock.image.ysize;
    }
}


void pix_movieDarwin :: closeMess(void)
{
  switch (m_haveMovie) {
  case GEM_MOVIE_MOV:
    ::DisposeMovie(m_movie);
    ::DisposeGWorld(m_srcGWorld);
    m_srcGWorld = NULL;
    m_haveMovie = GEM_MOVIE_NONE;
    break;
  case GEM_MOVIE_AVI:
    ::DisposeMovie(m_movie);
    ::DisposeGWorld(m_srcGWorld);
    m_srcGWorld = NULL;
    m_haveMovie = GEM_MOVIE_NONE;
    break;
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
// openMess
//
/////////////////////////////////////////////////////////

void pix_movieDarwin :: openMess(t_symbol *filename, int format)
{
  //  if (filename==x_filename)return;
  x_filename=filename;
  if (format)m_colorspace=format;

  char buf[MAXPDSTRING];
#if 0
  canvas_makefilename(getCanvas(), filename->s_name, buf, MAXPDSTRING);
#else
  snprintf(buf, MAXPDSTRING, "%s", findFile(filename->s_name).c_str());
#endif

  // Clean up any open files
  closeMess();

  m_haveMovie = GEM_MOVIE_NONE;
  realOpen(buf);
  if (m_haveMovie == GEM_MOVIE_NONE)return;

  t_atom ap[3];
  SETFLOAT(ap, m_numFrames);
  SETFLOAT(ap+1, m_xsize);
  SETFLOAT(ap+2, m_ysize);

  m_newFilm = 1;
  //outlet_float(m_outNumFrames, ((float)) *([a-zA-Z0-9._]));
  post("Loaded file: %s with %d frames (%dx%d)", buf, m_numFrames, m_xsize, m_ysize);
  outlet_list(m_outNumFrames, 0, 3, ap);
}

/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: realOpen(char *filename)
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
      error("Unable to find file: %s", filename);
      return;
    }
    m_haveMovie = GEM_MOVIE_MOV;
  }
    
  short	refnum = 0;
  err = ::OpenMovieFile(&theFSSpec, &refnum, fsRdPerm);
  if (err) {
    error("Couldn't open the movie file: %#s (%d)", theFSSpec.name, err);
    if (refnum) ::CloseMovieFile(refnum);
    return;
  }

	::NewMovieFromFile(&m_movie, refnum, NULL, NULL, newMovieActive, NULL);
	if (refnum) ::CloseMovieFile(refnum);

	m_reqFrame = 0;
	m_curFrame = -1;
	m_numTracks = static_cast<int>(GetMovieTrackCount(m_movie));

	// Get the length of the movie
	
	movieTrack = GetMovieIndTrackType(m_movie,1,VideoMediaType,movieTrackMediaType);  //get first video track
	
	trackMedia = GetTrackMedia(movieTrack);
	
	sampleCount = GetMediaSampleCount(trackMedia);
	
	m_numFrames = sampleCount;
	
	audioTrack = GetMovieIndTrackType(m_movie,1,SoundMediaType,movieTrackMediaType);
	
	SetTrackEnabled(audioTrack, FALSE);
        
	movieDur = static_cast<long>(GetMovieDuration(m_movie));
	movieScale = static_cast<long>(GetMovieTimeScale(m_movie));
        
  /*  //old method for getting length                                      
      OSType		whichMediaType = VisualMediaCharacteristic;
      short		flags = nextTimeMediaSample + nextTimeEdgeOK;
        
      TimeValue	theTime = 0;
	
      m_numFrames = -1;
      while (theTime >= 0) {
      m_numFrames++;
      ::GetMovieNextInterestingTime(m_movie,
      flags,
      1,
      &whichMediaType,
      theTime,
      0,
      &theTime,
      &duration);
      // after the first interesting time, don't include the time we
      //  are currently at.
      flags = nextTimeMediaSample;
      }
  */
  durationf = static_cast<float>(movieDur)/static_cast<float>(m_numFrames);

	// Get the bounds for the movie
	::GetMovieBox(m_movie, &m_srcRect);
  OffsetRect(&m_srcRect,  -m_srcRect.left,  -m_srcRect.top);
	SetMovieBox(m_movie, &m_srcRect);	
	m_xsize = m_srcRect.right - m_srcRect.left;
	m_ysize = m_srcRect.bottom - m_srcRect.top;
	
	
	ImageDescriptionHandle desc = NULL;
	
	desc = (ImageDescriptionHandle)NewHandle(0);
	
	GetMediaSampleDescription(trackMedia,1,(SampleDescriptionHandle)desc);
	
	//DVCPRO 720p
	if ((*desc)->cType == kDVCPROHD720pCodecType){
	
		post("kDVCPROHD720pCodecType");
		m_hiquality = 0;
		//SetMoviePlayHints(m_movie, hintsHighQuality, hintsHighQuality);
		//SetMoviePlayHints(m_movie, hintsDeinterlaceFields, 0);
		
		
		m_xsize = 960;
		SetRect( &m_srcRect, 0, 0, m_xsize, m_ysize );
			
		ScaleMatrix(&matrix,FloatToFixed(0.75),FloatToFixed(1.),FloatToFixed(1.),FloatToFixed(1.));
		SetMovieBox(m_movie, &m_srcRect);
		SetMovieMatrix(m_movie,&matrix);
		
	}
	
	
	
	//DVCPRO 1080i60
	if ((*desc)->cType == kDVCPROHD1080i60CodecType){
	
		post("kDVCPROHD1080i60CodecType");
		m_hiquality = 0;
		//SetMoviePlayHints(m_movie, hintsHighQuality, hintsHighQuality);
		//SetMoviePlayHints(m_movie, hintsDeinterlaceFields, 0);
		
		//kICMImageDescriptionPropertyID_CleanApertureClipRect
		
		SInt32 clap;
		ICMImageDescriptionGetProperty(desc,kQTPropertyClass_ImageDescription,kICMImageDescriptionPropertyID_CleanAperture,
                                   sizeof(clap),&clap,NULL);
		
		m_xsize = 1280;
		SetRect( &m_srcRect, 0, 0, m_xsize, m_ysize );
			
		ScaleMatrix(&matrix,FloatToFixed(2.f/3.f),FloatToFixed(1.),FloatToFixed(1.),FloatToFixed(1.));
		SetMovieBox(m_movie, &m_srcRect);
		SetMovieMatrix(m_movie,&matrix);
		
	}
	
	//HDV
	//I have no idea what to test for HDV	
	
	int bpp;
	if (m_colorspace == GL_BGRA_EXT)
		bpp = 32;
	else
		bpp = 16;
	UInt32 thePadOffset = m_xsize % bpp;
	if( thePadOffset != 0 )
    {
      m_xsize += (bpp - thePadOffset);
      SetRect( &m_srcRect, 0, 0, m_xsize, m_ysize );
    }
	
       
  if (m_colorspace == GL_BGRA_EXT){
    m_csize = 4;
    m_format = GL_BGRA_EXT;
    m_pixBlock.image.type = GL_UNSIGNED_INT_8_8_8_8_REV;
            
    createBuffer();
    prepareTexture();
    m_rowBytes = m_xsize * 4;
    if (m_hiquality) SetMoviePlayHints(m_movie, hintsHighQuality | hintsDeinterlaceFields, hintsHighQuality | hintsDeinterlaceFields);
    err = QTNewGWorldFromPtr(	&m_srcGWorld, 
                              k32ARGBPixelFormat,	
                              &m_srcRect, 
                              NULL, 
                              NULL, 
                              0, 
                              m_pixBlock.image.data, 
                              m_rowBytes);
                                        
  }else{
    m_csize = 2;
    m_format = GL_YCBCR_422_APPLE;
#ifndef i386
    m_pixBlock.image.type = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
#else
    m_pixBlock.image.type = GL_UNSIGNED_SHORT_8_8_APPLE;
#endif
            
			
    createBuffer();
    prepareTexture();
    m_rowBytes = m_xsize * 2;
    if (m_hiquality) SetMoviePlayHints(m_movie, hintsHighQuality | hintsDeinterlaceFields, hintsHighQuality | hintsDeinterlaceFields);
    err = QTNewGWorldFromPtr(	&m_srcGWorld, 
                              k422YpCbCr8CodecType,	
                              // kComponentVideoUnsigned,
                              &m_srcRect, 
                              NULL, 
                              NULL, 
                              0, 
                              m_pixBlock.image.data, 
                              m_rowBytes);
  }
	if (err) {
		error("Couldn't make QTNewGWorldFromPtr %d", err);
		m_haveMovie = 0;
		return;
	}
             
  /* movies task method */
  m_movieTime = GetMovieTime(m_movie,nil);
  playRate = GetMoviePreferredRate(m_movie);
    
	// *** set the graphics world for displaying the movie ***
	::SetMovieGWorld(m_movie, m_srcGWorld, GetGWorldDevice(m_srcGWorld));
        
  if (m_auto) { 
    // SetMovieRate(m_movie,X2Fix(m_rate));
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
  m_newFilm = 1;
  //StartMovie(m_movie);
        
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: getFrame()
{	
  short 	flags = nextTimeStep;
  OSType	whichMediaType = VisualMediaCharacteristic;
  if (!m_haveMovie) return;

  if (m_curFrame >= m_numFrames) m_curFrame = 0;
    
  // **********************************
  //
  // what follows is some of the worst hack work i've ever done to get QT to 'work'
  //
  // the problem is that QT is very good a playing media if it manages everything itself internally.
  // however, that doesn't fit well with GEM because GEM has it's own internal tasking callbacks, so
  // in order to get the two to play nice, a bunch of ugly, shit code has to be done.   below is a way to
  // track the internal state of QT MoviesTask() and figure out which frame it is currently processing.
  // this avoids the frame being processed twice by the GEM render chain by managing the newImage flag
  //
  // note all of the crap to check for the direction of the playback and loop points.
  //
  // THERE MUST BE A BETTER WAY!!!!!!!!!!!!!!
  //
  // ************************************
  if (m_auto){
    //play the startmovie() way
    if (!m_play) SetMovieRate(m_movie,X2Fix(m_rate));
    m_play = 1;
    
    if (m_rate > 0.f) {
      if (IsMovieDone(m_movie)) {
        outlet_bang(m_outEnd);
        GoToBeginningOfMovie(m_movie);
        prevTime = 0;
        flags |= nextTimeEdgeOK;
      }
      
      MoviesTask(m_movie, 0);	// *** this does the actual drawing into the GWorld *** 
      curTime = GetMovieTime(m_movie,NULL);
      
      //check to see if the current position is past our next frame
      if (prevTime < curTime){
        newImage = 1;
        prevTime = curTime;
                    
        //find next frame bounds using GetMovieNextIntertestingTime()
        GetMovieNextInterestingTime(m_movie,
                                    flags,
                                    1,
                                    &whichMediaType,
                                    prevTime,
                                    0,
                                    &prevTime,
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
      //MoviesTask(m_movie, 0);	// *** this does the actual drawing into the GWorld *** 
      // curTime = GetMovieTime(m_movie,NULL);
    }
                            
  }
  else
    {
      //play the manual way
      if (m_play) {
        SetMovieRate(m_movie,X2Fix(0.0));
        m_play = 0; //turn off play
        return;
      }
        
      //m_movieTime = m_reqFrame * duration;
      m_movieTime = static_cast<long>(static_cast<float>(m_reqFrame) * durationf);
        
      m_movieTime-=9; //total hack!! subtract an arbitrary amount and have nextinterestingtime find the exact place
      ::GetMovieNextInterestingTime(	m_movie,
                                      flags,
                                      1,
                                      &whichMediaType,
                                      m_movieTime,
                                      0,
                                      &m_movieTime,
                                      NULL);
      // &durationf);
                                         
      SetMovieTimeValue(m_movie, m_movieTime);
      MoviesTask(m_movie, 0);
        
    }

}

void pix_movieDarwin :: render(GemState *state)
{
  m_oldImage     = state->image;

  texFrame(state,1);
  getFrame();
  // MoviesTask(m_movie,0);

  if (m_textureObj)  {
    t_atom ap[5];
    SETFLOAT(ap, (t_float)m_textureObj); // OK
    SETFLOAT(ap+1, (t_float)m_xRatio);   // constructor
    SETFLOAT(ap+2, (t_float)m_yRatio);   // constructor
    SETFLOAT(ap+3, (t_float)m_textureType);
    SETFLOAT(ap+4, (t_float)upsidedown);    // always TRUE on Darwin (with QT backend)
    outlet_list(m_outTex, &s_list, 5, ap);
  }
}


/////////////////////////////////////////////////////////
// on opening a file, prepare for texturing
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: prepareTexture()
{
  float x2,y2;
  float xratio, yratio;
    
  x2 = powerOfTwo(m_pixBlock.image.xsize);
  y2 = powerOfTwo(m_pixBlock.image.ysize);
  xratio = x2 / m_pixBlock.image.xsize;
  yratio = y2 / m_pixBlock.image.ysize;
    
  if (m_rectangle) {
    
    m_coords[3].s = 0.f;
    m_coords[3].t = 0.f;
    
    m_coords[2].s = m_pixBlock.image.xsize;
    m_coords[2].t = 0.f;
    
    m_coords[1].s = m_pixBlock.image.xsize;
    m_coords[1].t = m_pixBlock.image.ysize;
    
    m_coords[0].s = 0.f;
    m_coords[0].t = m_pixBlock.image.ysize;
        
  }
  else
    {
      m_coords[3].s = 0.f;
      m_coords[3].t = 0.f;
    
      m_coords[2].s = xratio;
      m_coords[2].t = 0.f;
    
      m_coords[1].s = xratio;
      m_coords[1].t = yratio;
    
      m_coords[0].s = 0.f;
      m_coords[0].t = yratio;
    }
}

/////////////////////////////////////////////////////////
// setUpTextureState
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: setUpTextureState()
{
  if (m_rectangle && GemMan::texture_rectangle_supported) {
    logpost(NULL, 5, "using rectangle textures");
    m_textureType = GL_TEXTURE_RECTANGLE_ARB;
  }
  else  {
    m_textureType = GL_TEXTURE_2D;
    logpost(NULL, 5,"using power of two textures");
  }

  glTexParameterf(m_textureType, GL_TEXTURE_PRIORITY, 0.0);

  glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
     
  glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(m_textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(m_textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

}



/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: texFrame(GemState *state, int doit)
{
  m_oldTexCoords = state->texCoords;
  m_oldNumCoords = state->numTexCoords;
  m_oldTexture   = state->texture;

  state->texture = 2;
  state->texCoords = m_coords;
  state->numTexCoords = 4;
  
  state->image = &m_pixBlock;
  
  glActiveTexture(GL_TEXTURE0_ARB);
  
  if (GL_TEXTURE_2D == m_textureType){
    glEnable(m_textureType);
    glBindTexture(m_textureType, m_textureObj);
 
    if (m_pixBlock.image.csize != m_dataSize[0] ||
        m_pixBlock.image.xsize != m_dataSize[1] ||
        m_pixBlock.image.ysize != m_dataSize[2]) {
      m_dataSize[0] = m_pixBlock.image.csize;
      m_dataSize[1] = m_pixBlock.image.xsize;
      m_dataSize[2] = m_pixBlock.image.ysize;
      glTexImage2D(m_textureType, 0,
                   GL_RGBA,
                   m_pixBlock.image.xsize,
                   m_pixBlock.image.ysize, 0,
                   m_pixBlock.image.format,//       GL_YCBCR_422_APPLE,
                   m_pixBlock.image.type,//       GL_YCBCR_422_APPLE,
                   m_pixBlock.image.data);
            
      logpost(NULL, 5, "new power-of-two texture size - glTexImage2D");
    }
    m_xRatio = 1.0;
    m_yRatio = 1.0;

    glTexSubImage2D(m_textureType, 0,
                    0, 0,			// position
                    m_xsize,			// the x size of the data
                    m_ysize,			// the y size of the data
                    m_pixBlock.image.format,	// the format, e.g.  GL_YCBCR_422_APPLE,
                    m_pixBlock.image.type,	// the type, e.g.  GL_UNSIGNED_SHORT_8_8_REV_APPLE,
                    m_frame);		//  m_pixBlock.image.data);
  } else if (GL_TEXTURE_RECTANGLE_ARB == m_textureType) {
    glEnable(m_textureType);
    glBindTexture(m_textureType, m_textureObj);

    if (m_newFilm ){
      glTextureRangeAPPLE( m_textureType, 
                           m_pixBlock.image.xsize * m_pixBlock.image.ysize * m_pixBlock.image.csize, 
                           m_pixBlock.image.data );
				
			glTexParameteri(m_textureType, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE );
			
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
			
			glTexImage2D(m_textureType, 0,
                   GL_RGBA,
                   m_pixBlock.image.xsize,
                   m_pixBlock.image.ysize, 0,
                   m_pixBlock.image.format, //       GL_YCBCR_422_APPLE,
                   m_pixBlock.image.type,   //      GL_UNSIGNED_SHORT_8_8_REV_APPLE, //GL_UNSIGNED_SHORT_8_8_APPLE,
                   m_pixBlock.image.data);
			 
			logpost(NULL, 5, "new film");
			m_newFilm = 0; //just to be sure
    }	

    m_xRatio = m_xsize;
    m_yRatio = m_ysize;

    glTexSubImage2D(m_textureType, 0,
                    0, 0,			// position
                    m_xsize,			// the x size of the data
                    m_ysize,			// the y size of the data
                    GL_YCBCR_422_APPLE,               //  m_pixBlock.image.format,	// the format
                    GL_UNSIGNED_SHORT_8_8_REV_APPLE,  //  m_pixBlock.image.type,	// the type
                    m_pixBlock.image.data);           //   m_frame);		// the data + header offset
  }
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: postrender(GemState *state)
{
  state->texCoords   = m_oldTexCoords;
  state->numTexCoords= m_oldNumCoords;
  state->texture     = m_oldTexture;

  state->image = m_oldImage;


  m_pixBlock.newimage = 0;
  
  glActiveTexture(GL_TEXTURE0_ARB);

  glDisable(m_textureType);

  if (m_numFrames>0 && m_reqFrame>m_numFrames){
    m_reqFrame = m_numFrames;
    //outlet_bang(m_outEnd);
  }
}


/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: startRendering()
{
  glGenTextures(1, &m_textureObj);

  if ( ! m_rectangle )
    glBindTexture(GL_TEXTURE_2D, m_textureObj);
  else
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_textureObj);

  setUpTextureState();
    
  m_pixBlock.newimage = 1;
	m_newFilm = 1;
  m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;
	
	if (m_auto && m_haveMovie) SetMovieVolume(m_movie, (short)(m_volume * 255.f));
}

/////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: stopRendering()
{
  if (m_textureObj) glDeleteTextures(1, &m_textureObj);
  m_textureObj = 0;
  m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = 0;
  
  if (m_auto && m_haveMovie) SetMovieVolume(m_movie, kNoVolume);
}


void pix_movieDarwin :: MovRate(float rate)
{
  m_rate = rate;
  if (m_auto && m_haveMovie) {
		SetMovieRate(m_movie,X2Fix((double)m_rate));
  }
}

void pix_movieDarwin :: MovVolume(float volume)
{
  m_volume = volume;
  if (m_auto && m_haveMovie) {
    SetMovieVolume(m_movie,(short)(m_volume * 255.f));
  }
}

/////////////////////////////////////////////////////////
// changeImage
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: changeImage(int imgNum, int trackNum)
{
  if (imgNum < 0){
    // error("selection number must be > 0");
    imgNum=0;
  }
  if (trackNum < 0){
    error("track number must be > 0");
    trackNum=0;
  }

  if (m_haveMovie){
    if (imgNum > m_numFrames) {
      if (m_numFrames<0) m_reqFrame = imgNum;
      else m_reqFrame=m_numFrames;
      //      else error("frame %d exceeds max (%d)", imgNum, m_numFrames);
      //m_reqFrame = imgNum;
      return;
    } else m_reqFrame = imgNum;
  }
}

void pix_movieDarwin :: LoadRam()
{
  TimeValue	length;
  OSErr err;
  if (m_haveMovie){      
    m_movieTime = 0;
    length = GetMovieDuration(m_movie);
    err =LoadMovieIntoRam(m_movie,m_movieTime,length,keepInRam);
    if (err)
      {
        error("LoadMovieIntoRam failed miserably");
      }
  }else{
    error("no movie to load into RAM!");
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_movieDarwin :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_movieDarwin),
                   gensym("pix_movie"),
                   A_DEFSYM,A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::openMessCallback),
                  gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::changeImageCallback),
                  gensym("img_num"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::autoCallback),
                  gensym("auto"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::rateCallback),
                  gensym("rate"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::ramCallback),
                  gensym("ram"),  A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::hiqualityCallback),
                  gensym("hiquality"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::rectangleCallback),
                  gensym("rectangle"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieDarwin::volumeCallback),
                  gensym("volume"), A_DEFFLOAT, A_NULL);
}

void pix_movieDarwin :: openMessCallback(void *data, t_symbol *filename)
{
  GetMyClass(data)->openMess(filename,0);
}

void pix_movieDarwin :: changeImageCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
  //  GetMyClass(data)->changeImage(static_cast<int>imgNum);
  GetMyClass(data)->changeImage((argc<1)?0:atom_getint(argv), (argc<2)?0:atom_getint(argv+1));
}

void pix_movieDarwin :: autoCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_auto=!(!static_cast<int>(state));
}

void pix_movieDarwin :: rateCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->MovRate(state);
}

void pix_movieDarwin :: volumeCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->MovVolume(state);
}

void pix_movieDarwin :: ramCallback(void *data)
{
  GetMyClass(data)->LoadRam();
}

void pix_movieDarwin :: hiqualityCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_hiquality=static_cast<int>(state);
}

void pix_movieDarwin :: rectangleCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_rectangle=static_cast<int>(state);
}


#endif /* __APPLE__ */
