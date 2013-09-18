/*
 *  pix_videoDarwin.h
 *  gem_darwin
 *
 *  Created by James Tittle & Chris Clepper on Thu Aug 1 2002.
    Copyright (c) 2002 James Tittle & Chris Clepper
 *
 */

 
#ifndef _INCLUDE__GEM_PIXES_PIX_VIDEODARWIN_H_
#define _INCLUDE__GEM_PIXES_PIX_VIDEODARWIN_H_

#include "Gem/GemGL.h"

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include "pix_videoOS.h"
 
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	pix_video
    
    Loads in a video
    
KEYWORDS
    pix
    
DESCRIPTION

    "dimen" (int, int) - set the x,y dimensions
    "zoom" (int, int) - the zoom factor (1.0 is nominal) (num / denom)
    "bright" (int) - the brightness
    "contrast" (int) - the contrast
    "hue" (int) - the hue
    "sat" (int) - the saturation
    
-----------------------------------------------------------------*/
class GEM_EXTERN pix_videoDarwin : public pix_videoOS
{
    CPPEXTERN_HEADER(pix_videoDarwin, GemBase);

    public:

        //////////
        // Constructor
    	pix_videoDarwin(t_floatarg w, t_floatarg h );

    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_videoDarwin();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Clear the dirty flag on the pixBlock
    	virtual void 	postrender(GemState *state);

    	//////////
    	virtual void	startRendering();
		
		//////////
    	virtual void	stopRendering();
        
          //////////
          // Start up the video device
          // [out] int - returns 0 if bad
        virtual int	startTransfer();

        //////////
        // Stop the video device
        // [out] int - returns 0 if bad
        virtual int	stopTransfer();

        virtual void csMess(int format);
        virtual void csMess(t_symbol*format);



		virtual void qualityMess(int X);

        //////////
        // property-dialog
        virtual void	dialogMess(int,t_atom*);
		
		virtual void	derSwizzler(imageStruct &image);
		
		virtual void	setupCapture();
		
		virtual void	fileMess(int argc, t_atom *argv);
  
	//-----------------------------------
	// GROUP:	Macintosh specific video data
	//-----------------------------------
        void			tick();
        void			panelMess();
        void			makeOffscreen();
        void			makeVideoChannel();
        void			setupVideoChannel();
        void			disposeOffscreen();
        void			disposeVideoChannel();
        //OSErr	videoFrame(SGChannel c, short bufferNum, Boolean *done);
        
        void InitSeqGrabber();
        void resetSeqGrabber();
        void destroySeqGrabber();
        void DoVideoSettings();
        void dimenMess(int x, int y, int leftmargin, int rightmargin,
    	    	    	    int topmargin, int bottommargin);
							
		//void	qualityMess(int X);

        //-----------------------------------
        // GROUP:	Video data
        //-----------------------------------
    
    	int		m_newFrame; 
		bool	m_banged;
		bool	m_auto;
		char	m_filename[80];
		int		m_record;
        
        SeqGrabComponent	m_sg;		// Sequence Grabber Component
        SGChannel			m_vc;			// Video Channel
		SGOutput			m_sgout; //output for writing to disk
		Movie				m_movie;
        short				m_pixelDepth;	//
        int					m_vidXSize;		//
        int					m_vidYSize;		//
        Rect				m_srcRect;		// Capture Rectangle
        GWorldPtr			m_srcGWorld;	// Capture Destination
        PixMapHandle		m_pixMap;	// PixMap Handle for capture image
        Ptr					m_baseAddr;		// Base address of pixel Data
        long				m_rowBytes;		// Row bytes in a row
        int					m_quality;
        int					m_colorspace;
		
		int					m_inputDevice;
		int					m_inputDeviceChannel;
		VideoDigitizerComponent			m_vdig; //gotta have the vdig
		VideoDigitizerError	vdigErr;
		DigitizerInfo       m_vdigInfo; //the info about the VDIG
		
		FSSpec		theFSSpec;
		short		nFileRefNum;
		short		nResID;
		
		//functions and variables for controlling the vdig
		unsigned short		m_brightness;
		unsigned short		m_contrast;
		unsigned short		m_saturation;
		
		virtual void		brightnessMess(float X);
		virtual void		saturationMess(float X);
		virtual void		contrastMess(float X);
		
		//IIDC functions
		virtual void		exposureMess(float X);
		virtual void		gainMess(float X);
		virtual void		whiteBalanceMess(float U,float V);
		
    private:
    	
    	//////////
    	// static member functions
        static void qualityCallback(void *data, t_floatarg X);
        static void resetCallback(void *data);
        static void colorspaceCallback(void *data, t_symbol *cs);
        static void csMessCallback(void *data, t_symbol *cs);
		static void deviceCallback(void *data, t_floatarg X);
		static void brightnessCallback(void *data, t_floatarg X);
		static void saturationCallback(void *data, t_floatarg X);
		static void contrastCallback(void *data, t_floatarg X);
		static void exposureCallback(void *data, t_floatarg X);
		static void gainCallback(void *data, t_floatarg X);
		static void whiteBalanceCallback(void *data, t_floatarg U,t_floatarg V);
		static void bangMessCallback(void *data);
		static void autoCallback(void *data, t_floatarg X);
		static void fileMessCallback(void *data, t_symbol *s, int argc, t_atom *argv);
		static void recordCallback(void *data, t_floatarg X);
		static void inputCallback(void *data, t_floatarg X);
};

#endif
