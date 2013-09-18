
/*
 *   Pure Data Packet system module. - png glue code
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

#include "pdp_packet.h"
#include "pdp_mem.h"
#include "pdp_post.h"
#include "pdp_debug.h"

// this is an optional module
#include "pdp_config.h" 


#ifdef HAVE_PDP_PNG
//if 0

#include <png.h>

#define READING 1
#define WRITING 2
#define SIG_BYTES 8

#define D if(0)

typedef struct
{
    FILE *x_fp;

    int x_kindof; //READING or WRITING
    png_structp x_png;
    png_infop x_info;
    png_infop x_end_info;

    png_uint_32 x_width;
    png_uint_32 x_height;
    int x_bit_depth;
    int x_color_type;
    int x_interlace_type;
    int x_compression_type;
    int x_filter_type;

    int x_pdp_bitmap_type;
    int x_packet;

} t_png_image;

static t_png_image *_init(t_png_image *x)
{
    x->x_png = 0;
    x->x_info = 0;
    x->x_end_info = 0;
    x->x_fp = 0;
    x->x_packet = -1;
    return x;
}

static int _cleanup(t_png_image *x)
{
#define INTERNAL_ERROR 1
    if (!x) return 1;
    pdp_packet_mark_unused(x->x_packet);
    if (x->x_png)
	switch(x->x_kindof){
	case READING: png_destroy_read_struct(&x->x_png, &x->x_info, &x->x_end_info); break;
	case WRITING: png_destroy_write_struct(&x->x_png, &x->x_info); break;
	default: PDP_ASSERT(INTERNAL_ERROR);
	}
    if (x->x_fp) fclose(x->x_fp);
    return 1;
}

static int _open_read(t_png_image* x, char *file)
{
    unsigned char header[SIG_BYTES];
    int is_png;

    x->x_fp = fopen(file, "r");
    if (!x->x_fp) {
	D pdp_post("can't open %s for reading", file);
	return 0;
    }
    fread(header, 1, SIG_BYTES, x->x_fp);
    is_png = !png_sig_cmp(header, 0, SIG_BYTES);

    D pdp_post("%s is %s png file", file, is_png ? "a" : "not a");

    return is_png;
}

static int _open_write(t_png_image* x, char *file)
{
    char header[SIG_BYTES];
    int is_png;

    x->x_fp = fopen(file, "w");
    if (!x->x_fp) {
	D pdp_post("can't open %s for writing", file);
	return 0;
    }

    return 1;
}

/* progress callback */

static void _row_callback(png_structp p, png_uint_32 row, int pass)
{
    fprintf(stderr, ".");
}

static int _initio_read(t_png_image *x)
{
    png_init_io(x->x_png, x->x_fp);
    png_set_sig_bytes(x->x_png, SIG_BYTES);
    D png_set_read_status_fn(x->x_png, _row_callback);
    return 1;

}

static int _initio_write(t_png_image *x)
{
    png_init_io(x->x_png, x->x_fp);
    D png_set_write_status_fn(x->x_png, _row_callback);

    return 1;

}

static int _checkimagetype(t_png_image *x)
{
    png_read_info(x->x_png, x->x_info);
    png_get_IHDR(x->x_png, x->x_info, &x->x_width, &x->x_height,
       &x->x_bit_depth, &x->x_color_type, &x->x_interlace_type, 
		 &x->x_compression_type, &x->x_filter_type);

    D pdp_post("image info: w=%d, h=%d, depth=%d, type=%d",
	     (int)x->x_width, (int)x->x_height, (int)x->x_bit_depth, 
	     (int)x->x_color_type);


    /* handle paletted images: convert to 8 bit RGB(A) */
    if (x->x_color_type == PNG_COLOR_TYPE_PALETTE &&
        x->x_bit_depth <= 8) {
	png_set_expand(x->x_png);
	D pdp_post("convert palette");

	/* check if there's an alpha channel and set PDP_BITMAP type */
	x->x_pdp_bitmap_type = 
	    (png_get_valid(x->x_png, x->x_info, PNG_INFO_tRNS)) ?
	    PDP_BITMAP_RGBA : PDP_BITMAP_RGB;

	return 1;
    }

    /* handle bitdepth */
    if (x->x_bit_depth < 8) {
	png_set_expand(x->x_png);
	D pdp_post("convert greyscale to 8 bit");
    }
    if (x->x_bit_depth == 16){
	D pdp_post("stripping 16 bit to 8 bit");
	png_set_strip_16(x->x_png);
    }


    /* handle greyscale images */
    if (x->x_color_type == PNG_COLOR_TYPE_GRAY){
	x->x_pdp_bitmap_type = PDP_BITMAP_GREY;
	if (png_get_valid(x->x_png, x->x_info, PNG_INFO_tRNS)){
	    D pdp_post("no support for greyscale images with alpha info");
	    return 0;
	}
	return 1;
    }

    /* handle RGB imges */
    if (x->x_color_type = PNG_COLOR_TYPE_RGB){
	x->x_pdp_bitmap_type = PDP_BITMAP_RGB;
	return 1;
    }

    /* handle RGBA imges */
    if (x->x_color_type = PNG_COLOR_TYPE_RGBA){
	x->x_pdp_bitmap_type = PDP_BITMAP_RGBA;
	return 1;
    }

    
    /* can't handle image type */
    D pdp_post("image type not supported");
    return 0;

}

#define user_error_ptr NULL
#define user_error_fn NULL
#define user_warning_fn NULL

static int _buildstruct_read(t_png_image *x)
{
    x->x_kindof = READING;

    if (!(x->x_png = png_create_read_struct
	(PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr,
	 user_error_fn, user_warning_fn))) return 0;

    if (!(x->x_info = png_create_info_struct(x->x_png))) return 0;
    if (!(x->x_end_info = png_create_info_struct(x->x_png))) return 0;

    return 1;
}

static int _buildstruct_write(t_png_image *x)
{
    x->x_kindof = WRITING;

    if (!(x->x_png = png_create_write_struct
	(PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr,
	 user_error_fn, user_warning_fn))) return 0;

    if (!(x->x_info = png_create_info_struct(x->x_png))) return 0;

    return 1;
}

static int _getimagedata(t_png_image *x)
{
    int nbchans = 0;
    unsigned char *image_data;
    png_bytep row_pointers[x->x_height];
    png_uint_32 i;

    D pdp_post("reading %d rows ", (int)x->x_height);

    switch (x->x_pdp_bitmap_type){
    case PDP_BITMAP_GREY: nbchans = 1; break;
    case PDP_BITMAP_RGB:  nbchans = 3; break;
    case PDP_BITMAP_RGBA: nbchans = 4; break;
    default:
	return 0;
    }

    x->x_packet = 
	pdp_packet_new_bitmap(x->x_pdp_bitmap_type, x->x_width, x->x_height);
    if (!(image_data = pdp_packet_data(x->x_packet))) return 0;

    for(i=0; i<x->x_height; i++)
	row_pointers[i] = image_data + nbchans * i * x->x_width;

    png_read_image(x->x_png, row_pointers);

    D pdp_post("DONE");

    return 1;
}

static int _saveimagedata(t_png_image *x, int packet)
{
    png_bytep *row_pointers;
    png_uint_32 i;
    int nbchans;
    t_pdp *h = pdp_packet_header(packet);
    unsigned char *image_data = (unsigned char *)pdp_packet_data(packet);

    if (!h) return 0;
    if (PDP_BITMAP != h->type) return 0;
    if (!image_data) return 0;

    x->x_width = h->info.bitmap.width;
    x->x_height = h->info.bitmap.height;

    switch(h->info.image.encoding){
    case PDP_BITMAP_GREY: x->x_color_type = PNG_COLOR_TYPE_GRAY; nbchans = 1; break;
    case PDP_BITMAP_RGB:  x->x_color_type = PNG_COLOR_TYPE_RGB;  nbchans = 3; break;
    case PDP_BITMAP_RGBA: x->x_color_type = PNG_COLOR_TYPE_RGBA; nbchans = 4; break;
    default: return 0;
    }
								     
    png_set_IHDR(x->x_png, x->x_info, x->x_width, x->x_height, 8,
		 x->x_color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(x->x_png, x->x_info);

    D pdp_post("writing %d rows ", (int)x->x_height);

    row_pointers = (png_bytep *)pdp_alloc(sizeof(png_bytep) * x->x_height);
    for(i=0; i<x->x_height; i++)
	row_pointers[i] = image_data + nbchans * i * x->x_width;

    png_write_image(x->x_png, row_pointers);
    png_write_end(x->x_png, x->x_info);
    pdp_dealloc(row_pointers);

    D pdp_post("DONE");

    return 1;
}



/* simple functions to load and save png images */

int pdp_packet_bitmap_from_png_file(char *filename)
{
    int packet = -1;
    t_png_image _x;
    t_png_image *x = &_x;

    if (!_init(x)) goto exit;
    if (!_open_read(x, filename)) goto exit;
    if (!_buildstruct_read(x)) goto exit;
    if (!_initio_read(x)) goto exit;
    if (!_checkimagetype(x)) goto exit;
    if (!_getimagedata(x)) goto exit;

    packet = x->x_packet;
    x->x_packet = -1;
    _cleanup(x);
    return packet;
 exit:
    _cleanup(x);
    return -1;

}



int pdp_packet_bitmap_save_png_file(int packet, char *filename)
{

    t_png_image _x;
    t_png_image *x = &_x;

    if (!_init(x)) goto exit;
    if (!_open_write(x, filename)) goto exit;
    if (!_buildstruct_write(x)) goto exit;
    if (!_initio_write(x)) goto exit;
    if (!_saveimagedata(x, packet)) goto exit;

    _cleanup(x);
    return 1;
 exit:
    _cleanup(x);
    return 0;

}



#else //PDP_HAVE_PNG
int pdp_packet_bitmap_save_png_file(int packet, char *filename)
{
    pdp_post("WARNING: no png support, can't save png file %s", filename);
    return 0;
}

int pdp_packet_bitmap_from_png_file(char *filename)
{
    pdp_post("WARNING: no png support, can't load png file %s", filename);
    return -1;
}


#endif //PDP_HAVE_PNG













#if 0
int main(int argc, char **argv)
{
    char *image = 0;

    if (argc != 2) 
	pdp_post("usage: %s <png file>", argv[0]);
    else
	image = load_png(argv[1]);

    pdp_post ("%s", image ? "OK" : "ERROR");

}
#endif
