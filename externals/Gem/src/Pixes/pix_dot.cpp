////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// This object is an based on the DotTV effect from EffecTV
// Originally written by Fukuchi Kentaro
// Copyright (c) 2001 FUKUCHI Kentaro                              
//
// ported by tigital@mac.com
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2003 James Tittle
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Utils/PixPete.h"
#include "pix_dot.h"
#include <stdlib.h>

CPPEXTERN_NEW(pix_dot);

/////////////////////////////////////////////////////////
//
// pix_dot
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_dot :: pix_dot() : 
  m_xsize(0), m_ysize(0), m_csize(0),
  sharedbuffer(NULL), sharedbuffer_length(0),
  tail(0),
  alreadyInit(0),
  DOTDEPTH(5), DOTMAX(1<<5),
  dots_width(64), dots_height(64),
  dot_size(0), dot_hsize(0),
  sampx(NULL), sampy(NULL),
  state(0),
  m_scale(1.0), 
  pattern(NULL), 
  heart_pattern(NULL),
  mode(0),
  m_useScale(true)
{
  myImage.xsize=myImage.ysize=0;
  DOTDEPTH = 5;
  DOTMAX = (1<<DOTDEPTH);
  yuv_init();
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("scale"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_dot :: ~pix_dot()
{
    myImage.clear();
    sharedbuffer_length = 0;
    free(sharedbuffer);
    free(pattern);
}

/////////////////////////////////////////////////////////
// draw a basic Dot
//
/////////////////////////////////////////////////////////
void pix_dot :: makePattern(int format)
{
  int i, x, y, c;
  int u, v;
  double p, q, r;

  switch(format){
  default: // RGBA
    {
      U32 *pat;
      for (i=0; i<DOTMAX; i++)
	{
	  /* Generated pattern is a quadrant of a disk. */
	  pat = pattern + (i+1) * dot_hsize * dot_hsize - 1;
	  r = (0.2 * i / DOTMAX + 0.8) * dot_hsize;
	  r = r*r;
	  for(y=0; y<dot_hsize; y++) {
	    for(x=0; x<dot_hsize; x++) {
	      c = 0;
	      for(u=0; u<4; u++) {
		p = static_cast<double>(u)/4.0 + y;
		p = p*p;
		for(v=0; v<4; v++) {
		  q = static_cast<double>(v)/4.0 + x;
		  if(p+q*q<r) {
		    c++;
		  }
		}
	      }
	      c = (c>15)?15:c;
	      c<<=4;
	      *pat-- = (c<<SHIFT_RED)|(c<<SHIFT_GREEN)|(c<<SHIFT_BLUE);
	    }
	  }
	}
    }
    break;
  case GL_LUMINANCE:
    {
      unsigned char *pat;
      for (i=0; i<DOTMAX; i++)
	{
	  /* Generated pattern is a quadrant of a disk. */
	  pat = (reinterpret_cast<unsigned char*>(pattern)) + (i+1) * dot_hsize * dot_hsize - 1;
	  r = (0.2 * i / DOTMAX + 0.8) * dot_hsize;
	  r = r*r;
	  for(y=0; y<dot_hsize; y++) {
	    for(x=0; x<dot_hsize; x++) {
	      c = 0;
	      for(u=0; u<4; u++) {
		p = static_cast<double>(u)/4.0 + y;
		p = p*p;
		for(v=0; v<4; v++) {
		  q = static_cast<double>(v)/4.0 + x;
		  if(p+q*q<r) {
		    c++;
		  }
		}
	      }
	      c = (c>15)?15:c;
	      c<<=4;
	      *pat-- = c;
	    }
	  }
	}
    }
    break;
  case GL_YUV422_GEM:
      {
      U16 *pat;
      const unsigned char chroma = 128;

      for (i=0; i<DOTMAX; i++)
	{
	  /* Generated pattern is a quadrant of a disk. */
	  pat = (reinterpret_cast<U16*>(pattern)) + (i+1) * dot_hsize * dot_hsize - 1;
	  r = (0.2 * i / DOTMAX + 0.8) * dot_hsize;
	  r = r*r;
	  for(y=0; y<dot_hsize; y++) {
	    for(x=0; x<dot_hsize; x++) {
	      c = 0;
	      for(u=0; u<4; u++) {
		p = static_cast<double>(u)/4.0 + y;
		p = p*p;
		for(v=0; v<4; v++) {
		  q = static_cast<double>(v)/4.0 + x;
		  if(p+q*q<r) {
		    c++;
		  }
		}
	      }
	      c = (c>15)?15:c;
	      c<<=4;
#ifdef __APPLE__
/* LATER fix the defines in GemPixPete.h instead of the code here ...*/
/* (the same goes for pix_halftone.cpp) */
	      *pat-- = (chroma<<SHIFT_V)|((c&0xff)<<SHIFT_Y2);
#else
	      *pat-- = (chroma<<SHIFT_U)|((c&0xff)<<SHIFT_Y1);
#endif
	    }
	  }
	}
    }  
    break;
  }
  
  /* The upper left part of a disk is needed, but generated pattern is a bottom
   * right part. So I spin the pattern. */
}

void pix_dot :: drawDot(int xx, int yy, unsigned char c, U32 *dest)
{
  int x, y;
  U32 *pat;

  c = (c>>(8-DOTDEPTH));
  pat = pattern + c * dot_hsize * dot_hsize;
  dest = dest + yy * dot_size * m_xsize + xx * dot_size;
  for(y=0; y<dot_hsize; y++) {
    for(x=0; x<dot_hsize; x++) {
      *dest++ = *pat++;
    }
    pat -= 2;
    for(x=0; x<dot_hsize-1; x++) {
      *dest++ = *pat--;
    }
    dest += m_xsize - dot_size + 1;
    pat += dot_hsize + 1;
  }
  pat -= dot_hsize*2;
  for(y=0; y<dot_hsize-1; y++) {
    for(x=0; x<dot_hsize; x++) {
      *dest++ = *pat++;
    }
    pat -= 2;
    for(x=0; x<dot_hsize-1; x++) {
      *dest++ = *pat--;
    }
    dest += m_xsize - dot_size + 1;
    pat += -dot_hsize + 1;
  }
}

void pix_dot :: drawDotYUV(int xx, int yy, unsigned char c, U16 *dest)
{
  int x, y;
  U16 *pat;

  c = (c>>(8-DOTDEPTH));
  pat = (reinterpret_cast<U16*>(pattern)) + c * dot_hsize * dot_hsize;
  dest = dest + yy * dot_size * m_xsize + xx * dot_size;
  for(y=0; y<dot_hsize; y++) {
    for(x=0; x<dot_hsize; x++) {
      *dest++ = *pat++;
    }
    pat -= 2;
    for(x=0; x<dot_hsize-1; x++) {
      *dest++ = *pat--;
    }
    dest += m_xsize - dot_size + 1;
    pat += dot_hsize + 1;
  }
  pat -= dot_hsize*2;
  for(y=0; y<dot_hsize-1; y++) {
    for(x=0; x<dot_hsize; x++) {
      *dest++ = *pat++;
    }
    pat -= 2;
    for(x=0; x<dot_hsize-1; x++) {
      *dest++ = *pat--;
    }
    dest += m_xsize - dot_size + 1;
    pat += -dot_hsize + 1;
  }
}

void pix_dot :: drawDotGray(int xx, int yy, unsigned char c, unsigned char *dest)
{
  int x, y;
  unsigned char *pat;

  c = (c>>(8-DOTDEPTH));
  pat = ((unsigned char*)pattern) + c * dot_hsize * dot_hsize;
  dest = dest + yy * dot_size * m_xsize + xx * dot_size;
  for(y=0; y<dot_hsize; y++) {
    for(x=0; x<dot_hsize; x++) {
      *dest++ = *pat++;
    }
    pat -= 2;
    for(x=0; x<dot_hsize-1; x++) {
      *dest++ = *pat--;
    }
    dest += m_xsize - dot_size + 1;
    pat += dot_hsize + 1;
  }
  pat -= dot_hsize*2;
  for(y=0; y<dot_hsize-1; y++) {
    for(x=0; x<dot_hsize; x++) {
      *dest++ = *pat++;
    }
    pat -= 2;
    for(x=0; x<dot_hsize-1; x++) {
      *dest++ = *pat--;
    }
    dest += m_xsize - dot_size + 1;
    pat += -dot_hsize + 1;
  }
}


/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_dot :: sizeMess(int width, int height)
{
  if(width>0)dots_width=width;
  else  error("width must be > 0!");
  if(height>0)dots_height=height;
  else  error("height must be > 0!");
  m_useScale=false;
  alreadyInit=0;
  myImage.setBlack();
  setPixModified();
}

unsigned char pix_dot :: inline_RGB2Y(int rgb)
{
	int i;

	i = R2Y[(rgb>>16)&0xff];
	i += G2Y[(rgb>>8)&0xff];
	i += B2Y[rgb&0xff];
	return i;
}

/////////////////////////////////////////////////////////
// processRGBAImage
//
/////////////////////////////////////////////////////////
void pix_dot :: processRGBAImage(imageStruct &image)
{
  U32 *src = reinterpret_cast<U32*>(image.data);
  U32 *dest;

  int x, y, sx, sy;

  if (m_xsize!=image.xsize || m_ysize!=image.ysize || m_csize!=image.csize) alreadyInit = 0;

    if (!alreadyInit)    {
        m_xsize = image.xsize;
        m_ysize = image.ysize;
	m_csize = image.csize;

	if(m_useScale){
	  dot_size = static_cast<int>(8 * m_scale);
	  dot_size = dot_size & 0xfe;
	  if(dot_size==0)dot_size=2;
	  dots_width = m_xsize / dot_size;
	  dots_height = m_ysize / dot_size;
	} else {
	  dot_size=m_xsize / dots_width;
	  if(dot_size==0){
	    dot_size=2;
	    dots_width  = m_xsize / dot_size;
	    dots_height = m_ysize / dot_size;	    
	  }
	}
	dot_hsize = dot_size / 2;
  
        pattern = (U32 *)malloc(DOTMAX * dot_hsize * dot_hsize * sizeof(U32));
        if (pattern == NULL) {
            error("couldn't make RGBA pattern");
            return;
        }
       
        sharedbuffer_init();
        sharedbuffer_reset();
        sampx = (int *)sharedbuffer_alloc(m_xsize*sizeof(int));
        sampy = (int *)sharedbuffer_alloc(m_ysize*sizeof(int));
        if (sampx == NULL || sampy == NULL ){
            return;
        }
        makePattern();
        sampxy_table_init();
    }
  myImage.xsize = image.xsize;
  myImage.ysize = image.ysize;
  myImage.setCsizeByFormat(image.format);
  myImage.reallocate();

  if(!alreadyInit){
    myImage.setBlack();
    alreadyInit = 1;
  }

  dest = reinterpret_cast<U32*>(myImage.data);
  for ( y=0; y<dots_height; y++) {
    sy = sampy[y];
    for ( x=0; x<dots_width; x++){
        sx = sampx[x];
        drawDot(x, y, inline_RGB2Y( src[sy*image.xsize+sx]), dest);
    }
  }

  image.data=myImage.data;
}

/////////////////////////////////////////////////////////
// processYUVImage
//
/////////////////////////////////////////////////////////
void pix_dot :: processYUVImage(imageStruct &image)
{
    U16 *dest;
    U16 *src = reinterpret_cast<U16*>(image.data);
    int x, y, sx, sy;
    int luma = 0;
    int luma2 = 0;
    int avgluma = 0;

    if (m_xsize!=image.xsize || m_ysize!=image.ysize || m_csize!=image.csize) alreadyInit = 0;
    
    if (!alreadyInit)
    {
        m_xsize = image.xsize;
        m_ysize = image.ysize;
	m_csize = image.csize;

        dot_size = static_cast<int>(8 * m_scale);
        dot_size = dot_size & 0xfe;
        dot_hsize = dot_size / 2;
        dots_width = m_xsize / dot_size;
        dots_height = m_ysize / dot_size;
  
        pattern = (U32 *)malloc(DOTMAX * dot_hsize * dot_hsize * sizeof(U32));
        if (pattern == NULL) {
            error("couldn't make YUV pattern");
            return;
        }
        
        sharedbuffer_init();
        sharedbuffer_reset();
        sampx = (int *)sharedbuffer_alloc(m_xsize*sizeof(int));
        sampy = (int *)sharedbuffer_alloc(m_ysize*sizeof(int));
        if (sampx == NULL || sampy == NULL ){
            return;
        }
        makePattern(GL_YUV422_GEM);
        sampxy_table_init();
    }

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();

    if(!alreadyInit){
      myImage.setBlack();
      alreadyInit = 1;
    }
     

    dest = reinterpret_cast<U16*>(myImage.data);

    for ( y=0; y<dots_height; y++) {
        sy = sampy[y];
        for ( x=0; x<dots_width; x++){
            sx = sampx[x];
            //sx2 = sampx[x+1];
            luma  = ((src[sy*image.xsize+sx+1])>>SHIFT_Y1)&0xff;
	    luma2 = ((src[sy*image.xsize+sx+1])>>SHIFT_Y2)&0xff;
	    avgluma = (luma + luma2)>>1;
            drawDotYUV(x, y, avgluma, dest);
            //drawDot(x+1, y, luma2, dest);
        }
    }
    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_dot :: processGrayImage(imageStruct &image)
{
    unsigned char *dest;
    unsigned char *src = (unsigned char*)image.data;
    int x, y, sx, sy;

    if (m_xsize!=image.xsize || m_ysize!=image.ysize || m_csize!=image.csize) alreadyInit = 0;
    
    if (!alreadyInit)
    {
        m_xsize = image.xsize;
        m_ysize = image.ysize;
	m_csize = image.csize;

        dot_size = static_cast<int>(8 * m_scale);
        dot_size = dot_size & 0xfe;
        dot_hsize = dot_size / 2;
        dots_width = m_xsize / dot_size;
        dots_height = m_ysize / dot_size;
  
        pattern = (U32 *)malloc(DOTMAX * dot_hsize * dot_hsize * sizeof(U32));
        if (pattern == NULL) {
            error("couldn't make luma pattern");
            return;
        }
        
        sharedbuffer_init();
        sharedbuffer_reset();
        sampx = (int *)sharedbuffer_alloc(m_xsize*sizeof(int));
        sampy = (int *)sharedbuffer_alloc(m_ysize*sizeof(int));
        if (sampx == NULL || sampy == NULL ){
            return;
        }
        makePattern(GL_LUMINANCE);
        sampxy_table_init();
    }

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();

    if(!alreadyInit){
      myImage.setBlack();
      alreadyInit = 1;
    }
     
    dest = (unsigned char*)myImage.data;

    for ( y=0; y<dots_height; y++) {
        sy = sampy[y];
        for ( x=0; x<dots_width; x++){
            sx = sampx[x];
            const char luma  = src[sy*image.xsize+sx+1];
            drawDotGray(x, y, luma, dest);
        }
    }
    image.data = myImage.data;
}



void pix_dot :: scaleMess(float state)
{
  if(state<=0.f){
    error("scale-factor must not be < 0!");
    return;
  }
  m_scale=state; /* used to be as (int)cast, but i have removed this...*/
  alreadyInit = 0;
  //myImage.reallocate(dataSize);we dont need to reallocate the image, since nothing changed
  myImage.setBlack();
  free(sharedbuffer); sharedbuffer=NULL;
  free(pattern); pattern=NULL;
  m_useScale=true;
  setPixModified();

}

void pix_dot :: sampxy_table_init()
{
    int i, j;

    j = dot_hsize;
    for(i=0; i<dots_width; i++) {
        sampx[i] = j; //* m_xsize;
        j += dot_size;
    }
    j = dot_hsize;
    for(i=0; i<dots_height; i++) {
        sampy[i] = j;// * m_ysize / screen_height;
        j += dot_size;
    }
}

void pix_dot :: yuv_init()
{
    static int initialized = 0;
    int i;
    if(!initialized) {
        for(i=20; i<256; i++) {
	  R2Y[i] =  static_cast<int>(0.257f*i);
	  G2Y[i] =  static_cast<int>(0.504f*i);
	  B2Y[i] =  static_cast<int>(0.098f*i);
        }
        initialized = 1;
    }
}

/////////////////////////////////////////////////////////
// shared buffer functions
//
/////////////////////////////////////////////////////////
int pix_dot :: sharedbuffer_init()
{
	/* maximum size of the frame buffer is for screen size x 2 */
	sharedbuffer_length = m_xsize * m_ysize * sizeof(U32) * 2;

	sharedbuffer = (unsigned char *)malloc(sharedbuffer_length);
	if(sharedbuffer == NULL)
		return -1;
	else
		return 0;
}

/* The effects uses shared buffer must call this function at first in
 * each effect registrar.
 */
void pix_dot :: sharedbuffer_reset()
{
	tail = 0;
}

/* Allocates size bytes memory in shared buffer and returns a pointer to the
 * memory. NULL is returned when the rest memory is not enough for the request.
 */
unsigned char* pix_dot :: sharedbuffer_alloc(int size)
{
	unsigned char *head;

	if(sharedbuffer_length - tail < size) {
		return NULL;
	}

	head = sharedbuffer + tail;
	tail += size;

	return head;
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_dot :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_dot::sizeMessCallback),
  		  gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_dot::scaleMessCallback),
                    gensym("scale"), A_FLOAT, A_NULL);
}


void pix_dot :: sizeMessCallback(void *data, t_floatarg width, t_floatarg height)
{
  GetMyClass(data)->sizeMess(static_cast<int>(width), static_cast<int>(height));  
}

void pix_dot :: scaleMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->scaleMess( state );
}
