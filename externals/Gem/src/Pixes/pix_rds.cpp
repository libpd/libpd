////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// This object is an based on the RandomDotStereoTV effect from EffecTV
// Originally written by Fukuchi Kentarou
// Copyright (C) 2002 FUKUCHI Kentarou                         
//
// ported by tigital@mac.com
//
// Implementation file
//
//    Copyright (c) 2003 James Tittle
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "pix_rds.h"
#include <stdlib.h>

CPPEXTERN_NEW(pix_rds);

#define inline_fastrand() (fastrand_val=fastrand_val*1103515245+12345)

/////////////////////////////////////////////////////////
//
// pix_rds
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_rds :: pix_rds()
{
    myImage.xsize=myImage.ysize=myImage.csize=1;
    myImage.allocate(1);
    stride = 40;
    method = 0;

    doDots=1;

    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("stride"));

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_rds :: ~pix_rds()
{
  myImage.clear();
}

/////////////////////////////////////////////////////////
// processRGBAImage
//
/////////////////////////////////////////////////////////
void pix_rds :: processRGBAImage(imageStruct &image)
{
    int x, y, i;
    unsigned int *target;
    unsigned int *src = (unsigned int*)image.data;
    unsigned int *dest;
    unsigned int v;
    unsigned int R, G, B;

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.upsidedown = image.upsidedown;
    myImage.reallocate();

    dest = (unsigned int*)myImage.data;

    memset(dest, 0, image.xsize * image.ysize * image.csize);
    target = dest;

    if(method) {
        for(y=0; y<image.ysize; y++) {
            for(i=0; i<stride; i++) {
                if(inline_fastrand()&0xc0000000)
                    continue;

                x = image.xsize / 2 + i;
                *(dest + x) = 0xffffffff;
	
                while(x + stride/2 < image.xsize) {
                    v = *(src + x + stride/2);
                    R = (v&0xff0000)>>(16+6);
                    G = (v&0xff00)>>(8+6);
                    B = (v&0xff)>>7;
                    x += stride + R + G + B;
                    if(x >= image.xsize) break;
                    *(dest + x) = 0xffffffff;
                }

                x = image.xsize / 2 + i;
                while(x - stride/2 >= 0) {
                    v = *(src + x - stride/2);
                    R = (v&0xff0000)>>(16+6);
		    G = (v&0xff00)>>(8+6);
		    B = (v&0xff)>>7;
                    x -= stride + R + G + B;
                    if(x < 0) break;
                    *(dest + x) = 0xffffffff;
                }
            }
            src += image.xsize;
            dest += image.xsize;
        }
    } else {
        for(y=0; y<image.ysize; y++) {
            for(i=0; i<stride; i++) {
                if(inline_fastrand()&0xc0000000)
                    continue;

                x = image.xsize / 2 + i;
                *(dest + x) = 0xffffffff;
	
                while(x + stride/2 < image.xsize) {
                    v = *(src + x + stride/2);
                    R = (v&0xff0000)>>(16+6);
                    G = (v&0xff00)>>(8+6);
                    B = (v&0xff)>>7;
                    x += stride - R - G - B;
                    if(x >= image.xsize) break;
                    *(dest + x) = 0xffffffff;
                }

                x = image.xsize / 2 + i;
                while(x - stride/2 >= 0) {
                    v = *(src + x - stride/2);
                    R = (v&0xff0000)>>(16+6);
                    G = (v&0xff00)>>(8+6);
                    B = (v&0xff)>>7;
                    x -= stride - R - G - B;
                    if(x < 0) break;
                    *(dest + x) = 0xffffffff;
                }
            }
            src += image.xsize;
            dest += image.xsize;
        }
    }

    if(doDots){
      target += image.xsize + (image.xsize - stride) / 2;
      for(y=0; y<4; y++) {
        for(x=0; x<4; x++) {
	  target[x] = 0xffff0000;
	  target[x+stride] = 0xffff0000;
        }
        target += image.xsize;
      }
    }
    image.data = myImage.data;
}
void pix_rds :: processGrayImage(imageStruct &image)
{
    int x, y, i;
    unsigned char *target;
    unsigned char *src = (unsigned char*)image.data;
    unsigned char *dest;
    unsigned char v;
    unsigned char R, G, B;

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(GL_LUMINANCE);
    myImage.upsidedown = image.upsidedown;
    myImage.reallocate();

    dest = (unsigned char*)myImage.data;

    memset(dest, 0, image.xsize * image.ysize * image.csize);
    target = dest;

    if(method) {
      for(y=0; y<image.ysize; y++) {
	for(i=0; i<stride; i++) {
	  if(inline_fastrand()&0xc0000000) continue;
	  
	  x = image.xsize / 2 + i;
	  dest[x] = 0xff;
	
	  while(x + stride/2 < image.xsize) {
	    v = src[x + stride/2];
	    R=v>>6; G=v>>6; B=v>>7;
	    x += stride;
	    x += R + R + B;
	    if(x >= image.xsize) break;
	    dest[x] = 0xff;
	  }

	  x = image.xsize / 2 + i;
	  while(x - stride/2 >= 0) {
	    v = src[x - stride/2];
	    R=v>>6; G=v>>6; B=v>>7;
	    x -= stride;
	    x -= R + R + B;
	    if(x < 0) break;
	    dest[x] = 0xff;
	  }
	}
	src += image.xsize;
	dest += image.xsize;
      }
    } else {
      for(y=0; y<image.ysize; y++) {
	for(i=0; i<stride; i++) {
	  if(inline_fastrand()&0xc0000000) continue;

	  x = image.xsize / 2 + i;
	  dest[x] = 0xff;
	
	  while(x + stride/2 < image.xsize) {
	    v = src[x + stride/2];
	    R=v>>6; B=v>>7;
	    x += stride - R - R - B;
	    if(x >= image.xsize) break;
	    dest[x] = 0xff;
	  }
	  
	  x = image.xsize / 2 + i;
	  while(x - stride/2 >= 0) {
	    v = src[x - stride/2];
	    R=v>>6; B=v>>7;
	    x -= stride - R - R - B;
	    if(x < 0) break;
	    dest[x] = 0xff;
	  }
	}
	src += image.xsize;
	dest += image.xsize;
      }
    }

    if(doDots){
      target += image.xsize + (image.xsize - stride) / 2;
      for(y=0; y<4; y++) {
	for(x=0; x<4; x++) {
	  target[x] = 0xff    ;
	  target[x+stride] = 0xff    ;
	}
	target += image.xsize;
      }
    }
    image.data = myImage.data;
}
void pix_rds :: processYUVImage(imageStruct &image)
{
    int x, y, i;
    unsigned char *target, *dest;
    unsigned short *src = (unsigned short*)image.data;
    unsigned short v;
    unsigned short R, B;

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(GL_LUMINANCE);
    myImage.upsidedown = image.upsidedown;
    myImage.reallocate();

    dest = (unsigned char*)myImage.data;

    myImage.setBlack();
    target = dest;

    //    image.data = myImage.data;  return;

    if(method) {
      for(y=0; y<image.ysize; y++) {
	for(i=0; i<stride; i++) {
	  if(inline_fastrand()&0xc0000000) continue;
	  
	  x = image.xsize / 2 + i;
	  dest[x] = 0xff;
	
	  while(x + stride/2 < image.xsize) {
	    v = src[x + stride/2] & 0x00ff; // UYVY, we only want Y
	    R=v>>6; B=v>>7;
	    x += stride;
	    x += R + R + B;
	    if(x >= image.xsize) break;
	    dest[x] = 0xff;
	  }

	  x = image.xsize / 2 + i;
	  while(x - stride/2 >= 0) {
	    v = src[x - stride/2] & 0x00ff; // UYVY, we only want Y
	    R=v>>6; B=v>>7;
	    x -= stride;
	    x -= R + R + B;
	    if(x < 0) break;
	    dest[x] = 0xff;
	  }
	}
	src += image.xsize;
	dest += image.xsize;
      }
    } else {
      for(y=0; y<image.ysize; y++) {
	for(i=0; i<stride; i++) {
	  if(inline_fastrand()&0xc0000000) continue;

	  x = image.xsize / 2 + i;
	  dest[x] = 0xff;
	
	  while(x + stride/2 < image.xsize) {
	    v = src[x + stride/2] & 0x00ff;
	    R=v>>6; B=v>>7;
	    x += stride - R - R - B;
	    if(x >= image.xsize) break;
	    dest[x] = 0xff;
	  }
	  
	  x = image.xsize / 2 + i;
	  while(x - stride/2 >= 0) {
	    v = src[x - stride/2] & 0x00ff;
	    R=v>>6; B=v>>7;
	    x -= stride - R - R - B;
	    if(x < 0) break;
	    dest[x] = 0xff;
	  }
	}
	src += image.xsize;
	dest += image.xsize;
      }
    }

    if(doDots){
      target += image.xsize + (image.xsize - stride) / 2;
      for(y=0; y<4; y++) {
	for(x=0; x<4; x++) {
	  target[x] = 0xff    ;
	  target[x+stride] = 0xff    ;
	}
	target += image.xsize;
      }
    }
    image.fromGray(myImage.data);
    //    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_rds :: obj_setupCallback(t_class *classPtr)
{
  //  class_addfloat(classPtr, reinterpret_cast<t_method>(&pix_rds::methMessCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_rds::methMessCallback),
		  gensym("method"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_rds::strideMessCallback),
		  gensym("stride"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_rds::seedMessCallback),
		  gensym("seed"), A_FLOAT, A_NULL);
}

void pix_rds :: methMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->method=((int)state);
  GetMyClass(data)->setPixModified();
}
void pix_rds :: strideMessCallback(void *data, t_floatarg state)
{
  if(state<0.f){
    GetMyClass(data)->error("stride must be > 0!");
    return;
  }
  GetMyClass(data)->stride=((int)state);
  GetMyClass(data)->setPixModified();
}
void pix_rds :: seedMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->fastrand_val=((int)state);
  GetMyClass(data)->setPixModified();
}
