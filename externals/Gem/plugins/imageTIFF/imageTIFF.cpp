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

#ifdef HAVE_LIBTIFF
extern "C"
{
# include "tiffio.h"
}

#include <string.h>
#include "imageTIFF.h"
#include "plugins/PluginFactory.h"

#include "Gem/RTE.h"


using namespace gem::plugins;

REGISTER_IMAGEFACTORY("tiff", imageTIFF);


/////////////////////////////////////////////////////////
//
// imageTIFF
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

imageTIFF :: imageTIFF() 
{
  //post("imageTIFF");
}
imageTIFF :: ~imageTIFF()
{
  //post("~imageTIFF");
}

/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////
bool imageTIFF :: load(std::string filename, imageStruct&result, gem::Properties&props)
{
  ::logpost(NULL, 6, "reading '%s' with libTIFF", filename.c_str());
  TIFF *tif = TIFFOpen(filename.c_str(), "r");
  if (tif == NULL) {
      return(NULL);
    }

  uint32 width, height;
  short bits, samps;
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samps);
    
  int npixels = width * height;

  result.xsize=width;
  result.ysize=height;
  result.upsidedown=true;
  result.type=GL_UNSIGNED_BYTE; //?

  bool knownFormat = false;
  // Is it a gray8 image?
  if (bits == 8 && samps == 1)
    {
      result.setCsizeByFormat(GL_LUMINANCE);
      knownFormat = true;
    }
  // Is it an RGB image?
  else if (bits == 8 && samps == 3)
    {
      result.setCsizeByFormat(GL_RGBA);
      knownFormat = true;
    }
  // Is it an RGBA image?
  else if (bits == 8 && samps == 4)
    {
      result.setCsizeByFormat(GL_RGBA);
      knownFormat = true;
    }

  // can we handle the raw data?
  if (knownFormat) {
    unsigned char *buf = new unsigned char [TIFFScanlineSize(tif)];
    if (buf == NULL) {
      error("GemImageLoad(TIFF): can't allocate memory for scanline buffer: %s", filename.c_str());
      TIFFClose(tif);
      return(false);
    }
    
    result.reallocate();
    unsigned char *dstLine = result.data;
    int yStride = result.xsize * result.csize;
    for (uint32 row = 0; row < height; row++)
      {
	unsigned char *pixels = dstLine;
	if (TIFFReadScanline(tif, buf, row, 0) < 0) {
	  error("GemImageLoad(TIFF): bad image data read on line: %d: %s", row, filename.c_str());
	  TIFFClose(tif);
	  return false;
	}
	unsigned char *inp = buf;
	if (samps == 1) {
	  for (uint32 i = 0; i < width; i++) {
	    *pixels++ = *inp++;         // Gray8
	  }
	}
	else if (samps == 3)  {
	  for (uint32 i = 0; i < width; i++) {   
	    pixels[chRed]   = inp[0];   // Red
	    pixels[chGreen] = inp[1];   // Green
	    pixels[chBlue]  = inp[2];   // Blue
	    pixels[chAlpha] = 255;      // Alpha
	    pixels += 4;
	    inp += 3;
	  }
	} else {
	  for (uint32 i = 0; i < width; i++) {               
	    pixels[chRed]   = inp[0];   // Red
	    pixels[chGreen] = inp[1];   // Green
	    pixels[chBlue]  = inp[2];   // Blue
	    pixels[chAlpha] = inp[3];   // Alpha
	    pixels += 4;
	    inp += 4;
	  }
	}
	dstLine += yStride;
      }
    delete [] buf;
  }
  // nope, so use the automatic conversion
  else {
    char emsg[1024];
    TIFFRGBAImage img;
    if (TIFFRGBAImageBegin(&img, tif, 0, emsg) == 0) {
      //error("GemImageLoad(TIFF): Error reading in image file: %s : %s", filename, emsg);
      TIFFClose(tif);
      return(false);
    }

    uint32*raster = reinterpret_cast<uint32*>(_TIFFmalloc(npixels * sizeof(uint32)));
    if (raster == NULL) {
      error("GemImageLoad(TIFF): Unable to allocate memory for image: %s", filename.c_str());
      TIFFClose(tif);
      return(false);
    }

    if (TIFFRGBAImageGet(&img, raster, width, height) == 0) {
      //error("GemImageLoad(TIFF): Error getting image data in file: %s, %s", filename, emsg);
      _TIFFfree(raster);
      TIFFClose(tif);
      return(false);
    }

    TIFFRGBAImageEnd(&img);
    result.setCsizeByFormat(GL_RGBA);
    result.reallocate();

    unsigned char *dstLine = result.data;
    int yStride = result.xsize * result.csize;
    // transfer everything over
    int k = 0;
    for (uint32 i = 0; i < height; i++) {
      unsigned char *pixels = dstLine;
      for (uint32 j = 0; j < width; j++) {
	pixels[chRed]   = static_cast<unsigned char>(TIFFGetR(raster[k])); // Red
	pixels[chGreen] = static_cast<unsigned char>(TIFFGetG(raster[k])); // Green
	pixels[chBlue]  = static_cast<unsigned char>(TIFFGetB(raster[k])); // Blue
	pixels[chAlpha] = static_cast<unsigned char>(TIFFGetA(raster[k])); // Alpha
	k++;
	pixels += 4;
      }
      dstLine += yStride;
    }
    _TIFFfree(raster);
  }
   
  TIFFClose(tif);
  return true;
}
bool imageTIFF::save(const imageStruct&constimage, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  TIFF *tif = NULL;

  if(GL_YUV422_GEM==constimage.format) {
    error("don't know how to write YUV-images with libTIFF");
    return false;
  }

  tif=TIFFOpen(filename.c_str(), "w");
  if (tif == NULL) {
    return false;
  }
  imageStruct image; constimage.copy2Image(&image);
  image.fixUpDown();

  uint32 width=image.xsize, height = image.ysize;
  short bits=8, samps=image.csize;
  int npixels = width * height;
  //int planar_conf = PLANARCONFIG_CONTIG;
  const char *gemstring = "PD/GEM";

  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samps);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, 1);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

  TIFFSetField(tif, TIFFTAG_XRESOLUTION, 72);
  TIFFSetField(tif, TIFFTAG_YRESOLUTION, 72);
  TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

  TIFFSetField(tif, TIFFTAG_SOFTWARE, gemstring);

  int yStride = image.xsize * image.csize;
  unsigned char *srcLine = &(image.data[npixels * image.csize]);
  srcLine -= yStride;

  for (uint32 row = 0; row < height; row++) {
    unsigned char *buf = srcLine;
    if (TIFFWriteScanline(tif, buf, row, 0) < 0)
      {
        error("GEM: could not write line %d to image %s", row, filename.c_str());
        TIFFClose(tif);
        delete [] buf;
        return(false);
      }
    srcLine -= yStride;
  }
  TIFFClose(tif);

  return true;
}


float imageTIFF::estimateSave(const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  float result=0;
  if(mimetype == "image/tiff" || mimetype == "image/x-tiff")
    result += 100;

  // LATER check some properties....

  return result;
}

#endif
