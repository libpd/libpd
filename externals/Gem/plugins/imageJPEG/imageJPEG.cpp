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

#ifdef HAVE_LIBJPEG
#include <string.h>
#include "imageJPEG.h"
#include "plugins/PluginFactory.h"

#include "Gem/RTE.h"


extern "C"
{
# ifdef _WIN32
#  undef FAR
# endif
# undef EXTERN

# include "jpeglib.h"
#include <setjmp.h>

}

using namespace gem::plugins;

REGISTER_IMAGEFACTORY("jpeg", imageJPEG);


/////////////////////////////////////////////////////////
//
// imageJPEG
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

/***************************************************************************
 *
 * We have to do some funky error handling to keep the jpeg library
 *		from exiting on us.
 *
 ***************************************************************************/

/*****************************
 *
 * Here is the error handler
 *
 *****************************/
struct my_error_mgr
{
  struct jpeg_error_mgr pub;	// "public" fields
  jmp_buf setjmp_buffer;	// for return to caller
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */
METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  // cinfo->err really points to a my_error_mgr struct, so coerce pointer
  my_error_ptr myerr = reinterpret_cast<my_error_ptr> (cinfo->err);

  // Always display the message.
  // We could postpone this until after returning, if we chose.
  // (*cinfo->err->output_message) (cinfo);

  // Return control to the setjmp point
  longjmp(myerr->setjmp_buffer, 1);
}

imageJPEG :: imageJPEG() 
{
  //post("imageJPEG");
}
imageJPEG :: ~imageJPEG()
{
  //post("~imageJPEG");
}

/////////////////////////////////////////////////////////
// really open the file ! (OS dependent)
//
/////////////////////////////////////////////////////////
bool imageJPEG :: load(std::string filename, imageStruct&result, gem::Properties&props)
{
  // open up the file
  FILE * infile;
  ::logpost(NULL, 6, "reading '%s' with libJPEG", filename.c_str());
  if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
    //error("GemImageLoad(JPEG): Unable to open image file: %s", filename);
    return(false);
  }

  // create the jpeg structures
  jpeg_decompress_struct cinfo;
  my_error_mgr jerr;

  // We set up the normal JPEG error routines, then override error_exit
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  // Establish the setjmp return context for my_error_exit to use.
  if ( setjmp(jerr.setjmp_buffer) ) {
    // If we get here, the JPEG code has signaled an error.
    // We need to clean up the JPEG object, close the input file, and return.
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return(false);
  }

  // create the decompression structure
  jpeg_create_decompress(&cinfo);

  // associate the decompress struct with the file
  jpeg_stdio_src(&cinfo, infile);

  // read in the file info
  jpeg_read_header(&cinfo, TRUE);


  // do we have an RGB image?
  if (cinfo.jpeg_color_space == JCS_RGB) {
    result.setCsizeByFormat(GL_RGBA);
  } else if (cinfo.jpeg_color_space == JCS_GRAYSCALE) {
    // do we have a gray8 image?
    result.setCsizeByFormat(GL_LUMINANCE);
  } else {
    // something else, so decompress as RGB
    result.setCsizeByFormat(GL_RGBA);
    cinfo.out_color_space = JCS_RGB;
  }

  // start the decompression
  jpeg_start_decompress(&cinfo);
  int xSize = cinfo.output_width;
  int ySize = cinfo.output_height;
  int cSize = result.csize;
  result.upsidedown=true;
  result.xsize = xSize;
  result.ysize = ySize;
  result.reallocate();
   
  // cycle through the scan lines
  unsigned char *srcLine = new unsigned char[xSize * cSize];
  unsigned char *dstLine = result.data;
  int yStride = xSize * cSize;
  int lines = ySize;
  int pixes = xSize;

  // do RGBA/RGB data
  if (cSize == 4) {
    while (lines--) {
      unsigned char *src = srcLine;
      unsigned char *dst = dstLine;
      jpeg_read_scanlines(&cinfo, &src, 1);
      pixes = xSize;
      while (pixes--) {
	dst[chRed]   = src[0];
	dst[chGreen] = src[1];
	dst[chBlue]  = src[2];
	dst[chAlpha] = 255;
	dst += 4;
	src += 3;
      }
      dstLine += yStride;
    }
  } else {
    // do grayscale data
    while (lines--) {
      unsigned char *src = srcLine;
      unsigned char *dst = dstLine;
      jpeg_read_scanlines(&cinfo, &src, 1);
      pixes = xSize;
      while (pixes--) {
	*dst++ = *src++;
      }
      dstLine += yStride;
    }
  }

  // finish the decompression
  jpeg_finish_decompress(&cinfo);
   
  // cleanup
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
  delete [] srcLine;

  return true;
}
bool imageJPEG::save(const imageStruct&constimage, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer;	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  double fquality=100;
  props.get("quality", fquality);
  int quality=fquality;

  if(GL_YUV422_GEM==constimage.format) {
    error("don't know how to write YUV-images with libJPEG");
    return false;
  }

  cinfo.err = jpeg_std_error(&jerr);

  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
    error("can't open %s\n", filename.c_str());
    return (false);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  imageStruct image; 
  constimage.convertTo(&image, GL_RGB);
  //  image.fixUpDown();
  JSAMPLE *image_buffer = image.data;

  cinfo.image_width = image.xsize; 	/* image width and height, in pixels */
  cinfo.image_height = image.ysize;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&cinfo, TRUE);

  row_stride = image.xsize * image.csize;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    int rowindex=cinfo.next_scanline;
    if(!image.upsidedown)
      rowindex=(cinfo.image_height-cinfo.next_scanline-1);
    row_pointer = & image_buffer[rowindex * row_stride];

    if(jpeg_write_scanlines(&cinfo, &row_pointer, 1) < 0){
      error("GEM: could not write line %d to image %s", cinfo.next_scanline, filename.c_str());
      jpeg_finish_compress(&cinfo);
      fclose(outfile);
      jpeg_destroy_compress(&cinfo);
      return(false);
    }
  }

  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);

  return true;
}

float imageJPEG::estimateSave(const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
  float result=0.;
  if(mimetype == "image/jpeg")// || mimetype == "image/pjpeg")
    result += 100.;

  // LATER check some properties....
  if(gem::Properties::UNSET != props.type("quality"))
    result += 1.;

  return result;
}


#endif
