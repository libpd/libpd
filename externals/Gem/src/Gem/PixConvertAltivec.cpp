#ifdef __VEC__
/////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file for AltiVec-optimized color-conversion routines
//
//    Copyright (c) 2005-2006 James Tittle
//    Copyright (c) 2005-2006 Chris Clepper
//    Copyright (c) 2006-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "PixConvert.h"

void RGB_to_YCbCr_altivec(const unsigned char *rgbdata, size_t RGB_size, 
                          unsigned char *pixels)
{
  vector signed short  r0, r1, r2, g0, g1, g2, b0, b1, b2, c0, c16, c128;
  vector unsigned char z0, tc0, tc1, tc2, tc3;
  vector signed short tr0, tr1, tg0, tg1, tb0, tb1;
  vector signed short t0, t1, t2, t3, t4, t5;
  unsigned int i;
  
  const vector unsigned char	*RGB_ptr = reinterpret_cast<const vector unsigned char*>( rgbdata);
  vector unsigned char	*YCC_ptr = reinterpret_cast<vector unsigned char*>( pixels);

  /* Permutation vector is used to extract the interleaved RGB. */
  vector unsigned char vPerm1 =
    static_cast<vector unsigned char>( 0,  3,  6,  9, 12, 15, 18, 21, /* R0..R7    */
                            1,  4,  7, 10, 13, 16, 19, 22  /* G0..G7    */);
  vector unsigned char vPerm2 =
    static_cast<vector unsigned char>( 2,  5,  8, 11, 14, 17, 20, 23, /* B0..B7    */
                            0,  0,  0,  0,  0,  0,  0,  0  /* dont care */);
  vector unsigned char vPerm3 =
    static_cast<vector unsigned char>( 8, 11, 14, 17, 20, 23, 26, 29, /* R8..R15   */
                            9, 12, 15, 18, 21, 24, 27, 30  /* G8..G15   */);
  vector unsigned char vPerm4 =
    static_cast<vector unsigned char>(10, 13, 16, 19, 22, 25, 28, 31, /* B8..B15   */
                           0,  0,  0,  0,  0,  0,  0,  0  /* dont care */);

  /* Load the equation constants. */
  vector signed short vConst1 =
    static_cast<vector signed short>( 8432,  16425,  3176,
                           -4818,  -9527, 14345,
                           0,      0 );

  vector signed short vConst2 =
    static_cast<vector signed short>( 14345, -12045, -2300,
                           16, 128, 0, 0, 0 );

  r0 = vec_splat( vConst1, 0 ); /*  8432 */
  g0 = vec_splat( vConst1, 1 ); /* 16425 */
  b0 = vec_splat( vConst1, 2 ); /*  3176 */
  r1 = vec_splat( vConst1, 3 ); /* -4818 */
  g1 = vec_splat( vConst1, 4 ); /* -9527 */
  b1 = vec_splat( vConst1, 5 ); /* 14345 */
  r2 = vec_splat( vConst2, 0 ); /* 14345 */
  g2 = vec_splat( vConst2, 1 ); /*-12045 */
  b2 = vec_splat( vConst2, 2 ); /* -2300 */
  c16  = vec_splat( vConst2, 3 ); /*  16 */
  c128 = vec_splat( vConst2, 4 ); /* 128 */
  c0 = static_cast<vector signed short> (0); /*   0 */
  z0 = static_cast<vector unsigned char> (0); /*  0 */

  for ( i = 0; i < (RGB_size/sizeof(vector unsigned char)); i+=3 ) {

    /* Load the 3 RGB input vectors and seperate into red,
       green and blue from the interleaved format. */
    tc0 = vec_perm( RGB_ptr[i], RGB_ptr[i+1], vPerm1 );   /* R0..R7  G0..G7  */
    tc1 = vec_perm( RGB_ptr[i], RGB_ptr[i+1], vPerm2 );   /* B0..B7          */
    tc2 = vec_perm( RGB_ptr[i+1], RGB_ptr[i+2], vPerm3 ); /* R8..R15 G8..G15 */
    tc3 = vec_perm( RGB_ptr[i+1], RGB_ptr[i+2], vPerm4 ); /* B8..B15         */

    /* Unpack to 16 bit arithmatic for converstion. */
    tr0 = static_cast<vector signed short>(vec_mergeh( z0, tc0 ));  // tr0 = R0 .. R7
    tg0 = static_cast<vector signed short>(vec_mergel( z0, tc0 ));  // tg0 = G0 .. G7
    tb0 = static_cast<vector signed short>(vec_mergeh( z0, tc1 ));  // tb0 = B0 .. B7
    tr1 = static_cast<vector signed short>(vec_mergeh( z0, tc2 ));  // tr0 = R8 .. R15
    tg1 = static_cast<vector signed short>(vec_mergel( z0, tc2 ));  // tg0 = G8 .. G15
    tb1 = static_cast<vector signed short>(vec_mergeh( z0, tc3 ));  // tb0 = B8 .. B15

    /* Convert the first three input vectors.  Note that
       only the top 17 bits of the 32 bit product are
       stored.  This is the same as doing the divide by 32768. */

    t0 = vec_mradds( tr0, r0, c0 ); /* (R0 .. R7) *  8432 */
    t1 = vec_mradds( tr0, r1, c0 ); /* (R0 .. R7) * -4818 */
    t2 = vec_mradds( tr0, r2, c0 ); /* (R0 .. R7) * 14345 */

    t0 = vec_mradds( tg0, g0, t0 ); /* += (G0 .. G7) *  16425 */
    t1 = vec_mradds( tg0, g1, t1 ); /* += (G0 .. G7) *  -9527 */
    t2 = vec_mradds( tg0, g2, t2 ); /* += (G0 .. G7) * -12045 */

    t0 = vec_mradds( tb0, b0, t0 ); /* += (B0 .. B7) *  3176 */
    t1 = vec_mradds( tb0, b1, t1 ); /* += (B0 .. B7) * 14345 */
    t2 = vec_mradds( tb0, b2, t2 ); /* += (B0 .. B7) * -2300 */

    /* Convert the next three input vectors. */
    t3 = vec_mradds( tr1, r0, c0 ); /* (R8 .. R15) *  8432 */
    t4 = vec_mradds( tr1, r1, c0 ); /* (R8 .. R15) * -4818 */
    t5 = vec_mradds( tr1, r2, c0 ); /* (R8 .. R15) * 14345 */

    t3 = vec_mradds( tg1, g0, t3 ); /* += (G8 .. G15) *  16425 */
    t4 = vec_mradds( tg1, g1, t4 ); /* += (G8 .. G15) *  -9527 */
    t5 = vec_mradds( tg1, g2, t5 ); /* += (G8 .. G15) * -12045 */

    t3 = vec_mradds( tb1, b0, t3 ); /* += (B8 .. B15) *  3176 */
    t4 = vec_mradds( tb1, b1, t4 ); /* += (B8 .. B15) * 14345 */
    t5 = vec_mradds( tb1, b2, t5 ); /* += (B8 .. B15) * -2300 */

    /* Add the constants. */
    t0 = vec_adds( t0, c16 );
    t3 = vec_adds( t3, c16 );
    t1 = vec_adds( t1, c128 );
    t4 = vec_adds( t4, c128 );
    t2 = vec_adds( t2, c128 );
    t5 = vec_adds( t5, c128 );

    /* Pack the results, and store them. */
    YCC_ptr[i]   = vec_packsu( t0, t3 );  /*  Y0 .. Y15  */
    YCC_ptr[i+1] = vec_packsu( t1, t4 );  /* Cb0 .. Cb15 */
    YCC_ptr[i+2] = vec_packsu( t2, t5 );  /* Cr0 .. Cr15 */

  }
}
void RGBA_to_YCbCr_altivec(const unsigned char *rgbadata, size_t RGBA_size, 
                           unsigned char *pixels)
{
  vector signed short  r0, r1, r2, g0, g1, g2, b0, b1, b2, c0, c16, c128;
  vector unsigned char z0, tc0, tc1, tc2, tc3;
  vector signed short tr0, tr1, tg0, tg1, tb0, tb1;
  vector signed short t0, t1, t2, t3, t4, t5;
  unsigned int i;
  
  const vector unsigned char	*RGBA_ptr = reinterpret_cast<const vector unsigned char*>( rgbadata);
  vector unsigned char	*YCC_ptr = reinterpret_cast<vector unsigned char*>( pixels);

  /* Permutation vector is used to extract the interleaved RGBA. */
  vector unsigned char vPerm1 =
    static_cast<vector unsigned char>( 0,  4,  8, 12, 16, 20, 24, 28, /* R0..R7    */
                            1,  5,  9, 13, 17, 21, 25, 29  /* G0..G7    */);
  vector unsigned char vPerm2 =
    static_cast<vector unsigned char>( 2,  6, 10, 14, 18, 22, 26, 30, /* B0..B7    */
                            0,  0,  0,  0,  0,  0,  0,  0  /* dont care */);
  vector unsigned char vPerm3 =
    static_cast<vector unsigned char>( 8, 12, 16, 20, 24, 28, 32, 36, /* R8..R15   */
                            9, 13, 17, 21, 25, 29, 33, 37  /* G8..G15   */);
  vector unsigned char vPerm4 =
    static_cast<vector unsigned char>(10, 14, 18, 22, 26, 30, 34, 38, /* B8..B15   */
                           0,  0,  0,  0,  0,  0,  0,  0  /* dont care */);

  /* Load the equation constants. */
  vector signed short vConst1 =
    static_cast<vector signed short>( 8432,  16425,  3176,
                           -4818,  -9527, 14345,
                           0,      0 );

  vector signed short vConst2 =
    static_cast<vector signed short>( 14345, -12045, -2300,
                           16, 128, 0, 0, 0 );

  r0 = vec_splat( vConst1, 0 ); /*  8432 */
  g0 = vec_splat( vConst1, 1 ); /* 16425 */
  b0 = vec_splat( vConst1, 2 ); /*  3176 */
  r1 = vec_splat( vConst1, 3 ); /* -4818 */
  g1 = vec_splat( vConst1, 4 ); /* -9527 */
  b1 = vec_splat( vConst1, 5 ); /* 14345 */
  r2 = vec_splat( vConst2, 0 ); /* 14345 */
  g2 = vec_splat( vConst2, 1 ); /*-12045 */
  b2 = vec_splat( vConst2, 2 ); /* -2300 */
  c16  = vec_splat( vConst2, 3 ); /*  16 */
  c128 = vec_splat( vConst2, 4 ); /* 128 */
  c0 = static_cast<vector signed short> (0); /*   0 */
  z0 = static_cast<vector unsigned char> (0); /*  0 */

  for ( i = 0; i < (RGBA_size/sizeof(vector unsigned char)); i+=3 ) {

    /* Load the 3 RGB input vectors and seperate into red,
       green and blue from the interleaved format. */
    tc0 = vec_perm( RGBA_ptr[i], RGBA_ptr[i+1], vPerm1 );   /* R0..R7  G0..G7  */
    tc1 = vec_perm( RGBA_ptr[i], RGBA_ptr[i+1], vPerm2 );   /* B0..B7          */
    tc2 = vec_perm( RGBA_ptr[i+1], RGBA_ptr[i+2], vPerm3 ); /* R8..R15 G8..G15 */
    tc3 = vec_perm( RGBA_ptr[i+1], RGBA_ptr[i+2], vPerm4 ); /* B8..B15         */

    /* Unpack to 16 bit arithmatic for converstion. */
    tr0 = static_cast<vector signed short>(vec_mergeh( z0, tc0 ));  /* tr0 = R0 .. R7  */
    tg0 = static_cast<vector signed short>(vec_mergel( z0, tc0 ));  /* tg0 = G0 .. G7  */
    tb0 = static_cast<vector signed short>(vec_mergeh( z0, tc1 ));  /* tb0 = B0 .. B7  */
    tr1 = static_cast<vector signed short>(vec_mergeh( z0, tc2 ));  /* tr0 = R8 .. R15 */
    tg1 = static_cast<vector signed short>(vec_mergel( z0, tc2 ));  /* tg0 = G8 .. G15 */
    tb1 = static_cast<vector signed short>(vec_mergeh( z0, tc3 ));  /* tb0 = B8 .. B15 */

    /* Convert the first three input vectors.  Note that
       only the top 17 bits of the 32 bit product are
       stored.  This is the same as doing the divide by 32768. */

    t0 = vec_mradds( tr0, r0, c0 ); /* (R0 .. R7) *  8432 */
    t1 = vec_mradds( tr0, r1, c0 ); /* (R0 .. R7) * -4818 */
    t2 = vec_mradds( tr0, r2, c0 ); /* (R0 .. R7) * 14345 */

    t0 = vec_mradds( tg0, g0, t0 ); /* += (G0 .. G7) *  16425 */
    t1 = vec_mradds( tg0, g1, t1 ); /* += (G0 .. G7) *  -9527 */
    t2 = vec_mradds( tg0, g2, t2 ); /* += (G0 .. G7) * -12045 */

    t0 = vec_mradds( tb0, b0, t0 ); /* += (B0 .. B7) *  3176 */
    t1 = vec_mradds( tb0, b1, t1 ); /* += (B0 .. B7) * 14345 */
    t2 = vec_mradds( tb0, b2, t2 ); /* += (B0 .. B7) * -2300 */

    /* Convert the next three input vectors. */
    t3 = vec_mradds( tr1, r0, c0 ); /* (R8 .. R15) *  8432 */
    t4 = vec_mradds( tr1, r1, c0 ); /* (R8 .. R15) * -4818 */
    t5 = vec_mradds( tr1, r2, c0 ); /* (R8 .. R15) * 14345 */

    t3 = vec_mradds( tg1, g0, t3 ); /* += (G8 .. G15) *  16425 */
    t4 = vec_mradds( tg1, g1, t4 ); /* += (G8 .. G15) *  -9527 */
    t5 = vec_mradds( tg1, g2, t5 ); /* += (G8 .. G15) * -12045 */

    t3 = vec_mradds( tb1, b0, t3 ); /* += (B8 .. B15) *  3176 */
    t4 = vec_mradds( tb1, b1, t4 ); /* += (B8 .. B15) * 14345 */
    t5 = vec_mradds( tb1, b2, t5 ); /* += (B8 .. B15) * -2300 */

    /* Add the constants. */
    t0 = vec_adds( t0, c16 );
    t3 = vec_adds( t3, c16 );
    t1 = vec_adds( t1, c128 );
    t4 = vec_adds( t4, c128 );
    t2 = vec_adds( t2, c128 );
    t5 = vec_adds( t5, c128 );

    /* Pack the results, and store them. */
    YCC_ptr[i]   = vec_packsu( t0, t3 );  /*  Y0 .. Y15  */
    YCC_ptr[i+1] = vec_packsu( t1, t4 );  /* Cb0 .. Cb15 */
    YCC_ptr[i+2] = vec_packsu( t2, t5 );  /* Cr0 .. Cr15 */

  }
}

void BGR_to_YCbCr_altivec(const unsigned char *bgrdata, size_t BGR_size, 
                          unsigned char *pixels)
{
  vector signed short  r0, r1, r2, g0, g1, g2, b0, b1, b2, c0, c16, c128;
  vector unsigned char z0, tc0, tc1, tc2, tc3;
  vector signed short tr0, tr1, tg0, tg1, tb0, tb1;
  vector signed short t0, t1, t2, t3, t4, t5;
  unsigned int i;
  
  const vector unsigned char	*BGR_ptr = reinterpret_cast<const vector unsigned char*>( bgrdata);
  vector unsigned char	*YCC_ptr = reinterpret_cast<vector unsigned char*>( pixels);

  /* Permutation vector is used to extract the interleaved RGB. */
  vector unsigned char vPerm1 =
    static_cast<vector unsigned char>( 0,  3,  6,  9, 12, 15, 18, 21, /* R0..R7    */
                            1,  4,  7, 10, 13, 16, 19, 22  /* G0..G7    */);
  vector unsigned char vPerm2 =
    static_cast<vector unsigned char>( 2,  5,  8, 11, 14, 17, 20, 23, /* B0..B7    */
                            0,  0,  0,  0,  0,  0,  0,  0  /* dont care */);
  vector unsigned char vPerm3 =
    static_cast<vector unsigned char>( 8, 11, 14, 17, 20, 23, 26, 29, /* R8..R15   */
                            9, 12, 15, 18, 21, 24, 27, 30  /* G8..G15   */);
  vector unsigned char vPerm4 =
    static_cast<vector unsigned char>(10, 13, 16, 19, 22, 25, 28, 31, /* B8..B15   */
                           0,  0,  0,  0,  0,  0,  0,  0  /* dont care */);

  /* Load the equation constants. */
  vector signed short vConst1 =
    static_cast<vector signed short>( 8432,  16425,  3176,
                           -4818,  -9527, 14345,
                           0,      0 );

  vector signed short vConst2 =
    static_cast<vector signed short>( 14345, -12045, -2300,
                           16, 128, 0, 0, 0 );

  r0 = vec_splat( vConst1, 0 ); /*  8432 */
  g0 = vec_splat( vConst1, 1 ); /* 16425 */
  b0 = vec_splat( vConst1, 2 ); /*  3176 */
  r1 = vec_splat( vConst1, 3 ); /* -4818 */
  g1 = vec_splat( vConst1, 4 ); /* -9527 */
  b1 = vec_splat( vConst1, 5 ); /* 14345 */
  r2 = vec_splat( vConst2, 0 ); /* 14345 */
  g2 = vec_splat( vConst2, 1 ); /*-12045 */
  b2 = vec_splat( vConst2, 2 ); /* -2300 */
  c16  = vec_splat( vConst2, 3 ); /*  16 */
  c128 = vec_splat( vConst2, 4 ); /* 128 */
  c0 = static_cast<vector signed short> (0); /*   0 */
  z0 = static_cast<vector unsigned char> (0); /*  0 */

  for ( i = 0; i < (BGR_size/sizeof(vector unsigned char)); i+=3 ) {

    /* Load the 3 RGB input vectors and seperate into red,
       green and blue from the interleaved format. */
    tc0 = vec_perm( BGR_ptr[i], BGR_ptr[i+1], vPerm1 );   /* R0..R7  G0..G7  */
    tc1 = vec_perm( BGR_ptr[i], BGR_ptr[i+1], vPerm2 );   /* B0..B7          */
    tc2 = vec_perm( BGR_ptr[i+1], BGR_ptr[i+2], vPerm3 ); /* R8..R15 G8..G15 */
    tc3 = vec_perm( BGR_ptr[i+1], BGR_ptr[i+2], vPerm4 ); /* B8..B15         */

    /* Unpack to 16 bit arithmatic for converstion. */
    tr0 = static_cast<vector signed short>(vec_mergeh( z0, tc0 ));  /* tr0 = R0 .. R7  */
    tg0 = static_cast<vector signed short>(vec_mergel( z0, tc0 ));  /* tg0 = G0 .. G7  */
    tb0 = static_cast<vector signed short>(vec_mergeh( z0, tc1 ));  /* tb0 = B0 .. B7  */
    tr1 = static_cast<vector signed short>(vec_mergeh( z0, tc2 ));  /* tr0 = R8 .. R15 */
    tg1 = static_cast<vector signed short>(vec_mergel( z0, tc2 ));  /* tg0 = G8 .. G15 */
    tb1 = static_cast<vector signed short>(vec_mergeh( z0, tc3 ));  /* tb0 = B8 .. B15 */

    /* Convert the first three input vectors.  Note that
       only the top 17 bits of the 32 bit product are
       stored.  This is the same as doing the divide by 32768. */

    t0 = vec_mradds( tr0, r0, c0 ); /* (R0 .. R7) *  8432 */
    t1 = vec_mradds( tr0, r1, c0 ); /* (R0 .. R7) * -4818 */
    t2 = vec_mradds( tr0, r2, c0 ); /* (R0 .. R7) * 14345 */

    t0 = vec_mradds( tg0, g0, t0 ); /* += (G0 .. G7) *  16425 */
    t1 = vec_mradds( tg0, g1, t1 ); /* += (G0 .. G7) *  -9527 */
    t2 = vec_mradds( tg0, g2, t2 ); /* += (G0 .. G7) * -12045 */

    t0 = vec_mradds( tb0, b0, t0 ); /* += (B0 .. B7) *  3176 */
    t1 = vec_mradds( tb0, b1, t1 ); /* += (B0 .. B7) * 14345 */
    t2 = vec_mradds( tb0, b2, t2 ); /* += (B0 .. B7) * -2300 */

    /* Convert the next three input vectors. */
    t3 = vec_mradds( tr1, r0, c0 ); /* (R8 .. R15) *  8432 */
    t4 = vec_mradds( tr1, r1, c0 ); /* (R8 .. R15) * -4818 */
    t5 = vec_mradds( tr1, r2, c0 ); /* (R8 .. R15) * 14345 */

    t3 = vec_mradds( tg1, g0, t3 ); /* += (G8 .. G15) *  16425 */
    t4 = vec_mradds( tg1, g1, t4 ); /* += (G8 .. G15) *  -9527 */
    t5 = vec_mradds( tg1, g2, t5 ); /* += (G8 .. G15) * -12045 */

    t3 = vec_mradds( tb1, b0, t3 ); /* += (B8 .. B15) *  3176 */
    t4 = vec_mradds( tb1, b1, t4 ); /* += (B8 .. B15) * 14345 */
    t5 = vec_mradds( tb1, b2, t5 ); /* += (B8 .. B15) * -2300 */

    /* Add the constants. */
    t0 = vec_adds( t0, c16 );
    t3 = vec_adds( t3, c16 );
    t1 = vec_adds( t1, c128 );
    t4 = vec_adds( t4, c128 );
    t2 = vec_adds( t2, c128 );
    t5 = vec_adds( t5, c128 );

    /* Pack the results, and store them. */
    YCC_ptr[i]   = vec_packsu( t0, t3 );  /*  Y0 .. Y15  */
    YCC_ptr[i+1] = vec_packsu( t1, t4 );  /* Cb0 .. Cb15 */
    YCC_ptr[i+2] = vec_packsu( t2, t5 );  /* Cr0 .. Cr15 */

  }
}

void BGRA_to_YCbCr_altivec(const unsigned char *bgradata, size_t BGRA_size, 
                           unsigned char *pixels)
{
  vector signed short  r0, r1, r2, g0, g1, g2, b0, b1, b2, c0, c16, c128;
  vector unsigned char z0, tc0, tc1, tc2, tc3;
  vector signed short tr0, tr1, tg0, tg1, tb0, tb1;
  vector signed short t0, t1, t2, t3, t4, t5;
  vector signed short u1, u2, uAvg, v1, v2, vAvg, out1, out2, out3, out4, uv1, uv2;
  unsigned int i;
  
  const vector unsigned char	*BGRA_ptr = reinterpret_cast<const vector unsigned char*>( bgradata);
  vector unsigned char	*UYVY_ptr = reinterpret_cast<vector unsigned char*>( pixels);

  /* Permutation vector is used to extract the interleaved BGRA. */
  vector unsigned char vPerm1 =
    static_cast<vector unsigned char>( 3,  7, 11, 15, 19, 23, 27, 31, // B0..B7    
                            2,  6, 10, 14, 18, 22, 26, 30  /* G0..G7    */);
  vector unsigned char vPerm2 =
    static_cast<vector unsigned char>( 1,  5,  9, 13, 17, 21, 25, 29, /* R0..R7    */
                            0,  0,  0,  0,  0,  0,  0,  0  /* dont care */);

  /* Load the equation constants. */
  vector signed short vConst1 =
    static_cast<vector signed short>( 8432,  16425,  3176,
                           -4818,  -9527, 14345,
                           0,      0 );
  vector signed short vConst2 =
    static_cast<vector signed short>( 14345, -12045, -2300,
                           16, 128, 0, 0, 0 );
							  
  vector unsigned char avgPerm1 =
    static_cast<vector unsigned char>(  0,  1,  4,  5,  8,  9, 12, 13,
                             16, 17, 20, 21, 24, 25, 28, 29 );
  vector unsigned char avgPerm2 =
    static_cast<vector unsigned char>(  2,  3,  6,  7, 10, 11, 14, 15,
                             18, 19, 22, 23, 26, 27, 30, 31 );
  vector unsigned char Perm1 = 
    static_cast<vector unsigned char>( 0, 1, 16, 17, 2, 3, 18, 19,
                            4, 5, 20, 21, 6, 7, 22, 23 );
  vector unsigned char Perm2 = 
    static_cast<vector unsigned char>(  8,  9, 24, 25, 10, 11, 26, 27,
                             12, 13, 28, 29, 14, 15, 30, 31 );
								 							
  r0 = vec_splat( vConst1, 2 ); /*  8432 */
  g0 = vec_splat( vConst1, 1 ); /* 16425 */
  b0 = vec_splat( vConst1, 0 ); /*  3176 */
  r1 = vec_splat( vConst1, 5 ); /* -4818 */
  g1 = vec_splat( vConst1, 4 ); /* -9527 */
  b1 = vec_splat( vConst1, 3 ); /* 14345 */
  r2 = vec_splat( vConst2, 2 ); /* 14345 */
  g2 = vec_splat( vConst2, 1 ); /*-12045 */
  b2 = vec_splat( vConst2, 0 ); /* -2300 */
  c16  = vec_splat( vConst2, 3 ); /*  16 */
  c128 = vec_splat( vConst2, 4 ); /* 128 */
  c0 = static_cast<vector signed short> (0); /*   0 */
  z0 = static_cast<vector unsigned char> (0); /*  0 */

  for ( i = 0; i < (BGRA_size/sizeof(vector unsigned char)); i++ ) {

    /* Load the 4 BGRA input vectors and seperate into red,
       green and blue from the interleaved format. */
    const vector unsigned char *vec1 = BGRA_ptr++;
    const vector unsigned char *vec2 = BGRA_ptr++;
    const vector unsigned char *vec3 = BGRA_ptr++;
    const vector unsigned char *vec4 = BGRA_ptr++;
	
    tc0 = vec_perm( *vec1, *vec2, vPerm1 ); // B0..B7  G0..G7
    tc1 = vec_perm( *vec1, *vec2, vPerm2 ); // R0..R7
    tc2 = vec_perm( *vec3, *vec4, vPerm1 ); // B8..B15 G8..G15
    tc3 = vec_perm( *vec3, *vec4, vPerm2 ); // R8..R15

    /* Unpack to 16 bit arithmatic for conversion. */
    tr0 = static_cast<vector signed short>(vec_mergeh( z0, tc0 ));  /* tr0 = R0 .. R7  */
    tg0 = static_cast<vector signed short>(vec_mergel( z0, tc0 ));  /* tg0 = G0 .. G7  */
    tb0 = static_cast<vector signed short>(vec_mergeh( z0, tc1 ));  /* tb0 = B0 .. B7  */
    tr1 = static_cast<vector signed short>(vec_mergeh( z0, tc2 ));  /* tr0 = R8 .. R15 */
    tg1 = static_cast<vector signed short>(vec_mergel( z0, tc2 ));  /* tg0 = G8 .. G15 */
    tb1 = static_cast<vector signed short>(vec_mergeh( z0, tc3 ));  /* tb0 = B8 .. B15 */

    /* Convert the first three input vectors.  Note that
       only the top 17 bits of the 32 bit product are
       stored.  This is the same as doing the divide by 32768. */

    t0 = vec_mradds( tr0, r0, c0 ); /* (R0 .. R7) *  8432 */
    t1 = vec_mradds( tr0, r1, c0 ); /* (R0 .. R7) * -4818 */
    t2 = vec_mradds( tr0, r2, c0 ); /* (R0 .. R7) * 14345 */

    t0 = vec_mradds( tg0, g0, t0 ); /* += (G0 .. G7) *  16425 */
    t1 = vec_mradds( tg0, g1, t1 ); /* += (G0 .. G7) *  -9527 */
    t2 = vec_mradds( tg0, g2, t2 ); /* += (G0 .. G7) * -12045 */

    t0 = vec_mradds( tb0, b0, t0 ); /* += (B0 .. B7) *  3176 */
    t1 = vec_mradds( tb0, b1, t1 ); /* += (B0 .. B7) * 14345 */
    t2 = vec_mradds( tb0, b2, t2 ); /* += (B0 .. B7) * -2300 */

    /* Convert the next three input vectors. */
    t3 = vec_mradds( tr1, r0, c0 ); /* (R8 .. R15) *  8432 */
    t4 = vec_mradds( tr1, r1, c0 ); /* (R8 .. R15) * -4818 */
    t5 = vec_mradds( tr1, r2, c0 ); /* (R8 .. R15) * 14345 */

    t3 = vec_mradds( tg1, g0, t3 ); /* += (G8 .. G15) *  16425 */
    t4 = vec_mradds( tg1, g1, t4 ); /* += (G8 .. G15) *  -9527 */
    t5 = vec_mradds( tg1, g2, t5 ); /* += (G8 .. G15) * -12045 */

    t3 = vec_mradds( tb1, b0, t3 ); /* += (B8 .. B15) *  3176 */
    t4 = vec_mradds( tb1, b1, t4 ); /* += (B8 .. B15) * 14345 */
    t5 = vec_mradds( tb1, b2, t5 ); /* += (B8 .. B15) * -2300 */

    /* Add the constants. */
    t0 = vec_adds( t0, c16 );
    t3 = vec_adds( t3, c16 );
    t1 = vec_adds( t1, c128 );
    t4 = vec_adds( t4, c128 );
    t2 = vec_adds( t2, c128 );
    t5 = vec_adds( t5, c128 );
	
    u1 = vec_perm( t1, t4, avgPerm1 ); // rearrange U's for averaging
    u2 = vec_perm( t1, t4, avgPerm2 );
    uAvg = vec_avg( u1, u2 );
    v1 = vec_perm( t2, t5, avgPerm1 ); // rearrange V's for averaging
    v2 = vec_perm( t2, t5, avgPerm2 );
    vAvg = vec_avg( v1, v2 );
	
    uv1 = vec_perm( uAvg, vAvg, Perm1 );
    uv2 = vec_perm( uAvg, vAvg, Perm2 );
    out1 = vec_perm( uv1, t0, Perm1 );
    out2 = vec_perm( uv1, t0, Perm2 );
    out3 = vec_perm( uv2, t3, Perm1 );
    out4 = vec_perm( uv2, t3, Perm2 );

    *UYVY_ptr = vec_packsu( out1, out2 );	// pack down to char's
    UYVY_ptr++;
    *UYVY_ptr = vec_packsu( out3, out4 );
    UYVY_ptr++;
  }
}

void YV12_to_YUV422_altivec(const short*Y, const short*U, const short*V, 
			    unsigned char *data, int xsize, int ysize)
{
  // from geowar@apple.com, 3/15/2005
  // #1. Don't use the pointers. Use vec_ld with an index that you increment (by 16) instead.
  vector unsigned char *pixels1=reinterpret_cast<vector unsigned char *>(data);
  vector unsigned char *pixels2=reinterpret_cast<vector unsigned char *>(data+(xsize*2));
  const vector unsigned short *py1 = reinterpret_cast<const vector unsigned short *>(Y);
  const vector unsigned short *py2 = reinterpret_cast<const vector unsigned short *>(Y + xsize );
  const vector unsigned short *pu = reinterpret_cast<const vector unsigned short *>(U);
  const vector unsigned short *pv = reinterpret_cast<const vector unsigned short *>(V);
  vector unsigned short uvAdd = static_cast<vector unsigned short>( 128, 128, 128, 128,
                                                         128, 128, 128, 128 );
  vector unsigned short yShift = static_cast<vector unsigned short>( 7, 7, 7, 7, 7, 7, 7, 7 );
  vector unsigned short uvShift = static_cast<vector unsigned short>( 8, 8, 8, 8, 8, 8, 8, 8 );
  vector unsigned short tempU, tempV, doneU, doneV, tempY1, tempY2, tempY3, tempY4,
    uv1, uv2, out1, out2, out3, out4, out5, out6, out7, out8;
  vector unsigned char Perm1 = 
    static_cast<vector unsigned char>( 0, 1, 16, 17, 2, 3, 18, 19,
                            4, 5, 20, 21, 6, 7, 22, 23 );
  vector unsigned char Perm2 = 
    static_cast<vector unsigned char>(  8,  9, 24, 25, 10, 11, 26, 27,
                             12, 13, 28, 29, 14, 15, 30, 31 );
  int row=ysize>>1;
  int cols=xsize>>4;
#if 0
# ifndef PPC970
  UInt32	prefetchSize = GetPrefetchConstant( 16, 1, 256 );
  vec_dst( pu, prefetchSize, 0 );
  vec_dst( pv, prefetchSize, 0 );
  vec_dst( py1, prefetchSize, 0 );
  vec_dst( py2, prefetchSize, 0 );
# endif
#endif
  while(row--){
    int col=cols;
    while(col--){
#if 0
# ifndef PPC970
      vec_dst( );
# endif
#endif
      tempU = vec_sra( (*pu++), uvShift );
      tempV = vec_sra( (*pv++), uvShift );
      doneU = vec_add( tempU, uvAdd );
      doneV = vec_add( tempV, uvAdd );
	  
      uv1 = vec_perm( doneU, doneV, Perm1 ); // uvuvuvuv uvuvuvuv
      uv2 = vec_perm( doneU, doneV, Perm2 );
	  
      tempY1 = vec_sra( (*py1++), yShift );
      tempY2 = vec_sra( (*py2++), yShift );
	  
      out1 = vec_perm( uv1, tempY1, Perm1 ); //fill Y's, U's & V's
      out2 = vec_perm( uv1, tempY1, Perm2 );
      out3 = vec_perm( uv1, tempY2, Perm1 ); //fill 2nd Y's, U's & V's
      out4 = vec_perm( uv1, tempY2, Perm2 );
	  
      *pixels1 = vec_packsu( out1, out2 );
      *pixels2 = vec_packsu( out3, out4 );
      pixels1++; pixels2++; 
	  
      tempY3 = vec_sra( (*py1++), yShift ); // load second set of Y's
      tempY4 = vec_sra( (*py2++), yShift );
	  
      out5 = vec_perm( uv2, tempY3, Perm1 );
      out6 = vec_perm( uv2, tempY3, Perm2 );
      out7 = vec_perm( uv2, tempY4, Perm1 );
      out8 = vec_perm( uv2, tempY4, Perm2 );
	  
      *pixels1 = vec_packsu( out5, out6 );
      *pixels2 = vec_packsu( out7, out8 );
      pixels1++; pixels2++;
    }
    pixels1+=(xsize*2)>>4; pixels2+=(xsize*2)>>4;
    py1+=xsize>>3; py2+=xsize>>3;
  }
}

void YUV422_to_YV12_altivec(short*pY, short*pY2, short*pU, short*pV,
			    const unsigned char *gem_image, int xsize, int ysize)
{
  // UYVY UYVY UYVY UYVY
  const vector unsigned char *pixels1=reinterpret_cast<const vector unsigned char *>(gem_image);
  const vector unsigned char *pixels2=reinterpret_cast<const vector unsigned char *>(gem_image+(xsize*2));
  // PDP packet to be filled:
  // first Y plane
  vector signed short *py1 = reinterpret_cast<vector signed short *>(pY);
  // 2nd Y pixel
  vector signed short *py2 = reinterpret_cast<vector signed short *>(pY2);
  // U plane
  vector signed short *pCr = reinterpret_cast<vector signed short *>(pU);
  // V plane
  vector signed short *pCb = reinterpret_cast<vector signed short *>(pV);
  vector signed short uvSub = static_cast<vector signed short>( 128, 128, 128, 128,
													 128, 128, 128, 128 );
  vector unsigned short yShift = static_cast<vector unsigned short>( 7, 7, 7, 7, 7, 7, 7, 7 );
  vector unsigned short uvShift = static_cast<vector unsigned short>( 8, 8, 8, 8, 8, 8, 8, 8 );
  
  vector signed short tempY1, tempY2, tempY3, tempY4,
		tempUV1, tempUV2, tempUV3, tempUV4, tempUV5, tempUV6;

  vector unsigned char uvPerm = static_cast<vector unsigned char>( 16, 0, 17, 4, 18,  8, 19, 12,   // u0..u3
  														20, 2, 21, 6, 22, 10, 23, 14 ); // v0..v3

  vector unsigned char uPerm = static_cast<vector unsigned char>( 0, 1, 2, 3, 4, 5, 6, 7, 
													   16,17,18,19,20,21,22,23);
  vector unsigned char vPerm = static_cast<vector unsigned char>( 8, 9, 10,11,12,13,14,15,
													   24,25,26,27,28,29,30,31);
  
  vector unsigned char yPerm = static_cast<vector unsigned char>( 16, 1, 17,  3, 18,  5, 19,  7, // y0..y3
													   20, 9, 21, 11, 23, 13, 25, 15);// y4..y7
  vector unsigned char zeroVec = static_cast<vector unsigned char>(0);
  
  int row=ysize>>1;
  int cols=xsize>>4;
#if 0
# ifndef PPC970
  UInt32	prefetchSize = GetPrefetchConstant( 16, 1, 256 );
  vec_dst( pu, prefetchSize, 0 );
  vec_dst( pv, prefetchSize, 0 );
  vec_dst( py1, prefetchSize, 0 );
  vec_dst( py2, prefetchSize, 0 );
# endif
#endif  
  while(row--){
    int col=cols;
    while(col--){
#if 0
# ifndef PPC970
      vec_dst( );
# endif
#endif
      tempUV1 = static_cast<vector signed short>(vec_perm( *pixels1, zeroVec, uvPerm));
      tempY1  = static_cast<vector signed short>(vec_perm( *pixels1, zeroVec, yPerm));
      tempY2  = static_cast<vector signed short>(vec_perm( *pixels2, zeroVec, yPerm));
	  pixels1++;pixels2++;
      
      tempUV2 = static_cast<vector signed short>(vec_perm( *pixels1, zeroVec, uvPerm));
      tempY3  = static_cast<vector signed short>(vec_perm( *pixels1, zeroVec, yPerm));
      tempY4  = static_cast<vector signed short>(vec_perm( *pixels2, zeroVec, yPerm));
	  pixels1++;pixels2++;
  
	  tempUV3 = vec_sub( tempUV1, uvSub );
	  tempUV4 = vec_sub( tempUV2, uvSub );
	  tempUV5 = vec_sl( tempUV3, uvShift );
	  tempUV6 = vec_sl( tempUV4, uvShift );
	  
	  *pCb = vec_perm( tempUV5, tempUV6, uPerm );
	  *pCr = vec_perm( tempUV5, tempUV6, vPerm );
	  pCr++; pCb++;

	  *py1++ = vec_sl( tempY1, yShift);
      *py2++ = vec_sl( tempY2, yShift);
      *py1++ = vec_sl( tempY3, yShift);
      *py2++ = vec_sl( tempY4, yShift);      
	}

	py1+=(xsize>>3); py2+=(xsize>>3);
	pixels1+=(xsize*2)>>4; pixels2+=(xsize*2)>>4;
  }
}

#ifdef NO_VECTORINT_TO_VECTORUNSIGNEDINT
# warning disabling AltiVec for older gcc: please fix me
#else
void YUV422_to_BGRA_altivec(const unsigned char *yuvdata, 
			    size_t pixelnum, unsigned char *output)
{
  const vector unsigned char *UYVY_ptr=reinterpret_cast<const vector unsigned char *>(yuvdata);
  vector unsigned char *BGRA_ptr=reinterpret_cast<vector unsigned char *>(output);

  vector unsigned int vShift;
  vector signed short tempU, tempV, tempY, tempUV, out1, out2, out3, out4;

  vector signed short  v16, v128, a255, szero, one;
  vector unsigned char zero;
  vector signed short t0, t1, t2, tempGB1, tempGB2, tempRA1, tempRA2;
  vector signed short vU_G, vV_G, vU_B, vU_R, y0, hiImage, loImage;
  vector unsigned int   uv_rEven, uv_rOdd, uv_rHi, uv_rLo,
					  uv_gUEven, uv_gVEven, uv_gUOdd, uv_gVOdd, uv_gHi, uv_gLo,
					  uv_bEven, uv_bOdd;
  vector signed int	tempUhi, tempUlo, tempVhi, tempVlo;
  vector signed int yEven, yOdd;

  vector unsigned int t0Even, t0Odd, t1Even, t1Odd, t2Even, t2Odd;
  
  /* Load the equation constants. */
  vector signed short vConst =
    static_cast<vector signed short>(298, 519, 409, 16, 128, 255, -100, -210 );

  vector unsigned char vPerm1 = 
    static_cast<vector unsigned char>( 0, 1, 16, 17,  8,  9, 24, 25,
                            2, 3, 18, 19, 10, 11, 26, 27 );
  vector unsigned char vPerm2 = 
    static_cast<vector unsigned char>( 4, 5, 20, 21, 12, 13, 28, 29,
							6, 7, 22, 23, 14, 15, 30, 31 );
							 
  vector unsigned char vPermY =
    static_cast<vector unsigned char>(  2,  3,  6,  7, 10, 11, 14, 15,
	                        18, 19, 22, 23, 26, 27, 30, 31 );
  vector unsigned char vPermU =
    static_cast<vector unsigned char>(  0,  1, 16, 17,  4,  5, 20, 21,
	                         8,  9, 24, 25, 12, 13, 28, 29 );							
  vector unsigned char vPermV =
    static_cast<vector unsigned char>(  2,  3, 18, 19,  6,  7, 22, 23,
							10, 11, 26, 27, 14, 15, 30, 31 );
  vector unsigned char vOutPerm1 =
    static_cast<vector unsigned char>(  0,  1,  2,  3, 16, 17, 18, 19,
	                         4,  5,  6,  7, 20, 21, 22, 23 );
  vector unsigned char vOutPerm2 =
    static_cast<vector unsigned char>(  8,  9, 10, 11, 24, 25, 26, 27,
	                        12, 13, 14, 15, 28, 29, 30, 31 );
  vector unsigned char uvPerm =
    static_cast<vector unsigned char>(  0,  1,  4,  5,  8,  9, 12, 13,
	                        16, 17, 20, 21, 24, 25, 28, 29 );
															 							
  zero   = vec_splat_u8(0);
  szero  = vec_splat_s16(0);
  one    = vec_splat_s16(1);
  vShift = vec_splat_u32(8);
  a255   = vec_splat( vConst, 5 ); // alpha channel = 255
  vU_G   = vec_splat( vConst, 6 ); // -100
  vV_G   = vec_splat( vConst, 7 ); // -210
  vU_B   = vec_splat( vConst, 1 ); // 519
  vU_R   = vec_splat( vConst, 2 ); // 409
  y0     = vec_splat( vConst, 0 ); // 298
  v16    = vec_splat( vConst, 3 ); //  16
  v128   = vec_splat( vConst, 4 ); // 128

  for ( unsigned int i = 0; i < (pixelnum/sizeof(vector unsigned char)); i++ ) {

    // Load UYUV input vector
	const vector unsigned char *vec1 = UYVY_ptr++;

	//expand the UInt8's to short's
	hiImage = static_cast<vector signed short>(vec_mergeh( zero, *vec1 ));
	loImage = static_cast<vector signed short>(vec_mergel( zero, *vec1 ));

	tempUV = static_cast<vector signed short>(vec_perm( hiImage, loImage, uvPerm ));
	tempY  = static_cast<vector signed short>(vec_perm( hiImage, loImage, vPermY ));
	
	// subtract UV_OFFSET from UV's  (should this be saturated?)
	tempUV = static_cast<vector signed short>(vec_sub( tempUV, v128 ));
	// subtract Y-OFFSET from Y's    (should this be saturated?)
	tempY  = static_cast<vector signed short>(vec_sub( tempY, v16 ));
	
	// expand to UUUU UUUU and VVVV VVVV
	tempU = vec_perm(tempUV, tempUV, vPermU);
	tempV = vec_perm(tempUV, tempUV, vPermV);
	//below:
	// 
	//error: cannot convert `vector int' to `vector unsigned int' in assignment
	tempUhi = vec_mule( tempU, one );
	// unsigned int = vec_mule( signed short, signed short )
	// should be
	// signed int = vec_mule( signed short, signed short )
	tempUlo = vec_mulo( tempU, one );
	tempVhi = vec_mule( tempV, one );
	tempVlo = vec_mulo( tempV, one );
	
	// uv_r = YUV2RGB_12*u + YUV2RGB_13*v
	// uv_r = (-1)*u + 409*v (or "409*V - U")
	uv_rEven = vec_mule( tempV, vU_R );
	uv_rOdd  = vec_mulo( tempV, vU_R );
	uv_rHi = vec_sub( uv_rEven, tempUhi );
	uv_rLo = vec_sub( uv_rOdd, tempUlo );
	
	// uv_g = YUV2RGB_22*u + YUV2RGB_23*v
	// uv_g = -100*u + (-210)*v
	// multiply U by -100
	uv_gUEven = vec_mule( tempU, vU_G );
	uv_gUOdd  = vec_mulo( tempU, vU_G );
	// multiply V by -210
	uv_gVEven = vec_mule( tempV, vV_G );
	uv_gVOdd  = vec_mulo( tempV, vV_G );
	// add U & V products
	uv_gHi   = vec_add( uv_gUEven, uv_gVEven );
	uv_gLo   = vec_add( uv_gUOdd, uv_gVOdd );
	
	// uv_b = YUV2RGB_32*u + YUV2RGB_33*v
	// uv_b = 519*u + 0*v
	uv_bEven = vec_mule( tempU, vU_B );
	uv_bOdd  = vec_mulo( tempU, vU_B );
	
	// y = YUV2RGB_11 * tempY
	// y = 298* (tempY - 16)
	yEven = vec_mule( tempY, y0 );
	yOdd  = vec_mulo( tempY, y0 );

	// add while int's
	t0Even = vec_add( yEven, uv_bEven );
	t0Odd  = vec_add( yOdd, uv_bOdd );
	t1Even = vec_add( yEven, uv_gHi );
	t1Odd  = vec_add( yOdd, uv_gLo );
	t2Even = vec_add( yEven, uv_rHi );
	t2Odd  = vec_add( yOdd, uv_rLo );
	
	// shift while int's
	t0Even = vec_sra( t0Even, vShift );
	t0Odd  = vec_sra( t0Odd,  vShift );
	t1Even = vec_sra( t1Even, vShift );
	t1Odd  = vec_sra( t1Odd,  vShift );
	t2Even = vec_sra( t2Even, vShift );
	t2Odd  = vec_sra( t2Odd,  vShift );
	
	// pack down to shorts
	t0 = vec_packs( t0Even, t0Odd );
	t1 = vec_packs( t1Even, t1Odd );
	t2 = vec_packs( t2Even, t2Odd );

	// Permute to GBGBGBGB GBGBGBGB + re-interleave even & odd
	tempGB1 = vec_perm( t1,   t0, vPerm1 );
	tempGB2 = vec_perm( t1,   t0, vPerm2 );
	// Permute to ARARARAR ARARARAR + re-interleave even & odd
	tempRA1 = vec_perm( a255, t2, vPerm1 );
	tempRA2 = vec_perm( a255, t2, vPerm2 );
	
	// Permute to ARGB's
	out1 = vec_perm( tempRA1, tempGB1, vOutPerm1 );
	out2 = vec_perm( tempRA1, tempGB1, vOutPerm2 );
	out3 = vec_perm( tempRA2, tempGB2, vOutPerm1 );
	out4 = vec_perm( tempRA2, tempGB2, vOutPerm2 );
	
	// pack down to char's
	*BGRA_ptr = vec_packsu( out1, out2 );
	BGRA_ptr++;
	*BGRA_ptr = vec_packsu( out3, out4 );
	BGRA_ptr++;
  }
}
#endif /* NO_VECTORINT_TO_VECTORUNSIGNEDINT */

#endif /*  __VEC__ */
