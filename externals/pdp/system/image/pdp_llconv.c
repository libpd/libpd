/*
 *   Pure Data Packet system implementation. : low level format conversion code
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* this file contains low level image conversion code 
   nominated as "the ugliest part of pdp"
   some code is mmx, most is not. */

/* seem's there's some confusion between rgb and bgr formats.
   not that it matters much which is supposed to be the "real"
   rgb or bgr, but opengl and v4l seem to disagree on endianness
   grounds.. */

#include "pdp_llconv.h"
#include "pdp_mmx.h"
#include "pdp_post.h"


/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif


#define CLAMP8(x) (((x)<0) ? 0 : ((x>255)? 255 : (x)))
#define CLAMP16(x) (((x)<-0x7fff) ? -0x7fff : ((x>0x7fff) ? 0x7fff : (x)))
#define FP(x) ((int)(((float)(x)) * 256.0f))

#define CLAMP CLAMP8

/* some prototypes for functions defined elsewhere */
void llconv_yvu_planar_s16u8(short int *src, unsigned char *dst, unsigned int nbpixels);
void llconv_yuv_planar_u8s16(unsigned char* source, short int *dest, int nbpixels);
void llconv_grey_s16u8(short int *src, unsigned char *dst, unsigned int nbpixels);
void llconv_yvu_planar_u8s16(unsigned char* source, short int *dest, int nbpixels);


static inline int rgb2y(int r, int g, int b)
{
    return (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
}
static inline int rgb2v(int r, int g, int b)
{
    return (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b) + FP(128);
}
static inline int rgb2u(int r, int g, int b)
{
    return -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b) + FP(128);
}


/* swap top to bottom */
static inline void _exchange_row(char *row1, char *row2, int size)
{
    int mask = ~((unsigned int)sizeof(int)-1);
    int *irow1 = (int *)row1;
    int *irow2 = (int *)row2;

    /* transfer words */
    while (size & mask){
	int tmp = *irow1;
	*irow1++ = *irow2;
	*irow2++ = tmp;
	size -= sizeof(int);
    }

    row1 = (char *)irow1;
    row2 = (char *)irow2;

    /* transfer rest bytes */
    while (size){
	int tmp = *row1;
	*row1++ = *row2;
	*row2++ = tmp;
	size--;
    }
}

void pdp_llconv_flip_top_bottom(char *data, int width, int height, int pixelsize)
{
    int linesize = width * pixelsize;
    int i;
    char *row1 = data;
    char *row2 = data + linesize * (height-1);

    if (height <= 1) return;
    if (width  <= 0) return;

    while (row1 < row2){
	_exchange_row(row1, row2, linesize);
	row1 += linesize;
	row2 -= linesize;
    }
}

/* "standard" 8 bit conversion routine */
static void llconv_rgb2yvu(unsigned char* src, unsigned char* dst, int nbpixels)
{
    int r,g,b,y,v,u,i;
    for (i=0; i<nbpixels; i++){
	r = src[0];
	g = src[1];
	b = src[2];

	y = rgb2y(r,g,b);
	v = rgb2v(r,g,b);
	u = rgb2u(r,g,b);

	dst[0] = CLAMP(y>>8);
	dst[1] = CLAMP(v>>8);
	dst[2] = CLAMP(u>>8);

	src += 3;
	dst += 3;
    }
}

static void llconv_yvu16planar2rgbpacked(short int *src, unsigned char *dst, int w, int h)
{

/*
B = 1.164(Y - 16)                   + 2.018(U - 128)
G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
R = 1.164(Y - 16) + 1.596(V - 128)}
*/

    int r,g,b,y,u,v,b1,g1,r1,y1,xoff,yoff;
    int size = w*h;
    int voffset = size;
    int uoffset = size + (size>>2);
    int rgboff;
    int rgbw = w*3;
    int lumoff = 0;
    int chromoff = 0;

    for(yoff=0; yoff<w*h; yoff+=2*w){
	for(xoff=0; xoff<w; xoff+=2){

	    /* calculate offsets */
	    rgboff = 3 * (xoff + yoff);
	    lumoff = xoff + yoff;
	    chromoff =  (xoff >> 1) + (yoff >> 2);
	    
	    /* get uv values */
	    v = src[voffset + chromoff];
	    u = src[uoffset + chromoff];

	    /* calculate chroma contrib for 2x2 pixblock */
	    b1 = FP(2.018) * u;
	    g1 = FP(-0.813) * v + FP(-0.391) * u;
	    r1 = FP(1.596) * v;

	    /* TOP LEFT */

	    /* top left luma contrib */
	    y = src[lumoff] << 1;
	    y1 = FP(1.164) * y;
	    y1 -= FP(16*256);
	    
	    b = (b1 + y1)>>16;
	    g = (g1 + y1)>>16;
	    r = (r1 + y1)>>16; 

	    /* store top left rgb pixel */
	    dst[rgboff+0] = CLAMP8(r);
	    dst[rgboff+1] = CLAMP8(g);
	    dst[rgboff+2] = CLAMP8(b);

	    /* TOP RIGHT */

	    /* top right luma contrib */
	    y = src[lumoff + 1] << 1;
	    y1 = FP(1.164) * y;
	    y1 -= FP(16*256);
	    
	    b = (b1 + y1)>>16;
	    g = (g1 + y1)>>16;
	    r = (r1 + y1)>>16; 

	    /* store top right rgb pixel */
	    dst[rgboff+3] = CLAMP8(r);
	    dst[rgboff+4] = CLAMP8(g);
	    dst[rgboff+5] = CLAMP8(b);


	    /* BOTTOM LEFT */

	    /* bottom left luma contrib */
	    y = src[lumoff+w] << 1;
	    y1 = FP(1.164) * y;
	    y1 -= FP(16*256);
	    
	    b = (b1 + y1)>>16;
	    g = (g1 + y1)>>16;
	    r = (r1 + y1)>>16; 

	    /* store bottom left rgb pixel */
	    dst[rgboff+rgbw+0] = CLAMP8(r);
	    dst[rgboff+rgbw+1] = CLAMP8(g);
	    dst[rgboff+rgbw+2] = CLAMP8(b);

	    /* BOTTOM RIGHT */

	    /* bottom right luma contrib */
	    y = src[lumoff + w + 1] << 1;
	    y1 = FP(1.164) * y;
	    y1 -= FP(16*256);
	    
	    b = (b1 + y1)>>16;
	    g = (g1 + y1)>>16;
	    r = (r1 + y1)>>16; 

	    /* store bottom right rgb pixel */
	    dst[rgboff+rgbw+3] = CLAMP8(r);
	    dst[rgboff+rgbw+4] = CLAMP8(g);
	    dst[rgboff+rgbw+5] = CLAMP8(b);

	}

    }

}



/* common 8 bit rgb -> 16 bit yvu */
inline static void llconv_rgb2yvu_planar16sub_indexed(unsigned char* src, short int* dst, int w, int h, int ir, int ig, int ib, int stride)
{
    int r,g,b,y,v,u,i,j,k;
    int size = w*h;

    int voffset = size;
    int uoffset = size + (size>>2);


    int loffset = w * stride;

    k=0;
    for (j=0; j<w*h; j+=(w<<1)){
	k = stride * j;
	for (i=0; i<w; i+=2){


	    // well, this seems to work... strange though
	    r = src[k+ir];
	    g = src[k+ig];
	    b = src[k+ib];
	    
	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v =  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u = -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[i+j] = CLAMP16(y >> 1);

	    r = src[k+stride+ir];
	    g = src[k+stride+ig];
	    b = src[k+stride+ib];
	    
	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v +=  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u += -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[i+j+1] = CLAMP16(y >> 1);



	    r = src[loffset + k+ir];
	    g = src[loffset + k+ig];
	    b = src[loffset + k+ib];
	    
	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v =  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u = -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[w+i+j] = CLAMP16(y >> 1);

	    r = src[loffset + k+stride+ir];
	    g = src[loffset + k+stride+ig];
	    b = src[loffset + k+stride+ib];
	    
	    k += 2 * stride;

	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v +=  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u += -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[w+i+j+1] = CLAMP16(y >> 1);

	    dst[uoffset+ (i>>1) + (j>>2)] = (CLAMP16(u >> 1));
	    dst[voffset+ (i>>1) + (j>>2)] = (CLAMP16(v >> 1));
	}
    }
}

/* 8 bit rgb to 16 bit planar subsampled yvu */
static void llconv_rgb2yvu_planar16sub(unsigned char* src, short int* dst, int w, int h)
{
    llconv_rgb2yvu_planar16sub_indexed(src,dst,w,h,0,1,2,3);
}

/* 8 bit rgba to 16 bit planar subsampled yvu */
static void llconv_rgba2yvu_planar16sub(unsigned char* src, short int* dst, int w, int h)
{
    llconv_rgb2yvu_planar16sub_indexed(src,dst,w,h,0,1,2,4);
}

/* 8 bit bgr to 16 bit planar subsampled yvu */
static void llconv_bgr2yvu_planar16sub(unsigned char* src, short int* dst, int w, int h)
{
    llconv_rgb2yvu_planar16sub_indexed(src,dst,w,h,2,1,0,3);
}

/* 8 bit bgra to 16 bit planar subsampled yvu */
static void llconv_bgra2yvu_planar16sub(unsigned char* src, short int* dst, int w, int h)
{
    llconv_rgb2yvu_planar16sub_indexed(src,dst,w,h,2,1,0,4);
}


/* 8 bit rgb to 8 bit planar subsampled yvu */
static void llconv_rgb2yvu_planar8sub(unsigned char* src, unsigned char *dst, int w, int h)
{
    int r,g,b,y,v,u,i,j,k;
    int size = w*h;

    int voffset = size;
    int uoffset = size + (size>>2);


    int loffset = w * 3;

    k=0;
    for (j=0; j<w*h; j+=(w<<1)){
	k = 3 * j;
	for (i=0; i<w; i+=2){


	    // well, this seems to work... strange though
	    r = src[k];
	    g = src[k+1];
	    b = src[k+2];
	    
	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v =  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u = -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[i+j] = CLAMP8(y >> 8);

	    r = src[k+3];
	    g = src[k+4];
	    b = src[k+5];
	    
	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v +=  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u += -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[i+j+1] = CLAMP8(y >> 8);



	    r = src[loffset + k];
	    g = src[loffset + k+1];
	    b = src[loffset + k+2];
	    
	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v =  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u = -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[w+i+j] = CLAMP8(y >> 8);

	    r = src[loffset + k+3];
	    g = src[loffset + k+4];
	    b = src[loffset + k+5];
	    
	    k += 6;

	    y =  (FP(0.257) * r) + (FP(0.504) * g) + (FP(0.098) * b) + FP(16);
	    v +=  (FP(0.439) * r) - (FP(0.368) * g) - (FP(0.071) * b);
	    u += -(FP(0.148) * r) - (FP(0.291) * g) + (FP(0.439) * b);

	    dst[w+i+j+1] = CLAMP8(y >> 8);

	    dst[uoffset+ (i>>1) + (j>>2)] = (CLAMP8((u >> 9)+128));
	    dst[voffset+ (i>>1) + (j>>2)] = (CLAMP8((v >> 9)+128));
	}
    }
}


/* these seem to be pretty slow */

static void llconv_yvu2rgb(unsigned char* src, unsigned char* dst, int nbpixels)
{
    int r,g,b,y,v,u,i;
    for (i=0; i<nbpixels; i++){
	y = src[0];
	v = src[1];
	u = src[2];


	b = FP(1.164) * (y - 16)                         + FP(2.018) * (u - 128);
	g = FP(1.164) * (y - 16) - FP(0.813) * (v - 128) - FP(0.391) * (u - 128);
	r = FP(1.164) * (y - 16) + FP(1.596) * (v - 128);

	dst[0] = CLAMP(r>>8);
	dst[1] = CLAMP(g>>8);
	dst[2] = CLAMP(b>>8);

	src += 3;
	dst += 3;
    }
}



/* convert yvu to yuyv */
static void llconv_yvu2yuyv(unsigned char *src, unsigned char *dst, unsigned int nbpixels)
{
    unsigned int y1, y2, u, v, i;

    for (i = 0; i < nbpixels/2; i++){

	y1 = src[0];
	y2 = src[3];
	v = (src[1] + src[4]) >> 1;
	u = (src[2] + src[5]) >> 1;
	dst[0] = y1;
	dst[1] = u;
	dst[2] = y2;
	dst[3] = v;

	src += 6;
	dst += 4;

    }

}



/* convert yuvu packed 8 bit unsigned to yv12 planar 16bit signed */
static void llconv_yuyv_packed_u8s16(unsigned char* ucsource, short int *sidest, unsigned int w, unsigned int h)
{
    unsigned int i, j;
    unsigned int *source = (unsigned int *)ucsource;

    unsigned int *dest = (unsigned int *)sidest;
    unsigned int uoffset = (w*h)>>1;
    unsigned int voffset = (w*h + ((w*h) >> 2)) >> 1;

    for(j=0; j < (h*w)>>1; j +=(w)){
	for(i=0; i< (w>>1); i+=2){
	    unsigned int y,u,v;
	    unsigned int v00, v01, v10, v11;
	    v00 = source[i+j];
	    v01 = source[i+j+1];
	    v10 = source[i+j+(w>>1)];
	    v11 = source[i+j+(w>>1)+1];
	    
	    // save luma
	    dest[i+j]          = ((v00 & 0x00ff00ff) << 7);
	    dest[i+j+1]        = ((v01 & 0x00ff00ff) << 7);
	    dest[i+j+(w>>1)]   = ((v10 & 0x00ff00ff) << 7);
	    dest[i+j+(w>>1)+1] = ((v11 & 0x00ff00ff) << 7);

	    // compute chroma

	    // mask out luma & shift right
	    v00 = (v00 & 0xff00ff00)>>1;
	    v01 = (v01 & 0xff00ff00)>>1;
	    v10 = (v10 & 0xff00ff00)>>1;
	    v11 = (v11 & 0xff00ff00)>>1;
	    
	    // average 2 scan lines
	    v00 += v10;
	    v01 += v11;

	    // combine
	    v = (v01 << 16) | (v00 & 0x0000ffff);
	    u = (v01 & 0xffff0000) | (v00 >> 16);

	    // flip sign bits for u,v
	    u ^= 0x80008000;
	    v ^= 0x80008000;

	    // save chroma
	    dest[uoffset + (i>>1) + (j>>2)] = u;
	    dest[voffset + (i>>1) + (j>>2)] = v;
	}
    }


}



/* convert yuvu packed 8 bit unsigned to yv12 planar 16bit signed */
/* search for bithacks */
static void llconv_uyvy_packed_u8s16(unsigned char* ucsource, short int *sidest, unsigned int w, unsigned int h)
{
    unsigned int i, j;
    unsigned int *source = (unsigned int *)ucsource;

    unsigned int *dest = (unsigned int *)sidest;
    unsigned int uoffset = (w*h)>>1;
    unsigned int voffset = (w*h + ((w*h) >> 2)) >> 1;

    for(j=0; j < (h*w)>>1; j +=(w)){
	for(i=0; i< (w>>1); i+=2){
	    unsigned int y,u,v;
	    unsigned int v00, v01, v10, v11;
	    v00 = source[i+j];
	    v01 = source[i+j+1];
	    v10 = source[i+j+(w>>1)];
	    v11 = source[i+j+(w>>1)+1];
	    
	    // save luma
	    dest[i+j]          = ((v00 & 0xff00ff00)>>1);
	    v11 = source[i+j+(w>>1)+1];
	    
	    // save luma
	    dest[i+j]          = ((v00 & 0xff00ff00)>>1);
	    dest[i+j+1]        = ((v01 & 0xff00ff00)>>1);
	    dest[i+j+(w>>1)]   = ((v10 & 0xff00ff00)>>1);
	    dest[i+j+(w>>1)+1] = ((v11 & 0xff00ff00)>>1);

	    // compute chroma

	    
	    v00 = (v00 & 0x00ff00ff) << 7;
	    v01 = (v01 & 0x00ff00ff) << 7;
	    v10 = (v10 & 0x00ff00ff) << 7;
	    v11 = (v11 & 0x00ff00ff) << 7;
	    
	    // average 2 scan lines
	    v00 += v10;
	    v01 += v11;

	    // combine TWO VALUES IN ONE WORD (32bits) 
	    v = (v01 << 16) | (v00 & 0x0000ffff);
	    u = (v01 & 0xffff0000) | (v00 >> 16);

	    // flip sign bits for u,v FOR PDP FORMAT
	    u ^= 0x80008000;
	    v ^= 0x80008000;

	    // save chroma
	    dest[uoffset + (i>>1) + (j>>2)] = u;
	    dest[voffset + (i>>1) + (j>>2)] = v;
	}
    }

}

#define CONVERT(x,y) ((x) + ((y)<<16))

void pdp_llconv(void *src, int stype, void *dst, int dtype, int w, int h)
{
    int conversion = CONVERT(stype, dtype);
    void *tmpbuf;

    switch(CONVERT(stype, dtype)){

    case CONVERT( RIF_YVU__P411_U8, RIF_YVU__P411_S16 ):
	llconv_yvu_planar_u8s16((unsigned char*)src, (short int *)dst, w*h);
	break;

    case CONVERT( RIF_YUV__P411_U8, RIF_YVU__P411_S16 ):
	llconv_yuv_planar_u8s16((unsigned char*)src, (short int *)dst, w*h);
	break;

    case CONVERT( RIF_YUYV_P____U8, RIF_YVU__P411_S16 ):
	llconv_yuyv_packed_u8s16((unsigned char*)src, (short int *)dst, w, h);
	break;

    case CONVERT( RIF_UYVY_P____U8, RIF_YVU__P411_S16 ):
	llconv_uyvy_packed_u8s16((unsigned char*)src, (short int *)dst, w, h);
	break;

    case CONVERT( RIF_RGB__P____U8, RIF_YVU__P411_U8 ):
	llconv_rgb2yvu_planar8sub((unsigned char*) src, (unsigned char*) dst, w, h);
	break;

    case CONVERT( RIF_RGB__P____U8, RIF_YVU__P411_S16 ):
	llconv_rgb2yvu_planar16sub((unsigned char*) src, (short int*) dst, w, h);
	break;

    case CONVERT( RIF_RGBA_P____U8, RIF_YVU__P411_S16 ):
	llconv_rgba2yvu_planar16sub((unsigned char*) src, (short int*) dst, w, h);
	break;

    case CONVERT( RIF_BGR__P____U8, RIF_YVU__P411_S16 ):
	llconv_bgr2yvu_planar16sub((unsigned char*) src, (short int*) dst, w, h);
	break;

    case CONVERT( RIF_BGRA_P____U8, RIF_YVU__P411_S16 ):
	llconv_bgra2yvu_planar16sub((unsigned char*) src, (short int*) dst, w, h);
	break;

    case CONVERT( RIF_YVU__P411_S16, RIF_RGB__P____U8 ):
	llconv_yvu16planar2rgbpacked((short int*) src, (unsigned char*) dst, w, h);
	break;

    case CONVERT( RIF_YVU__P411_S16, RIF_YVU__P411_U8 ):
	llconv_yvu_planar_s16u8((short int*)src, (unsigned char*)dst, w*h);
	break;

    case CONVERT( RIF_GREY______S16, RIF_GREY______U8 ):
	llconv_grey_s16u8((short int*)src, (unsigned char*)dst, w*h);
	break;
    default:
	pdp_post("pdp_llconv: WARNING: no conversion routine defined for (%d)->(%d)", stype, dtype);

    }

}


#ifdef __cplusplus
}
#endif
