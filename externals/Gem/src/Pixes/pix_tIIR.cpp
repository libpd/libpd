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
// pix_tIIR
//
//   IOhannes m zmoelnig
//   mailto:zmoelnig@iem.kug.ac.at
//
//   this code is published under the Gnu GeneralPublicLicense that should be distributed with gem & pd
//
/////////////////////////////////////////////////////////

#include "pix_tIIR.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_tIIR, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_tIIR
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_tIIR :: pix_tIIR(t_floatarg fb_numf, t_floatarg ff_numf)
{ 
  int fb_num = (fb_numf>0.)?static_cast<int>(fb_numf):0;
  int ff_num = (ff_numf>0.)?static_cast<int>(ff_numf):0;
  ff_count=ff_num;fb_count=fb_num;
  fb_num++;ff_num++;
  m_inlet = new t_inlet*[fb_num+ff_num];
  t_inlet **inlet = m_inlet;

  m_fb = new t_float[fb_num];
  m_ff = new t_float[ff_num];

  int i=0;
  while(i<fb_num){
    m_fb[i]=0.0;
    *inlet++=floatinlet_new(this->x_obj, m_fb+i);
    i++;
  }
  m_fb[0]=1.0;
  i=0;
  while(i<ff_num){
    m_ff[i]=0.0;
    *inlet++=floatinlet_new(this->x_obj, m_ff+i);
    i++;
  }
  m_ff[0]=1.0;

  set = false;
  set_zero = false;

  m_bufnum=(fb_num>ff_num)?fb_num:ff_num;
  m_counter=0;

  m_buffer.xsize=64;
  m_buffer.ysize=64;
  m_buffer.csize=4;
  m_buffer.format=GL_RGBA;
  m_buffer.allocate(m_buffer.xsize*m_buffer.ysize*m_buffer.csize*m_bufnum);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_tIIR :: ~pix_tIIR()
{
  // clean my buffer
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_tIIR :: processImage(imageStruct &image)
{
  t_float f;
  int i, j;
  int imagesize = image.xsize*image.ysize*image.csize;
  unsigned char *dest, *source;

  // assume that the pix_size does not change !
  // if (oldsize<newsize){}
  dest=m_buffer.data;
  m_buffer.reallocate(image.xsize*image.ysize*image.csize*m_bufnum);
  if (m_buffer.xsize!=image.xsize || m_buffer.ysize!=image.ysize || m_buffer.format!=image.format){
    m_buffer.xsize=image.xsize;
    m_buffer.ysize=image.ysize;
    m_buffer.csize=image.csize;
    m_buffer.format=image.format;

    set=true;
    set_zero=true;
  }

  // set!(if needed)
  if (set){
    if (set_zero)m_buffer.setBlack();
    else{
      j=m_bufnum;
      while(j--){
	source=image.data;
	dest=m_buffer.data+j*imagesize;
	i=imagesize;while(i--)*dest++=*source++;
      }
    }
    set=false;
    set_zero=false;
  }  

  // do the filtering
  // feed-back
  f=m_fb[0];
  source=image.data;
  dest=m_buffer.data+m_counter*imagesize;
  int factor=static_cast<int>(f*256.);
  i=imagesize;while(i--)    *dest++ = (unsigned char)((factor**source++)>>8);
  j=fb_count;while(j--){
    f=m_fb[j+1];
    source=m_buffer.data+imagesize*((m_bufnum+m_counter-j-1)%m_bufnum);
    dest=m_buffer.data+m_counter*imagesize;
    factor=static_cast<int>(f*256.);
    if (factor!=0){
      i=imagesize;while(i--)*dest++ += (unsigned char)((factor**source++)>>8);
    }
  }

  // feed-forward
  f=m_ff[0];
  source=m_buffer.data+m_counter*imagesize;
  dest=image.data;
  factor=static_cast<int>(f*256.);
  i=imagesize;while(i--)*dest++ = (unsigned char)((factor**source++)>>8);
  j=ff_count;while(j--){
    f=m_ff[j+1];
    dest=image.data;
    source=m_buffer.data+imagesize*((m_bufnum+m_counter-j-1)%m_bufnum);
    factor=static_cast<int>(f*256.);
    if (factor!=0){
      i=imagesize;while(i--)*dest++ += (unsigned char)((factor**source++)>>8);
    }
  }

  m_counter++;
  m_counter%=m_bufnum;
}

#ifdef __MMX__
void pix_tIIR :: processRGBAMMX(imageStruct &image)
{
  int i, j;

  short *s_ff = new short[ff_count+1];
  short *s_fb = new short[fb_count+1];

  i=ff_count+1;while(i--){
    s_ff[i]=static_cast<short>(m_ff[i]*256.+0.5);
  }
  i=fb_count+1;while(i--){
    s_fb[i]=static_cast<short>(m_fb[i]*256.+0.5);
  }


  int imagesize = image.xsize*image.ysize*image.csize;
  __m64 *dest, *source;

  imagesize=imagesize/(sizeof(__m64))+((imagesize%sizeof(__m64))!=0);

  // assume that the pix_size does not change !
  // if (oldsize<newsize){}
  if (m_buffer.xsize!=image.xsize || m_buffer.ysize!=image.ysize || m_buffer.format!=image.format){
    m_buffer.xsize=image.xsize;
    m_buffer.ysize=image.ysize;
    m_buffer.setCsizeByFormat(image.format);

    set=true;
    set_zero=true;
  }
  m_buffer.reallocate(imagesize*m_bufnum*sizeof(__m64));
  dest=(__m64*)m_buffer.data;

  // set!(if needed)
  if (set){
    if (set_zero)m_buffer.setBlack();
    else{
      j=m_bufnum;
      while(j--){
	source=(__m64*)image.data;
	dest=((__m64*)m_buffer.data)+j*imagesize;
	i=imagesize;while(i--)dest[i]=source[i];
      }
    }
    set=false;
    set_zero=false;
  }  

  // do the filtering
  // feed-back
  source=(__m64*)image.data;
  dest =((__m64*)m_buffer.data)+m_counter*imagesize;
  __m64 null_64=_mm_setzero_si64();
  __m64 factor=_mm_set1_pi16(s_fb[0]);

  __m64 a0, a1, b0, b1;
  _mm_empty();
  i=imagesize;while(i--){
    a0= source[i];
    a1=_mm_unpackhi_pi8 (a0, null_64);
    a0=_mm_unpacklo_pi8 (a0, null_64);
    a1=_mm_mullo_pi16   (a1, factor);
    a0=_mm_mullo_pi16   (a0, factor);
    a1=_mm_srli_pi16    (a1, 8);
    a0=_mm_srli_pi16    (a0, 8);
    a0=_mm_packs_pu16   (a0, a1);
    dest[i] = a0;
  } 

  j=fb_count;while(j--){
    if (s_fb[j+1]!=0){
      source=((__m64*)m_buffer.data)+imagesize*((m_bufnum+m_counter-(j+1))%m_bufnum);
      dest=  ((__m64*)m_buffer.data)+m_counter*imagesize;
      factor =_mm_set1_pi16(s_fb[j+1]);
      null_64=_mm_setzero_si64();
      i=imagesize;while(i--){
	a0 = source[i];
	b0 = dest[i];

	a1=_mm_unpackhi_pi8 (a0, null_64);
	a0=_mm_unpacklo_pi8 (a0, null_64);
	b1=_mm_unpackhi_pi8 (b0, null_64);
	b0=_mm_unpacklo_pi8 (b0, null_64);

	a1=_mm_mullo_pi16   (a1, factor);
	a0=_mm_mullo_pi16   (a0, factor);

	a1=_mm_srli_pi16    (a1, 8);
	a0=_mm_srli_pi16    (a0, 8);

	a1=_mm_adds_pu16    (a1, b1);
	a0=_mm_adds_pu16    (a0, b0);

	a0=_mm_packs_pu16   (a0, a1);

	dest[i]=a0;
	// *dest++ += (unsigned char)((factor**source++)>>8);
      }
    }
  }
  _mm_empty();

  // feed-forward
  source=((__m64*)m_buffer.data)+m_counter*imagesize;
  dest=(__m64*)image.data;
  factor =_mm_set1_pi16(s_ff[0]);
  null_64=_mm_setzero_si64();
  i=imagesize;while(i--){
    a0= source[i];
    a1=_mm_unpackhi_pi8 (a0, null_64);
    a0=_mm_unpacklo_pi8 (a0, null_64);
    a1=_mm_mullo_pi16   (a1, factor);
    a0=_mm_mullo_pi16   (a0, factor);
    a1=_mm_srli_pi16    (a1, 8);
    a0=_mm_srli_pi16    (a0, 8);
    a0=_mm_packs_pu16   (a0, a1);
    dest[i] = a0;
    //*dest++ = (unsigned char)((factor**source++)>>8);
  }  
  _mm_empty();


  j=ff_count;while(j--){
    if (s_ff[j+1]!=0){
      dest=(__m64*)image.data;
      source=((__m64*)m_buffer.data)+imagesize*((m_bufnum+m_counter-j-1)%m_bufnum);
      factor =_mm_set1_pi16(s_ff[j+1]);
      null_64=_mm_setzero_si64();
      i=imagesize;while(i--){
	a0 = source[i];
	b0 = dest[i];

	a1=_mm_unpackhi_pi8 (a0, null_64);
	a0=_mm_unpacklo_pi8 (a0, null_64);
	b1=_mm_unpackhi_pi8 (b0, null_64);
	b0=_mm_unpacklo_pi8 (b0, null_64);

	a1=_mm_mullo_pi16   (a1, factor);
	a0=_mm_mullo_pi16   (a0, factor);

	a1=_mm_srli_pi16    (a1, 8);
	a0=_mm_srli_pi16    (a0, 8);

	a1=_mm_adds_pu16    (a1, b1);
	a0=_mm_adds_pu16    (a0, b0);

	a0=_mm_packs_pu16   (a0, a1);
	dest[i]=a0;
	//*dest++ += (unsigned char)((factor**source++)>>8);
      }
  _mm_empty();

    }
  }
  _mm_empty();

  m_counter++;
  m_counter%=m_bufnum;
}
void pix_tIIR :: processYUVMMX(imageStruct &image)
{ processRGBAMMX(image); }
void pix_tIIR :: processGrayMMX(imageStruct &image)
{ processRGBAMMX(image); }
#endif

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_tIIR :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_tIIR::setMessCallback),
		  gensym("set"), A_GIMME, A_NULL);
}

void pix_tIIR :: setMessCallback(void *data, t_symbol *s, int argc, t_atom* argv)
{
  GetMyClass(data)->set = true;
  GetMyClass(data)->set_zero = (argc>0 && atom_getint(argv)==0);
  GetMyClass(data)->setPixModified();
}
