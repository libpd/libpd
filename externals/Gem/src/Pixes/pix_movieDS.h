#ifndef _INCLUDE__GEM_PIXES_PIX_MOVIEDS_H_
#define _INCLUDE__GEM_PIXES_PIX_MOVIEDS_H_

#ifndef HAVE_DIRECTSHOW
# error pix_movieDS without DirectShow
#endif

#pragma warning (disable : 4251)

#include <dshow.h>
#include <qedit.h>

 
#include "Base/GemBase.h"
#include "Gem/Image.h"

class GEM_EXTERN pix_movieDS : public GemBase
{

	CPPEXTERN_HEADER(pix_movieDS, GemBase);

   
  public:  
	//////////
	// Constructor
	pix_movieDS(t_symbol *filename);
	~pix_movieDS(void);
	
  protected:
	
	//////////
	// Do the rendering
	virtual void texFrame(GemState *state, int doit);
    	
	//////////
	virtual void setUpTextureState();


	//////////
	// create and delete buffers
	virtual void createBuffer();

	virtual void deleteBuffer();

	//////////
	// prepare for texturing (on open)
	virtual void prepareTexture();

	//////////
	// Clear the dirty flag on the pixBlock
	virtual void postrender(GemState *state);
  
	virtual void render(GemState *state);

	//////////
	// open a movie up
	virtual void openMess(t_symbol *filename, int format);
	virtual void realOpen(char *filename);
	virtual void closeMess();

	//////////
	// Do the rendering
	virtual void getFrame();

	virtual void MovRate(float rate);
  
	virtual void changeImage(int imgNum, int trackNum);
  
	//////////
	// load film into RAM
	//virtual void LoadRam();


	//-----------------------------------
	// GROUP:	Texture data
	//-----------------------------------

	//////////
	// The texture coordinates
	TexCoord    m_coords[4];
	//////////
	// The size of the texture (so we can use sub image)
	int			m_dataSize[3];

	GLuint		m_textureObj;	
	float		m_xRatio;
	float		m_yRatio;
  
	//////////
	// the current file
	t_symbol	*x_filename;
	
	//////////
	// a outlet for information like #frames and "reached end"
	t_outlet	*m_outNumFrames;
	t_outlet	*m_outEnd;
        
	//////////
	// frame data
	unsigned char	*m_frame;  /* this points to the main texture (might be black) */
	unsigned char	*m_data;   /* this points always to the real data */
  
  
	//////////
	// If a movie was loaded and what kind of Movie this is
	int				m_haveMovie;

	int				m_auto;

	//////////
	// frame infromation
	int				m_numFrames;
	int				m_reqFrame;
	int				m_curFrame;

	//////////
	// track information
	int				m_numTracks;
	int				m_track;

  
	pixBlock		m_pixBlock;
	imageStruct		m_imageStruct;
  

	int				m_xsize;
	int				m_ysize;
	int				m_csize;


	bool			m_film; // are we in film- or in movie-mode
	int				m_newFilm;
	int				newImage;
	int				m_colorspace;
	int				m_format;
	int				m_rectangle;
  
	//-----------------------------------
	// GROUP:	Movie data
	//-----------------------------------
   
//	Rect			m_srcRect;
//	TimeValue		m_movieTime;
//	Track			m_movieTrack;
//	TimeValue		m_timeScale;
//	TimeValue		duration;
	float			durationf;
	long			movieDur, movieScale;
	int				m_play;
	float			m_rate;
//	Fixed			playRate;
//	TimeValue 		prevTime;
//	TimeValue		curTime;

  private:
	IBaseFilter				*VideoFilter;		// Base Filter for video
	IBaseFilter				*SampleFilter;		// Sample filter
	IBaseFilter				*NullFilter;		// Null render base Filter for video
	ISampleGrabber			*SampleGrabber;		// Sample grabber
	IGraphBuilder			*FilterGraph;		// Filter Graph for movie playback
	ISampleGrabber			*VideoGrabber;		// Video grabber
	IMediaControl			*MediaControl;		// MediaControl interface
	IMediaSeeking			*MediaSeeking;		// MediaSeeking interface
	IMediaPosition			*MediaPosition;		// MediaPosition interface
	LONGLONG				m_Duration;			// Duration of video
	LONGLONG				m_LastFrame;		// Last frame
	
 protected:	
	//////////
	// static member functions
	static void openMessCallback(void *data, t_symbol *filename);
	static void changeImageCallback(void *data, t_symbol *, int argc, t_atom *argv);
	static void autoCallback(void *data, t_floatarg state);
	static void rateCallback(void *data, t_floatarg state);
	static void rectangleCallback(void *data, t_floatarg state);
};

HRESULT movieGetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin);
HRESULT movieConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond);

#endif
