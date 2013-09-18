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
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "Gem/GemConfig.h"

// I hate Microsoft...I shouldn't have to do this!
#ifdef _WIN32
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
# pragma warning( disable : 4091 )
#endif

#include "m_pd.h"
#include "Image.h"
#include "PixConvert.h"

// utility functions from PeteHelpers.h
//#include "Utils/PixPete.h"


#include <string.h>
#include <ctype.h>

#include<new> 

/* this is some magic for debugging:
 * to time execution of a code-block use 
 *   'START_TIMING;' at the beginning of the block and
 *   'STOP_TIMING("something");' at the end of the block
 */
#ifdef __TIMING__
# ifdef __linux__
#  include <sys/time.h>

#  define START_TIMING float mseconds=0.f;	\
  timeval startTime, endTime;			\
  gettimeofday(&startTime, 0)
#  define STOP_TIMING(x) gettimeofday(&endTime, 0);	\
  mseconds = (endTime.tv_sec - startTime.tv_sec)*1000 +	\
    (endTime.tv_usec - startTime.tv_usec) * 0.001;	\
  post("%s frame time = %f ms", x, mseconds)
# elif defined __APPLE__
#  define START_TIMING float mseconds=0.f;	\
  UnsignedWide start, end;			\
  Microseconds(&start)
#  define STOP_TIMING(x) Microseconds(&end);			\
  mseconds = static_cast<float>((end.lo - start.lo) / 1000.f);	\
  post("%s frame time = %f ms", x, mseconds)
# else
#  define START_TIMING
#  define STOP_TIMING(x)
# endif /* timing for OS */
#else /* !__TIMING__ */
# define START_TIMING
# define STOP_TIMING(x)
#endif /* __TIMING__ */

#ifdef __VEC__
static int m_simd=3;
#else
static int m_simd=GemSIMD::getCPU();
#endif

pixBlock :: pixBlock()
  : image(imageStruct()), newimage(0), newfilm(0)
{}


imageStruct :: imageStruct() 
  : xsize (0),ysize(0),csize(0),
#ifdef __APPLE__
    // or should type be GL_UNSIGNED_INT_8_8_8_8_REV ? i don't know: jmz
# ifdef __BIG_ENDIAN__
    type(GL_UNSIGNED_SHORT_8_8_REV_APPLE),
# else 
    type(GL_UNSIGNED_SHORT_8_8_APPLE),
# endif /* __BIG_ENDIAN__ */
    format(GL_YCBCR_422_GEM),
#else /* !__APPLE__ */
    type(GL_UNSIGNED_BYTE), format(GL_RGBA),
#endif /* __APPLE__ */
    notowned(0),data(NULL),pdata(NULL),datasize(0),
#ifdef __APPLE__
    upsidedown(1)
#else /* !__APPLE__ */
  upsidedown(0)
#endif /* __APPLE__ */
{}

imageStruct :: imageStruct(const imageStruct&org) 
  : xsize (0),ysize(0),csize(0),
    type(GL_UNSIGNED_BYTE), format(GL_RGBA),
    notowned(0),data(NULL),pdata(NULL),datasize(0),
    upsidedown(0)
{
  org.copy2Image(this);
}

imageStruct :: ~imageStruct()
{
  clear();
}

/* align the memory to 128bit (GEM_VECTORALIGNMENT is in Utils/SIMD.h)
 * this code is taken from pd-devel (written by t.grill)
 * there used to be something in here written by g.geiger
 */
GEM_EXTERN unsigned char* imageStruct::allocate(size_t size) 
{
  if (pdata){
    delete [] pdata;
    pdata=NULL;
  }

#ifdef __APPLE__ 
  try {
    data = pdata =  new unsigned char [size];
  } catch ( const std::bad_alloc & e) {
    error("out of memory!");
    data=pdata=NULL;
    datasize=0;
    return NULL;
  }

  datasize=size;  
#else
  size_t array_size= size+(GEM_VECTORALIGNMENT/8-1);
  try {
    pdata = new unsigned char[array_size];
  } catch ( const std::bad_alloc & e) {
    error("out of memory!");
    data=pdata=NULL;
    datasize=0;
    return NULL;
  }

  size_t alignment = (reinterpret_cast<size_t>(pdata))&(GEM_VECTORALIGNMENT/8-1);
  size_t offset    = (alignment == 0?0:(GEM_VECTORALIGNMENT/8-alignment));
  data = pdata+offset;
  datasize=array_size-offset;
#endif
  notowned=false;
  //post("created data [%d] @ %x: [%d]@%x", array_size, pdata, datasize, data);
  return data; 
}

GEM_EXTERN unsigned char* imageStruct::allocate() 
{
  return allocate(xsize*ysize*csize);  
}

GEM_EXTERN unsigned char* imageStruct::reallocate(size_t size)
{
  if (size>datasize){
    return allocate(size);
  }
  size_t alignment = (reinterpret_cast<size_t>(pdata))&(GEM_VECTORALIGNMENT/8-1);
  size_t offset    = (alignment == 0?0:(GEM_VECTORALIGNMENT/8-alignment));
  notowned=false;
  data=pdata+offset;
  return data;
}
GEM_EXTERN unsigned char* imageStruct::reallocate() 
{  
  return reallocate(xsize*ysize*csize);  
}
 
GEM_EXTERN void imageStruct::clear() 
{
  if (pdata) { // pdata is always owned by imageStruct
    delete [] pdata;
  }
  data = pdata = NULL;      
  datasize=0;
}


GEM_EXTERN void imageStruct::copy2ImageStruct(imageStruct *to) const
{
  if (!to || !this || !this->data) {
    error("GEM: Someone sent a bogus pointer to copy2ImageStruct");
    if (to) to->data = NULL;
    return;
  }

  // copy the information over
  to->xsize 	= xsize;
  to->ysize 	= ysize;
  to->csize 	= csize;
  to->format 	= format;
  to->type 	= type;
  to->data    = data;
  /* from SIMD-branch: datasize refers to the private pdata
   * thus we shouldn't set it to something else, in order to not break
   * reallocate() and friends...
   */
  //to->datasize= datasize;
  to->upsidedown=upsidedown;
  to->notowned= true; /* but pdata is always owned by us */
}
GEM_EXTERN void imageStruct::info() {
  post("imageStruct\t:%dx%dx%d\n\t\t%X\t(%x) %d\n\t\t%x\t%x\t%d",
       xsize, ysize, csize,
       data, pdata, datasize,
       format, type, notowned);
}

GEM_EXTERN void imageStruct::copy2Image(imageStruct *to) const
{
  if (!to || !this || !this->data)
    {
      error("GEM: Someone sent a bogus pointer to copy2Image");
      if (to)
	to->data = NULL;
      return;
    }

  /* copy without new allocation if possible (speedup in convolve ..) */
  to->xsize 	= xsize;
  to->ysize 	= ysize;
  to->csize 	= csize;
  to->format 	= format;
  to->type 	= type;
  to->reallocate();
  to->upsidedown 	= upsidedown;

  memcpy(to->data, data, xsize*ysize*csize);
}

GEM_EXTERN void imageStruct::refreshImage(imageStruct *to) const {
  if (!to || !data)
    {
      error("GEM: Someone sent a bogus pointer to refreshImage");
      return;
    }

  // check if we need to reallocate memory
  if (to->xsize != xsize ||
      to->ysize != ysize ||
      to->csize != csize ||
      !to->data)
    {
      to->clear();
      copy2Image(to);
    }
  else
    // copy the data over
    memcpy(to->data, this->data, to->xsize * to->ysize * to->csize);
}

imageStruct&imageStruct::operator=(const imageStruct&org) {
  org.copy2Image(this);
  return *this;
}


GEM_EXTERN int imageStruct::setCsizeByFormat(int setformat) {
#ifdef __APPLE__
  switch(setformat){
  case GL_LUMINANCE:  
    format=GL_LUMINANCE;  
    type  =GL_UNSIGNED_BYTE; 
    csize =1; 
    break;

  case GL_YUV422_GEM:
  default:
    format=GL_YUV422_GEM; 
    type  =
#ifdef __BIG_ENDIAN__
      GL_UNSIGNED_SHORT_8_8_REV_APPLE;
#else
    GL_UNSIGNED_SHORT_8_8_APPLE;
#endif
    csize =2; 
    break;

  case GL_RGB:  case GL_BGR_EXT: 
    format=GL_BGR_EXT;    
    type  =GL_UNSIGNED_BYTE; 
    csize =3; 
    break;
    
  case GL_RGBA:  case GL_BGRA_EXT:   
    format=GL_BGRA_EXT;
#ifdef __BIG_ENDIAN__
    type  =GL_UNSIGNED_INT_8_8_8_8_REV;
#else
    type  =GL_UNSIGNED_INT_8_8_8_8;
#endif
    csize =4; 
    break;
  }
#else /* !__APPLE__ */
  switch(setformat){
  case GL_LUMINANCE:  
    format=GL_LUMINANCE;  
    type=GL_UNSIGNED_BYTE; 
    csize=1; 
    break;
    
  case GL_YUV422_GEM:
    format=GL_YUV422_GEM; 
    type=GL_UNSIGNED_BYTE;
    csize=2;
    break;
    
  case GL_RGB: 
    format=GL_RGB;
    type=GL_UNSIGNED_BYTE; 
    csize=3;
    break;

  case GL_RGBA:
  default:
    format=GL_RGBA;
#ifdef __BIG_ENDIAN__
    type  =GL_UNSIGNED_INT_8_8_8_8_REV;
#else
    type=GL_UNSIGNED_BYTE;
    //type  =GL_UNSIGNED_INT_8_8_8_8;
#endif
    csize=4; 
    break;
  }
#endif /* __APPLE__ */

  return csize;
}
GEM_EXTERN int imageStruct::setCsizeByFormat() {
  return setCsizeByFormat(format);
}

void pix_addsat(unsigned char *leftPix, unsigned char *rightPix, size_t datasize)
{
  while(datasize--)
    {           
      *leftPix = CLAMP_HIGH(static_cast<int>(*leftPix) + static_cast<int>(*rightPix));
      leftPix++;
      rightPix++;
    }
}


void pix_sub(unsigned char *leftPix, unsigned char *rightPix, size_t datasize)
{
  while(datasize--){
    *leftPix = CLAMP_LOW(static_cast<int>(*leftPix) - static_cast<int>(*rightPix++));
    leftPix++;
  }
}

GEM_EXTERN void imageStruct::setBlack() {
  size_t i = datasize;
  unsigned char* dummy=data;
  switch (format){
  case GL_YCBCR_422_GEM:
    i/=4;
    while(i--){
      *dummy++=128;*dummy++=0;
      *dummy++=128;*dummy++=0;
    }
    break;
  default:
    memset(data, 0, datasize);
    break;
  }
}
GEM_EXTERN void imageStruct::setWhite() {
  size_t i = datasize;
  unsigned char* dummy=data;
  switch (format){
  case GL_YCBCR_422_GEM:
    i/=4;
    while(i--){
      *dummy++=128;*dummy++=255;
      *dummy++=128;*dummy++=255;
    }
    break;
  default:
    memset(data, 1, datasize);
    break;
  }
}
GEM_EXTERN void imageStruct::convertFrom(const imageStruct *from, GLenum to_format) {
  if (!from || !this || !from->data) {
    error("GEM: Someone sent a bogus pointer to convert");
    return;
  }
  xsize=from->xsize;
  ysize=from->ysize;

  if(to_format>0)
    setCsizeByFormat(to_format);
  else setCsizeByFormat();

  upsidedown=from->upsidedown;

  switch (from->format){
  case GL_RGBA: 
    fromRGBA(from->data);
    break;
  case GL_RGB:  
    fromRGB(from->data);
    break;
  case GL_BGR_EXT:
    fromBGR(from->data);
    break;
  case GL_BGRA_EXT: /* "RGBA" on apple */
    fromBGRA(from->data);
    break;
  case GL_LUMINANCE:
    fromGray(from->data);
    break;
  case GL_YCBCR_422_GEM: // YUV
    fromUYVY(from->data);
    break;
  }
}

GEM_EXTERN void imageStruct::convertTo(imageStruct *to, GLenum fmt) const {
  if (!to || !this || !this->data) {
    error("GEM: Someone sent a bogus pointer to convert");
    if (to) to->data = NULL;
    return;
  }
  to->xsize=xsize;
  to->ysize=ysize;
  if(fmt>0)
    to->setCsizeByFormat(fmt);
  else
    to->setCsizeByFormat();

  to->upsidedown=upsidedown;

  switch (format){
  case GL_RGBA: 
    to->fromRGBA(data);
    break;
  case GL_RGB:  
    to->fromRGB(data);
    break;
  case GL_BGR_EXT:
    to->fromBGR(data);
    break;
  case GL_BGRA_EXT: /* "RGBA" on apple */
    to->fromBGRA(data);
    break;
  case GL_LUMINANCE:
    to->fromGray(data);
    break;
  case GL_YCBCR_422_GEM: // YUV
    to->fromUYVY(data);
    break;
  }
}

GEM_EXTERN void imageStruct::fromRGB(const unsigned char *rgbdata) {
  if(!rgbdata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_RGB:
    memcpy(data, rgbdata, pixelnum*csize);
    break;
  case GL_BGR_EXT:
    while(pixelnum--){
      pixels[0]=rgbdata[2];
      pixels[1]=rgbdata[1];
      pixels[2]=rgbdata[0];
      pixels+=3; rgbdata+=3;
    }
    break;
  case GL_RGBA:
    while(pixelnum--){
      *pixels++=*rgbdata++;
      *pixels++=*rgbdata++;
      *pixels++=*rgbdata++;
      *pixels++=255;
    }
    break;
  case GL_BGRA_EXT:
    while(pixelnum--){
      pixels[0]=rgbdata[2];
      pixels[1]=rgbdata[1];
      pixels[2]=rgbdata[0];
      pixels[3]=255;
      pixels+=4;rgbdata+=3;
    }
    break;
  case GL_LUMINANCE:
    while(pixelnum--){
      *pixels++=(rgbdata[0]*RGB2GRAY_RED+rgbdata[1]*RGB2GRAY_GREEN+rgbdata[2]*RGB2GRAY_BLUE)>>8;
      rgbdata+=3;
    }
    break;
  case GL_YUV422_GEM:
#if 0
    RGB_to_YCbCr_altivec(rgbdata, pixelnum, pixels);
#else
    pixelnum>>=1;
    while(pixelnum--){
      *pixels++=((RGB2YUV_21*rgbdata[0]+RGB2YUV_22*rgbdata[1]+RGB2YUV_23*rgbdata[2])>>8)+UV_OFFSET; // U
      *pixels++=((RGB2YUV_11*rgbdata[0]+RGB2YUV_12*rgbdata[1]+RGB2YUV_13*rgbdata[2])>>8)+ Y_OFFSET; // Y
      *pixels++=((RGB2YUV_31*rgbdata[0]+RGB2YUV_32*rgbdata[1]+RGB2YUV_33*rgbdata[2])>>8)+UV_OFFSET; // V
      *pixels++=((RGB2YUV_11*rgbdata[3]+RGB2YUV_12*rgbdata[4]+RGB2YUV_13*rgbdata[5])>>8)+ Y_OFFSET; // Y
      rgbdata+=6;
    }
#endif
    break;
  }
}

GEM_EXTERN void imageStruct::fromRGB16(const unsigned char *rgb16data) {
  //   B B B B B G G G   G G G R R R R R
  //   R R R R R G G G   G G G B B B B B
  if(!rgb16data)return;
  const unsigned short*rgbdata=(const unsigned short*)rgb16data;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  unsigned short rgb;
  switch (format){
  case GL_RGBA:
    while(pixelnum--){
      rgb=*rgbdata++;
      pixels[0]=((rgb>>8)&0xF8);
      pixels[1]=((rgb>>3)&0xFC);
      pixels[2]=((rgb<<3)&0xF8);
      pixels[3]=255;
      pixels+=4;
    }
    break;
  case GL_LUMINANCE:
    while(pixelnum--){
      rgb=*rgbdata++;
      *pixels++=(((rgb>>8)&0xF8)*RGB2GRAY_RED+((rgb>>3)&0xFC)*RGB2GRAY_GREEN+((rgb<<3)&0xF8)*RGB2GRAY_BLUE)>>8;
    }
    break;
  case GL_YUV422_GEM:
    pixelnum>>=1;
    while(pixelnum--){
      rgb=*rgbdata++;
      unsigned char r=((rgb>>8)&0xF8);
      unsigned char g=((rgb>>3)&0xFC);
      unsigned char b=((rgb<<3)&0xF8);
      *pixels++=((RGB2YUV_21*r+RGB2YUV_22*g+RGB2YUV_23*b)>>8)+UV_OFFSET; // U
      *pixels++=((RGB2YUV_11*r+RGB2YUV_12*g+RGB2YUV_13*b)>>8)+ Y_OFFSET;  // Y
      *pixels++=((RGB2YUV_31*r+RGB2YUV_32*g+RGB2YUV_33*b)>>8)+UV_OFFSET; // V

      rgb=*rgbdata++;
      r=((rgb>>8)&0xF8);
      g=((rgb>>3)&0xFC);
      b=((rgb<<3)&0xF8);
      *pixels++=((RGB2YUV_11*r+RGB2YUV_12*g+RGB2YUV_13*b)>>8)+ Y_OFFSET;     // Y
    }
    break;
  }
}

GEM_EXTERN void imageStruct::fromRGBA(const unsigned char *rgbadata) {
  if(!rgbadata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_RGB:
    while(pixelnum--){
      *pixels++=*rgbadata++;
      *pixels++=*rgbadata++;
      *pixels++=*rgbadata++;
      rgbadata++;
    }
    break;
  case GL_BGR_EXT:
    while(pixelnum--){
      pixels[0]=rgbadata[2];
      pixels[1]=rgbadata[1];
      pixels[2]=rgbadata[0];
      pixels+=3; rgbadata+=4;
    }
    break;
  case GL_RGBA:
    memcpy(data, rgbadata, pixelnum*csize);
    break;
  case GL_BGRA_EXT:
    if(pixels==rgbadata){
      unsigned char dummy=0;
      while(pixelnum--){
        dummy=pixels[2];
        pixels[2]=pixels[0];
        pixels[0]=dummy;
        pixels+=4;
      }
    } else {
      while(pixelnum--){
        pixels[0]=rgbadata[2];
        pixels[1]=rgbadata[1];
        pixels[2]=rgbadata[0];
        pixels[3]=rgbadata[3];
        pixels+=4;rgbadata+=4;
      }
    }
    break;
  case GL_LUMINANCE:
    while(pixelnum--){
      *pixels++=(rgbadata[0]*RGB2GRAY_RED+rgbadata[1]*RGB2GRAY_GREEN+rgbadata[2]*RGB2GRAY_BLUE)>>8;

      rgbadata+=4;
    }
    break;
  case GL_YUV422_GEM:
    START_TIMING;
    switch(m_simd){
#ifdef __VEC__
    case GEM_SIMD_ALTIVEC:
      BGRA_to_YCbCr_altivec(rgbadata,pixelnum,pixels);
      break;
#endif
#ifdef __SSE2__
    case GEM_SIMD_SSE2:
      RGBA_to_UYVY_SSE2(rgbadata,pixelnum,pixels);
      break;
#endif
    case GEM_SIMD_NONE: default:
      pixelnum>>=1;
      while(pixelnum--){
	*pixels++=((RGB2YUV_21*rgbadata[chRed]+
		    RGB2YUV_22*rgbadata[chGreen]+
		    RGB2YUV_23*rgbadata[chBlue])>>8)+UV_OFFSET; // U
	*pixels++=((RGB2YUV_11*rgbadata[chRed]+
		    RGB2YUV_12*rgbadata[chGreen]+
		    RGB2YUV_13*rgbadata[chBlue])>>8)+ Y_OFFSET; // Y
	*pixels++=((RGB2YUV_31*rgbadata[chRed]+
		    RGB2YUV_32*rgbadata[chGreen]+
		    RGB2YUV_33*rgbadata[chBlue])>>8)+UV_OFFSET; // V
	*pixels++=((RGB2YUV_11*rgbadata[4+chRed]+
		    RGB2YUV_12*rgbadata[4+chGreen]+
		    RGB2YUV_13*rgbadata[4+chBlue])>>8)+ Y_OFFSET; // Y
	rgbadata+=8;
      }
    }
    STOP_TIMING("RGBA to UYVY");
    break;
  }
}


GEM_EXTERN void imageStruct::fromBGR(const unsigned char *bgrdata) {
  if(!bgrdata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_BGR_EXT:
    memcpy(data, bgrdata, pixelnum*csize);
    break;
  case GL_RGB:
    while(pixelnum--){
      pixels[0]=bgrdata[2];
      pixels[1]=bgrdata[1];
      pixels[2]=bgrdata[0];
      pixels+=3; bgrdata+=3;
    }
    break;
  case GL_BGRA_EXT:
    while(pixelnum--){
      *pixels++=*bgrdata++;
      *pixels++=*bgrdata++;
      *pixels++=*bgrdata++;
      *pixels++=255;
    }
    break;
  case GL_RGBA:
    while(pixelnum--){
      pixels[0]=bgrdata[2];
      pixels[1]=bgrdata[1];
      pixels[2]=bgrdata[0];
      pixels[3]=255;
      pixels+=4;bgrdata+=3;
    }
    break;
  case GL_LUMINANCE:
    while(pixelnum--){
      *pixels++=(bgrdata[2]*RGB2GRAY_RED+bgrdata[1]*RGB2GRAY_GREEN+bgrdata[0]*RGB2GRAY_BLUE)>>8;
      bgrdata+=3;
    }
    break;
  case GL_YUV422_GEM:
    pixelnum>>=1;
    while(pixelnum--){
      *pixels++=((RGB2YUV_21*bgrdata[2]+RGB2YUV_22*bgrdata[1]+RGB2YUV_23*bgrdata[0])>>8)+UV_OFFSET; // U
      *pixels++=((RGB2YUV_11*bgrdata[2]+RGB2YUV_12*bgrdata[1]+RGB2YUV_13*bgrdata[0])>>8)+ Y_OFFSET; // Y
      *pixels++=((RGB2YUV_31*bgrdata[2]+RGB2YUV_32*bgrdata[1]+RGB2YUV_33*bgrdata[0])>>8)+UV_OFFSET; // V
      *pixels++=((RGB2YUV_11*bgrdata[5]+RGB2YUV_12*bgrdata[4]+RGB2YUV_13*bgrdata[3])>>8)+ Y_OFFSET; // Y
      bgrdata+=6;
    }
    break;
  }
}

GEM_EXTERN void imageStruct::fromBGRA(const unsigned char *bgradata) {
  if(!bgradata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_BGR_EXT:
    while(pixelnum--){
      *pixels++=*bgradata++;
      *pixels++=*bgradata++;
      *pixels++=*bgradata++;
      bgradata++;
    }
    break;
  case GL_RGB:
    while(pixelnum--){
      pixels[0]=bgradata[2];
      pixels[1]=bgradata[1];
      pixels[2]=bgradata[0];
      pixels+=3; bgradata+=4;
    }
    break;
  case GL_BGRA_EXT:
    memcpy(data, bgradata, pixelnum*csize);
    break;
  case GL_RGBA:
    if(bgradata==data){
      // in place conversion
      unsigned char dummy=0;
      while(pixelnum--){
        dummy    =pixels[2];
        pixels[2]=pixels[0];
        pixels[0]=dummy;
        pixels+=4;
      } 
    } else {
      while(pixelnum--){
        pixels[0]=bgradata[2];
        pixels[1]=bgradata[1];
        pixels[2]=bgradata[0];
        pixels[3]=bgradata[3];
        pixels+=4;bgradata+=4;
      }
    }
    break;
  case GL_LUMINANCE:
    while(pixelnum--){
#ifdef __APPLE__
      const int R=1;
      const int G=2;
      const int B=3;
#else
      const int R=2;
      const int G=1;
      const int B=0;
#endif
      *pixels++=(bgradata[R]*RGB2GRAY_RED+bgradata[G]*RGB2GRAY_GREEN+bgradata[B]*RGB2GRAY_BLUE)>>8;
      bgradata+=4;
    }
    break;
  case GL_YUV422_GEM:
    START_TIMING;
    switch(m_simd){
#ifdef __VEC__
    case GEM_SIMD_ALTIVEC:
      BGRA_to_YCbCr_altivec(bgradata,pixelnum,pixels);
      break;
#endif
    case GEM_SIMD_NONE: default:
      pixelnum>>=1;
      while(pixelnum--){
	*pixels++=((RGB2YUV_21*bgradata[chRed]+
		    RGB2YUV_22*bgradata[chGreen]+
		    RGB2YUV_23*bgradata[chBlue])>>8)+UV_OFFSET; // U
	*pixels++=((RGB2YUV_11*bgradata[chRed]+
		    RGB2YUV_12*bgradata[chGreen]+
		    RGB2YUV_13*bgradata[chBlue])>>8)+ Y_OFFSET; // Y
	*pixels++=((RGB2YUV_31*bgradata[chRed]+
		    RGB2YUV_32*bgradata[chGreen]+
		    RGB2YUV_33*bgradata[chBlue])>>8)+UV_OFFSET; // V
	*pixels++=((RGB2YUV_11*bgradata[4+chRed]+
		    RGB2YUV_12*bgradata[4+chGreen]+
		    RGB2YUV_13*bgradata[4+chBlue])>>8)+ Y_OFFSET; // Y
	bgradata+=8;
      }
    }
    STOP_TIMING("BGRA_to_YCbCr");
    break;
  }
}



GEM_EXTERN void imageStruct::fromABGR(const unsigned char *abgrdata) {
  if(!abgrdata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_BGR_EXT:
    while(pixelnum--){
      abgrdata++;
      *pixels++=*abgrdata++;
      *pixels++=*abgrdata++;
      *pixels++=*abgrdata++;
    }
    break;
  case GL_RGB:
    while(pixelnum--){
      pixels[0]=abgrdata[3]; // R 
      pixels[1]=abgrdata[2]; // G
      pixels[2]=abgrdata[1]; // B
      pixels+=3; abgrdata+=4;
    }
    break;
  case GL_ABGR_EXT:
    memcpy(data, abgrdata, pixelnum*csize);
    break;
  case GL_RGBA:
    if(abgrdata==data){
      // in place conversion
      unsigned char dummy=0;
      while(pixelnum--){

        dummy    =pixels[3]; pixels[3]=pixels[0]; pixels[0]=dummy;
        dummy    =pixels[1]; pixels[1]=pixels[2]; pixels[2]=dummy;
        pixels+=4;
      } 
    } else {
      while(pixelnum--){
        pixels[0]=abgrdata[3]; // R
        pixels[1]=abgrdata[2]; // G
        pixels[2]=abgrdata[1]; // B
        pixels[3]=abgrdata[0]; // A
        pixels+=4;abgrdata+=4;
      }
    }
    break;
  case GL_LUMINANCE:
    while(pixelnum--){
#ifdef __APPLE__
      const int R=2;
      const int G=3;
      const int B=0;
#else
      const int R=3;
      const int G=2;
      const int B=1;
#endif
      *pixels++=(abgrdata[R]*RGB2GRAY_RED+abgrdata[G]*RGB2GRAY_GREEN+abgrdata[B]*RGB2GRAY_BLUE)>>8;
      abgrdata+=4;
    }
    break;
  case GL_YUV422_GEM:
    START_TIMING;
    switch(m_simd){
    case GEM_SIMD_NONE: default:
      pixelnum>>=1;
      while(pixelnum--){
	*pixels++=((RGB2YUV_21*abgrdata[chAlpha]+
		    RGB2YUV_22*abgrdata[chBlue]+
		    RGB2YUV_23*abgrdata[chGreen])>>8)+UV_OFFSET; // U
	*pixels++=((RGB2YUV_11*abgrdata[chAlpha]+
		    RGB2YUV_12*abgrdata[chBlue]+
		    RGB2YUV_13*abgrdata[chGreen])>>8)+ Y_OFFSET; // Y
	*pixels++=((RGB2YUV_31*abgrdata[chAlpha]+
		    RGB2YUV_32*abgrdata[chBlue]+
		    RGB2YUV_33*abgrdata[chGreen])>>8)+UV_OFFSET; // V
	*pixels++=((RGB2YUV_11*abgrdata[4+chAlpha]+
		    RGB2YUV_12*abgrdata[4+chBlue]+
		    RGB2YUV_13*abgrdata[4+chGreen])>>8)+ Y_OFFSET; // Y
	abgrdata+=8;
      }
    }
    STOP_TIMING("ABGR_to_YCbCr");
    break;
  }
}

GEM_EXTERN void imageStruct::fromARGB(const unsigned char *argbdata) {
  if(!argbdata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_BGR_EXT:
    while(pixelnum--){
      pixels[0]=argbdata[3]; // B
      pixels[1]=argbdata[2]; // G
      pixels[2]=argbdata[1]; // R
      pixels+=3; argbdata+=4;
    }
    break;
  case GL_RGB:
    while(pixelnum--){
      argbdata++;
      *pixels++=*argbdata++;
      *pixels++=*argbdata++;
      *pixels++=*argbdata++;
    }
    break;
#if 0
  case GL_ARGB_EXT:
    memcpy(data, argbdata, pixelnum*csize);
    break;
#endif
  case GL_RGBA:
    while(pixelnum--){
      pixels[0]=argbdata[1]; // R
      pixels[1]=argbdata[2]; // G
      pixels[2]=argbdata[3]; // B
      pixels[3]=argbdata[0]; // A
      pixels+=4;argbdata+=4;
    }
    break;
  case GL_LUMINANCE:
    while(pixelnum--){
#ifdef __APPLE__
      const int R=0;
      const int G=3;
      const int B=2;
#else
      const int R=1;
      const int G=2;
      const int B=3;
#endif
      *pixels++=(argbdata[R]*RGB2GRAY_RED+argbdata[G]*RGB2GRAY_GREEN+argbdata[B]*RGB2GRAY_BLUE)>>8;
      argbdata+=4;
    }
    break;
  case GL_YUV422_GEM:
    START_TIMING;
    switch(m_simd){
    case GEM_SIMD_NONE: default:
      pixelnum>>=1;
      while(pixelnum--){
	*pixels++=((RGB2YUV_21*argbdata[chGreen]+ // R 
		    RGB2YUV_22*argbdata[chBlue]+  // G
		    RGB2YUV_23*argbdata[chAlpha]  // B
		    )>>8)+UV_OFFSET; // U
	*pixels++=((RGB2YUV_11*argbdata[chGreen]+
		    RGB2YUV_12*argbdata[chBlue]+
		    RGB2YUV_13*argbdata[chAlpha])>>8)+ Y_OFFSET; // Y
	*pixels++=((RGB2YUV_31*argbdata[chGreen]+
		    RGB2YUV_32*argbdata[chBlue]+
		    RGB2YUV_33*argbdata[chAlpha])>>8)+UV_OFFSET; // V
	*pixels++=((RGB2YUV_11*argbdata[4+chGreen]+
		    RGB2YUV_12*argbdata[4+chBlue]+
		    RGB2YUV_13*argbdata[4+chAlpha])>>8)+ Y_OFFSET; // Y
	argbdata+=8;
      }
    }
    STOP_TIMING("ARGB_to_YCbCr");
    break;
  }
}

GEM_EXTERN void imageStruct::fromGray(const unsigned char *greydata) {
  if(!greydata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  register unsigned char grey=0;
  switch (format){
  case GL_RGB:
  case GL_BGR_EXT:
    while(pixelnum--){
      grey=*greydata++;
      *pixels++=grey;
      *pixels++=grey;
      *pixels++=grey;
      greydata++;
    }
    break;
  case GL_RGBA:
  case GL_BGRA_EXT:
    while(pixelnum--){
      grey=*greydata++;
      pixels[chRed]=grey;
      pixels[chGreen]=grey;
      pixels[chBlue]=grey;
      pixels[chAlpha]=255;
      pixels+=4;
    }
    break;
  case GL_LUMINANCE:
    memcpy(data, greydata, pixelnum);
    break;
  case GL_YUV422_GEM:
    pixelnum>>=1;
    while(pixelnum--){
      pixels[chY0]=*greydata++;
      pixels[chY1]=*greydata++;
      pixels[chU]=pixels[chV]=128;
      pixels+=4;      
    }
    break;
  }
}

GEM_EXTERN void imageStruct::fromGray(short *greydata) {
  if(!greydata)return;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  register short grey=0;
  switch (format){
  case GL_RGB:
  case GL_BGR_EXT:
    while(pixelnum--){
      grey=*greydata++;
      *pixels++=grey;
      *pixels++=grey;
      *pixels++=grey;
      greydata++;
    }
    break;
  case GL_RGBA:
  case GL_BGRA_EXT:
    while(pixelnum--){
      grey=*greydata++;
      pixels[chRed]=grey;
      pixels[chGreen]=grey;
      pixels[chBlue]=grey;
      pixels[chAlpha]=255;
      pixels+=4;
    }
    break;
  case GL_LUMINANCE:
    memcpy(data, greydata, pixelnum);
    break;
  case GL_YUV422_GEM:
    pixelnum>>=1;
    while(pixelnum--){
      pixels[chY0]=(*greydata++)>>7;
      pixels[chY1]=(*greydata++)>>7;
      pixels[chU]=pixels[chV]=128;
      pixels+=4;      
    }
    break;
  }
}

GEM_EXTERN void imageStruct::fromYU12(const unsigned char*yuvdata) {
  if(!yuvdata)return;
  size_t pixelnum=xsize*ysize;
  fromYV12((yuvdata), yuvdata+(pixelnum), yuvdata+(pixelnum+(pixelnum>>2)));
}
GEM_EXTERN void imageStruct::fromYV12(const unsigned char*yuvdata) {
  if(!yuvdata)return;
  size_t pixelnum=xsize*ysize;
  fromYV12((yuvdata), yuvdata+(pixelnum+(pixelnum>>2)), yuvdata+(pixelnum));
}
GEM_EXTERN void imageStruct::fromYV12(const unsigned char*Y, const unsigned char*U, const unsigned char*V) {
  // planar: 8bit Y-plane + 8bit 2x2-subsampled V- and U-planes
  if(!U && !V)fromGray(Y);
  if(!Y || !U || !V)return;

  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  switch (format){
  case GL_LUMINANCE:
    memcpy(data, Y, pixelnum);
    break;
  case GL_RGB:  case GL_BGR_EXT: // of course this is stupid, RGB isn't BGR
    {
      unsigned char *pixels1=data;
      unsigned char *pixels2=data+xsize*3;

      const unsigned char*py1=Y;
      const unsigned char*py2=Y+xsize; // plane_1 is luminance (csize==1)
      const unsigned char*pv=(format==GL_BGR_EXT)?U:V;
      const unsigned char*pu=(format==GL_RGB)?U:V;

      int y, u, v, yy;
      int uv_r, uv_g, uv_b;
      int row=ysize>>1;
      int cols=xsize>>1;
      yy=128;
      while(row--){
	int col=cols;
	while(col--){
	  u=*pu++ -UV_OFFSET;
	  v=*pv++ -UV_OFFSET;
	  uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	  uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	  uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	  // 1st row - 1st pixel
	  y=YUV2RGB_11*(*py1++ -Y_OFFSET);
	  *pixels1++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels1++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels1++ = CLAMP((y + uv_r) >> 8); // r

	  // 1st row - 2nd pixel
	  y=YUV2RGB_11*(*py1++ -Y_OFFSET);
	  *pixels1++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels1++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels1++ = CLAMP((y + uv_r) >> 8); // r

	  // 2nd row - 1st pixel
	  y=YUV2RGB_11*(*py2++ -Y_OFFSET);
	  *pixels2++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels2++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels2++ = CLAMP((y + uv_r) >> 8); // r

	  // 2nd row - 2nd pixel
	  y=YUV2RGB_11*(*py2++ -Y_OFFSET);
	  *pixels2++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels2++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels2++ = CLAMP((y + uv_r) >> 8); // r
	}
	pixels1+=xsize*csize;	pixels2+=xsize*csize;
	py1+=xsize*1;	py2+=xsize*1;
      }
    }
    break;
  case GL_RGBA:
  case GL_BGRA_EXT:
    {
      unsigned char *pixels1=data;
      unsigned char *pixels2=data+xsize*4;

      const unsigned char*py1=Y;//yuvdata;
      const unsigned char*py2=Y+xsize;//yuvdata+xsize; // plane_1 is luminance (csize==1)
      const unsigned char*pv=(format==GL_BGRA_EXT)?V:U;
      const unsigned char*pu=(format==GL_RGBA)?V:U;
 
      int y, u, v, yy;
      int uv_r, uv_g, uv_b;
      int row=ysize>>1;
      int cols=xsize>>1;
      yy=128;
      while(row--){
	int col=cols;
	while(col--){
	  u=*pu++-UV_OFFSET;
	  v=*pv++-UV_OFFSET;
	  uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	  uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	  uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	  // 1st row - 1st pixel
	  y=YUV2RGB_11*(*py1++ -Y_OFFSET);
	  *pixels1++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels1++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels1++ = CLAMP((y + uv_r) >> 8); // r
	  *pixels1++ = 255; // a

	  // 1st row - 2nd pixel
	  y=YUV2RGB_11*(*py1++ -Y_OFFSET);
	  *pixels1++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels1++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels1++ = CLAMP((y + uv_r) >> 8); // r
	  *pixels1++ = 255; // a

	  // 2nd row - 1st pixel
	  y=YUV2RGB_11*(*py2++ -Y_OFFSET);
	  *pixels2++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels2++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels2++ = CLAMP((y + uv_r) >> 8); // r
	  *pixels2++ = 255; // a

	  // 2nd row - 2nd pixel
	  y=YUV2RGB_11*(*py2++ -Y_OFFSET);
	  *pixels2++ = CLAMP((y + uv_b) >> 8); // b
	  *pixels2++ = CLAMP((y + uv_g) >> 8); // g
	  *pixels2++ = CLAMP((y + uv_r) >> 8); // r
	  *pixels2++ = 255; // a
	}
	pixels1+=xsize*csize;	pixels2+=xsize*csize;
	py1+=xsize*1;	py2+=xsize*1;
      }
    }
    break;
  case GL_YUV422_GEM:
    {
      unsigned char *pixels1=data;
      unsigned char *pixels2=data+xsize*csize;
      const unsigned char*py1=Y;
      const unsigned char*py2=Y+xsize; // plane_1 is luminance (csize==1)
      const unsigned char*pu=U;
      const unsigned char*pv=V;
      int row=ysize>>1;
      int cols=xsize>>1;
      unsigned char u, v;
      /* this is only re-ordering of the data */
      while(row--){
	int col=cols;
	while(col--){
	  // yuv422 is U Y0 V Y1
	  u=*pu++;	  v=*pv++;
	  *pixels1++=u;
	  *pixels1++=*py1++;
	  *pixels1++=v;
	  *pixels1++=*py1++;
	  *pixels2++=u;
	  *pixels2++=*py2++;
	  *pixels2++=v;
	  *pixels2++=*py2++;	  
	}
	pixels1+=xsize*csize;	pixels2+=xsize*csize;
	py1+=xsize*1;	py2+=xsize*1;
      }
    }
    break;
  }
}
//  for gem2pdp
GEM_EXTERN void imageStruct::fromYV12(const short*yuvdata) {
  if(!yuvdata)return;
  int pixelnum=xsize*ysize;
  fromYV12((yuvdata), yuvdata+(pixelnum+(pixelnum>>2)), yuvdata+(pixelnum));
}
GEM_EXTERN void imageStruct::fromYV12(const short*Y, const short*U, const short*V) {
  // planar: 8bit Y-plane + 8bit 2x2-subsampled V- and U-planes
  if(!U && !V)fromGray(reinterpret_cast<unsigned char*>(*Y>>7));
  if(!Y || !U || !V)return;

  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  switch (format){
  case GL_LUMINANCE:
    memcpy(data, Y, pixelnum);
    break;
  case GL_RGB:  case GL_BGR_EXT: // of course this is stupid, RGB isn't BGR
    {
      unsigned char *pixels1=data;
      unsigned char *pixels2=data+xsize*csize;

      const short*py1=Y;
      const short*py2=Y+xsize; // plane_1 is luminance (csize==1)
      const short*pv=V;//(format==GL_BGR_EXT)?V:U;
      const short*pu=U;//(format==GL_RGB)?V:U;

      int y, u, v, yy;
      int uv_r, uv_g, uv_b;
      int row=ysize>>1;
      int cols=xsize>>1;
      yy=128;
      while(row--){
	int col=cols;
	while(col--){
	  // these are from http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC30
	  u=(*pu++)>>8;
	  v=(*pv++)>>8;
	  uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	  uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	  uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	  // 1st row - 1st pixel
	  y=YUV2RGB_11*((*py1++)>>7); // what about the "16"-offset ?
#ifndef __APPLE__
	  pixels1[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels1[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels1[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels1+=3;

	  // 1st row - 2nd pixel
	  y=YUV2RGB_11*((*py1++)>>7);
	  pixels1[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels1[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels1[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels1+=3;

	  // 2nd row - 1st pixel
	  y=YUV2RGB_11*((*py2++)>>7);
	  pixels2[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels2[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels2[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels2+=3;

	  // 2nd row - 2nd pixel
	  y=YUV2RGB_11*((*py2++)>>7);
	  pixels2[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels2[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels2[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels2+=3;

#else
	  pixels1[2 ] = CLAMP((y + uv_r) >> 8); // r
	  pixels1[1] = CLAMP((y + uv_g) >> 8); // g
	  pixels1[0] = CLAMP((y + uv_b) >> 8); // b
	  pixels1+=3;

	  // 1st row - 2nd pixel
	  y=YUV2RGB_11*((*py1++)>>7);
	  pixels1[2 ] = CLAMP((y + uv_r) >> 8); // r
	  pixels1[1] = CLAMP((y + uv_g) >> 8); // g
	  pixels1[0] = CLAMP((y + uv_b) >> 8); // b
	  pixels1+=3;

	  // 2nd row - 1st pixel
	  y=YUV2RGB_11*((*py2++)>>7);
	  pixels2[2 ] = CLAMP((y + uv_r) >> 8); // r
	  pixels2[1] = CLAMP((y + uv_g) >> 8); // g
	  pixels2[0 ] = CLAMP((y + uv_b) >> 8); // b
	  pixels2+=3;

	  // 2nd row - 2nd pixel
	  y=YUV2RGB_11*((*py2++)>>7);
	  pixels2[2 ] = CLAMP((y + uv_r) >> 8); // r
	  pixels2[1] = CLAMP((y + uv_g) >> 8); // g
	  pixels2[0] = CLAMP((y + uv_b) >> 8); // b
	  pixels2+=3;
#endif

	}
	pixels1+=xsize*csize;	pixels2+=xsize*csize;
	py1+=xsize*1;	py2+=xsize*1;
      }
    }
    break;
  case GL_RGBA:
  case GL_BGRA_EXT:
    {
      unsigned char *pixels1=data;
      unsigned char *pixels2=data+xsize*csize;

      const short*py1=Y;//yuvdata;
      const short*py2=Y+xsize;//yuvdata+xsize; // plane_1 is luminance (csize==1)
      const short*pv=V;//(format==GL_BGRA_EXT)?U:V;
      const short*pu=U;//(format==GL_RGBA)?U:V;

      int y, u, v, yy;
      int uv_r, uv_g, uv_b;
      int row=ysize>>1;
      int cols=xsize>>1;
      yy=128;
      while(row--){
	int col=cols;
	while(col--){
	  u=(*pu++)>>8;
	  v=(*pv++)>>8;
	  uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	  uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	  uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	  // 1st row - 1st pixel
	  y=YUV2RGB_11*((*py1++)>>7); // what about the "16"-offset ?
	  pixels1[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels1[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels1[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels1[chAlpha] = 255; // a
	  pixels1+=4;

	  // 1st row - 2nd pixel
	  y=YUV2RGB_11*((*py1++)>>7);
	  pixels1[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels1[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels1[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels1[chAlpha] = 255; // a
	  pixels1+=4;

	  // 2nd row - 1st pixel
	  y=YUV2RGB_11*((*py2++)>>7);
	  pixels2[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels2[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels2[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels2[chAlpha] = 255; // a
	  pixels2+=4;

	  // 2nd row - 2nd pixel
	  y=YUV2RGB_11*((*py2++)>>7);
	  pixels2[chRed  ] = CLAMP((y + uv_r) >> 8); // r
	  pixels2[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels2[chBlue ] = CLAMP((y + uv_b) >> 8); // b
	  pixels2[chAlpha] = 255; // a
	  pixels2+=4;
	}
	pixels1+=xsize*csize;	pixels2+=xsize*csize;
	py1+=xsize*1;	py2+=xsize*1;
      }
    }

    break;
  case GL_YUV422_GEM:
    {
      START_TIMING;
      switch(m_simd){
#ifdef __VEC__
      case GEM_SIMD_ALTIVEC:
	YV12_to_YUV422_altivec(Y, U, V, data, xsize, ysize);
	break;
#endif
      case GEM_SIMD_NONE: default:
	unsigned char *pixels1=data;
	unsigned char *pixels2=data+xsize*csize;
	const short*py1=Y;
	const short*py2=Y+xsize; // plane_1 is luminance (csize==1)
	const short*pu=U;
	const short*pv=V;
	int row=ysize>>1;
	int cols=xsize>>1;
	unsigned char u, v;
	/* this is only re-ordering of the data */
	while(row--){
	  int col=cols;
	  while(col--){
	    // yuv422 is U Y0 V Y1
	    u=((*pu++)>>8)+128;	  v=((*pv++)>>8)+128;
	    *pixels1++=u;
	    *pixels1++=(*py1++)>>7;
	    *pixels1++=v;
	    *pixels1++=(*py1++)>>7;
	    *pixels2++=u;
	    *pixels2++=(*py2++)>>7;
	    *pixels2++=v;
	    *pixels2++=(*py2++)>>7;	  
	  }
	  pixels1+=xsize*csize;	pixels2+=xsize*csize;
	  py1+=xsize*1;	py2+=xsize*1;
	}
      }
      STOP_TIMING("YV12_to_YUV422");
    }
    break;
  }
}

GEM_EXTERN void imageStruct::fromUYVY(const unsigned char *yuvdata) {
  // this is the yuv-format with Gem
  if(!yuvdata)return;
  data=data;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_YUV422_GEM:
    memcpy(data, yuvdata, pixelnum*csize);
    break;
  case GL_LUMINANCE:
    pixelnum>>=1;
    while(pixelnum--){
      *pixels++=yuvdata[1];
      *pixels++=yuvdata[3];
      yuvdata+=4;
    }
    break;
  case GL_RGB:
  case GL_BGR:
    {
      unsigned char *pixels=data;
      int y, u, v;
      int uv_r, uv_g, uv_b;
      START_TIMING;
      switch(m_simd){
#ifdef __SSE2__
      case GEM_SIMD_SSE2:
	UYVY_to_RGB_SSE2(yuvdata, pixelnum, pixels);
	break;
#endif
      case GEM_SIMD_NONE: default:
	pixelnum>>=1;

	while(pixelnum--){
	  u=yuvdata[0]-UV_OFFSET;
	  v=yuvdata[2]-UV_OFFSET;
	  uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	  uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	  uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	  // 1st pixel
	  y=YUV2RGB_11*(yuvdata[1] -Y_OFFSET);
	  pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	  pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	  pixels+=3;
	  // 2nd pixel
	  y=YUV2RGB_11*(yuvdata[3] -Y_OFFSET);
	  pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	  pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	  pixels+=3;

	  yuvdata+=4;
	}
      }
      STOP_TIMING("YUV2RGB");
    }
    break;
  case GL_RGBA:
  case GL_BGRA: /* ==GL_BGRA_EXT */
    {
      START_TIMING;
      switch(m_simd){
#ifdef __VEC__
      case GEM_SIMD_ALTIVEC:
	YUV422_to_BGRA_altivec( yuvdata, pixelnum*2, data);
	break;
#endif
#ifdef __SSE2__
      case GEM_SIMD_SSE2:
	UYVY_to_RGBA_SSE2(yuvdata, pixelnum, data);
	break;
#endif
      case GEM_SIMD_NONE: default:
	unsigned char *pixels=data;
	int y, u, v;
	int uv_r, uv_g, uv_b;
	pixelnum>>=1;
	while(pixelnum--){
	  u=yuvdata[0]-UV_OFFSET;
	  v=yuvdata[2]-UV_OFFSET;
	  uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	  uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	  uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	  // 1st pixel
	  y=YUV2RGB_11*(yuvdata[1] -Y_OFFSET);
	  pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	  pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	  pixels[chAlpha] = 255;
	  pixels+=4;
	  // 2nd pixel
	  y=YUV2RGB_11*(yuvdata[3] -Y_OFFSET);
	  pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	  pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	  pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	  pixels[chAlpha] = 255;
	  pixels+=4;

	  yuvdata+=4;
	}
	STOP_TIMING("UYVY_to_RGBA/BGRA");
      }
    }
    break;
  }
}

GEM_EXTERN void imageStruct::fromYUY2(const unsigned char *yuvdata) { // YUYV
  if(!yuvdata)return;
  data=data;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_YUV422_GEM:
    pixelnum>>=1;
    while(pixelnum--){
      pixels[0]=yuvdata[1]; // u
      pixels[1]=yuvdata[0]; // y
      pixels[2]=yuvdata[3]; // v
      pixels[3]=yuvdata[2]; // y
      pixels+=4;
      yuvdata+=4;
    }
    break;
  case GL_LUMINANCE:
    pixelnum>>=1;
    while(pixelnum--){
      *pixels++=yuvdata[0];
      *pixels++=yuvdata[2];
      yuvdata+=4;
    }
    break;
  case GL_RGB:
  case GL_BGR:
    {
      unsigned char *pixels=data;
      int y, u, v;
      int uv_r, uv_g, uv_b;
      pixelnum>>=1;

      while(pixelnum--){
	u=yuvdata[1]-UV_OFFSET;
	v=yuvdata[3]-UV_OFFSET;
	uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	// 1st pixel
	y=YUV2RGB_11*(yuvdata[0] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels+=3;
	// 2nd pixel
	y=YUV2RGB_11*(yuvdata[2] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels+=3;

	yuvdata+=4;
      }
    }
    break;
  case GL_RGBA:
  case GL_BGRA: /* ==GL_BGRA_EXT */
    {
      unsigned char *pixels=data;
      int y, u, v;
      int uv_r, uv_g, uv_b;
      pixelnum>>=1;
      while(pixelnum--){
	u=yuvdata[1]-UV_OFFSET;
	v=yuvdata[3]-UV_OFFSET;
	uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	// 1st pixel
	y=YUV2RGB_11*(yuvdata[0] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels[chAlpha] = 255;
	pixels+=4;
	// 2nd pixel
	y=YUV2RGB_11*(yuvdata[2] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels[chAlpha] = 255;
	pixels+=4;

	yuvdata+=4;
      }
    }
    break;
  }
}

GEM_EXTERN void imageStruct::fromYVYU(const unsigned char *yuvdata) {
  // this is the yuv-format with Gem
  if(!yuvdata)return;
  data=data;
  size_t pixelnum=xsize*ysize;
  setCsizeByFormat();
  reallocate();
  unsigned char *pixels=data;
  switch (format){
  case GL_YUV422_GEM:
    pixelnum>>=1;
    while(pixelnum--){
      pixels[chU]=yuvdata[1]; // u
      pixels[chY0]=yuvdata[0]; // y
      pixels[chV]=yuvdata[3]; // v
      pixels[chY1]=yuvdata[2]; // y
      pixels+=4;
      yuvdata+=4;
    }
    break;
  case GL_LUMINANCE:
    pixelnum>>=1;
    while(pixelnum--){
      *pixels++=yuvdata[0];
      *pixels++=yuvdata[2];
      yuvdata+=4;
    }
    break;
  case GL_RGB:  case GL_BGR:
    {
      unsigned char *pixels=data;
      int y, u, v;
      int uv_r, uv_g, uv_b;
      pixelnum>>=1;

      while(pixelnum--){
	u=yuvdata[3]-UV_OFFSET;
	v=yuvdata[1]-UV_OFFSET;
	uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	// 1st pixel
	y=YUV2RGB_11*(yuvdata[0] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels+=3;
	// 2nd pixel
	y=YUV2RGB_11*(yuvdata[2] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels+=3;

	yuvdata+=4;
      }
    }
    break;
  case GL_RGBA:
  case GL_BGRA: /* ==GL_BGRA_EXT */
    {
      unsigned char *pixels=data;
      int y, u, v;
      int uv_r, uv_g, uv_b;
      pixelnum>>=1;
      while(pixelnum--){
	u=yuvdata[3]-UV_OFFSET;
	v=yuvdata[1]-UV_OFFSET;
	uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
	uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
	uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

	// 1st pixel
	y=YUV2RGB_11*(yuvdata[0] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels[chAlpha] = 255;
	pixels+=4;
	// 2nd pixel
	y=YUV2RGB_11*(yuvdata[2] -Y_OFFSET);
	pixels[chRed]   = CLAMP((y + uv_r) >> 8); // r
	pixels[chGreen] = CLAMP((y + uv_g) >> 8); // g
	pixels[chBlue]  = CLAMP((y + uv_b) >> 8); // b
	pixels[chAlpha] = 255;
	pixels+=4;

	yuvdata+=4;
      }
    }
    break;
  }
}

GEM_EXTERN extern int getPixFormat(char*cformat){
  char c=tolower(*cformat);
  switch(c){
  case 'g': return GL_LUMINANCE;
  case 'y': return GL_YUV422_GEM;
  case 'r': return GL_RGBA_GEM;
  }
  return 0;
}


/* flip the image if it is upside down */
GEM_EXTERN void imageStruct::fixUpDown() {
  if(!upsidedown)return; /* everything's fine! */

  int linewidth = xsize*csize;
  unsigned char*line = new unsigned char[linewidth];
  unsigned char*line0, *line1;


  int y0=0, y1=ysize-1;
  for(y0=0; y0<ysize/2; y0++, y1--) {
    line0=data+y0*linewidth;
    line1=data+y1*linewidth;
    memcpy(line , line0, linewidth);
    memcpy(line0, line1, linewidth);
    memcpy(line1, line , linewidth);
  }

  upsidedown=false;
}

/* swap the Red and Blue channel _in-place_ */
GEM_EXTERN void imageStruct::swapRedBlue() {
  size_t pixelnum=xsize*ysize;
  unsigned char *pixels=data;
  unsigned char dummy=0;
  switch (format){
  case GL_YUV422_GEM:
    pixelnum>>=1;
    while(pixelnum--){
      dummy=pixels[chU];
      pixels[chU]=pixels[chV]; // u
      pixels[chV]=dummy; // v
      pixels+=4;
    }
    break;
  case GL_RGB:  case GL_BGR:
    while(pixelnum--){
      dummy=pixels[chRed];
      pixels[chRed]=pixels[chBlue];
      pixels[chBlue]=dummy;
      pixels+=3;
    }
    break;
  case GL_RGBA:
  case GL_BGRA: /* ==GL_BGRA_EXT */
    while(pixelnum--){
      dummy=pixels[chRed];
      pixels[chRed]=pixels[chBlue];
      pixels[chBlue]=dummy;
      pixels+=4;
    }
    break;
  }
}


GEM_EXTERN void imageStruct::getRGB(int X, int Y, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a) const
{
  unsigned char red=0, green=0, blue=0, alpha=255;
  int position = (X+(upsidedown?(ysize-Y-1):Y)*xsize);
  unsigned char*pixels=data+position*csize;
    
  switch(format) {
  case GL_LUMINANCE:
    red=green=blue=pixels[0];
    alpha=255;
    break;
  case GL_RGB:
    red=pixels[0];
    green=pixels[1];
    blue=pixels[2];
    break;
  case GL_BGR_EXT:
    red=pixels[0];
    green=pixels[1];
    blue=pixels[2];
    break;
  case GL_RGBA:
    red=pixels[0];
    green=pixels[1];
    blue=pixels[2];
    alpha=pixels[3];
    break;
  case GL_BGRA_EXT:
#ifdef __APPLE__
    red=pixels[1];
    green=pixels[2];
    blue=pixels[3];
    alpha=pixels[0];
#else
    red=pixels[2];
    green=pixels[1];
    blue=pixels[0];
    alpha=pixels[3];
#endif
    break;
  case GL_YUV422_GEM:
    {
      position = (((X+(upsidedown?(ysize-Y-1):Y)*xsize)>>1)<<1);
      pixels=data+position*csize;
      int y=YUV2RGB_11*(pixels[(X%2)?chY1:chY0]-Y_OFFSET);
      int u=pixels[chU] - UV_OFFSET;
      int v=pixels[chV] - UV_OFFSET;
      int uv_r=YUV2RGB_12*u+YUV2RGB_13*v;
      int uv_g=YUV2RGB_22*u+YUV2RGB_23*v;
      int uv_b=YUV2RGB_32*u+YUV2RGB_33*v;

      red =   CLAMP((y + uv_r) >> 8);
      green = CLAMP((y + uv_g) >> 8);
      blue =  CLAMP((y + uv_b) >> 8);
    }
    break;
  default:
    break;
  }
  if(r)*r=red;
  if(g)*g=green;
  if(b)*b=blue;
  if(a)*a=alpha;
}
GEM_EXTERN void imageStruct::getGrey(int X, int Y, unsigned char*g) const
{
  unsigned char grey=0;
  int position = (X+(upsidedown?(ysize-Y-1):Y)*xsize);
  unsigned char*pixels=data+position*csize;
  switch(format) {
  case GL_LUMINANCE:
    grey=pixels[0];
    break;
  case GL_RGB:
    grey=(pixels[0]*RGB2GRAY_RED+pixels[1]*RGB2GRAY_GREEN+pixels[2]*RGB2GRAY_BLUE)>>8;
    break;
  case GL_BGR_EXT:
    grey=(pixels[2]*RGB2GRAY_RED+pixels[1]*RGB2GRAY_GREEN+pixels[0]*RGB2GRAY_BLUE)>>8;
    break;
  case GL_RGBA:
    grey=(pixels[0]*RGB2GRAY_RED+pixels[1]*RGB2GRAY_GREEN+pixels[2]*RGB2GRAY_BLUE)>>8;
    break;
  case GL_BGRA_EXT:
    grey=(pixels[2]*RGB2GRAY_RED+pixels[1]*RGB2GRAY_GREEN+pixels[0]*RGB2GRAY_BLUE)>>8;
    break;
  case GL_YUV422_GEM:
    {
      position = (((X+(upsidedown?(ysize-Y-1):Y)*xsize)>>1)<<1);
      pixels=data+position*csize;
      grey = CLAMP(pixels[((X%2)?chY1:chY0)]-Y_OFFSET);
    }
    break;
  default:
    break;
  }
  if(g)*g=grey;
}
GEM_EXTERN void imageStruct::getYUV(int X, int Y, unsigned char*y, unsigned char*u, unsigned char*v) const
{
  unsigned char luma=0, chromaU=128, chromaV=128;
  int position = (X+(upsidedown?(ysize-Y-1):Y)*xsize);
  unsigned char*pixels=data+position*csize;
  switch(format) {
  case GL_LUMINANCE:
    luma=pixels[0];
    break;
  case GL_RGB:
  case GL_BGR_EXT:
    error("getYUV not implemented for RGB");
    break;
  case GL_RGBA:
  case GL_BGRA_EXT:
    error("getYUV not implemented for RGBA");
    break;
  case GL_YUV422_GEM:
    position = (((X+(upsidedown?(ysize-Y-1):Y)*xsize)>>1)<<1);
    pixels=data+position*csize;
    luma=pixels[((X%2)?chY1:chY0)];
    chromaU=pixels[chU];
    chromaV=pixels[chV];
    break;
  default:
    break;
  }
  if(y)*y=luma;
  if(u)*u=chromaU;
  if(v)*v=chromaV;
}
