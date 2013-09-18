/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Detect markers with 3D poses and positions using ARToolKit

  Copyright (c) 2004 Shigeyuki Hirai. shigeyuki@pop01.odn.ne.jp
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_ARTOOLKIT_H_
#define INCLUDE_PIX_ARTOOLKIT_H_

/* config stuff */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined HAVE_AR_AR_H
//# warning AR/ar.h found
#endif

#if defined HAVE_LIBAR
//# warning libAR found
#endif


#if defined HAVE_AR_AR_H && defined HAVE_LIBAR
# define HAVE_ARTOOLKIT
#endif



#include "Base/GemPixObj.h"

#ifdef HAVE_ARTOOLKIT
# include "AR/param.h"
# include "AR/ar.h"
# include "AR/arMulti.h"

# define FILENAME_LENGTH 512
# define MAX_OBJECTS		16

typedef struct {
  t_symbol *patt_name;
  int     patt_id;			//
  int     model_id;			//
  int     visible;			//
  bool	contFlag;			// continuous start flag
  double  width;			//
  double  center[2];			//
  double  trans[3][4];		        //
} OBJECT_T;
#endif /* ARTOOLKIT */

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_artoolkit
    
  Detect markers and its 3d positions using ARToolKit

  KEYWORDS
  pix
    
  DESCRIPTION

  "loadmarker" - load marker data file
  "loadcpara" - load camera parameter file
  "continuous" - continuous mode
  "reset" - reset imported markers and camera parameters
   
  -----------------------------------------------------------------*/
class GEM_EXPORT pix_artoolkit : public GemPixObj
{
  CPPEXTERN_HEADER(pix_artoolkit, GemPixObj)

    public:

  //////////
  // Constructor
  pix_artoolkit();

  enum outputMode {
    OUTPUT_NORMAL = 0,
    OUTPUT_QUATERNION = 1,
    OUTPUT_EULER = 2
  };
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_artoolkit();

#ifdef HAVE_ARTOOLKIT	
  //////////
  // Do the processing
  virtual void 	processRGBAImage(imageStruct &image);
  virtual void  processYUVImage(imageStruct &image);
  virtual void 	processGrayImage(imageStruct &image);

  //////////
  // Load files
  void			loadmarkerMess(t_int n, t_symbol *filename);
  void			objectSizeMess(t_int n, t_floatarg f);
  void			loadcparaMess(t_symbol *filename);
  void			outputmodeMess(t_int outputmode);

  //////////
  // 
  void			continuousMess(t_int continuousmode);
  void			thresholdMess(t_int threshold);

  //////////
  // The clear routine
  void			clearMess(void);

  //////////
  // Initialize 
  void			init(void);

  //////////
  // resetMess
  void			resetMess(void);
#endif /* HAVE_ARTOOLKIT */

  t_outlet	*m_outMarker;

#ifdef HAVE_ARTOOLKIT	
  imageStruct	m_image;			// 

  ARParam	wparam;				//
  int		m_xsize;			// image size (x)
  int		m_ysize;			// image size (y)
  int		m_thresh;			// threshold
  int		m_count;			// 
  outputMode	m_outputMode;		// output mode (OUTPUT_QUARTANION, OUTPUT_CARTESIAN)
  bool		m_continuous;		// continuous mode (true:continuous, false:one shot)
  t_symbol	*m_cparam_name;		// camera parameter file name
  ARParam 	m_cparam;			// camera parameter
  OBJECT_T	m_object[MAX_OBJECTS];		//
  //		char                *config_name;	//"Data/multi/marker.dat"
  //		ARMultiMarkerInfoT  *config;		//
#endif /* HAVE_ARTOOLKIT */
    
  //////////
  // Static member functions
  static void 	loadcparaMessCallback(void *data, t_symbol *filename);
# ifdef GEM4MAX
  static void 	loadmarkerMessCallback(void *data, t_int n, t_symbol *filename);
  static void	objectSizeMessCallback(void *data, t_int n, t_floatarg f);
  static void	outputmodeMessCallback(void *data, t_int outputmode);
  static void	continuousMessCallback(void *data, t_int continuousmode);
  static void	thresholdMessCallback(void *data, t_int threshold);
# else
  static void 	loadmarkerMessCallback(void *data, t_symbol*,int,t_atom*);
  static void	objectSizeMessCallback(void *data, t_float n, t_floatarg f);
  static void	outputmodeMessCallback(void *data, t_floatarg outputmode);
  static void	continuousMessCallback(void *data, t_floatarg continuousmode);
  static void	thresholdMessCallback(void *data, t_floatarg threshold);
# endif /* GEM4MAX */
  static void	resetMessCallback(void *data);
  static void 	clearMessCallback(void *data);
};

#endif	// for header file
