////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
//
// pix_movement
//
//
/////////////////////////////////////////////////////////

#include "pix_movement.h"
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef __APPLE__
# include <Carbon/Carbon.h>
#endif

#include "Gem/PixConvert.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_movement,t_floatarg, A_DEFFLOAT);

  /////////////////////////////////////////////////////////
//
// pix_movement
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_movement :: pix_movement(t_floatarg f)
{ 
  buffer.xsize  = buffer.ysize = 64;
  buffer.setCsizeByFormat(GL_LUMINANCE);
  buffer.reallocate();
  buffer2.xsize  = buffer2.ysize = 64;
  buffer2.setCsizeByFormat(GL_LUMINANCE);
  buffer2.reallocate();

  if(f<=0.)f=0.5;
  if(f>1.f)f=1.0;
  threshold = (unsigned char)(255*f);
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("thresh"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_movement :: ~pix_movement()
{
  // clean my buffer
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_movement :: processRGBAImage(imageStruct &image)
{
  // assume that the pix_size does not change !
  bool doclear=(image.xsize*image.ysize != buffer.xsize*buffer.ysize);
  buffer.xsize = image.xsize;
  buffer.ysize = image.ysize;
  buffer.reallocate();
  if(doclear) buffer.setWhite();

  int pixsize = image.ysize * image.xsize;

  unsigned char *rp = image.data; // read pointer
  unsigned char *wp=buffer.data; // write pointer

  while(pixsize--) {
    //    unsigned char grey = (unsigned char)(rp[chRed] * 0.3086f + rp[chGreen] * 0.6094f + rp[chBlue] * 0.0820f);
    //   rp[chAlpha] = 255*(fabs((unsigned char)grey-*wp)>threshold);
    unsigned char grey = (rp[chRed]*RGB2GRAY_RED+rp[chGreen]*RGB2GRAY_GREEN+rp[chBlue]*RGB2GRAY_BLUE)>>8;
    rp[chAlpha] = 255*(abs(grey-*wp)>threshold);
    *wp++=(unsigned char)grey;
    rp+=4;
  } 
}
void pix_movement :: processYUVImage(imageStruct &image)
{
  // assume that the pix_size does not change !
  bool doclear=(image.xsize*image.ysize != buffer.xsize*buffer.ysize);
  buffer.xsize = image.xsize;
  buffer.ysize = image.ysize;
  buffer.reallocate();
  if(doclear)buffer.setWhite();
  
  int pixsize = image.ysize * image.xsize;

  int Y1, Y0;
  unsigned char thresh;
  //these get rid of the invariant loads of the global chY0 and chY1
  Y1 = chY1;
  Y0 = chY0;
  thresh = threshold;
  
  unsigned char *rp = image.data; // read pointer
  unsigned char *wp=buffer.data; // write pointer to the copy
  unsigned char grey,grey1;

  grey  = 0;
  grey1 = 0;
  pixsize/=2;
  while(pixsize--) {
    grey = rp[Y0];
    rp[Y0]=CLAMP_Y(255*(abs(grey-*wp)>thresh));
    *wp++=grey;

    grey1 = rp[Y1];
    rp[Y1]=CLAMP_Y(255*(abs(grey1-*wp)>thresh));
    *wp++=grey1;

    // looks cool (C64), but what for ?

    if(true) { // black&white
      rp[chU]=128;
      rp[chV]=128;
    }
    rp+=4;
  }
}

#ifdef __VEC__
void pix_movement :: processYUVAltivec(imageStruct &image)
{
    if (image.xsize*image.ysize != buffer.xsize*buffer.ysize){
        buffer.xsize = image.xsize;
        buffer.ysize = image.ysize;
        buffer.reallocate(buffer.xsize*buffer.ysize*2);
    }
    int pixsize = image.ysize * image.xsize/8;

    union{
        signed short  c[8];
        vector signed short  v;
    }shortBuffer;

    union{
        unsigned short  c[8];
        vector unsigned short  v;
    }ushortBuffer;

    int i;

    vector signed short thresh;
    shortBuffer.c[0] = threshold;
    thresh = shortBuffer.v;
    thresh = (vector signed short)vec_splat(thresh,0);

    vector unsigned char *rp = (vector unsigned char *) image.data; // read pointer
    vector unsigned char *wp = (vector unsigned char *) buffer.data; // write pointer to the copy
    vector unsigned char grey0,grey1;
    vector unsigned char one = vec_splat_u8(1);
    vector unsigned short Y0,Ywp0,hiImage0,loImage0;
    vector unsigned short Y1,Ywp1,hiImage1,loImage1;
    vector unsigned short UVwp0,UVwp1;
    vector signed short temp0,temp1;

    ushortBuffer.c[0]=127;
    vector unsigned short UV0= (vector unsigned short)vec_splat(ushortBuffer.v, 0);
    vector unsigned short UV1= (vector unsigned short)vec_splat(ushortBuffer.v, 0);

#ifndef PPC970
    //setup the cache prefetch -- A MUST!!!
    UInt32 prefetchSize = GetPrefetchConstant( 16, 0, 256 );
    vec_dst( rp, prefetchSize, 0 );
    vec_dst( wp, prefetchSize, 1 );
#endif

    int j = 16;
    
    pixsize/=2;
    for (i=0; i < pixsize; i++) {
# ifndef PPC970
        //setup the cache prefetch -- A MUST!!!
        UInt32 prefetchSize = GetPrefetchConstant( j, 0, j * 16 );
        vec_dst( rp, prefetchSize, 0 );
        vec_dst( wp, prefetchSize, 1 );
        vec_dst( rp+16, prefetchSize, 2 );
        vec_dst( wp+16, prefetchSize, 3 );
# endif
        
        grey0 = rp[0];
        grey1 = rp[1];
            
//      rp[Y0]=255*(abs(grey0-*wp)>thresh);

//      UV0= (vector unsigned short)vec_mule(grey0,one);
        Y0 = (vector unsigned short)vec_mulo(grey0,one);

//      UV1= (vector unsigned short)vec_mule(grey1,one);
        Y1 = (vector unsigned short)vec_mulo(grey1,one);

        //wp is actually 1/2 the size of the image because it is only Y??
        
        //here the full U Y V Y is stored
//      UVwp0= (vector unsigned short)vec_mule(wp[0],one);
        Ywp0 = (vector unsigned short)vec_mulo(wp[0],one);

//      UVwp1= (vector unsigned short)vec_mule(wp[1],one);
        Ywp1 = (vector unsigned short)vec_mulo(wp[1],one);

        //store the current pixels as the history for next time
        wp[0]=grey0;
        wp++;
        wp[0]=grey1;
        wp++;

        temp0 = vec_abs(vec_sub((vector signed short)Y0,(vector signed short)Ywp0));
        Y0 = (vector unsigned short)vec_cmpgt(temp0,thresh);

        temp1 = vec_abs(vec_sub((vector signed short)Y1,(vector signed short)Ywp1));
        Y1 = (vector unsigned short)vec_cmpgt(temp1,thresh);
       
        hiImage0 = vec_mergeh(UV0,Y0);
        loImage0 = vec_mergel(UV0,Y0);

        hiImage1 = vec_mergeh(UV1,Y1);
        loImage1 = vec_mergel(UV1,Y1);
        
        grey0 = vec_packsu(hiImage0,loImage0);
        grey1 = vec_packsu(hiImage1,loImage1);
        
        rp[0]=grey0;
        rp++;
        rp[0]=grey1;
        rp++;
       // grey = rp[0];
       // rp[Y1]=255*(abs(grey-*wp)>thresh);
       // *wp++=grey;
        
       // rp+=4;
       // rp++;
    }
    
# ifndef PPC970
    vec_dss(0);
    vec_dss(1);
    vec_dss(2);
    vec_dss(3);
# endif
}
#endif /* __VEC__ */



void pix_movement :: processGrayImage(imageStruct &image)
{
  // assume that the pix_size does not change !
  bool doclear=(image.xsize*image.ysize != buffer.xsize*buffer.ysize);
  buffer.xsize = image.xsize;
  buffer.ysize = image.ysize;
  buffer.reallocate();
  if(doclear) buffer.setWhite();
  buffer2.xsize = image.xsize;
  buffer2.ysize = image.ysize;
  buffer2.reallocate();

  int pixsize = image.ysize * image.xsize;

  unsigned char *rp = image.data; // read pointer
  unsigned char *wp=buffer.data;  // write pointer to the copy
  unsigned char *wp2=buffer2.data; // write pointer to the diff-image

  while(pixsize--) {
    unsigned char grey = *rp++;
    *wp2++=255*(abs(grey-*wp)>threshold);
    //*wp2++=(abs(grey-*wp));
    *wp++=grey;
  }
  image.data = buffer2.data;
}
#ifdef __MMX__
void pix_movement :: processGrayMMX(imageStruct &image)
{
  // assume that the pix_size does not change !
  bool doclear=(image.xsize*image.ysize != buffer.xsize*buffer.ysize);
  buffer.xsize = image.xsize;
  buffer.ysize = image.ysize;
  buffer.reallocate();
  if(doclear) buffer.setWhite();
  buffer2.xsize = image.xsize;
  buffer2.ysize = image.ysize;
  buffer2.reallocate();

  int pixsize = image.ysize * image.xsize / sizeof(__m64);

  unsigned char thresh=threshold;

  __m64*rp = (__m64*)image.data; // read pointer
  __m64*wp = (__m64*)buffer.data; // write pointer to the copy
  __m64*wp2= (__m64*)buffer2.data;      // write pointer to the diff-image

  __m64 m1, m2, grey;
  __m64 thresh8=_mm_set_pi8(thresh,thresh,thresh,thresh,
                            thresh,thresh,thresh,thresh);

  // there is still one problem with the threshold: is the cmpgt only for signed ?
  while(pixsize--) {
    grey = rp[pixsize]; // image.data
    m2   = wp[pixsize]; // buffer.data

    //m0 =_mm_cmpgt_pi8(grey, m2); // (grey>m2)
    //m1 =_mm_subs_pu8 (grey, m2); // (grey-m2)
    //m2 =_mm_subs_pu8 (m2, grey); // (m2-grey)
    //m1 =_mm_and_si64   (m1, m0); // (m2-grey)&(grey>m2)   ((??))
    //m0 =_mm_andnot_si64(m0, m2); // !(grey>m2)&(grey-m2)  ((??))
    //m2 =_mm_or_si64    (m2, m0); // [(a-b)&(a>b)]|[(b-a)&!(a>b)]=abs(a-b)

    // this is better: use saturated arithmetic!

    m1 =_mm_subs_pu8 (grey, m2); // (grey-m2)
    m2 =_mm_subs_pu8 (m2, grey); // (m2-grey)
    wp[pixsize]=grey; // buffer.data

    m2 = _mm_or_si64 (m2, m1); // |grey-m2|

    m2 =_mm_subs_pu8 (m2, thresh8);
    m2 =_mm_cmpgt_pi8(m2, _mm_setzero_si64());

    wp2[pixsize]=m2;  // output.data
  }
  _mm_empty();
  image.data = buffer2.data;
}
#endif
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_movement :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movement::threshMessCallback),
                  gensym("threshold"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movement::threshMessCallback),
                  gensym("thresh"), A_FLOAT, A_NULL);
}
void pix_movement :: threshMessCallback(void *data, t_floatarg newmode)
{
  GetMyClass(data)->threshold=CLAMP((float)255.*newmode);
}
