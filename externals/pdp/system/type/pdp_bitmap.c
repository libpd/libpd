/*
 *   Pure Data Packet system implementation. : 8 bit image packet implementation
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


#include <stdio.h>
#include <string.h>
#include "pdp_packet.h"
#include "pdp_type.h"
#include "pdp_llconv.h"
#include "pdp_internals.h"
#include "pdp_post.h"


/* the class object */
static t_pdp_class* bitmap_class;


t_bitmap *pdp_packet_bitmap_info(int packet)
{
    return (t_bitmap *)pdp_packet_subheader(packet);
}

/* check if packet is a valid image packet */
int pdp_packet_bitmap_isvalid(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *image = pdp_packet_bitmap_info(packet);
    if (!header) return 0;
    if (PDP_BITMAP != header->type) return 0;

    return 1;

}



/* bitmap constructors */
t_pdp_symbol *pdp_packet_bitmap_get_description(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    char description[1024];
    char *c = description;
    int encoding;

    if (!header) return pdp_gensym("invalid");
    else if (!header->desc){
	/* if description is not defined, try to reconstruct it (for backwards compat) */
	if (header->type == PDP_BITMAP){
	    c += sprintf(c, "bitmap");
	    encoding = bitmap->encoding;
	    switch(encoding){
	    case PDP_BITMAP_RGB: c += sprintf(c, "/rgb"); break;
	    case PDP_BITMAP_RGBA: c += sprintf(c, "/rgba"); break;
	    case PDP_BITMAP_GREY: c += sprintf(c, "/grey"); break;
	    case PDP_BITMAP_YV12: c += sprintf(c, "/yv12"); break;
	    case PDP_BITMAP_I420: c += sprintf(c, "/i420"); break;
	    default:
		c += sprintf(c, "/unknown"); goto exit;
	    }
	    c += sprintf(c, "/%dx%d", 
			 bitmap->width, 
			 bitmap->height);
	exit:
	    return pdp_gensym(description);
	} 
	else return pdp_gensym("unknown");
    }
    else return header->desc;
}

int pdp_packet_new_bitmap_yv12(u32 w, u32 h)
{
    t_pdp *header;
    t_bitmap *bitmap;
    int packet;


    u32 size = w*h;
    u32 totalnbpixels = size;
    u32 packet_size = size + (size >> 1);

    packet = pdp_packet_new(PDP_BITMAP, packet_size);
    header = pdp_packet_header(packet);
    bitmap = pdp_packet_bitmap_info(packet);
    if (!header) return -1;

    bitmap->encoding = PDP_BITMAP_YV12;
    bitmap->width = w;
    bitmap->height = h;
    header->desc = pdp_packet_bitmap_get_description(packet);
    header->theclass = bitmap_class;

    return packet;
}

int pdp_packet_new_bitmap_i420(u32 w, u32 h){
    int p = pdp_packet_new_bitmap_yv12(w,h);
    if (-1 == p) return -1;
    t_pdp *header    = pdp_packet_header(p);
    t_bitmap *bitmap = pdp_packet_subheader(p);
    bitmap->encoding = PDP_BITMAP_I420;
    header->desc = 0; // structured programming.. ha!
    header->desc = pdp_packet_bitmap_get_description(p);
    return p;
}

int pdp_packet_new_bitmap_grey(u32 w, u32 h)
{
    t_pdp *header;
    t_bitmap *bitmap;
    int packet;

    u32 size = w*h;
    u32 totalnbpixels = size;
    u32 packet_size = totalnbpixels;

    packet = pdp_packet_new(PDP_BITMAP, packet_size);
    header = pdp_packet_header(packet);
    bitmap = pdp_packet_bitmap_info(packet);
    if (!header) return -1;

    bitmap->encoding = PDP_BITMAP_GREY;
    bitmap->width = w;
    bitmap->height = h;
    header->desc = pdp_packet_bitmap_get_description(packet);
    header->theclass = bitmap_class;

    return packet;
}

int pdp_packet_new_bitmap_rgb(u32 w, u32 h)
{
    t_pdp *header;
    t_bitmap *bitmap;
    int packet;


    u32 size = w*h;
    u32 totalnbpixels = size;
    u32 packet_size = totalnbpixels * 3;

    packet = pdp_packet_new(PDP_BITMAP, packet_size);
    header = pdp_packet_header(packet);
    bitmap = pdp_packet_bitmap_info(packet);
    if (!header) return -1;

    bitmap->encoding = PDP_BITMAP_RGB;
    bitmap->width = w;
    bitmap->height = h;
    header->desc = pdp_packet_bitmap_get_description(packet);
    header->theclass = bitmap_class;

    return packet;
}

int pdp_packet_new_bitmap_rgba(u32 w, u32 h)
{
    t_pdp *header;
    t_bitmap *bitmap;
    int packet;


    u32 size = w*h;
    u32 totalnbpixels = size;
    u32 packet_size = totalnbpixels * 4;

    packet = pdp_packet_new(PDP_BITMAP, packet_size);
    header = pdp_packet_header(packet);
    bitmap = pdp_packet_bitmap_info(packet);
    if (!header) return -1;

    bitmap->encoding = PDP_BITMAP_RGBA;
    bitmap->width = w;
    bitmap->height = h;
    header->desc = pdp_packet_bitmap_get_description(packet);
    header->theclass = bitmap_class;

    return packet;
}

int pdp_packet_new_bitmap(int type, u32 w, u32 h)
{
    switch(type){
    case PDP_BITMAP_GREY: return pdp_packet_new_bitmap_grey(w,h);
    case PDP_BITMAP_YV12: return pdp_packet_new_bitmap_yv12(w,h);
    case PDP_BITMAP_I420: return pdp_packet_new_bitmap_i420(w,h);
    case PDP_BITMAP_RGB:  return pdp_packet_new_bitmap_rgb(w,h);
    case PDP_BITMAP_RGBA: return pdp_packet_new_bitmap_rgba(w,h);
    default: return -1;
    }
}


/* some utility methods */
void pdp_llconv_flip_top_bottom(char *data, int width, int height, int pixelsize);

/* flip top & bottom */
void pdp_packet_bitmap_flip_top_bottom(int packet)
{
    t_bitmap *b = (t_bitmap *)pdp_packet_subheader(packet);
    char *d = (char *)pdp_packet_data(packet);
    int w,h;
    if (!pdp_packet_bitmap_isvalid(packet)) return;
    if (!b) return;
    w = b->width;
    h = b->height;

    switch(b->encoding){
    case PDP_BITMAP_GREY: pdp_llconv_flip_top_bottom(d,w,h,1); break;
    case PDP_BITMAP_RGB:  pdp_llconv_flip_top_bottom(d,w,h,3); break;
    case PDP_BITMAP_RGBA: pdp_llconv_flip_top_bottom(d,w,h,4); break;
    default: break;
    }

}



/* conversion methods */
// image/grey/* -> bitmap/grey/*
static int _pdp_packet_bitmap_convert_grey_to_grey8(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    s16 *data = (s16 *)pdp_packet_data(packet);
    u8 *new_data;
    u32 w,h;
    int new_p;

    if (!pdp_packet_image_isvalid(packet)) return -1;
    w = image->width;
    h = image->height;

    if (!((image->encoding == PDP_IMAGE_GREY) ||
	(image->encoding == PDP_IMAGE_YV12))) return -1;

    new_p = pdp_packet_new_bitmap_grey(w,h);
    if (-1 == new_p) return -1;
    new_data = (u8 *)pdp_packet_data(new_p);

    /* convert image to greyscale 8 bit */
    pdp_llconv(data,RIF_GREY______S16, new_data, RIF_GREY______U8, w, h);

    return new_p;
}

static int _pdp_packet_bitmap_convert_YCrCb_to_rgb8(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    s16 *data = (s16 *)pdp_packet_data(packet);
    u8 *new_data;
    u32 w,h;
    int new_p;

    if (!pdp_packet_image_isvalid(packet)) return -1;
    w = image->width;
    h = image->height;

    if (!((image->encoding == PDP_IMAGE_YV12))) return -1;

    new_p = pdp_packet_new_bitmap_rgb(w,h);
    if (-1 == new_p) return -1;
    new_data = (u8 *)pdp_packet_data(new_p);

    /* convert image to greyscale 8 bit */
    pdp_llconv(data,RIF_YVU__P411_S16, new_data, RIF_RGB__P____U8, w, h);

    return new_p;
}

static int _pdp_packet_bitmap_convert_image_to_yv12(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = pdp_packet_image_info(packet);
    s16 *data = (s16 *)pdp_packet_data(packet);
    u8 *new_data;
    u32 w,h, nbpixels;
    int new_p;
    int encoding = image->encoding;

    if (!pdp_packet_image_isvalid(packet)) return -1;
    w = image->width;
    h = image->height;
    nbpixels = w*h;

    new_p = pdp_packet_new_bitmap_yv12(w,h);
    if (-1 == new_p) return -1;
    new_data = (u8 *)pdp_packet_data(new_p);

    switch (encoding){
    case PDP_IMAGE_YV12:
	pdp_llconv(data, RIF_YVU__P411_S16, new_data, RIF_YVU__P411_U8, w, h);
	break;
    case PDP_IMAGE_GREY:
	pdp_llconv(data, RIF_GREY______S16, new_data, RIF_GREY______U8, w, h);
	memset(new_data + nbpixels, 0x80, nbpixels>>1);
	break;
    default:
	/* not supported, $$$TODO add more */
	pdp_packet_mark_unused(new_p);
	new_p = -1;
	break;
    }

    return new_p;

}

static int _pdp_packet_bitmap_convert_rgb8_to_YCrCb(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    u8 *data = (u8 *)pdp_packet_data(packet);
    s16 *new_data;
    u32 w,h;
    int new_p;

    w = bitmap->width;
    h = bitmap->height;
    new_p = pdp_packet_new_image_YCrCb(w,h);
    if (-1 == new_p) return -1;
    new_data = (s16 *)pdp_packet_data(new_p);

    /* convert image to greyscale 8 bit */
    pdp_llconv(data, RIF_RGB__P____U8, new_data, RIF_YVU__P411_S16, w, h);

    return new_p;
}

static int _pdp_packet_bitmap_convert_rgb8_to_bitmap_YCrCb(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    u8 *data = (u8 *)pdp_packet_data(packet);
    u8 *new_data;
    u32 w,h;
    int new_p;

    w = bitmap->width;
    h = bitmap->height;
    new_p = pdp_packet_new_bitmap_yv12(w,h);
    if (-1 == new_p) return -1;
    new_data = (u8 *)pdp_packet_data(new_p);

    /* convert image to greyscale 8 bit */
    pdp_llconv(data, RIF_RGB__P____U8, new_data, RIF_YVU__P411_U8, w, h);

    return new_p;
}

static int _pdp_packet_bitmap_convert_grey8_to_grey(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    s16 *data = (s16 *)pdp_packet_data(packet);
    u8 *new_data;
    u32 w,h;
    int new_p;

    w = bitmap->width;
    h = bitmap->height;
    new_p = pdp_packet_new_image_grey(w,h);
    if (-1 == new_p) return -1;
    new_data = (u8 *)pdp_packet_data(new_p);

    /* convert image to greyscale 8 bit */
    pdp_llconv(data, RIF_GREY______U8, new_data, RIF_GREY______S16, w, h);

    return new_p;
}
static int _pdp_packet_bitmap_convert_rgb8_to_rgba8(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    u8 *data = (u8 *)pdp_packet_data(packet);
    u8 *new_data;
    int w,h, new_p, i;

    w = bitmap->width;
    h = bitmap->height;
    new_p = pdp_packet_new_bitmap_rgba(w,h);
    if (-1 == new_p) return -1;
    new_data = (u8 *)pdp_packet_data(new_p);

    for(i=0; i<w*h; i++){
	new_data[4*i+0]    = data[3*i + 0];
	new_data[4*i+1]    = data[3*i + 1];
	new_data[4*i+2]    = data[3*i + 2];
	new_data[4*i+3]    = 0;
    }

    return new_p;
}
static int _pdp_packet_bitmap_convert_rgba8_to_rgb8(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    u8 *data = (u8 *)pdp_packet_data(packet);
    u8 *new_data;
    int w,h, new_p, i;

    w = bitmap->width;
    h = bitmap->height;
    new_p = pdp_packet_new_bitmap_rgb(w,h);
    if (-1 == new_p) return -1;
    new_data = (u8 *)pdp_packet_data(new_p);

    for(i=0; i<w*h; i++){
	new_data[3*i+0]    = data[4*i + 0];
	new_data[3*i+1]    = data[4*i + 1];
	new_data[3*i+2]    = data[4*i + 2];
    }

    return new_p;
}
static int _pdp_packet_bitmap_convert_rgb8_to_mchp(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    u8 *data = (u8 *)pdp_packet_data(packet);
    s16 *new_data;
    int w,h;
    int new_p, i, plane;

    w = bitmap->width;
    h = bitmap->height;
    plane = w*h;
    new_p = pdp_packet_new_image_multi(w,h,3);
    if (-1 == new_p) return -1;
    new_data = (s16 *)pdp_packet_data(new_p);

    for(i=0; i<w*h; i++){
	new_data[i]          = ((u32)data[3*i + 0]) << 7;
	new_data[i+plane]    = ((u32)data[3*i + 1]) << 7;
	new_data[i+plane*2]  = ((u32)data[3*i + 2]) << 7;
    }

    return new_p;
}

static int _pdp_packet_bitmap_convert_yv12_tofrom_i420(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *in = pdp_packet_bitmap_info(packet);
    int w = in->width;
    int h = in->height;
    int out_encoding;
    if (in->encoding == PDP_BITMAP_YV12)      out_encoding = PDP_BITMAP_I420;
    else if (in->encoding == PDP_BITMAP_I420) out_encoding = PDP_BITMAP_YV12;
    else return -1;

    int new_p = pdp_packet_new_bitmap(out_encoding, w,h);
    // t_pdp *out_h = pdp_packet_header(new_p);
    // pdp_post("%x %s", out_encoding, out_h->desc->s_name);


    if (-1 == new_p) return -1;
    unsigned char *in_d = pdp_packet_data(packet);
    unsigned char *out_d = pdp_packet_data(new_p);
    int plane = w*h;
    memcpy(out_d, in_d, plane);
    out_d += plane;
    in_d += plane;
    plane /= 4;
    memcpy(out_d, in_d+plane, plane);
    memcpy(out_d+plane, in_d, plane);
 


   return new_p;    
}


static int _pdp_packet_bitmap_convert_yv12_to_image(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_bitmap *bitmap = pdp_packet_bitmap_info(packet);
    u8 *data = (u8 *)pdp_packet_data(packet);
    s16 *new_data;
    int w,h;
    int new_p;

    w = bitmap->width;
    h = bitmap->height;
    new_p = pdp_packet_new_image_YCrCb(w,h);
    new_data = pdp_packet_data(new_p);
    if (-1 == new_p || !new_data) return -1;
    pdp_llconv(data, RIF_YVU__P411_U8, new_data, RIF_YVU__P411_S16, w, h);

    return new_p;
}

static int _pdp_packet_bitmap_convert_mchp_to_rgb8(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    t_image *image = (t_image *)pdp_packet_subheader(packet);
    s16 *data = (s16 *)pdp_packet_data(packet);
    u8 *new_data;
    int w = image->width;
    int h = image->height;
    int plane = w*h;
    int nb_channels = image->depth;
    int new_p, i;

    //    static inline u8 _map(s32 pixel){
    inline u8 _map(s32 pixel){
	s32 mask = ~(pixel>>16);
	return ((pixel >> 7) & mask);
    }

    switch(nb_channels){
    default: return -1;
    case 1:
	if (-1 == (new_p = pdp_packet_new_bitmap_grey(w,h))) return -1;
	new_data = (u8*)pdp_packet_data(new_p);
	for(i=0; i<plane; i++) new_data[i] = _map(data[i]); 
	break;
    case 3:
	if (-1 == (new_p = pdp_packet_new_bitmap_rgb(w,h)))  return -1;
	new_data = (u8*)pdp_packet_data(new_p);
	for(i=0; i<plane; i++){
	    new_data[3*i+0] = _map(data[i]); 
	    new_data[3*i+1] = _map(data[i+plane]); 
	    new_data[3*i+2] = _map(data[i+plane*2]);
	}
	break;
    case 4:
	if (-1 == (new_p = pdp_packet_new_bitmap_rgba(w,h))) return -1;
	new_data = (u8*)pdp_packet_data(new_p);
	for(i=0; i<plane; i++){
	    new_data[4*i+0] = _map(data[i]); 
	    new_data[4*i+1] = _map(data[i+plane]); 
	    new_data[4*i+2] = _map(data[i+plane*2]);
	    new_data[4*i+3] = _map(data[i+plane*3]);
	}
	break;
	
	
    }

    return new_p;

}

static int _pdp_packet_bitmap_convert_fallback(int packet, t_pdp_symbol *dest_template)
{
    pdp_post("can't convert image type %s to %s",
	 pdp_packet_get_description(packet)->s_name, dest_template->s_name);

    return -1;
}

static int pdp_bitmap_factory(t_pdp_symbol *type)
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
    s = pdp_list_pop(l).w_symbol; // first element is "bitmap"
    s = pdp_list_pop(l).w_symbol;

    /* get image type */
    if (s == pdp_gensym("grey")) t = PDP_BITMAP_GREY;
    else if (s == pdp_gensym("yv12")) t = PDP_BITMAP_YV12;
    else if (s == pdp_gensym("rgb")) t = PDP_BITMAP_RGB;
    else goto exit;
    
    /* get image dimensions and create image */
    s = pdp_list_pop(l).w_symbol;
    sscanf(s->s_name, "%dx%d", &w, &h);
    p = pdp_packet_new_bitmap(t,w,h);

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

void pdp_bitmap_setup(void)
{
    t_pdp_conversion_program *program;

    /* setup class object */
    bitmap_class = pdp_class_new(pdp_gensym("bitmap/*/*"), pdp_bitmap_factory);
				 

    /* setup conversion programs */
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_grey_to_grey8, 0);
    pdp_type_register_conversion(pdp_gensym("image/grey/*"), pdp_gensym("bitmap/grey/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_grey8_to_grey, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/grey/*"), pdp_gensym("image/grey/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_YCrCb_to_rgb8, 0);
    pdp_type_register_conversion(pdp_gensym("image/YCrCb/*"), pdp_gensym("bitmap/rgb/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_image_to_yv12, 0);
    pdp_type_register_conversion(pdp_gensym("image/YCrCb/*"), pdp_gensym("bitmap/yv12/*"), program);
    pdp_type_register_conversion(pdp_gensym("image/grey/*"), pdp_gensym("bitmap/yv12/*"), program);
    pdp_type_register_conversion(pdp_gensym("image/YCrCb/*"), pdp_gensym("bitmap/*"), program);
    pdp_type_register_conversion(pdp_gensym("image/grey/*"), pdp_gensym("bitmap/*"), program);


    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_yv12_to_image, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/yv12/*"), pdp_gensym("image/YCrCb/*"), program);

    /* rgb->YCrCb converts the colour space */
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_rgb8_to_YCrCb, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/rgb/*"), pdp_gensym("image/YCrCb/*"), program);


    /* rgb <-> multi does not convert the colour space */
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_rgb8_to_mchp, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/rgb/*"), pdp_gensym("image/multi/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_mchp_to_rgb8, 0);
    pdp_type_register_conversion(pdp_gensym("image/multi/*"), pdp_gensym("bitmap/*/*"), program);


    /* yv12 <-> i420 */
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_yv12_tofrom_i420, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/yv12/*"), pdp_gensym("bitmap/i420/*"), program);
    pdp_type_register_conversion(pdp_gensym("bitmap/i420/*"), pdp_gensym("bitmap/yv12/*"), program);

    /* rgb <-> rgba */
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_rgb8_to_rgba8, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/rgb/*"), pdp_gensym("bitmap/rgba/*"), program);
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_rgba8_to_rgb8, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/rgba/*"), pdp_gensym("bitmap/rgb/*"), program);



    /* fallback rgb convertor */
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_rgb8_to_YCrCb, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/rgb/*"), pdp_gensym("image/*/*"), program);

    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_rgb8_to_bitmap_YCrCb, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/rgb/*"), pdp_gensym("bitmap/yv12/*"), program);


    /* fallbacks */
    program = pdp_conversion_program_new(_pdp_packet_bitmap_convert_fallback, 0);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("bitmap/*/*"), program);
    pdp_type_register_conversion(pdp_gensym("bitmap/*/*"), pdp_gensym("image/*/*"), program);
    pdp_type_register_conversion(pdp_gensym("bitmap/*/*"), pdp_gensym("bitmap/*/*"), program);

}
