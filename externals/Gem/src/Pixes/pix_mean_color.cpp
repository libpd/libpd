// Copyright (c) 2004 Tim Blechmann
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "GEM.LICENSE.TERMS"  in this distribution.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// See file GEM.LICENSE.TERMS for further informations on licensing terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "pix_mean_color.h"


CPPEXTERN_NEW_WITH_GIMME(pix_mean_color);

/* const, destructor */
pix_mean_color::pix_mean_color(int argc, t_atom *argv)
{
	m_list = outlet_new(this->x_obj, 0);
}

pix_mean_color::~pix_mean_color()
{
	outlet_free(m_list);
}

void pix_mean_color::processYUVImage(imageStruct &image)
{
	t_atom out[4];
	unsigned int datasize = (image.xsize * image.ysize) >> 1;
	unsigned char *base = image.data;
	
	unsigned long sum[4] = {0,0,0,0};

	for (unsigned int i = 0; i != datasize; ++i)
	{
		for(int j = 0; j!= 4; ++j)
		{
			sum[j]  += base[j];
		}
		base += 4;
	}
        // use formulae from http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC29
        /*
         * [R]   [ 0.00456621  0.00000000  0.00625893 ]   [Y-16 ]
         * [G] = [ 0.00456621 -0.00153632 -0.00318811 ] * [U-128]
         * [B]   [ 0.00456621  0.00791071  0.00000000 ]   [V-128]
         */

        t_float y = (t_float)(sum[chY0]+sum[chY1]) / (t_float)(2*datasize) - 16;
        t_float u = (t_float) sum[chU] / (t_float)(datasize) - 128;
        t_float v = (t_float) sum[chV] / (t_float)(datasize) - 128;

        t_float r = FLOAT_CLAMP((t_float)(0.00456621*y             +0.00625893*v));
        t_float g = FLOAT_CLAMP((t_float)(0.00456621*y-0.00153632*u-0.00318811*v));
        t_float b = FLOAT_CLAMP((t_float)(0.00456621*y+0.00791071*u             ));

	SETFLOAT(out,   r);
	SETFLOAT(out+1, g);
        SETFLOAT(out+2, b);
	SETFLOAT(out+3, 1.0);

	outlet_list(m_list, 0, 4, out);	

}

void pix_mean_color::processGrayImage(imageStruct &image)
{
	t_atom out[4];
	unsigned int datasize = image.xsize * image.ysize;
	unsigned char *base = image.data;
	
	unsigned long sum = 0;

	for (unsigned int i = 0; i != datasize; ++i)
	{
		sum  += *base;
		base++;
	}

        t_float grey = (t_float)sum / (t_float)(datasize*255);
	
	SETFLOAT(out,   grey );
	SETFLOAT(out+1, grey);
	SETFLOAT(out+2, grey);
	SETFLOAT(out+3, 1.0);

	outlet_list(m_list, 0, 4, out);
}

void pix_mean_color::processRGBImage(imageStruct &image)
{
	t_atom out[4];
	unsigned int datasize = image.xsize * image.ysize;
	unsigned char *base = image.data;
	
	unsigned long sum[3] = {0,0,0};

	for (unsigned int i = 0; i != datasize; ++i)
	{
		for(int j = 0; j!= 3; ++j)
		{
			sum[j]  += base[j];
		}
		base += 3;
	}
	
	
	SETFLOAT(out,   (t_float)sum[0] / (t_float)(datasize*255));
        SETFLOAT(out+1, (t_float)sum[1] / (t_float)(datasize*255));
	SETFLOAT(out+2, (t_float)sum[2] / (t_float)(datasize*255));
	SETFLOAT(out+3, 1.0);

	outlet_list(m_list, 0, 4, out);
}

void pix_mean_color::processRGBAImage(imageStruct &image)
{
	unsigned int datasize = image.xsize * image.ysize;
	unsigned char *base = image.data;
	
	unsigned long sum[4] = {0,0,0,0};

	for (unsigned int i = 0; i != datasize; ++i)
	{
		for(int j = 0; j!= 4; ++j)
		{
			sum[j]  += base[j];
		}
		base += 4;
	}
	
	t_atom out[4];
	
	SETFLOAT(out,   (t_float)sum[chRed  ] / (t_float)(datasize*255) );
	SETFLOAT(out+1, (t_float)sum[chGreen] / (t_float)(datasize*255) );
	SETFLOAT(out+2, (t_float)sum[chBlue ] / (t_float)(datasize*255) );
	SETFLOAT(out+3, (t_float)sum[chAlpha] / (t_float)(datasize*255) );

	outlet_list(m_list, 0, 4, out);
}

void pix_mean_color :: obj_setupCallback(t_class *classPtr)
{

}
