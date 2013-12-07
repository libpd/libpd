/*
 *   Cellular Automata Extension Module for pdp - Main system code
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
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

#include "pdp_ca.h"
#include "pdp_internals.h"

/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif


/* check if packet is a valid ca packet */
int pdp_packet_ca_isvalid(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    if (!header) return 0;
    if (PDP_CA != header->type) return 0;
    if (PDP_CA_STANDARD != pdp_type_ca_info(header)->encoding) return 0;

    return 1;
}


static t_pdp_symbol *pdp_packet_ca_get_description(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    char description[1024];
    char *c = description;
    int encoding;

    if (!header) return pdp_gensym("invalid");
    if (header->type == PDP_CA){
	c += sprintf(c, "ca");
	switch(pdp_type_ca_info(header)->encoding){
	case PDP_CA_STANDARD: c += sprintf(c, "/1bit2D"); break;
	default:
	    c += sprintf(c, "/unknown"); goto exit;
	}
	c += sprintf(c, "/%dx%d", 
		     pdp_type_ca_info(header)->width, 
		     pdp_type_ca_info(header)->height);

    exit:
	return pdp_gensym(description);
    } 
    else return pdp_gensym("unknown");
}

/* create a new ca packet */
int pdp_packet_new_ca(int encoding, int width, int height)
{
    int p;
    int w = (int)width;
    int h = (int)height;
    int bytesize;
    t_pdp *header;

    /* ensure with = multiple of 64 */
    w &= 0xffffffc0;

    /* ensure height = multiple of 4 */
    w &= 0xfffffffc;

    w = (w<64) ? 64 : w;
    h = (h<4) ? 4 : h;

    bytesize = (w>>3) * h;


    /* create new packets */
    p = pdp_packet_new(PDP_CA, bytesize);
    header = pdp_packet_header(p);
    if (!header) {
	pdp_post("error: can't create CA packet");
	return -1;
    }

    pdp_type_ca_info(header)->encoding = PDP_CA_STANDARD;
    pdp_type_ca_info(header)->width = w;
    pdp_type_ca_info(header)->height = h;
    pdp_type_ca_info(header)->offset = 0;
    pdp_type_ca_info(header)->currow = 0; /* only used for 1D ca */
    pdp_type_ca_info(header)->currow = 0;
    header->desc = 0;
    header->desc = pdp_packet_ca_get_description(p);
    //post("creating %s", header->desc->s_name);
    return p;
   
}


/* convert a CA packet to greyscale */

inline void _pdp_type_ca2grey_convert_word(unsigned short int source, short int  *dest)
{

    int i;
    for (i = 15; i>=0; i--){
	dest[i] =  ((unsigned short)(((short int)(source & 0x8000)) >> 14)) >> 1;  
	source <<= 1;
    }
}

int pdp_type_ca2grey(int packet)
{
    int w, h, s, x, y, srcindex;
    long long offset, xoffset, yoffset;
    short int *dest;
    unsigned short int *source;
    t_pdp *header;
    t_pdp *newheader;
    int newpacket;
    if (!(pdp_packet_ca_isvalid(packet))) return -1;

    header = pdp_packet_header(packet);
    w = pdp_type_ca_info(header)->width;
    h = pdp_type_ca_info(header)->height;
    s = w*h;
    source = (unsigned short int *)pdp_packet_data(packet);
    offset = pdp_type_ca_info(header)->offset;
    yoffset = (offset / w) * w;
    xoffset = offset % w;

    //post("pdp_type_ca2grey: offset: %d, xoffset: %d, yoffset: %d", offset, xoffset, yoffset);

    newpacket = pdp_packet_new_image_grey(w, h);
    newheader = pdp_packet_header(newpacket);

    if (!newheader) return -1;

    //newheader->info.image.width = w;
    //newheader->info.image.height = h;
    //newheader->info.image.encoding = PDP_IMAGE_GREY;
    dest = (short int *)pdp_packet_data(newpacket);


#define check_srcindex \
if (srcindex >= (s >> 4)) post ("pdp_type_ca2grey: srcindex out of bound");

#define check_dstindex \
if ((x+y) >= s) post ("pdp_type_ca2grey: dstindex out of bound");


    /* debug : dont' shift offset 
    if (0){
       for(y=0; y< (h*w); y+=w){
          for(x=0; x<w; x+=16){
	    _pdp_type_ca2grey_convert_word (source[(x+y)>>4],  &dest[x+y]);
	  }
       }
       return newpacket;
    }
    */

    /* create top left */
    for (y=0; y < (h*w) - yoffset; y+=w) { 
	for (x=0; x< (w - xoffset); x+=16) {
	    srcindex = (x+xoffset + y+yoffset) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    
    /* create top right */
    for (y=0; y < (h*w) - yoffset; y+=w) { 
	for (x = (w - xoffset); x < w; x+=16) {
	    srcindex = (x+xoffset-w + y+yoffset) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    
    /* create bottom left */
    for (y=(h*w) - yoffset; y < h*w; y+=w) { 
	for (x=0; x< (w - xoffset); x+=16) {
	    srcindex = (x+xoffset + y+yoffset-(w*h)) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    
    /* create bottom right */
    for (y=(h*w) - yoffset; y < h*w; y+=w) { 
	for (x = (w - xoffset); x < w; x+=16) {
	    srcindex = (x+xoffset-w + y+yoffset-(w*h)) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    

    return newpacket;

}


inline unsigned short int _pdp_type_grey2ca_convert_word(short int  *src, short int threshold)
{
    short int tmp;
    short int dest = 0;
    int i;

    for (i = 15; i >= 0; i--){
	dest <<= 1;
	dest |= (src[i] > threshold);
    }

    return dest;
}



int pdp_type_grey2ca(int packet, short int threshold)
{
    int w, h, s, x, y, srcindex;
    long long offset, xoffset, yoffset;
    short int *dest;
    short int *source;
    t_pdp *header;
    t_pdp *newheader;
    int newpacket;
    if (!(pdp_packet_image_isvalid(packet))) return -1;

    header = pdp_packet_header(packet);
    w = header->info.image.width;
    h = header->info.image.height;
    s = w*h;
    source = (unsigned short int *)pdp_packet_data(packet);

    if ( (PDP_IMAGE_GREY != header->info.image.encoding)
	 && (PDP_IMAGE_YV12 != header->info.image.encoding)) return -1;

    newpacket = pdp_packet_new_ca(PDP_CA_STANDARD, w, h);
    newheader = pdp_packet_header(newpacket);

    if (!newheader) return -1;

    dest = (short int *)pdp_packet_data(newpacket);

    for(y=0; y< (h*w); y+=w){
	for(x=0; x<w; x+=16){
	    dest[(x+y)>>4] = _pdp_type_grey2ca_convert_word (&source[x+y], threshold);
	}
    }
    return newpacket;


}

/* returns a pointer to the ca subheader given the pdp header */
t_ca *pdp_type_ca_info(t_pdp *x){return (t_ca *)(&x->info.raw);}


void pdp_ca_setup(void);
void pdp_ca2image_setup(void);
void pdp_image2ca_setup(void);


static int _ca_to_image(int packet, t_pdp_symbol *template)
{
    return pdp_type_ca2grey(packet);
}

static int _image_to_ca(int packet, t_pdp_symbol *template)
{
    // convert with default threshold == 0.5
    return pdp_type_grey2ca(packet, 0.5f);
}

void pdp_scaf_setup(void)
{

    t_pdp_conversion_program *program;

    /* babble */
    post ("PDP: cellular automata extension library");

    /* setup modules */
    pdp_ca_setup();
    pdp_ca2image_setup();
    pdp_image2ca_setup();

    /* setup type conversion */
    program = pdp_conversion_program_new(_ca_to_image, 0);
    pdp_type_register_conversion(pdp_gensym("ca/*/*"), pdp_gensym("image/*/*"), program);
    pdp_type_register_conversion(pdp_gensym("ca/*/*"), pdp_gensym("image/grey/*"), program);

    program = pdp_conversion_program_new(_image_to_ca, 0);
    pdp_type_register_conversion(pdp_gensym("image/grey/*"), pdp_gensym("ca/*/*"), program);


   
    
}




#ifdef __cplusplus
}
#endif
