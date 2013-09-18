/*
 *   Pure Data Packet system implementation. : 16 bit image packet implementation
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

/*
  This file contains methods for the image packets
  pdp_packet_new_* methods are several image packet constructors
  pdp_type_* are image type checkers & converters

*/

#include <string.h>
#include <stdio.h>

#include "pdp_internals.h"
#include "pdp_packet.h"
#include "pdp_imageproc.h"
#include "pdp_resample.h"
#include "pdp_list.h"
#include "pdp_post.h"
#include "pdp_type.h"


/* the class object */
static t_pdp_class* image_class;



/* check dimensions */
static void _checkdim(u32 width, u32 height){
    if 	((pdp_imageproc_legalwidth(width) != width) ||
	 (pdp_imageproc_legalheight(height) != height)){
	pdp_post("WARNING: request to create image packet with illegal dimensions %d x %d", width, height);
    }
}	    


/* image packet constructors */
int pdp_packet_new_image_YCrCb(u32 w, u32 h)
{
    t_pdp *header;
    t_image *image;
    int packet;


    u32 size = w*h;
    u32 totalnbpixels = size + (size >> 1);
    u32 packet_size = totalnbpixels << 1;

    _checkdim(w,h);

    packet = pdp_packet_new(PDP_IMAGE, packet_size);
    header = pdp_packet_header(packet);
    image = pdp_packet_image_info(packet);
    if (!header) return -1;

    image->encoding = PDP_IMAGE_YV12;
    image->width = w;
    image->height = h;
    header->desc = pdp_packet_image_get_description(packet);
    header->theclass = image_class;

    return packet;
}

int pdp_packet_new_image_grey(u32 w, u32 h)
{
    t_pdp *header;
    t_image *image;
    int packet;


    u32 size = w*h;
    u32 totalnbpixels = size;
    u32 packet_size = totalnbpixels << 1;
    //pdp_post("grey %d x %d = %d bytes", w,h,packet_size);

    _checkdim(w,h);

    packet = pdp_packet_new(PDP_IMAGE, packet_size);
    header = pdp_packet_header(packet);
    image = pdp_packet_image_info(packet);
    if (!header) return -1;

    image->encoding = PDP_IMAGE_GREY;
    image->width = w;
    image->height = h;
    header->desc = pdp_packet_image_get_description(packet);
    header->theclass = image_class;

    return packet;
}

int pdp_packet_new_image_mchp(u32 w, u32 h, u32 d)
{
    t_pdp *header;
    t_image *image;
    int packet;


    u32 size = w*h*d;
    u32 totalnbpixels = size;
    u32 packet_size = totalnbpixels << 1;

    _checkdim(w,h);

    packet = pdp_packet_new(PDP_IMAGE, packet_size);
    header = pdp_packet_header(packet);
    image = pdp_packet_image_info(packet);
    if (!header) return -1;


    image->encoding = PDP_IMAGE_MCHP;
    image->width = w;
    image->height = h;
    image->depth = d;
    header->desc = pdp_packet_image_get_description(packet);
    header->theclass = image_class;

    return packet;
}


int pdp_packet_new_image(u32 type, u32 w, u32 h)
{
    switch (type){
    case PDP_IMAGE_YV12:
	return pdp_packet_new_image_YCrCb(w,h);
    case PDP_IMAGE_GREY:
	return pdp_packet_new_image_grey(w,h);
    default:
	return -1;
    }
}


/****************** packet type checking and conversion methods ********************/



/* check if two image packets are allocated and of the same type */
int pdp_packet_image_compat(int packet0, int packet1)
{
    t_pdp *header0 = pdp_packet_header(packet0);
    t_pdp *header1 = pdp_packet_header(packet1);
    t_image *image0 = pdp_packet_image_info(packet0);
    t_image *image1 = pdp_packet_image_info(packet1);


    if (!(pdp_packet_compat(packet0, packet1))) return 0;
    if (header0->type != PDP_IMAGE){
	//pdp_post("pdp_type_compat_image: not a PDP_IMAGE");
	return 0;
    }
    if (image0->encoding != image1->encoding){
	//pdp_post("pdp_type_compat_image: encodings differ");
	return 0;
    }
    if (image0->width != image1->width){
	//pdp_post("pdp_type_compat_image: image withs differ");
	return 0;
    }
    if (image0->height != image1->height){
	//pdp_post("pdp_type_compat_image: image heights differ");
	return 0;
    }
    return 1;
}

/* check if packet is a valid image packet */
int pdp_packet_image_isvalid(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    if (!header) return 0;
    if (PDP_IMAGE != header->type) return 0;
    if ((PDP_IMAGE_YV12 != image->encoding)
	&& (PDP_IMAGE_GREY != image->encoding)
	&& (PDP_IMAGE_MCHP != image->encoding)) return 0;

    return 1;

}

/* set the channel mask for the image */
void pdp_packet_image_set_chanmask(int packet, unsigned int chanmask)
{
    if (pdp_packet_image_isvalid(packet)) pdp_packet_image_info(packet)->chanmask = chanmask;
    
}


t_image *pdp_packet_image_info(int packet)
{
    return (t_image *)pdp_packet_subheader(packet);
}


t_pdp_symbol *pdp_packet_image_get_description(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    char description[1024];
    char *c = description;
    int encoding;

    if (!header) return pdp_gensym("invalid");
    else if (!header->desc){
	/* if description is not defined, try to reconstruct it (for backwards compat) */
	if (header->type == PDP_IMAGE){
	    c += sprintf(c, "image");
	    encoding = image->encoding;
	    switch(encoding){
	    case PDP_IMAGE_YV12: c += sprintf(c, "/YCrCb"); break;
	    case PDP_IMAGE_GREY: c += sprintf(c, "/grey"); break;
	    case PDP_IMAGE_MCHP: c += sprintf(c, "/multi"); break;
	    default:
		c += sprintf(c, "/unknown"); goto exit;
	    }
	    if (encoding == PDP_IMAGE_MCHP){
		c += sprintf(c, "/%dx%dx%d", 
			     image->width, 
			     image->height,
			     image->depth);
	    }
	    else {
		c += sprintf(c, "/%dx%d", 
			     image->width, 
			     image->height);
	    }

	exit:
	    return pdp_gensym(description);
	} 
	else return pdp_gensym("unknown");
    }
    else return header->desc;
}





/* IMAGE PACKAGE CONVERSION ROUTINES */

/* note: these are internal: no extra checking is done
   it is assumed the packets are of correct type (the type template associated with the conversion program) */

// image/YCrCb/* -> image/grey/*
// image/multi/* -> image/grey/*  (only first channel)
static int _pdp_packet_image_convert_YCrCb_to_grey(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    int w = image->width;
    int h = image->height;
    int p = pdp_packet_new_image_grey(w,h);
    if (p == -1) return p;
    memcpy(pdp_packet_data(p), pdp_packet_data(packet), 2*w*h);
    return p;
}

// image/grey/* -> image/YCrCb/*
static int _pdp_packet_image_convert_grey_to_YCrCb(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    int w = image->width;
    int h = image->height;
    int p = pdp_packet_new_image_YCrCb(w,h);
    int y_bytes = 2*w*h;
    void *data;
    if (p == -1) return p;
    data = pdp_packet_data(p);
    memcpy(data, pdp_packet_data(packet), y_bytes);
    memset(data+y_bytes, 0, y_bytes >> 1);
    return p;
}

// image/grey/* -> image/multi/*
static int _pdp_packet_image_convert_grey_to_multi(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    int w = image->width;
    int h = image->height;
    int p = pdp_packet_new_image_mchp(w,h,1);
    int y_bytes = 2*w*h;
    void *data;
    if (p == -1) return p;
    data = pdp_packet_data(p);
    memcpy(data, pdp_packet_data(packet), y_bytes);
    return p;
}

// image/multi/* -> image/YCrCb/*  (only first 3 channels)
static int _pdp_packet_image_convert_multi_to_YCrCb(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    s16 *data, *newdata;

    /* get info */
    int w = image->width;
    int h = image->height;
    int d = image->depth;
    int plane_size = w*h;

    /* create new packet */
    int newpacket = pdp_packet_new_image_YCrCb(w, h);
    if (-1 == newpacket) return -1;

    data = pdp_packet_data(packet);
    newdata = pdp_packet_data(newpacket);

    /* copy channel 0 */
    memcpy(newdata, data, plane_size<<1);
    newdata += plane_size;
    data += plane_size;

    /* copy channel 1 */
    if (d >= 1) pdp_resample_halve(data, newdata, w, h);
    else        memset(newdata, 0, plane_size >> 1);
    data += plane_size;
    newdata += (plane_size >> 2);


    /* copy channel 2 */
    if (d >= 2) pdp_resample_halve(data, newdata, w, h);
    else        memset(newdata, 0, plane_size >> 1);

    return newpacket;
    
}

// image/YCrCb/* -> image/multi/*
static int _pdp_packet_image_convert_YCrCb_to_multi(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    s16 *data, *newdata;

    /* get info */
    int w = image->width;
    int h = image->height;
    int plane_size = w*h;

    /* create new packet */
    int newpacket = pdp_packet_new_image_mchp(w, h, 3);
    if (-1 == newpacket) return -1;

    data = pdp_packet_data(packet);
    newdata = pdp_packet_data(newpacket);

    /* copy channel 0 */
    memcpy(newdata, data, plane_size<<1);
    newdata += plane_size;
    data += plane_size;
    w >>= 1;
    h >>= 1;

    /* copy channel 1 */
    pdp_resample_double(data, newdata, w, h);
    data += (plane_size >> 2);
    newdata += plane_size;

    /* copy channel 2 */
    pdp_resample_double(data, newdata, w, h);

    return newpacket;
    
}

static void _pdp_description_get_dims(t_pdp_symbol *template, int *w, int *h, int *d)
{
    char *c = template->s_name;
    // get requested dimensions
    *w = 0;
    *h = 0;
    *d = 0;
    while (*c++ != '/');
    while (*c++ != '/');
    sscanf(c, "%dx%dx%d", w, h, d);

}

// resample image/YCrCb/*
static int _pdp_packet_image_convert_resample_YCrCb(int packet, t_pdp_symbol *dest_template)
{
    int quality = 1;
    int dst_w, dst_h, dummy;
    int new_packet;
    unsigned int src_size, src_voffset, src_uoffset;
    unsigned int dst_size, dst_voffset, dst_uoffset;
    t_pdp *header0 = pdp_packet_header(packet);
    t_image *image0 = pdp_packet_image_info(packet);
    unsigned int src_w = image0->width;
    unsigned int src_h = image0->height;
    short int *src_image = (short int *)pdp_packet_data(packet);
    short int *dst_image;
    _pdp_description_get_dims(dest_template, &dst_w, &dst_h, &dummy);
    new_packet = pdp_packet_new_image_YCrCb(dst_w, dst_h);
    if (-1 == new_packet) return -1;
    dst_image = (short int*)pdp_packet_data(new_packet);
    src_size = src_w*src_h;
    src_voffset = src_size;
    src_uoffset = src_size + (src_size>>2);
    dst_size = dst_w*dst_h;
    dst_voffset = dst_size;
    dst_uoffset = dst_size + (dst_size>>2);
    if (quality){
	pdp_resample_scale_bilin(src_image, dst_image, src_w, src_h, dst_w, dst_h);
	pdp_resample_scale_bilin(src_image+src_voffset, dst_image+dst_voffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
	pdp_resample_scale_bilin(src_image+src_uoffset, dst_image+dst_uoffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
    }
    else{
	pdp_resample_scale_nn(src_image, dst_image, src_w, src_h, dst_w, dst_h);
	pdp_resample_scale_nn(src_image+src_voffset, dst_image+dst_voffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
	pdp_resample_scale_nn(src_image+src_uoffset, dst_image+dst_uoffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
    }
    return new_packet;
}

// resample image/grey/* and image/multi/*
static int _pdp_packet_image_convert_resample_multi(int packet, t_pdp_symbol *dest_template)
{
    int quality = 1;
    int dst_w, dst_h, depth;
    int new_packet;
    unsigned int src_size;
    unsigned int dst_size;
    t_pdp *header0 = pdp_packet_header(packet);
    t_image *image0 = pdp_packet_image_info(packet);
    unsigned int src_w = image0->width;
    unsigned int src_h = image0->height;
    short int *src_image = (short int *)pdp_packet_data(packet);
    short int *dst_image;
    _pdp_description_get_dims(dest_template, &dst_w, &dst_h, &depth);

    if (depth == 0){
	depth = 1;
	new_packet = pdp_packet_new_image_grey(dst_w, dst_h);
    }
    else {
	new_packet = pdp_packet_new_image_mchp(dst_w, dst_h, depth);
    }
    if (-1 == new_packet) return -1;

    dst_image = (short int*)pdp_packet_data(new_packet);

    src_size = src_w*src_h;
    dst_size = dst_w*dst_h;
    while (depth--){
	if (quality){
	    pdp_resample_scale_bilin(src_image, dst_image, src_w, src_h, dst_w, dst_h);
	}
	else{
	    pdp_resample_scale_nn(src_image, dst_image, src_w, src_h, dst_w, dst_h);
	}
	src_image += src_size;
	dst_image += dst_size;
    }

    return new_packet;
}

static int _pdp_packet_image_convert_fallback(int packet, t_pdp_symbol *dest_template)
{
    pdp_post("can't convert image type %s to %s",
	 pdp_packet_get_description(packet)->s_name, dest_template->s_name);

    return -1;
}



/* the expensive factory method */
static int pdp_image_factory(t_pdp_symbol *type)
{
    t_pdp_list *l;
    t_pdp_symbol *s;
    int t;
    int w = 0;
    int h = 0;
    int d = 0;
    int p = -1;
    int n = 0;
    int m = 0;
    char *garbage = 0;

    //pdp_post("creating:");
    //pdp_post("%s", type->s_name);

    l = pdp_type_to_list(type);
    s = pdp_list_pop(l).w_symbol; // first element is "image"
    s = pdp_list_pop(l).w_symbol;

    /* get image type */
    if (s == pdp_gensym("grey")) t = PDP_IMAGE_GREY;
    else if (s == pdp_gensym("YCrCb")) t = PDP_IMAGE_YV12;
    else if (s == pdp_gensym("multi")) t = PDP_IMAGE_MCHP;
    else goto exit;
    
    /* get image dimensions and create image */
    s = pdp_list_pop(l).w_symbol;
    switch (t){
    case PDP_IMAGE_MCHP:
	m  = sscanf(s->s_name, "%dx%dx%d", &w, &h, &d);
	p = pdp_packet_new_image_mchp(w,h,d);
	break;
    default:
	sscanf(s->s_name, "%dx%d", &w, &h);
	p = pdp_packet_new_image(t,w,h);
	break;
    }
    if (p != -1){
	t_pdp *header = pdp_packet_header(p);
	/* if type is not exact, delete the packet */
	if (type != header->desc) {
	    pdp_packet_delete(p);
	    p = -1;
	}
    }
 exit:
    pdp_list_free(l);
    return p;
}



void pdp_image_words_setup(t_pdp_class *c);

void pdp_image_setup(void)
{
    t_pdp_conversion_program *program;

    /* setup the class object */
    image_class = pdp_class_new(pdp_gensym("image/*/*"), pdp_image_factory);
    

    /* setup conversion programs */
    program = pdp_conversion_program_new(_pdp_packet_image_convert_YCrCb_to_grey, 0);
    pdp_type_register_conversion(pdp_gensym("image/YCrCb/*"), pdp_gensym("image/grey/*"), program);
    pdp_type_register_conversion(pdp_gensym("image/multi/*"), pdp_gensym("image/grey/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_image_convert_grey_to_YCrCb, 0);
    pdp_type_register_conversion(pdp_gensym("image/grey/*"), pdp_gensym("image/YCrCb/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_image_convert_grey_to_multi, 0);
    pdp_type_register_conversion(pdp_gensym("image/grey/*"), pdp_gensym("image/multi/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_image_convert_multi_to_YCrCb, 0);
    pdp_type_register_conversion(pdp_gensym("image/multi/*"), pdp_gensym("image/YCrCb/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_image_convert_YCrCb_to_multi, 0);
    pdp_type_register_conversion(pdp_gensym("image/YCrCb/*"), pdp_gensym("image/multi/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_image_convert_resample_YCrCb, 0);
    pdp_type_register_conversion(pdp_gensym("image/YCrCb/*"), pdp_gensym("image/YCrCb/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_image_convert_resample_multi, 0);
    pdp_type_register_conversion(pdp_gensym("image/multi/*"), pdp_gensym("image/multi/*"), program);
    pdp_type_register_conversion(pdp_gensym("image/grey/*"), pdp_gensym("image/grey/*"), program);

    /* catch-all fallback */
    program = pdp_conversion_program_new(_pdp_packet_image_convert_fallback, 0);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("image/*/*"), program);
							     
}
