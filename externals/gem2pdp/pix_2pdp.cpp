/*
 *  pix_2pdp : pix to pdp bridge
 *
 *  Capture the contents of the Gem pix and transform it to a PDP Packet whenever a bang is received
 *
 *  Based on code of gem2pdp by Yves Degoyon
 *  Many thanks to IOhannes M Zmölnig
 *
 *  Copyright (c) 2005 Georg Holzmann <grh@mur.at>
 *  parts Copyright (c) 2005-2006 James Tittle II <tigital@mac.com>
 *
 */

#include "pix_2pdp.h"
#include "yuv.h"

//#define __TIMING__

CPPEXTERN_NEW(pix_2pdp)

pix_2pdp::pix_2pdp(void)
{
  gem_image = NULL;
  m_pdpoutlet = outlet_new(this->x_obj, &s_anything);
}

pix_2pdp::~pix_2pdp()
{
  gem_image = NULL;
  gem_xsize = 0;
  gem_ysize = 0;
  gem_csize = 0;
  gem_upsidedown = 0;
}

// Image processing
void pix_2pdp::processImage(imageStruct &image)
{
  gem_image = image.data;
  gem_xsize = image.xsize;
  gem_ysize = image.ysize;
  gem_csize = image.csize;
  gem_format = image.format;
  gem_upsidedown = image.upsidedown;
}

// pdp processing
void pix_2pdp::bangMess()
{
  t_int psize, px, py;
  short int *pY, *pY2, *pU, *pV;
  unsigned char g1,g2,g3,g4;
  t_int helper;

  if(gem_image)
  {
    // make pdp packet
    psize = gem_xsize * gem_ysize;
    m_packet0 = pdp_packet_new_image_YCrCb( gem_xsize, gem_ysize);
    if(!m_packet0) {
      error("couldn't allocate new pdp packet...");
      return;
    }
    m_header = pdp_packet_header(m_packet0);
    m_data = (short int *)pdp_packet_data(m_packet0);

    pY = m_data;
    pY2 = m_data + gem_xsize;
    pV = m_data+psize;
    pU = m_data+psize+(psize>>2);
    
    switch(gem_format)
    {
      // RGB
      case GL_RGB:
      case GL_RGBA:
	  case GL_BGR:
	  case GL_BGRA:
       for ( py=0; py<gem_ysize; py++)
        {
          const t_int py2=(gem_upsidedown)?py:(gem_ysize-py);
          for ( px=0; px<gem_xsize; px++)
          {
            // the way to access the pixels: (C=chRed, chBlue, ...)
            // image[Y * xsize * csize + X * csize + C]
            helper = py2*gem_xsize*gem_csize + px*gem_csize;
            g1=gem_image[helper+chRed];   // R
            g2=gem_image[helper+chGreen]; // G
            g3=gem_image[helper+chBlue];  // B
            
            *(pY) = yuv_RGBtoY( (g1<<16) +  (g2<<8) +  g3 ) << 7;
            *(pV) = ( yuv_RGBtoV( (g1<<16) +  (g2<<8) +  g3 ) - 128 ) << 8;
            *(pU) = ( yuv_RGBtoU( (g1<<16) +  (g2<<8) +  g3 ) - 128 ) << 8;
            pY++;
            if ( (px%2==0) && (py%2==0) ) {
              pV++; pU++;
	    }
          }
        }
        pdp_packet_pass_if_valid(m_pdpoutlet, &m_packet0);
        break;
        
      // YUV
      case GL_YUV422_GEM: {
#ifdef __TIMING__
	    UnsignedWide start, end;
	    Microseconds(&start);
#endif

#ifdef __VEC__
 		YUV422_to_YV12_altivec(pY, pY2, pV, pU, gem_image, gem_xsize, gem_ysize);
#else
	    int row=gem_ysize>>1;
		int cols=gem_xsize>>1;
		short u,v;
		unsigned char *pixel = gem_image;
		unsigned char *pixel2 = gem_image + gem_xsize * gem_csize;
        const int row_length = gem_xsize* gem_csize;
                if(0==gem_upsidedown){
                  pixel=gem_image+row_length * (gem_ysize-1);
                  pixel2=gem_image+row_length * (gem_ysize-2);
                }
		while (row--){
		  int col=cols;
		  while(col--){
				u=(pixel[0]-128)<<8; v=(pixel[2]-128)<<8;
				*pU = u;
				*pY++ = (pixel[1])<<7;
				*pV = v;
				*pY++ = (pixel[3])<<7;
				*pY2++ = (pixel2[1])<<7;
				*pY2++ = (pixel2[3])<<7;
				pixel+=4;
				pixel2+=4;
				pU++; pV++;
		  }
                  if(gem_upsidedown){
                    pixel += row_length;
                    pixel2 += row_length;
                  } else {
                    pixel -= 3*row_length;
                    pixel2 -= 3*row_length;
                  }
                  pY += gem_xsize; pY2 += gem_xsize;
		}
#endif // __VEC__
#ifdef __TIMING__
	  Microseconds(&end);
	  float seconds = (float)(end.lo - start.lo) / 1000000.f;
	  post("UYVYtoYV12 frame time = %f", seconds);
#endif
        pdp_packet_pass_if_valid(m_pdpoutlet, &m_packet0);
        } break;
      
      // grey
      case GL_LUMINANCE:
        for ( py=0; py<gem_ysize; py++)
        {
          const t_int py2=(gem_upsidedown)?py:(gem_ysize-py);
          for ( px=0; px<gem_xsize; px++)
          {
            *pY = gem_image[py2*gem_xsize*gem_csize + px*gem_csize] << 7;
            pY++;
            if ( (px%2==0) && (py%2==0) )
            {
              *pV++=128;
              *pU++=128;
            }
          }
        }
        pdp_packet_pass_if_valid(m_pdpoutlet, &m_packet0);
        break;
        
      default:
        post( "pix_2pdp: Sorry, wrong input type!" );
    }
  }
}

void pix_2pdp::obj_setupCallback(t_class *classPtr)
{
  ::post( "pix_2pdp : a bridge between a Gem pix and PDP/PiDiP, Georg Holzmann 2005 <grh@mur.at> & tigital 2005 <tigital@mac.com>" );
  class_addmethod(classPtr, (t_method)&pix_2pdp::bangMessCallback,
    	    gensym("bang"), A_NULL);
}

void pix_2pdp::bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}
