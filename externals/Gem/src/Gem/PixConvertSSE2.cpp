#ifdef __SSE2__
/////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file for SSE2-optimized color-conversion routines
//
//    Copyright (c) 2006-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

/* darned: it seems like i just cannot get SIMD-code right!
 * to my eye, there appear to me WAY too much shuffle's down there
 * if somebody would want to have a look i'd be grateful
 */


#if defined _MSC_VER
/* data conversion with possible loss of data */
# pragma warning( disable : 4309 )
#endif

#include "PixConvert.h"
#include "Gem/Image.h"


#define RGB2YUV_14 0
#define RGB2YUV_24 0
#define RGB2YUV_34 0


/* for post() */
#include "m_pd.h"

/* just some debugging stuff ... */

#define PRINT_MASK "%03d "
static void print_char(__m128i m){
  _mm_pause();
  unsigned char*s=(unsigned char*)&m;
  int i=0;
  for(i=0; i<(sizeof(__m128i)/sizeof(unsigned char)); i++){
    startpost(PRINT_MASK, *s);s++;
  }
  endpost();
}
static void print_short(__m128i m){
  _mm_pause();
  signed short*s=(signed short*)&m;
  int i=0;
  for(i=0; i<(sizeof(__m128i)/sizeof(signed short)); i++){
    startpost(PRINT_MASK, *s);s++;
  }
  endpost();
}
static void print_int(__m128i m){
  _mm_pause();
  signed int*s=(signed int*)&m;
  int i=0;
  for(i=0; i<(sizeof(__m128i)/sizeof(signed int)); i++){
    startpost(PRINT_MASK, *s);s++;
  }
  endpost();
}


/* convert RGBA to YUV422 */
void RGBA_to_UYVY_SSE2(const unsigned char *rgbadata, 
                       size_t size, 
                       unsigned char *yuvdata)
{
  const __m128i *rgba_p = (const __m128i*)rgbadata; /*  4 RGBA pixels */
  __m128i *yuv_p  = (__m128i*)yuvdata;  /* 4*2 YUV pixels */

  const __m128i zero = _mm_setzero_si128();

  const __m128i RG2Y=_mm_set_epi16(RGB2YUV_12, RGB2YUV_11, RGB2YUV_12, RGB2YUV_11, 
                                   RGB2YUV_12, RGB2YUV_11, RGB2YUV_12, RGB2YUV_11);
  const __m128i BA2Y=_mm_set_epi16(RGB2YUV_14, RGB2YUV_13, RGB2YUV_14, RGB2YUV_13, 
                                   RGB2YUV_14, RGB2YUV_13, RGB2YUV_14, RGB2YUV_13);

  const __m128i RG2U=_mm_set_epi16(RGB2YUV_22, RGB2YUV_21, RGB2YUV_22, RGB2YUV_21, 
                                   RGB2YUV_22, RGB2YUV_21, RGB2YUV_22, RGB2YUV_21);
  const __m128i BA2U=_mm_set_epi16(RGB2YUV_24, RGB2YUV_23, RGB2YUV_24, RGB2YUV_23, 
                                   RGB2YUV_24, RGB2YUV_23, RGB2YUV_24, RGB2YUV_23);

  const __m128i RG2V=_mm_set_epi16(RGB2YUV_32, RGB2YUV_31, RGB2YUV_32, RGB2YUV_31, 
                                   RGB2YUV_32, RGB2YUV_31, RGB2YUV_32, RGB2YUV_31);
  const __m128i BA2V=_mm_set_epi16(RGB2YUV_34, RGB2YUV_33, RGB2YUV_34, RGB2YUV_33, 
                                   RGB2YUV_34, RGB2YUV_33, RGB2YUV_34, RGB2YUV_33);

  const __m128i OFFSET=_mm_set_epi16(Y_OFFSET, UV_OFFSET,
                                     Y_OFFSET, UV_OFFSET,
                                     Y_OFFSET, UV_OFFSET,
                                     Y_OFFSET, UV_OFFSET);

  /* nomenclatura:
   *   lower-case letters denote  8bit values (like "r" is red, 8bit)
   *   upper-case letters denote 16bit (or 32bit) values (like "G" is green, 16bit)
   *
   */
  __m128i rgba0, rgba1, RGBA0, RGBA1;
  __m128i RGRG, BABA, RGRG0, BABA0, RGRG1, BABA1;
  __m128i RGRG_BABA0, RGRG_BABA1;
  __m128i Y0, Y1, U, V, UY, VY, UV, YY;

  const int shuffle =  _MM_SHUFFLE(3, 1, 2, 0);

  int i=size>>3; /* we do 2*128bit per cycle: this is 2*4*32bit == 8 pixels */
  while(i-->0){
    rgba0=*rgba_p++; /* r0 g0 b0 a0 r1 g1 ... b3 a3 */
    rgba1=*rgba_p++; /* r4 g4 b4 a4 r5 g5 ... b7 a7 */

    /* 1st 4 pixels */
    RGBA0 = _mm_unpacklo_epi8(rgba0, zero); /* R0 G0 B0 A0 ... B1 A1 */
    RGBA1 = _mm_unpackhi_epi8(rgba0, zero); /* R2 G2 B2 A2 ... B3 A3 */

    RGRG_BABA0  = _mm_unpacklo_epi32(RGBA0, RGBA1); /* R0 G0 R2 G2 B0 A0 B2 A2 */
    RGRG_BABA1  = _mm_unpackhi_epi32(RGBA0, RGBA1); /* R1 G1 R3 G3 B1 A1 B3 A3 */

    RGRG0 = _mm_unpacklo_epi64(RGRG_BABA0, RGRG_BABA1); /* R0 G0 R2 G2 R1 G1 R3 G3 */
    BABA0 = _mm_unpackhi_epi64(RGRG_BABA0, RGRG_BABA1); /* B0 A0 B2 A2 B1 A1 B3 A3 */

    // get Y for the 1st 4 pixels (thats 32bit)
    // Y_RG32 = _mm_madd_epi16(RGRG0, RG2Y); /* R0*a+G0*b R2*a+G2*b R1*a+G1*b R3*a+G3*b */
    // Y_BA32 = _mm_madd_epi16(BABA0, BA2Y); /* B0*c+A0*d B2*c+A2*d B1*c+A1*d B3*c+A3*d */
    Y0    = _mm_add_epi32(_mm_madd_epi16(RGRG0, RG2Y), _mm_madd_epi16(BABA0, BA2Y));
    Y0    = _mm_shuffle_epi32(Y0, shuffle);
    //startpost("Y0: "); print_int(Y0); /* Y0, Y1, Y2, Y3 */

    /* 2nd 4 pixels */
    RGBA0 = _mm_unpacklo_epi8(rgba1, zero);     /* R4 G4 B4 A4 R5 G5 B5 A5 */
    RGBA1 = _mm_unpackhi_epi8(rgba1, zero);     /* R6 G6 B6 A6 R7 G7 B7 A7 */

    RGRG_BABA0  = _mm_unpacklo_epi32(RGBA0, RGBA1); /* R4 G4 R6 G6 B4 A4 B6 A6 */
    RGRG_BABA1  = _mm_unpackhi_epi32(RGBA0, RGBA1); /* R5 G5 R7 G7 B5 A5 B7 A7 */

    RGRG1  = _mm_unpacklo_epi64(RGRG_BABA0, RGRG_BABA1); /* R4 G4 R6 G6 R5 G5 R7 G7 */
    BABA1  = _mm_unpackhi_epi64(RGRG_BABA0, RGRG_BABA1); /* B4 A4 B6 A6 B5 A5 B7 A7 */

    // get Y for the 2nd 4 pixels (thats 32bit)
    // Y_RG32 = _mm_madd_epi16(RGRG1, RG2Y); /* R4*a+G4*b R6*a+G6*b R5*a+G5*b R7*a+G7*b */
    // Y_BA32 = _mm_madd_epi16(BABA1, BA2Y); /* B4*c+A4*d B6*c+A6*d B5*c+A5*d B7*c+A7*d */
    Y1       = _mm_add_epi32(_mm_madd_epi16(RGRG1, RG2Y), _mm_madd_epi16(BABA1, BA2Y));
    Y1       = _mm_shuffle_epi32(Y1, shuffle);
    //startpost("Y1: "); print_int(Y1);

    // now get UV
    /* (R01 G01 R23 G23 R45 G45 R67 G67) / 2 */
    RGRG  = _mm_avg_epu16(_mm_unpackhi_epi64(RGRG0, RGRG1), _mm_unpacklo_epi64(RGRG0, RGRG1));
    /* (B01 A01 B23 A23 B45 A45 B67 A67) / 2 */
    BABA  = _mm_avg_epu16(_mm_unpackhi_epi64(BABA0, BABA1), _mm_unpacklo_epi64(BABA0, BABA1));
    
    // get 4 U for 8 pixels (32bit each)
    //U_RG32 = _mm_madd_epi16(RGRG, RG2U); /* R4*a+G4*b R6*a+G6*b R5*a+G5*b R7*a+G7*b */
    //U_BA32 = _mm_madd_epi16(BABA, AB2U); /* B4*c+A4*d B6*c+A6*d B5*c+A5*d B7*c+A7*d */
    U      = _mm_add_epi32 (_mm_madd_epi16(RGRG, RG2U), _mm_madd_epi16(BABA, BA2U));

    // get 4 V for 8 pixels (32bit each)
    //V_RG32 = _mm_madd_epi16(RGRG, RG2V); /* R4*a+G4*b R6*a+G6*b R5*a+G5*b R7*a+G7*b */
    //V_BA32 = _mm_madd_epi16(BABA, AB2V); /* B4*c+A4*d B6*c+A6*d B5*c+A5*d B7*c+A7*d */
    V      = _mm_add_epi32 (_mm_madd_epi16(RGRG, RG2V), _mm_madd_epi16(BABA, BA2V));

    // 32 instructions so far

    // so now we have (all values in 32bit)
    /*
     *  U  U  U  U
     * Y1 Y1 Y1 Y1
     *  V  V  V  V
     * Y2 Y2 Y2 Y2
     */

    // we still need to right-shift everything by 8
    // and press it into 8bit values, so we have one vector with UYVYUYVYUYVYUYVY
    // (or just take the 3rd 8bit-tuple)

    Y0 = _mm_srai_epi32(Y0, 8);
    U  = _mm_srai_epi32(U , 8);
    Y1 = _mm_srai_epi32(Y1, 8);
    V  = _mm_srai_epi32(V , 8);

    UV = _mm_packs_epi32(U, V);
    YY = _mm_packs_epi32(Y0, Y1);

    UV = _mm_shuffle_epi32(UV, shuffle);
    UV = _mm_shufflehi_epi16(UV, shuffle);
    UV = _mm_shufflelo_epi16(UV, shuffle);

    UY = _mm_unpacklo_epi16(UV, YY);
    VY = _mm_unpackhi_epi16(UV, YY);

    UY = _mm_adds_epi16(UY, OFFSET);
    VY = _mm_adds_epi16(VY, OFFSET);

    _mm_stream_si128(yuv_p++,  _mm_packus_epi16(UY, VY));
    // 32+15 instructions
  }
}

/* convert RGBA to YUV422 */
void UYVY_to_RGBA_SSE2(const unsigned char *yuvdata, 
                       size_t size, 
                       unsigned char *rgbadata)
{
  __m128i *rgba_p = (__m128i*)rgbadata; /*  4 RGBA pixels */
  const __m128i *yuv_p  = (const __m128i*)yuvdata;  /* 4*2 YUV pixels */
  
  const __m128i Y2RGB = _mm_set_epi16(YUV2RGB_11,0,YUV2RGB_11,0,YUV2RGB_11,0,YUV2RGB_11,0);
  const __m128i UV2R  = _mm_set_epi16(YUV2RGB_13, YUV2RGB_12, YUV2RGB_13, YUV2RGB_12, 
                                      YUV2RGB_13, YUV2RGB_12, YUV2RGB_13, YUV2RGB_12);
  const __m128i UV2G  = _mm_set_epi16(YUV2RGB_23, YUV2RGB_22, YUV2RGB_23, YUV2RGB_22, 
                                      YUV2RGB_23, YUV2RGB_22, YUV2RGB_23, YUV2RGB_22);
  const __m128i UV2B  = _mm_set_epi16(YUV2RGB_33, YUV2RGB_32, YUV2RGB_33, YUV2RGB_32, 
                                      YUV2RGB_33, YUV2RGB_32, YUV2RGB_33, YUV2RGB_32);
  const __m128i offset= _mm_set_epi16(Y_OFFSET, UV_OFFSET, Y_OFFSET, UV_OFFSET, 
                                      Y_OFFSET, UV_OFFSET, Y_OFFSET, UV_OFFSET);
  const __m128i  A32  = _mm_set_epi32(255, 255, 255, 255);

  /* nomenclatura:
   *   lower-case letters denote  8bit values (like "r" is red, 8bit)
   *   upper-case letters denote 16bit (or 32bit) values (like "G" is green, 16bit)
   */

  __m128i uyvy, UYVY0, UYVY1;
  __m128i UV, YZ, Y, Z;
  __m128i UV_R, UV_G, UV_B;
  __m128i R, G, B, A;
  __m128i RB0, RB1, GA0, GA1;

  const int shuffle =  _MM_SHUFFLE(3, 1, 2, 0);

  int i=size>>3; /* we do 2*128bit per cycle: this is 2*4*32bit == 8 pixels */
  while(i-->0){
    uyvy=*yuv_p++; /* u0 y0 v0 z0 u1 y1 v1 z1 u2 y2 v2 z2 u3 y3 v3 z3 */

    UYVY0 = _mm_unpacklo_epi8(uyvy, _mm_setzero_si128()); /* U0 Y0 V0 Z0 U1 Y1 V1 Z1 */
    UYVY1 = _mm_unpackhi_epi8(uyvy, _mm_setzero_si128()); /* U2 Y2 V2 Z2 U3 Y3 V3 Z3 */

    UYVY0 = _mm_sub_epi16(UYVY0, offset);
    UYVY1 = _mm_sub_epi16(UYVY1, offset);

    UYVY0 = _mm_shufflelo_epi16(UYVY0, shuffle);
    UYVY0 = _mm_shufflehi_epi16(UYVY0, shuffle);
    UYVY0 = _mm_shuffle_epi32  (UYVY0, shuffle); /* U0 V0 U1 V1 Y0 Z0 Y1 Z1 */

    UYVY1 = _mm_shufflelo_epi16(UYVY1, shuffle);
    UYVY1 = _mm_shufflehi_epi16(UYVY1, shuffle); 
    UYVY1 = _mm_shuffle_epi32  (UYVY1, shuffle); /* U2 V2 U3 V3 Y2 Z2 Y3 Z3 */

    UV = _mm_unpacklo_epi32(UYVY0, UYVY1); /* U0 V0 U2 V2 U1 V1 U3 V3 */
    YZ = _mm_unpackhi_epi32(UYVY0, UYVY1); /* Y0 Z0 Y2 Z2 Y1 Z1 Y3 Z3 */

    Z = _mm_madd_epi16(YZ, Y2RGB);                    /* Z0' Z2' Z1' Z3' */
    Y = _mm_madd_epi16(YZ, _mm_srli_si128(Y2RGB, 2)); /* Y0' Y2' Y1' Y3' */

    UV_R = _mm_madd_epi16(UV, UV2R);
    UV_G = _mm_madd_epi16(UV, UV2G);
    UV_B = _mm_madd_epi16(UV, UV2B);

    R  = _mm_srai_epi32(_mm_add_epi32(Y, UV_R), 8);
    G  = _mm_srai_epi32(_mm_add_epi32(Y, UV_G), 8);
    B  = _mm_srai_epi32(_mm_add_epi32(Y, UV_B), 8);

    RB0 = _mm_packs_epi32(R, B);
    GA0 = _mm_packs_epi32(G, A32);

    R  = _mm_srai_epi32(_mm_add_epi32(Z, UV_R), 8);
    G  = _mm_srai_epi32(_mm_add_epi32(Z, UV_G), 8);
    B  = _mm_srai_epi32(_mm_add_epi32(Z, UV_B), 8);

    RB1 = _mm_packs_epi32(R, B);
    GA1 = _mm_packs_epi32(G, A32);

    R  = _mm_unpacklo_epi16(RB0, RB1);  /* R0 R1 R4 R5 R2 R3 R6 R7 */
    R  = _mm_shuffle_epi32 (R, shuffle);/* R0 R1 R2 R3 R4 R5 R6 R7 */
    B  = _mm_unpackhi_epi16(RB0, RB1);
    B  = _mm_shuffle_epi32 (B, shuffle);
    G  = _mm_unpacklo_epi16(GA0, GA1);
    G  = _mm_shuffle_epi32 (G, shuffle);
    A  = _mm_unpackhi_epi16(GA0, GA1); /* no need to shuffle, since A0=A1=...=255 */

    RB0= _mm_unpacklo_epi16(R, B);
    RB1= _mm_unpackhi_epi16(R, B);
    RB0= _mm_packus_epi16  (RB0, RB1); /* R0 B0 R1 B1 R2 B2 R3 B3 R4 B4 R5 B5 R6 B6 R7 B7 */

    GA0= _mm_unpacklo_epi16(G, A);
    GA1= _mm_unpackhi_epi16(G, A);
    GA0= _mm_packus_epi16  (GA0, GA1);

    _mm_stream_si128(rgba_p++,  _mm_unpacklo_epi8(RB0, GA0));
    _mm_stream_si128(rgba_p++,  _mm_unpackhi_epi8(RB0, GA0));
  }
}


/* convert RGB24 to YUV422 */
void UYVY_to_RGB_SSE2(const unsigned char *yuvdata, 
                       size_t size, 
                       unsigned char *rgbdata)
{
  const __m128i *yuv_p  = (const __m128i*)yuvdata;  /* 4*2 YUV pixels */
  
  const __m128i Y2RGB = _mm_set_epi16(YUV2RGB_11,0,YUV2RGB_11,0,YUV2RGB_11,0,YUV2RGB_11,0);
  const __m128i UV2R  = _mm_set_epi16(YUV2RGB_13, YUV2RGB_12, YUV2RGB_13, YUV2RGB_12, 
                                      YUV2RGB_13, YUV2RGB_12, YUV2RGB_13, YUV2RGB_12);
  const __m128i UV2G  = _mm_set_epi16(YUV2RGB_23, YUV2RGB_22, YUV2RGB_23, YUV2RGB_22, 
                                      YUV2RGB_23, YUV2RGB_22, YUV2RGB_23, YUV2RGB_22);
  const __m128i UV2B  = _mm_set_epi16(YUV2RGB_33, YUV2RGB_32, YUV2RGB_33, YUV2RGB_32, 
                                      YUV2RGB_33, YUV2RGB_32, YUV2RGB_33, YUV2RGB_32);
  const __m128i offset= _mm_set_epi16(Y_OFFSET, UV_OFFSET, Y_OFFSET, UV_OFFSET, 
                                      Y_OFFSET, UV_OFFSET, Y_OFFSET, UV_OFFSET);
  const __m128i  A32  = _mm_set_epi32(255, 255, 255, 255);
  const __m128i  all  =  _mm_set_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
                                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

  /* nomenclatura:
   *   lower-case letters denote  8bit values (like "r" is red, 8bit)
   *   upper-case letters denote 16bit (or 32bit) values (like "G" is green, 16bit)
   */

  __m128i uyvy, UYVY0, UYVY1;
  __m128i UV, YZ, Y, Z;
  __m128i UV_R, UV_G, UV_B;
  __m128i R, G, B, A;
  __m128i RB0, RB1, GA0, GA1;

  vector_128 v0, v1;

  const int shuffle =  _MM_SHUFFLE(3, 1, 2, 0);

  int i=size>>3; /* we do 2*128bit per cycle: this is 2*4*32bit == 8 pixels */
  while(i-->0){
    uyvy=*yuv_p++; /* u0 y0 v0 z0 u1 y1 v1 z1 u2 y2 v2 z2 u3 y3 v3 z3 */

    UYVY0 = _mm_unpacklo_epi8(uyvy, _mm_setzero_si128()); /* U0 Y0 V0 Z0 U1 Y1 V1 Z1 */
    UYVY1 = _mm_unpackhi_epi8(uyvy, _mm_setzero_si128()); /* U2 Y2 V2 Z2 U3 Y3 V3 Z3 */

    UYVY0 = _mm_sub_epi16(UYVY0, offset);
    UYVY1 = _mm_sub_epi16(UYVY1, offset);

    UYVY0 = _mm_shufflelo_epi16(UYVY0, shuffle);
    UYVY0 = _mm_shufflehi_epi16(UYVY0, shuffle);
    UYVY0 = _mm_shuffle_epi32  (UYVY0, shuffle); /* U0 V0 U1 V1 Y0 Z0 Y1 Z1 */

    UYVY1 = _mm_shufflelo_epi16(UYVY1, shuffle);
    UYVY1 = _mm_shufflehi_epi16(UYVY1, shuffle); 
    UYVY1 = _mm_shuffle_epi32  (UYVY1, shuffle); /* U2 V2 U3 V3 Y2 Z2 Y3 Z3 */

    UV = _mm_unpacklo_epi32(UYVY0, UYVY1); /* U0 V0 U2 V2 U1 V1 U3 V3 */
    YZ = _mm_unpackhi_epi32(UYVY0, UYVY1); /* Y0 Z0 Y2 Z2 Y1 Z1 Y3 Z3 */

    Z = _mm_madd_epi16(YZ, Y2RGB);                    /* Z0' Z2' Z1' Z3' */
    Y = _mm_madd_epi16(YZ, _mm_srli_si128(Y2RGB, 2)); /* Y0' Y2' Y1' Y3' */

    UV_R = _mm_madd_epi16(UV, UV2R);
    UV_G = _mm_madd_epi16(UV, UV2G);
    UV_B = _mm_madd_epi16(UV, UV2B);

    R  = _mm_srai_epi32(_mm_add_epi32(Y, UV_R), 8);
    G  = _mm_srai_epi32(_mm_add_epi32(Y, UV_G), 8);
    B  = _mm_srai_epi32(_mm_add_epi32(Y, UV_B), 8);

    RB0 = _mm_packs_epi32(R, G);
    GA0 = _mm_packs_epi32(B, _mm_setzero_si128());

    R  = _mm_srai_epi32(_mm_add_epi32(Z, UV_R), 8);
    G  = _mm_srai_epi32(_mm_add_epi32(Z, UV_G), 8);
    B  = _mm_srai_epi32(_mm_add_epi32(Z, UV_B), 8);

    RB1 = _mm_packs_epi32(R, G);
    GA1 = _mm_packs_epi32(B, _mm_setzero_si128());

    v0.v= _mm_packus_epi16  (RB0, GA0);
    v1.v= _mm_packus_epi16  (RB1, GA1);

    rgbdata[chRed   ]=v0.c[ 0];
    rgbdata[chGreen ]=v0.c[ 4];
    rgbdata[chBlue  ]=v0.c[ 8];
    rgbdata+=3;

    rgbdata[chRed   ]=v1.c[ 0];
    rgbdata[chGreen ]=v1.c[ 4];
    rgbdata[chBlue  ]=v1.c[ 8];
    rgbdata+=3;

    rgbdata[chRed   ]=v0.c[ 2];
    rgbdata[chGreen ]=v0.c[ 6];
    rgbdata[chBlue  ]=v0.c[10];
    rgbdata+=3;

    rgbdata[chRed   ]=v1.c[ 2];
    rgbdata[chGreen ]=v1.c[ 6];
    rgbdata[chBlue  ]=v1.c[10];
    rgbdata+=3;

    rgbdata[chRed   ]=v0.c[ 1];
    rgbdata[chGreen ]=v0.c[ 5];
    rgbdata[chBlue  ]=v0.c[ 9];
    rgbdata+=3;

    rgbdata[chRed   ]=v1.c[ 1];
    rgbdata[chGreen ]=v1.c[ 5];
    rgbdata[chBlue  ]=v1.c[ 9];
    rgbdata+=3;

    rgbdata[chRed   ]=v0.c[ 3];
    rgbdata[chGreen ]=v0.c[ 7];
    rgbdata[chBlue  ]=v0.c[11];
    rgbdata+=3;

    rgbdata[chRed   ]=v1.c[ 3];
    rgbdata[chGreen ]=v1.c[ 7];
    rgbdata[chBlue  ]=v1.c[11];
    rgbdata+=3;
  }
}
#endif
