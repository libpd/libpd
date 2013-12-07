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


#if defined __APPLE__ 
# if !defined __x86_64__
// with OSX10.6, apple has removed loads of Carbon functionality (in 64bit mode)
// LATER make this a real check in configure
#  define HAVE_CARBONQUICKTIME
# elif defined HAVE_QUICKTIME
#  undef HAVE_QUICKTIME
# endif
#endif

#ifdef HAVE_QUICKTIME
# ifdef HAVE_CARBONQUICKTIME
  //
# elif defined _WIN32
  //
# else
#  undef HAVE_QUICKTIME
# endif
#endif

#ifdef HAVE_QUICKTIME

# include "Gem/RTE.h"
# include "imageQT.h"
#include "plugins/PluginFactory.h"

# ifdef HAVE_CARBONQUICKTIME
#  include <Carbon/Carbon.h>
#  include <QuickTime/QuickTime.h>
#  include <QuickTime/ImageCompression.h>
#  define IMAGEQT_RGBA_PIXELFORMAT k32ARGBPixelFormat
# elif defined _WIN32
#  include <QTML.h>
#  include <Movies.h>
#  include <QuickTimeComponents.h>
#  include "Gem/Exception.h"
#  define OffsetRect MacOffsetRect
#  define IMAGEQT_RGBA_PIXELFORMAT k32RGBAPixelFormat
# endif

#ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif

#define QT_MAX_FILENAMELENGTH 256
#include <stdio.h>
# include <map>

//# include <string.h>
//# include <fcntl.h> 

using namespace gem::plugins;


REGISTER_IMAGEFACTORY("QT", imageQT);


#if defined __APPLE__
static OSStatus
FSPathMakeFSSpec(
                 const UInt8 *path,
                 FSSpec *spec)
{
  OSStatus   result;
  FSRef      ref;
  Boolean *isDirectory=NULL;
  
  /* check parameters */
  require_action(NULL != spec, BadParameter, result = paramErr);
   
  /* convert the POSIX path to an FSRef */
  result = FSPathMakeRef(path, &ref, isDirectory);
  require_noerr(result, FSPathMakeRef);
   
  /* and then convert the FSRef to an FSSpec */
  result = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, spec, NULL);
  require_noerr(result, FSGetCatalogInfo);
   
 FSGetCatalogInfo:
 FSPathMakeRef:
 BadParameter:

  return ( result );
}

#elif defined _WIN32
static OSStatus
FSPathMakeFSSpec(
		 const UInt8 *path,
		 FSSpec *spec)
{
    char filename[QT_MAX_FILENAMELENGTH];
    UInt8*filename8=reinterpret_cast<UInt8*>(filename);
    FILE*fil=NULL;
    
    snprintf(filename, QT_MAX_FILENAMELENGTH, "%s", path);
    c2pstr(filename);
    OSErr err=FSMakeFSSpec (0, 0L, filename8, spec);
    if (err != noErr && err != -37){
      error("GEM: recordQT: error %d in FSMakeFSSpec()", err);
    } else {
        err = noErr;
    }
    return err;
}
#endif

// MemAlloc
// Simple memory allocation wrapper
static void* MemAlloc(unsigned long memsize)
{
  if(memsize <= 0)
    return NULL;
  else
    return (malloc(memsize));
}

static unsigned char* CStringToPString(char *string)
{
  unsigned char *newString = (unsigned char*)MemAlloc(strlen(string) + 1);
  size_t i = 0;
    
  for(i = 0; i < strlen(string); i++)
    newString[i+1] = string[i];
    
  newString[0] = i;
    
  return newString;
}


static void InvertGLImage( unsigned char *imageData, unsigned char * outData, long imageSize, long rowBytes )
{
  long i, j;
  // This is not an optimized routine!

  // FIXXME use a flip function in GemPixUtils for this

  // Copy rows into tmp buffer one at a time, reversing their order
  for (i = 0, j = imageSize - rowBytes; i < imageSize; i += rowBytes, j -= rowBytes) {
    memcpy( &outData[j], &imageData[i], static_cast<size_t>(rowBytes) );
  }
}

static std::map<std::string, OSType> s_mime2type;

static bool mime2type(const std::string&mimetype, OSType&filetype) {
  static bool done=false;
  if(!done) {
    done=true;
    //s_mime2type["image/"]=kQTFileTypeAIFF;
    //s_mime2type["image/"]=kQTFileTypeAIFC;
    //s_mime2type["image/"]=kQTFileTypeDVC;
    //s_mime2type["image/"]=kQTFileTypeMIDI;
    s_mime2type["image/pict"]=kQTFileTypePicture;//-
    //s_mime2type["image/"]=kQTFileTypeMovie;
    //s_mime2type["image/"]=kQTFileTypeText;
    //s_mime2type["image/"]=kQTFileTypeWave;
    //s_mime2type["image/"]=kQTFileTypeSystemSevenSound;
    //s_mime2type["image/"]=kQTFileTypeMuLaw;
    //s_mime2type["image/"]=kQTFileTypeAVI;
    //s_mime2type["image/"]=kQTFileTypeSoundDesignerII;
    //s_mime2type["image/"]=kQTFileTypeAudioCDTrack;
    //s_mime2type["image/pict"]=kQTFileTypePICS;
    s_mime2type["image/gif"]=kQTFileTypeGIF;//-
    s_mime2type["image/png"]=kQTFileTypePNG;//-
    s_mime2type["image/tiff"]=kQTFileTypeTIFF;//-
    s_mime2type["image/psd"]=kQTFileTypePhotoShop;//-
    s_mime2type["image/sgi"]=kQTFileTypeSGIImage;//-
    s_mime2type["image/bmp"]=kQTFileTypeBMP;//-
    s_mime2type["image/jpeg"]=kQTFileTypeJPEG;//-
    //  s_mime2type["image/"]=kQTFileTypeJFIF;
    s_mime2type["image/mac"]=kQTFileTypeMacPaint;//-
    s_mime2type["image/targa"]=kQTFileTypeTargaImage;//-
    //  s_mime2type["image/"]=kQTFileTypeQuickDrawGXPicture;
    s_mime2type["image/x-quicktime"]=kQTFileTypeQuickTimeImage;//-
    //s_mime2type["image/"]=kQTFileType3DMF;
    //s_mime2type["image/"]=kQTFileTypeFLC;
    //s_mime2type["image/"]=kQTFileTypeFlash;
    //s_mime2type["image/"]=kQTFileTypeFlashPix;
    //s_mime2type["image/"]=kQTFileTypeMP4;
    //s_mime2type["image/"]=kQTFileTypePDF;
    //s_mime2type["image/"]=kQTFileType3GPP;
    //s_mime2type["image/"]=kQTFileTypeAMR;
    //s_mime2type["image/"]=kQTFileTypeSDV;
    //s_mime2type["image/"]=kQTFileType3GP2;
    //s_mime2type["image/"]=kQTFileTypeAMC;
    s_mime2type["image/jp2"]=kQTFileTypeJPEG2000;//-
  }

  std::map<std::string, OSType>::iterator it = s_mime2type.find(mimetype);
  if(s_mime2type.end() != it) {
    filetype=it->second;
    return true;
  }


  return false;
}


/////////////////////////////////////////////////////////
//
// imageQT
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

imageQT :: imageQT() : imageBase(false)
{
  static bool firsttime=true;
  if(firsttime) { 
#ifdef _WIN32
    // Initialize QuickTime Media Layer
    OSErr		err = noErr;
    if ((err = InitializeQTML(0))) {
      throw(GemException("unable to initialize QuickTime"));
    }	
    // start QuickTime
    if (err = EnterMovies()) {
      throw(GemException("unable to initialize QuickTime/Movies"));
    }
    logpost(NULL, 5, "imageQT: QT init done");
#endif // WINDOWS
    firsttime=false;
  }
  //post("imageQT");
}
imageQT :: ~imageQT()
{
  //post("~imageQT");
}

/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////

static bool QuickTimeImage2mem(GraphicsImportComponent inImporter, imageStruct&result)
{
  Rect      r;
  if (::GraphicsImportGetNaturalBounds(inImporter, &r)) return NULL;   //get an image size
  ::OffsetRect(&r, -r.left, -r.top);                           
  if (::GraphicsImportSetBoundsRect(inImporter, &r)) return NULL;      
  ImageDescriptionHandle imageDescH = NULL;
  if (::GraphicsImportGetImageDescription(inImporter, &imageDescH)) return NULL;
      
  result.xsize   = (*imageDescH)->width;
  result.ysize   = (*imageDescH)->height;
  result.upsidedown = true;

  OSType pixelformat = 0;

  /* afaik, there is no 8bit grayscale format....
   * and even if it was, k8GrayPixelFormat would not be a define...
   */
#ifdef k8GrayPixelFormat
  /* from the docs on "depth": what depth is this data (1-32) or ( 33-40 grayscale ) */
  if ((*imageDescH)->depth <= 32) {
    result.setCsizeByFormat(GL_RGBA_GEM);
    pixelformat = IMAGEQT_RGBA_PIXELFORMAT;
  } else {
    result.setCsizeByFormat(GL_LUMINANCE);
    pixelformat = k8GrayPixelFormat;
  }
#else
  result.setCsizeByFormat(GL_RGBA_GEM);
  pixelformat = IMAGEQT_RGBA_PIXELFORMAT;
#endif
  
  ::DisposeHandle(reinterpret_cast<Handle>(imageDescH));
  imageDescH = NULL;
  result.reallocate();

#ifdef __DEBUG__
  post("QuickTimeImage2mem() : allocate %d bytes", result.xsize*result.ysize*result.csize);
#endif
  if (result.data == NULL) {
    error("Can't allocate memory for an image.");
    return false;
  }

  GWorldPtr   gw = NULL;

  OSErr err = QTNewGWorldFromPtr(&gw,
                                 /* taken from pix_filmDarwin */
                                 pixelformat,   // gives noErr
                                 &r, NULL, NULL, 0,
                                 // keepLocal,   
                                 //useDistantHdwrMem, 
                                 result.data, 
                                 static_cast<long>(result.xsize * result.csize));
  if (err) {
    error("Can't create QTNewWorld");
  }

  ::GraphicsImportSetGWorld(inImporter, gw, NULL);
  ::GraphicsImportDraw(inImporter);
  ::DisposeGWorld(gw);         //dispose the offscreen
  gw = NULL;

  return true;
}

bool imageQT :: load(std::string filename, imageStruct&result, gem::Properties&props)
{
  OSErr            err;
  GraphicsImportComponent    importer = NULL;

  ::logpost(NULL, 6, "reading '%s' with QuickTime", filename.c_str());
  std::string myfilename=filename;
  // does the file even exist?
  if (!filename.empty()) {
    FSSpec   spec;
    err = ::FSPathMakeFSSpec( reinterpret_cast<const UInt8*>(myfilename.c_str()), &spec);
    if (err) {
      error("GemImageLoad: Unable to find file: %s", filename.c_str());
      error("parID : %d", spec.parID); 
      return false;
    }
    err = ::GetGraphicsImporterForFile(&spec, &importer);
    if (err) {
      error("GemImageLoad: Unable to import image '%s'", filename.c_str());
      return false;
    }
  }
  bool ret = QuickTimeImage2mem(importer, result);

  ::CloseComponent(importer);

  return ret;
}

static bool touch(std::string filename) {
#ifdef __APPLE__
  int fd;
  fd = open(filename.c_str(), O_CREAT | O_RDWR, 0600);
  if (fd < 0)
    return false;
  write(fd, " ", 1);
  close(fd);

  return true;
#elif defined _WIN32
  FILE*outfile=fopen(filename.c_str(), "");
  if(NULL==outfile)
      return false;
  fclose(outfile);
  return true;
#endif
    return false;
}

bool imageQT::save(const imageStruct&constimage, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  OSErr			err=noErr;
  ComponentResult		cErr 	= 0;
    
  GWorldPtr 			img	= NULL;
  GraphicsExportComponent 	geComp 	= NULL;
  Rect			r;
    
  FSSpec			spec;

  OSType			osFileType=kQTFileTypeTIFF;
  mime2type(mimetype, osFileType);
  std::string myfilename=filename.c_str();

  const UInt8*filename8=reinterpret_cast<const UInt8*>(myfilename.c_str());
    
#if defined __APPLE__
  FSRef			ref;
  err = ::FSPathMakeRef(filename8, &ref, NULL );

  if (err == fnfErr) {
    // if the file does not yet exist, then let's create the file
    touch(myfilename);
    err = FSPathMakeRef(filename8, &ref, NULL);
  }
    
  if (err != noErr) {
    logpost(NULL, 5, "ERROR: %d in FSPathMakeRef()", err);
  }

  err = ::FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags, NULL, NULL, &spec, NULL);

  if (err != noErr)  {
    logpost(NULL, 5, "ERROR: %d in FSGetCatalogInfo()", err);
  }

  err = FSMakeFSSpec(spec.vRefNum, spec.parID, filename8, &spec);  //this always gives an error -37 ???

#elif defined _WIN32
  touch(myfilename);
  err = FSMakeFSSpec (0, 0L, filename8, &spec);
#endif
  if (err != noErr && err != -37){
    logpost(NULL, 5, "GEM: imageQT: error %d in FSMakeFSSpec()", err);
  }

  err = OpenADefaultComponent(GraphicsExporterComponentType, osFileType, &geComp);
  if (err != noErr)  {
    error("ERROR: %d in OpenADefaultComponent()", err);
    return false; // FIXME:
  }
    
  r.top = 0;
  r.left = 0;
  r.bottom = constimage.ysize;
  r.right = constimage.xsize;

  imageStruct rgbaimg;
  rgbaimg.convertFrom(&constimage, GL_RGBA_GEM);

  unsigned char *data = NULL;
  if(!rgbaimg.upsidedown) { // the image is openGL-oriented, not quicktime-oriented! flip it!
    int rowBytes = rgbaimg.xsize * rgbaimg.csize;
    int imageSize = rgbaimg.ysize * rowBytes;

    data = new unsigned char[imageSize];

    InvertGLImage(rgbaimg.data, data, imageSize, rowBytes);
  }
    
  err = QTNewGWorldFromPtr(&img,  
                           IMAGEQT_RGBA_PIXELFORMAT,			   //k32RGBAPixelFormat,
                           &r, NULL, NULL, 0,
                           (data?data:rgbaimg.data),
                           static_cast<long>(rgbaimg.xsize * rgbaimg.csize));

  // is this the right place to free the "data" buffer (if used)?
  // i don't know, whether quicktime still needs the buffer...
                
  if (err != noErr) {
    error("ERROR: %d in QTNewGWorldFromPtr()", err);
    if(data)delete[]data;
    return false; // FIXME:
  }
       
  // Set the input GWorld for the exporter
  cErr = GraphicsExportSetInputGWorld(geComp, img);
  if (cErr != noErr)  {
    error("ERROR: %d in GraphicsExportSetInputGWorld()", cErr);
    if(data)delete[]data;
    return false; // FIXME:
  }
  
  // Set the output file to our FSSpec
  cErr = GraphicsExportSetOutputFile(geComp, &spec);
  if (cErr != noErr) {
    error("ERROR: %i in GraphicsExportSetOutputFile()", cErr);
    if(data)delete[]data;
    return false; // FIXME:
  }
        
  // Set the compression quality (needed for JPEG, not necessarily for other formats)
  /*
    codecMinQuality
    codecLowQuality
    codecNormalQuality
    codecHighQuality
    codecMaxQuality
    codecLosslessQuality
  */

  CodecQ quality=codecHighQuality;

  double d=0.;
  if(props.get("quality", d)) {
    // <0 = minqality
    // >=100 = lossless
    if(d<0.)d=0.;
    else if(d>100.)d=100.;

    CodecQ maxQ=codecLosslessQuality;
    double maxQ_d=(double)maxQ;
    double quality_d=maxQ_d * d / 100.; // 0..maxQ

    quality=(CodecQ)quality_d;
  }

  cErr = GraphicsExportSetCompressionQuality(geComp,  quality);

  // Export it
  cErr = GraphicsExportDoExport(geComp, NULL);
  if (cErr != noErr) {
    error("ERROR: %i in GraphicsExportDoExport()", cErr);
    if(data)delete[]data;
    return false; // FIXME:
  }
        
  // finally, close the component
  if (geComp != NULL)
    CloseComponent(geComp);

  if(data)delete[]data;
  return true;
}


float imageQT::estimateSave(const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  float result=0.;

  OSType			filetype; // just a dummy
  if(mime2type(mimetype, filetype))
    result+=100.;

  // LATER check some properties....
  if(gem::Properties::UNSET != props.type("quality"))
    result += 1.;

  return result;
}

#endif /* have_quicktime */
