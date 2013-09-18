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
// pix_biquad
//
//   IOhannes m zmoelnig
//   mailto:zmoelnig@iem.kug.ac.at
//
//   this code is published under the Gnu GeneralPublicLicense that should be distributed with gem & pd
//
/////////////////////////////////////////////////////////

#include "pix_biquad.h"
#include <string.h>
#include <math.h>

CPPEXTERN_NEW_WITH_GIMME(pix_biquad);

  /////////////////////////////////////////////////////////
//
// pix_biquad
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_biquad :: pix_biquad(int argc, t_atom*argv) :
  set(false), fb0(1), fb1(0), fb2(0), ff1(1), ff2(0), ff3(0), m_mode(0)
{ 
  prev.xsize = 64;
  prev.ysize = 64;
  prev.setCsizeByFormat(GL_RGBA);
  prev.reallocate();
  prev.setBlack();

  last.xsize = 64;
  last.ysize = 64;
  last.setCsizeByFormat(GL_RGBA);
  last.csize = 4;
  last.reallocate();
  last.setBlack();

  if(argc)faktorMess(argc, argv);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_biquad :: ~pix_biquad()
{
  // clean my buffer
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_biquad :: processRGBAImage(imageStruct &image)
{
  // assume that the pix_size does not change !
  bool do_blank=(image.xsize!=prev.xsize || image.ysize!=prev.ysize || image.csize!=prev.csize);
  prev.xsize = image.xsize;
  prev.ysize = image.ysize;
  prev.csize = image.csize;
  prev.reallocate();
  last.xsize = image.xsize;
  last.ysize = image.ysize;
  last.csize = image.csize;
  last.reallocate();

  if (set) { 
    memcpy(prev.data, image.data, image.ysize * image.xsize * image.csize);
    memcpy(last.data, image.data, image.ysize * image.xsize * image.csize);
    set = false;
  } else if (do_blank){
    prev.setBlack();
    last.setBlack();
  }

  int pixsize = image.ysize * image.xsize * image.csize;

  unsigned char *this_p = image.data;
  unsigned char *last_p= last.data;
  unsigned char *prev_p= prev.data;

  //  post("%f %f %f\t%f %f %f", fb0, fb1, fb2, ff1, ff2, ff3);

  if (m_mode){
    // slow, because calculations are done in float !
    while(pixsize--) {
      float output;
      output = fb0 * *this_p + fb1 * *last_p + fb2 * *prev_p;
      *this_p++    = (unsigned char)(ff1 * output + ff2 * *last_p + ff3 * *prev_p);
      *prev_p++	 = *last_p;
      *last_p++	 = (unsigned char)output;
    }
  }else{
    // fast, because calculations are done in int !
    int ifb0,ifb1,ifb2,iff1,iff2,iff3;
    int ioutput;
    ifb0 = static_cast<int>(256. * fb0);
    ifb1 = static_cast<int>(256. * fb1);
    ifb2 = static_cast<int>(256. * fb2);
    iff1 = static_cast<int>(256. * ff1);
    iff2 = static_cast<int>(256. * ff2);
    iff3 = static_cast<int>(256. * ff3);
    
    int max=0;//JMZ

    while(pixsize--) {
      ioutput = (((ifb0 * *this_p) + (ifb1 * *last_p) + (ifb2 * *prev_p))>>8);
      if(max<ioutput)max=ioutput;//JMZ
      *this_p++    = (unsigned char)CLAMP(((iff1 * ioutput) + (iff2 * *last_p) + (iff3 * *prev_p))>>8);
      *prev_p++	 = *last_p;
      *last_p++	 = (unsigned char)CLAMP(ioutput);
    }
    //post("maxval=%d", max);//JMZ
  }
}


/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_biquad :: processYUVImage(imageStruct &image)
{
    // assume that the pix_size does not change !
    bool do_blank=(image.xsize!=prev.xsize || image.ysize!=prev.ysize || image.csize!=prev.csize);
    prev.xsize = image.xsize;
    prev.ysize = image.ysize;
    prev.csize = image.csize;
    prev.reallocate();
    last.xsize = image.xsize;
    last.ysize = image.ysize;
    last.csize = image.csize;
    last.reallocate();

    if (set) {
        memcpy(prev.data, image.data, image.ysize * image.xsize * image.csize);
        memcpy(last.data, image.data, image.ysize * image.xsize * image.csize);
        set = false;
    } else if (do_blank){
        prev.setBlack();
        last.setBlack();
    }

    //unroll 4x
    int pixsize = (image.ysize * image.xsize * image.csize)/4;

    unsigned char *this_p = image.data;
    unsigned char *last_p= last.data;
    unsigned char *prev_p= prev.data;
 
    
        // fast, because calculations are done in int !
        int ifb0,ifb1,ifb2,iff1,iff2,iff3;
        int Youtput,UVoutput,Youtput1,UVoutput1;
        ifb0 = static_cast<int>(256. * fb0);
        ifb1 = static_cast<int>(256. * fb1);
        ifb2 = static_cast<int>(256. * fb2);
        iff1 = static_cast<int>(256. * ff1);
        iff2 = static_cast<int>(256. * ff2);
        iff3 = static_cast<int>(256. * ff3);

        //it's unrolled but GCC still can't schedule this well at all
        //needs some manual scheduling...
        
        while(pixsize--) {
            UVoutput  = (((ifb0 * (*this_p-128)) + (ifb1 * (*last_p-128)) + (ifb2 * (*prev_p-128)))>>8);
            *this_p++ = (unsigned char)CLAMP_Y((((iff1 * UVoutput) + (iff2 * (*last_p-128)) + (iff3 * (*prev_p-128)))>>8)+128);
            *prev_p++ = *last_p;
            *last_p++ = (unsigned char)CLAMP_Y(UVoutput+128);
            
            Youtput   = (((ifb0 * *this_p) + (ifb1 * *last_p) + (ifb2 * *prev_p))>>8);
            *this_p++ = (unsigned char)CLAMP_Y(((iff1 * Youtput) + (iff2 * *last_p) + (iff3 * *prev_p))>>8);
            *prev_p++ = *last_p;
            *last_p++ = (unsigned char)CLAMP_Y(Youtput);

            UVoutput1  = (((ifb0 * (*this_p-128)) + (ifb1 * (*last_p-128)) + (ifb2 * (*prev_p-128)))>>8);
            *this_p++ = (unsigned char)CLAMP_Y((((iff1 * UVoutput1) + (iff2 * (*last_p-128)) + (iff3 * (*prev_p-128)))>>8)+128);
            *prev_p++ = *last_p;
            *last_p++ = (unsigned char)CLAMP_Y(UVoutput1+128);
            
            Youtput1   = (((ifb0 * *this_p) + (ifb1 * *last_p) + (ifb2 * *prev_p))>>8);
            *this_p++ = (unsigned char)CLAMP_Y(((iff1 * Youtput1) + (iff2 * *last_p) + (iff3 * *prev_p))>>8);
            *prev_p++ = *last_p;
            *last_p++ = (unsigned char)CLAMP_Y(Youtput1);
        }
}

#ifdef __MMX__
/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_biquad :: processRGBAMMX(imageStruct &image)
{
  // assume that the pix_size does not change !
  bool do_blank=(image.xsize!=prev.xsize || image.ysize!=prev.ysize || image.csize!=prev.csize);
  prev.xsize = image.xsize;
  prev.ysize = image.ysize;
  prev.setCsizeByFormat(image.format);
  prev.reallocate();
  last.xsize = image.xsize;
  last.ysize = image.ysize;
  last.setCsizeByFormat(image.format);
  last.reallocate();

  if (set) { 
    memcpy(prev.data, image.data, image.ysize * image.xsize * image.csize);
    memcpy(last.data, image.data, image.ysize * image.xsize * image.csize);
    set = false;
  } else if (do_blank){
    prev.setBlack();
    last.setBlack();
  }
  int pixsize = image.ysize * image.xsize * image.csize / sizeof(__m64);
  
  //post("%f %f %f\t%f %f %f", fb0, fb1, fb2, ff1, ff2, ff3);
  //post("alpha-pre=%d", image.data[chAlpha]);

  // fast, because calculations are done in int !

  const short ifb0 = (short)(256.f * fb0);
  const short ifb1 = (short)(256.f * fb1);
  const short ifb2 = (short)(256.f * fb2);
  const short iff1 = (short)(256.f * ff1);
  const short iff2 = (short)(256.f * ff2);
  const short iff3 = (short)(256.f * ff3);

  //post("%d %d %d\t%d %d %d", ifb0, ifb1, ifb2, iff1, iff2, iff3);
  _mm_empty();

  const __m64 fb0_64 = _mm_set1_pi16(ifb0);
  const __m64 fb1_64 = _mm_set1_pi16(ifb1);
  const __m64 fb2_64 = _mm_set1_pi16(ifb2);
  const __m64 ff1_64 = _mm_set1_pi16(iff1);
  const __m64 ff2_64 = _mm_set1_pi16(iff2);
  const __m64 ff3_64 = _mm_set1_pi16(iff3);

  __m64 this_64, last_64, prev_64;
  __m64 a0,a1;
  __m64 b0,b1;
  __m64 A0,A1;
  __m64 B0,B1;
  
  __m64*this_p= (__m64*)image.data;
  __m64*last_p= (__m64*)last.data;
  __m64*prev_p= (__m64*)prev.data;
  
  __m64 null_64 = _mm_setzero_si64();
  
  /*
    float output = fb0 * *this_p + fb1 * *last_p + fb2 * *prev_p;
    *this_p++    = (unsigned char)(ff1 * output + ff2 * *last_p + ff3 * *prev_p);
    *prev_p++	 = *last_p;
    *last_p++	 = (unsigned char)output;
    */

  while(pixsize--) {
    this_64 = this_p[pixsize];
    last_64 = last_p[pixsize];
    prev_64 = prev_p[pixsize];
    
    a0=_mm_unpacklo_pi8 (this_64, null_64);
    a1=_mm_unpackhi_pi8 (this_64, null_64);

    b0=_mm_unpacklo_pi8 (last_64, null_64);
    b1=_mm_unpackhi_pi8 (last_64, null_64);

    a0 = _mm_mullo_pi16 (a0, fb0_64);
    a1 = _mm_mullo_pi16 (a1, fb0_64);

    b0 = _mm_mullo_pi16 (b0, fb1_64);
    b1 = _mm_mullo_pi16 (b1, fb1_64);

    a0 = _mm_adds_pu16  (a0, b0); // this might already by saturated !
    a1 = _mm_adds_pu16  (a1, b1);

    b0 =_mm_unpacklo_pi8(prev_64, null_64);
    b1 =_mm_unpackhi_pi8(prev_64, null_64);
    b0 = _mm_mullo_pi16 (b0, fb2_64);
    b1 = _mm_mullo_pi16 (b1, fb2_64);

    a0 = _mm_adds_pu16  (a0, b0);
    a1 = _mm_adds_pu16  (a1, b1);

    a0 = _mm_srli_pi16  (a0, 8);
    a1 = _mm_srli_pi16  (a1, 8);



    A0=a0;
    A1=a1;

    B0=_mm_unpacklo_pi8 (last_64, null_64);
    B1=_mm_unpackhi_pi8 (last_64, null_64);

    A0 = _mm_mullo_pi16 (A0, ff1_64);
    A1 = _mm_mullo_pi16 (A1, ff1_64);

    B0 = _mm_mullo_pi16 (B0, ff2_64);
    B1 = _mm_mullo_pi16 (B1, ff2_64);

    A0 = _mm_adds_pu16  (A0, B0); // this might already By saturated !
    A1 = _mm_adds_pu16  (A1, B1);

    B0 =_mm_unpacklo_pi8(prev_64, null_64);
    B1 =_mm_unpackhi_pi8(prev_64, null_64);
    B0 = _mm_mullo_pi16 (B0, ff3_64);
    B1 = _mm_mullo_pi16 (B1, ff3_64);

    A0 = _mm_adds_pu16  (A0, B0);
    A1 = _mm_adds_pu16  (A1, B1);

    A0 = _mm_srli_pi16  (A0, 8);
    A1 = _mm_srli_pi16  (A1, 8);

    prev_p[pixsize]=last_p[pixsize];
    last_p[pixsize]=_mm_packs_pu16(a0, a1);
    this_p[pixsize]=_mm_packs_pu16(A0, A1);
  }
  _mm_empty();
  //  post("alpha-post=%d\n", image.data[chAlpha]);
}
void pix_biquad :: processYUVMMX(imageStruct &image)
{ processRGBAMMX(image); }
void pix_biquad :: processGrayMMX(imageStruct &image)
{ processRGBAMMX(image); }

#endif /* __MMX__ */

#ifdef __VEC__
/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_biquad :: processYUVAltivec(imageStruct &image)
{
    // assume that the pix_size does not change !
    bool do_blank=(image.xsize!=prev.xsize || image.ysize!=prev.ysize || image.csize!=prev.csize);
    prev.xsize = image.xsize;
    prev.ysize = image.ysize;
    prev.csize = image.csize;
    prev.reallocate();
    last.xsize = image.xsize;
    last.ysize = image.ysize;
    last.csize = image.csize;
    last.reallocate();

    if (set) {
        memcpy(prev.data, image.data, image.ysize * image.xsize * image.csize);
        memcpy(last.data, image.data, image.ysize * image.xsize * image.csize);
        set = false;
    } else if (do_blank){
        prev.setBlack();
        last.setBlack();
    }

    /*
    union {
        unsigned char		c[16];
        vector unsigned char 	v;
    }charBuffer;
*/
    union {
        signed short		s[8];
        vector signed short 	v;
    }shortBuffer;

    union {
        signed int		i[4];
        vector signed int 	v;
    }intBuffer;
    
    //unroll 4x
    int pixsize = (image.ysize * image.xsize * image.csize)/16;

    vector unsigned char *this_p = (vector unsigned char *) image.data;
    vector unsigned char *last_p = (vector unsigned char *) last.data;
    vector unsigned char *prev_p = (vector unsigned char *) prev.data;

    vector unsigned char one = vec_splat_u8(1),output;
    vector signed short UVoffset;
    vector signed short Ythis, UVthis, Ylast, UVlast, Yprev, UVprev;
    vector signed int  Yt0, Yt1, UVt0, UVt1, Yl0, Yl1, UVl0, UVl1, Yp0, Yp1, UVp0, UVp1,intOffset;
    vector signed short ifb0, ifb1, ifb2, iff1, iff2, iff3;
    vector unsigned int shift;
    vector signed int loImage, hiImage;

    intBuffer.i[0] = 128;
    intOffset = intBuffer.v;
    intOffset = vec_splat(intOffset,0);

    intBuffer.i[0] = 8;
    shift = (vector unsigned int)intBuffer.v;
    shift = vec_splat(shift,0);
    
    shortBuffer.s[0] = 128;
    UVoffset = shortBuffer.v;
    UVoffset = vec_splat(UVoffset,0);

    shortBuffer.s[0] = (short)(256. * fb0);
    ifb0 = shortBuffer.v;
    ifb0 = vec_splat(ifb0,0);

    shortBuffer.s[0] = (short)(256. * fb1);
    ifb1 = shortBuffer.v;
    ifb1 = vec_splat(ifb1,0);

    shortBuffer.s[0] = (short)(256. * fb2);
    ifb2 = shortBuffer.v;
    ifb2 = vec_splat(ifb2,0);

    shortBuffer.s[0] = (short)(256. * ff1);
    iff1 = shortBuffer.v;
    iff1 = vec_splat(iff1,0);

    shortBuffer.s[0] = (short)(256. * ff2);
    iff2 = shortBuffer.v;
    iff2 = vec_splat(iff2,0);

    shortBuffer.s[0] = (short)(256. * ff3);
    iff3 = shortBuffer.v;
    iff3 = vec_splat(iff3,0);
    
    //setup the cache prefetch -- A MUST!!!
    //this gave a 30-40% speedup - the gain so far is about 450%
#ifndef PPC970
    UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
    vec_dst( this_p, prefetchSize, 0 );
    vec_dst( last_p, prefetchSize, 1 );
    vec_dst( prev_p, prefetchSize, 2 );
#endif //PPC970

    while(pixsize--) {

#ifndef PPC970
        vec_dst( this_p, prefetchSize, 0 );
        vec_dst( last_p, prefetchSize, 1 );
        vec_dst( prev_p, prefetchSize, 2 );
#endif //PPC970
       // UVoutput  = (((ifb0 * (*this_p-128)) + (ifb1 * (*last_p-128)) + (ifb2 * (*prev_p-128)))>>8);

         //mult into UV and Y
        UVthis = (vector signed short)vec_mule(this_p[0],one);
        Ythis = (vector signed short)vec_mulo(this_p[0],one);

        UVlast = (vector signed short)vec_mule(last_p[0],one);
        Ylast = (vector signed short)vec_mulo(last_p[0],one);

        UVprev = (vector signed short)vec_mule(prev_p[0],one);
        Yprev = (vector signed short)vec_mulo(prev_p[0],one);
        
        
         //subtract -128 offset from UV

        UVthis = vec_sub(UVthis,UVoffset);

        UVlast = vec_sub(UVlast,UVoffset);

        UVprev = vec_sub(UVprev,UVoffset);

        //pack back to chars???  this would use less registers - maybe allow for unrolling?
        
        //multiply by coeffecients into ints
        Yt0 = vec_mule(Ythis,ifb0);
        Yt1 = vec_mulo(Ythis,ifb0);

        Yp0 = vec_mule(Yprev,ifb2);
        Yp1 = vec_mulo(Yprev,ifb2);

        Yl0 = vec_mule(Ylast,ifb1);
        Yl1 = vec_mulo(Ylast,ifb1);

        UVt0 = vec_mule(UVthis,ifb0);
        UVt1 = vec_mulo(UVthis,ifb0);

        UVp0 = vec_mule(UVprev,ifb2);
        UVp1 = vec_mulo(UVprev,ifb2);

        UVl0 = vec_mule(UVlast,ifb1);
        UVl1 = vec_mulo(UVlast,ifb1);
        
         //add  

        Yt0 = vec_adds(vec_adds(Yl0,Yp0),Yt0);
        Yt1 = vec_adds(vec_adds(Yl1,Yp1),Yt1);

        UVt0 = vec_adds(vec_adds(UVl0,UVp0),UVt0);
        UVt1 = vec_adds(vec_adds(UVl1,UVp1),UVt1);

         //shift down by 8
        Yt0 = vec_sra(Yt0,shift);
        UVt0 = vec_sra(UVt0,shift);
        Yt1 = vec_sra(Yt1,shift);
        UVt1 = vec_sra(UVt1,shift);
        UVt0 = vec_adds(UVt0, intOffset);
        UVt1 = vec_adds(UVt1, intOffset);

        //merge then pack back to short
        
        hiImage = vec_mergeh(Yt0,Yt1);
        loImage = vec_mergel(Yt0,Yt1);

        Ythis = vec_packs(hiImage,loImage);

        hiImage = vec_mergeh(UVt0,UVt1);
        loImage = vec_mergel(UVt0,UVt1);

        UVthis = vec_packs(hiImage,loImage);
         
        output = vec_packsu(vec_mergeh(UVthis,Ythis),  vec_mergel(UVthis,Ythis));

        //restore UV offset for next set of processing
        UVthis = vec_subs(UVthis,UVoffset);
        
        /* 
        *this_p++ = (unsigned char)CLAMP_Y((((iff1 * UVoutput) + (iff2 * (*last_p-128)) + (iff3 * (*prev_p-128)))>>8)+128);
         */

        Yt0 = vec_mule(Ythis,iff1);
        Yt1 = vec_mulo(Ythis,iff1);

        Yp0 = vec_mule(Yprev,iff3);
        Yp1 = vec_mulo(Yprev,iff3);

        Yl0 = vec_mule(Ylast,iff2);
        Yl1 = vec_mulo(Ylast,iff2);

        UVt0 = vec_mule(UVthis,iff1);
        UVt1 = vec_mulo(UVthis,iff1);

        UVp0 = vec_mule(UVprev,iff3);
        UVp1 = vec_mulo(UVprev,iff3);

        UVl0 = vec_mule(UVlast,iff2);
        UVl1 = vec_mulo(UVlast,iff2);

        //add

        Yt0 = vec_adds(vec_adds(Yl0,Yp0),Yt0);
        Yt1 = vec_adds(vec_adds(Yl1,Yp1),Yt1);

        UVt0 = vec_adds(vec_adds(UVl0,UVp0),UVt0);
        UVt1 = vec_adds(vec_adds(UVl1,UVp1),UVt1);

        //shift down by 8
        Yt0 = vec_sra(Yt0,shift);
        UVt0 = vec_sra(UVt0,shift);
        Yt1 = vec_sra(Yt1,shift);
        UVt1 = vec_sra(UVt1,shift);
        UVt0 = vec_adds(UVt0, intOffset);
        UVt1 = vec_adds(UVt1, intOffset);

        //pack back to short
        hiImage = vec_mergeh(Yt0,Yt1);
        loImage = vec_mergel(Yt0,Yt1);

        Ythis = vec_packs(hiImage,loImage);

        hiImage = vec_mergeh(UVt0,UVt1);
        loImage = vec_mergel(UVt0,UVt1);

        UVthis = vec_packs(hiImage,loImage);

        this_p[0] = vec_packsu((vector signed short) vec_mergeh(UVthis,Ythis), (vector signed short) vec_mergel(UVthis,Ythis));

        prev_p[0] = last_p[0];
        last_p[0] = output;

        this_p++;
        prev_p++;
        last_p++;
        
    }
#ifndef PPC970
    vec_dss(2);
    vec_dss(1);
    vec_dss(0);
#endif
}
#endif /* __VEC__ */


void pix_biquad :: faktorMess(int argc, t_atom*argv){
  if (argc<5 || argc>6){
    error("illegal number of arguments");
    return;
  }

  if (argc==6)fb0=atom_getfloat(argv++);
  else fb0=1;

  fb1=atom_getfloat(argv++);
  fb2=atom_getfloat(argv++);
  ff1=atom_getfloat(argv++);
  ff2=atom_getfloat(argv++);
  ff3=atom_getfloat(argv++);
}

///////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_biquad :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_biquad::setMessCallback),
		  gensym("set"), A_NULL);
                  
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_biquad::modeMessCallback),
		  gensym("mode"), A_DEFFLOAT, A_NULL);
  class_addlist(classPtr, reinterpret_cast<t_method>(&pix_biquad::faktorMessCallback));
}

void pix_biquad :: faktorMessCallback(void *data, t_symbol *s, int argc, t_atom* argv)
{
  GetMyClass(data)->faktorMess(argc, argv);
}

void pix_biquad :: setMessCallback(void *data)
{
  GetMyClass(data)->set = true;
}

void pix_biquad :: modeMessCallback(void *data, float value)
{
  GetMyClass(data)->m_mode = static_cast<int>(value);
}
