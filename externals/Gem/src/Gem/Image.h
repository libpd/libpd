/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    GemPixUtil.h
       - contains image functions for pix objects
       - part of GEM

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_IMAGE_H_
#define _INCLUDE__GEM_GEM_IMAGE_H_

#include "Gem/GemGL.h"

#include <string.h>
#include <stdlib.h>

// basic helper functions, like CLAMP and powerOfTwo
#include "Utils/Functions.h"


///////////////////////////////////////////////////////////////////////////////
// Color component defines
//
// These should be used to reference the various color channels
///////////////////////////////////////////////////////////////////////////////

/* RGBA */

#if GL_RGBA_GEM == GL_RGBA
const int chRed		= 0;
const int chGreen	= 1;
const int chBlue	= 2;
const int chAlpha	= 3;
#else
const int chAlpha	= 0;
const int chRed		= 1;
const int chGreen	= 2;
const int chBlue	= 3;
#endif


/* Gray */
const int chGray	= 0;

/* YUV422 */
const int chU           = 0;
const int chY0          = 1;
const int chV           = 2;
const int chY1          = 3;

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    imageStruct
    
    The basic image structure

-----------------------------------------------------------------*/
// we now have a class "imageStruct";
// since i need to compile some of the sources with an older version of Gem
// there is a  new define here:
#define IMAGE_CLASS

struct GEM_EXTERN imageStruct
{
  imageStruct(void);
  imageStruct(const imageStruct&);
  virtual ~imageStruct(void);

  virtual void info(void);
  //////////
  // columns
  virtual unsigned char* allocate(size_t size);
  virtual unsigned char* allocate(void);

  // if we have allocated some space already, only re-allocate when needed.
  virtual unsigned char* reallocate(size_t size);
  virtual unsigned char* reallocate(void);
 
  // delete the buffer (if it is ours)
  virtual void clear(void);


  //////////
  // dimensions of the image
  GLint         xsize;
  GLint   	    ysize;

  //////////
  // (average) width of 1 pixel (LUMINANCE = 1, RGBA = 4, YUV = 2)
  GLint   	    csize;

  //////////
  // data type - always GL_UNSIGNED_BYTE (except for OS X)
  GLenum          type;

  //////////
  // the format - either GL_RGBA, GL_LUMINANCE
  // or GL_YCBCR_422_GEM (which is on mac-computers GL_YCBCR_422_APPLE)
  GLenum          format;
  
  //////////
  // is this owned by us (? what about underscores ?)
  int notowned;
  
  //////////
  // gets a pixel
  /* X,Y are the coordinates
   * C is the offset in the interleaved data (like chRed==0 for red)
   * you should use chRed instead of 0 (because it might not be 0)
   */
  // heck, why are X&Y swapped ?? (JMZ)
  inline unsigned char GetPixel(int Y, int X, int C) const
  { 
    return(data[clampFunc(Y, 0, ysize-1) * xsize * csize + clampFunc(X, 0, xsize-1) * csize + C]); }
  
  //////////
  // sets a pixel
  /* while X and Y should be clear (coordinates), 
   * C is the offset (like chRed==0 for red).
   * VAL is the value to set.
   */
  inline void SetPixel(int Y, int X, int C, unsigned char VAL)
  { data[clampFunc(Y, 0, ysize-1) * xsize * csize + clampFunc(X, 0, xsize-1) * csize + C] = VAL; }


  /////////
  // gets the color of a pixel
  virtual void getRGB(int X, int Y, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a=NULL) const;
  virtual void getGrey(int X, int Y, unsigned char*g) const;
  virtual void getYUV(int X, int Y, unsigned char*y, unsigned char*u, unsigned char*v) const;
  
  /* following will set the whole image-data to either black or white
   * the size of the image-data is NOT xsize*ysize*csize but datasize
   * this is mostly slower
   * i have put the datasize into private (like pdata) (bad idea?)
   */
  virtual void setBlack(void);
  virtual void setWhite(void);

  /* certain formats are bound to certain csizes,
   * it's quite annoying to have to think again and again (ok, not much thinking)
   * so we just give the format (like GL_LUMINANCE) 
   * and it will set the image format to this format
   * and set and return the correct csize (like 1)
   * if no format is given the current format is used
   */
  virtual int setCsizeByFormat(int format);
  virtual int setCsizeByFormat(void);

  
  /* various copy functions
   * sometimes we want to copy the whole image (including pixel-data),
   * but often it is enough to just copy the meta-data (without pixel-data)
   * into a new imageStruct
   */
  virtual void copy2Image(imageStruct *to) const;
  virtual void copy2ImageStruct(imageStruct *to) const; // copy the imageStruct (but not the actual data)
  /* this is a sort of better copy2Image, 
   * which only copies the imageStruct-data if it is needed
   */
  virtual void refreshImage(imageStruct *to) const;


  /* inplace swapping Red and Blue channel */
  virtual void swapRedBlue(void);

  ///////////////////////////////////////////////////////////////////////////////
  // acquiring data including colour-transformations
  // should be accelerated if possible
  /* i wonder whether this is the right place to put these routines
   * they should be stored somewhere centrally
   * (because maybe a lot of objects would like them) (like [pix_rgba]...)
   * but it might be better to put them (the actual conversion routines) into
   * separate files (a separate library?)
   * orgdata points to the actual data in the given format
   * the datasize will be read from image.xsize, image.ysize
   * the dest-format will be given by image.format
   *   this is maybe not really clean (the meta-data is stored in the destination, 
   *   while the source has no meta-data of its own)
   */
  virtual void convertTo  (imageStruct*to,   GLenum dest_format=0) const;
  virtual void convertFrom(const imageStruct*from, GLenum dest_format=0);

  virtual void fromRGB    (const unsigned char* orgdata);
  virtual void fromRGBA   (const unsigned char* orgdata);
  virtual void fromBGR    (const unsigned char* orgdata);
  virtual void fromBGRA   (const unsigned char* orgdata);
  virtual void fromRGB16  (const unsigned char* orgdata);
  virtual void fromABGR   (const unsigned char* orgdata);
  virtual void fromARGB   (const unsigned char* orgdata);
  virtual void fromGray   (const unsigned char* orgdata);
  virtual void fromGray   (short* orgdata);
  virtual void fromUYVY   (const unsigned char* orgdata);
  virtual void fromYUY2   (const unsigned char* orgdata); // YUYV
  virtual void fromYVYU   (const unsigned char* orgdata);
  /* planar YUV420: this is rather generic and not really YV12 only */
  virtual void fromYV12   (const unsigned char* Y, const unsigned char*U, const unsigned char*V);
  /* assume that the planes are near each other: YVU */
  virtual void fromYV12   (const unsigned char* orgdata);
  /* assume that the planes are near each other: YVU */
  virtual void fromYU12   (const unsigned char* orgdata);
  /* overloading the above two in order to accept pdp YV12 packets */
  virtual void fromYV12   (const short* Y, const short*U, const short*V);
  virtual void fromYV12   (const short* orgdata);
  
  /* aliases */
  virtual void fromYUV422 (const unsigned char* orgdata){fromUYVY(orgdata);}
  virtual void fromYUV420P(const unsigned char* orgdata){fromYV12(orgdata);}
  virtual void fromYUV420P(const unsigned char*Y,const unsigned char*U,const unsigned char*V){fromYV12(Y,U,V);}

  // "data" points to the image.
  // the memory could(!) be reserved by this class or someone else
  // "notowned" should be set to "1", if "data" points to foreign memory
  // "data" is not freed directly, when the destructor is called
  unsigned char   *data;    // the pointer to the data
  private:
  // "pdata" is the private data, and is the memory reserved by this class
  // this data is freed when the destructor is called
  unsigned char   *pdata;
  // "datasize" is the size of data reserved at "pdata"    
  size_t    datasize;
  
  public:
  //////////
  // true if the image is flipped horizontally (origin is upper-left)
  // false if the image is openGL-conformant (origin is lower-left)
  GLboolean       upsidedown;

  /* make the image orientation openGL-conformant */
  virtual void fixUpDown(void);

  imageStruct& operator=(const imageStruct&);
};

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pixBlock
    
    The pix block structure

-----------------------------------------------------------------*/
struct GEM_EXTERN pixBlock
{
  pixBlock();

    //////////
    // the block's image
    imageStruct     image;

    //////////
    // is this a newimage since last time?
    //	ie, has it been refreshed
    bool     	    newimage;
    
    //////////
    // keeps track of when new films are loaded
    //	useful for rectangle_textures on OSX
    bool		    newfilm;
};

///////////////////////////////////////////////////////////////////////////////
// imageStruct utility functions
//
///////////////////////////////////////////////////////////////////////////////
//////////
// copies all of the data over and mallocs memory
GEM_EXTERN extern void copy2Image(imageStruct *to, imageStruct *from);

//////////
// assumes that it only has to refresh the data
GEM_EXTERN extern void refreshImage(imageStruct *to, imageStruct *from);

GEM_EXTERN extern int getPixFormat(char*);
#endif // GEMPIXUTIL_H_
