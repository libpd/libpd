/*
 *  pix_recordQT.h
 *  GEM_darwin
 *
 *  Created by chris clepper on 7/18/05.
 *  Copyright 2005. All rights reserved.
 *
 */
 
 //this will record QT movies
#ifndef _INCLUDE__GEM_PIXES_PIX_RECORDQT_H_
#define _INCLUDE__GEM_PIXES_PIX_RECORDQT_H_

#if 1
#include "Base/GemBase.h"
#include "Gem/Image.h"

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#include <QTML.h>
#include <Movies.h>
#include <QuicktimeComponents.h>
#include <Files.h>
#endif

#ifdef __APPLE__
#include <Quicktime/QuickTime.h>
#include <Carbon/Carbon.h>

#include <unistd.h> //needed for Unix file open() type functions
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 

#endif


/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_recordQT
    
    Writes a pix of the render buffer
    
KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vert_size"
    Inlet for a list - "vert_pos"

    "file" - filename to write to
    "bang" - do write now
    "auto 0/1" - stop/start writing automatically

    "vert_size" - Set the size of the pix
    "vert_pos" - Set the position of the pix
    
-----------------------------------------------------------------*/
class GEM_EXTERN pix_recordQT : public GemBase
{
    CPPEXTERN_HEADER(pix_recordQT, GemBase);

    public:

        //////////
        // Constructor
    	pix_recordQT(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_recordQT();
		
		virtual void	setupQT();
		
		virtual void	compressFrame();
		
		//virtual void	startRecording();
		
		virtual void	stopRecording();
    	
    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Clear the dirty flag on the pixBlock
    	virtual void 	postrender(GemState *state) {};

    	//////////
    	// Set the filename and filetype
    	virtual void	fileMess(int argc, t_atom *argv);
 
    	//////////
    	// When a size message is received
    	virtual void	sizeMess(int width, int height);
    	
    	//////////
    	// When a position message is received
    	virtual void	posMess(int x, int y);
		
		/////////
		// call up compression dialog
		virtual void	dialogMess();
		
		virtual void	getCodecList();
		
		virtual void	codecMess(int argc, t_atom *argv);
		
		virtual void	csMess(int format);
		
		
    	
    	//////////
    	// Clean up the image
   // 	void	    	cleanImage();
    	
    	//////////
    	// The original pix_recordQT
    //	imageStruct 	*m_originalImage;


	//////////
	// Manual writing
	bool            m_banged;
    	
	//////////
	// Automatic writing
	bool            m_automatic;

	//////////
	// Counter for automatic writing
	int             m_autocount;
	
	/////////
	// recording start
	bool			m_recordStart;
	
	/////////
	// recording start
	bool			m_recordStop;
	
	//////
	// is recording setup and ready to go?
	bool			m_recordSetup;
	
	bool			m_dialog;
    	
    	//////////
	// path to write to
    	char	    	m_pathname[80];
    	//////////
	// current file to write to
    	char	    	m_filename[80];

    	//////////
	// current file to write to
    	int	    	m_filetype; // 0=tiff, [1..6=jpeg]

    	//////////
    	// The x position
    	int     	m_xoff;
    	
    	//////////
    	// The y position
    	int     	m_yoff;
    	
    	//////////
    	// The width
    	int     	m_width;
    	
    	//////////
    	// The height
    	int     	m_height;
		
		//////////
		// previous dimensions to check
		int			m_prevHeight;
		
		int			m_prevWidth;
		
		pixBlock	*m_pixBlock;
		
		imageStruct	m_compressImage;
		
		//////////
		// a outlet for information like #frames
		t_outlet     *m_outNumFrames;
		
		int				m_currentFrame; //keep track of the number of frames
		
		///////////
		/// QT stuff
		
		GWorldPtr				m_srcGWorld;
		Rect					m_srcRect;
		int						m_rowBytes;
		Movie					m_movie;
		Track					track;
		Media					media;
		ComponentInstance		stdComponent;
		SCTemporalSettings		TemporalSettings;
		SCSpatialSettings		SpatialSettings;
		SCDataRateSettings		DataRateSetting;
		SCDataRateSettings		datarate;
		long					dataSize;
    	ImageDescriptionHandle	hImageDesc;
		
		//these are for the programmatic setting of the compressor
		CodecType				m_codecType;
		CodecComponent			m_codec;
		short					m_depth;
		CodecQ					m_spatialQuality;
		//set these to reflect if the codec settings are good or not
		bool				m_codecSet;
		bool				m_codecQualitySet;

		
		short		nFileRefNum;
		short		nResID;
		
		//this will hold the ctype value of the codecs listed by getCodecList()
		typedef struct codecListStorage{
			int		position;
			int		ctype;
			CodecComponent		codec;
		};

		codecListStorage	codecContainer[64];//anyone with more than 64 codecs can change this
		
		
				
		int					m_fps;
		
		//duration of frames in ms
		int					m_frameDuration;
		
		//number of QT ticks for a frame 600/frameDuration (used by AddMediaSample)
		int					m_ticks;
		
		bool	m_firstRun;
#ifdef __APPLE__
		UnsignedWide startTime, endTime;
#endif
#ifdef _WIN32
		LARGE_INTEGER freq, startTime, endTime;
#endif
		float seconds;
		
		Component			aComponent;
		ComponentInstance	aClock;
		TimeRecord			aTime;
	
		int		m_colorspace;
		
		
    private:
    	
    	//////////
    	// static member functions
    	static void 	fileMessCallback(void *data, t_symbol *s, int argc, t_atom *argv);
    	static void 	autoMessCallback(void *data, t_floatarg on);
    	static void 	bangMessCallback(void *data);
    	static void 	sizeMessCallback(void *data, t_floatarg width, t_floatarg height );
    	static void 	posMessCallback(void *data, t_floatarg x, t_floatarg y);
		static void 	recordMessCallback(void *data, t_floatarg on);
		static void 	dialogMessCallback(void *data);
		static void 	codeclistMessCallback(void *data);
		static void 	codecMessCallback(void *data, t_symbol *s, int argc, t_atom *argv);
		static void		colorspaceCallback(void *data, t_symbol *state);
};
#endif //for __APPLE__
#endif	// for header file
