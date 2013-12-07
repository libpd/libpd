/*
 *   OpenGL Extension Module for pdp - texture packet implementation
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


/* this modules implemtents the opengl texture packet
   it contains only portable opengl code */

#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "pdp_opengl.h"
#include "pdp_texture.h"
#include "pdp_dpd_command.h"


static t_pdp_class *texture_class;

static t_pdp_dpd_commandfactory _tex_cf;

typedef struct _texture_command
{
    t_pdp_dpd_command base;
    int p_src;
    int p_dst;
} t_texture_command;


/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif



/* returns a pointer to the packet subheader given the pdp header */
static t_texture *_pdp_tex_info(t_pdp *x)
{
    return (t_texture *)&(x->info.raw);
}


/* create a pow of 2 texture dimension, ge than 8 */
static int _round_to_pow_2(int n){
    int r = 8;
    while (n > r) r <<= 1;
    return r;
}

t_pdp_symbol *_pdp_get_tex_description_from_params(GLsizei width, GLsizei height, GLint format)
{
    char description[1024];
    char *c = description;
    
    c += sprintf(c, "texture");
    switch(format){
    case GL_LUMINANCE: c += sprintf(c, "/grey"); break;
    case GL_RGB:       c += sprintf(c, "/rgb"); break;
    case GL_RGBA:      c += sprintf(c, "/rgba"); break;
    default:
	c += sprintf(c, "/unknown"); goto exit;
    }
    c += sprintf(c, "/%dx%d", width, height);
	    
 exit:
    return pdp_gensym(description);
}

t_pdp_symbol *_pdp_tex_get_description(t_pdp *header)
{
    t_texture *texture = _pdp_tex_info(header);
    int encoding;

    if (!header) return pdp_gensym("invalid");
    else if (!header->desc){
	if (header->type == PDP_TEXTURE){
	    /* if description is not defined, try to construct it */
	    return _pdp_get_tex_description_from_params(texture->width, texture->height, texture->format);
	}
	else return pdp_gensym("unknown");
    }
    else return header->desc;
}


static int _pdp_packet_texture_old_or_dummy(u32 width, u32 height, s32 format);
static void _pdp_packet_gentexture(int packet);

static void texture_command_convert_bitmap_to_texture(t_texture_command *c)
{
    t_texture *t = (t_texture *)pdp_packet_subheader(c->p_dst);

    /* make sure packet contains a texture, since it is created with _pdp_packet_reuse_texture */
    _pdp_packet_gentexture(c->p_dst);

    /* flip source image before uploading */
    pdp_packet_bitmap_flip_top_bottom(c->p_src);

    /* fill texture */
    pdp_packet_texture_make_current(c->p_dst);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t->sub_width, t->sub_height, 
		    t->format, GL_UNSIGNED_BYTE, (char *)pdp_packet_data(c->p_src));

    /* decrease refcount */
    pdp_packet_mark_unused(c->p_src);
    pdp_packet_mark_unused(c->p_dst);

    //post("conversion done");
    pdp_dpd_command_suicide(c);
}


/* converters to standard pdp types */
int _pdp_packet_texture_convert_image_to_texture(int packet, t_pdp_symbol *dest_template)
{
    int p_temp, p;

    //post ("converting to bitmap");
    p_temp = pdp_packet_convert_rw(packet, pdp_gensym("bitmap/*/*"));
    if (p_temp == -1) return -1;

    //post ("converting to texture");
    p = pdp_packet_convert_rw(p_temp, pdp_gensym("texture/*/*"));
    pdp_packet_mark_unused(p_temp);
    return p;
}



/* converters to standard pdp types */
int _pdp_packet_texture_convert_bitmap_to_texture(int packet, t_pdp_symbol *dest_template)
{
    t_pdp *header = pdp_packet_header(packet);
    void  *data   = pdp_packet_data  (packet);
    int new_p;
    u32 w;
    u32 h;
    t_texture_command *c;

    if (!pdp_packet_bitmap_isvalid(packet)) return -1;

    w = header->info.image.width;
    h = header->info.image.height;

    switch (header->info.image.encoding){
    case PDP_BITMAP_GREY:
	/* create greyscale texture */
	new_p = _pdp_packet_texture_old_or_dummy(w,h, GL_LUMINANCE);
	break;
    case PDP_BITMAP_RGB:
	/* create rgb texture */
	new_p = _pdp_packet_texture_old_or_dummy(w,h, GL_RGB);
	break;
    case PDP_BITMAP_RGBA:
	/* create greyscale texture */
	new_p = _pdp_packet_texture_old_or_dummy(w,h, GL_RGBA);
	break;
    default:
	new_p = -1;
	break;
    }

    if (new_p != -1){

	/* remark: this is a hack. a texture has to be created
	   when a rendering context is active. this means it has
	   to be created in the correct thread. therefore a dpd
	   command is added to the 3dp queue. this seems to work,
	   but without a dropping mechanism, this can overload the
	   queue. the real solution would be to add a converter
	   object to a 3dp chain, or to accept image or bitmap
	   packets in 3dp objects */


	/* dispatch command */
	c = (t_texture_command *)pdp_dpd_commandfactory_get_new_command(&_tex_cf);
	c->p_src = pdp_packet_copy_rw(packet);
	c->p_dst = pdp_packet_copy_ro(new_p);
	pdp_procqueue_add(pdp_opengl_get_queue(), c, texture_command_convert_bitmap_to_texture, 0, 0);
    }
    return new_p;

}



int _pdp_packet_texture_convert_texture_to_bitmap(int packet, t_pdp_symbol *dest_template0)
{
    post("_pdp_packet_texture_convert_texture_to_bitmap not implemented."); 
    return -1;
}


t_texture *pdp_packet_texture_info(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    if (pdp_packet_texture_isvalid(packet)) return _pdp_tex_info(header);
    else return 0;
}

/* check if valid texture packet. all other methods assume packet is valid */
int pdp_packet_texture_isvalid(int packet)
{
    t_pdp *header;
    if (!(header = pdp_packet_header(packet))) return 0;
    if (PDP_TEXTURE != header->type) return 0;
    return glIsTexture(_pdp_tex_info(header)->tex_obj);
}



static void _tex_init_obj(t_texture *t)
{
    //u8 *dummydata;
    //int i;

    glBindTexture(GL_TEXTURE_2D, t->tex_obj);
    glTexImage2D(GL_TEXTURE_2D, 0, t->format, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    /* debug
    dummydata = (u8 *)malloc(t->width*t->height*4);
    for (i=0; i<t->width*t->height*4; i++){dummydata[i] = random(); }
    glTexImage2D(GL_TEXTURE_2D, 0, t->format, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dummydata);
    free(dummydata);
    */

}


static void _pdp_tex_copy(t_pdp *dst, t_pdp *src);
static void _pdp_tex_clone(t_pdp *dst, t_pdp *src);
static void _pdp_tex_reinit(t_pdp *dst);
static void _pdp_tex_cleanup(t_pdp *dst);

static void _pdp_tex_init_methods(t_pdp *h)
{
    h->theclass = texture_class;
}

static void _pdp_tex_reinit(t_pdp *dst)
{
    /* this does nothing. texture is assumed to be in a valid state */
}
static void _pdp_tex_clone(t_pdp *dst, t_pdp *src)
{
    t_texture *dst_t = _pdp_tex_info(dst);
    t_texture *src_t = _pdp_tex_info(src);

    //post("WARNING: _pdp_tex_clone: should not be called from outside 3d thread");

    /* determine if destination texture is valid */
    if (glIsTexture(dst_t->tex_obj)){
	/* check format */
	if ((dst_t->width >= src_t->width) 
	    && (dst_t->height >= src_t->height) 
	    && (dst_t->format == src_t->format)){
	    dst_t->sub_width = src_t->sub_width;
	    dst_t->sub_height = src_t->sub_height;
	    return;
	}
    }
    /* not initialized, so we need to create a new one */
    else {
	glGenTextures(1, (GLuint*)&dst_t->tex_obj);
    }

    /* setup header */
    dst_t->width = src_t->width;
    dst_t->height = src_t->height;
    dst_t->format = src_t->format;
    dst_t->sub_width = src_t->sub_width;
    dst_t->sub_height = src_t->sub_height;

    /* setup packet methods */
    _pdp_tex_init_methods(dst);

    /* init description */
    dst->desc = _pdp_tex_get_description(dst);

}
static void _pdp_tex_copy(t_pdp *dst, t_pdp *src)
{
    /* texture copying is inefficient. for the tex extensions there is no analogy
       for "efficient in-place processing"
       this means the pdp_packet_register_rw() call should be avoided
       this inconsistency should be tucked away in a texture base class */

    /* todo: use texture combining extensions for this */

    post("WARNING: fanout is not yet implemented correctly for texture packets");

    /* not implemented yet, just a call to the clone method */
    _pdp_tex_clone(dst, src);
}

static void _pdp_tex_cleanup(t_pdp *dst)
{
    t_texture *t = _pdp_tex_info(dst);
    glDeleteTextures(1, (GLuint*)&t->tex_obj);
    t->tex_obj = -1; /* is this value guaranteed to be invalid? */
}


/* texture constructors */

/* reuse a texture, or create a "dummy" == packet with everything except a valid texture object */
static int _pdp_packet_texture_old_or_dummy(u32 width, u32 height, s32 format)
{
    int p = -1;
    t_pdp *h;
    t_texture *t;

    int p2_w = _round_to_pow_2(width);
    int p2_h = _round_to_pow_2(height);


    /* try to reuse a texture packet or get a new one */
    p = pdp_packet_reuse(_pdp_get_tex_description_from_params(p2_w, p2_h, format));
    if (-1 == p) p = pdp_packet_create(PDP_TEXTURE, 0);
    if (-1 == p) return -1;

    h = pdp_packet_header(p);
    t = _pdp_tex_info(h);
    
    /* check if alloc succeded */
    if (!h) return -1;
    
    /* check if tex is already initialized */
    if (pdp_packet_texture_isvalid(p)){
	/* check format */
	if ((t->width >= width) && (t->height >= height) && (t->format == format)){
	    //post("pdp_packet_new_tex: reused");
	    t->sub_width = width;
	    t->sub_height = height;
	    return p;
	}
	post("ERROR: pdp_packet_new_texture: pdp_packet_reuse returned wrong type");
    }

    /* determine the texture dims * setup rest of data struct */
    t->width = 64;
    t->height = 64;
    while (t->width < width) t->width <<= 1;
    while (t->height < height) t->height <<= 1;

    t->format = format;
    t->sub_width = width;
    t->sub_height = height;

    _pdp_tex_init_methods(h);


    /* init the texture */
    //_tex_init_obj(t); 

    /* init description */
    h->desc = _pdp_tex_get_description(h);


    return p;
}

/* don't call this method on a non-texture object! */
static void _pdp_packet_gentexture(int p)
{
    t_texture *t;
    if (!pdp_packet_texture_isvalid(p)){
	/* not initialized, so we need to create a new one */
	// post("generating texture");
	t = (t_texture *)pdp_packet_subheader(p);

	/* create the texture object */
	glGenTextures(1, (GLuint *)&t->tex_obj);

	/* init the texture */
	_tex_init_obj(t);

    }
}

int pdp_packet_new_texture(u32 width, u32 height, s32 format)
{
    t_texture *t;
    int p = _pdp_packet_texture_old_or_dummy(width, height, format);

    //post("WARNING: pdp_packet_new_texture: this method should not be called outside the 3dp thread");

    if (p == -1) return -1;
    _pdp_packet_gentexture(p);
    return p;
}


/* high level texture packet operators */

/* make a texture the current texture context */
void pdp_packet_texture_make_current(int packet)
{
    t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return;
    glBindTexture(GL_TEXTURE_2D, t->tex_obj);
}               

void pdp_packet_texture_make_current_enable(int packet)
{
    glEnable(GL_TEXTURE_2D);
    pdp_packet_texture_make_current(packet);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

float pdp_packet_texture_fracx(int packet)
{
    t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return 0.0;
    return (float)t->sub_width/t->width;
}

float pdp_packet_texture_fracy(int packet)
{
    t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return 0.0;
    return (float)t->sub_height/t->height;
}

u32 pdp_packet_texture_total_width(int packet)
{
     t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return 0;
    return t->width;
   
}
u32 pdp_packet_texture_total_height(int packet)
{
     t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return 0;
    return t->height;
   
}

u32 pdp_packet_texture_sub_width(int packet)
{
     t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return 0;
    return t->sub_width;
   
}
u32 pdp_packet_texture_sub_height(int packet)
{
     t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return 0;
    return t->sub_height;
}

float pdp_packet_texture_sub_aspect(int packet)
{
    t_texture *t = pdp_packet_texture_info(packet);
    if (!t) return 0;
    return (float)t->sub_width/t->sub_height;
}

/* setup for 2d operation from texture dimensions */
void pdp_packet_texture_setup_2d_context(int p)
{
    u32 w;
    u32 h;
    float asp;
    if (!pdp_packet_texture_isvalid(p)) return;
    w = pdp_packet_texture_sub_width(p);
    h = pdp_packet_texture_sub_height(p);
    asp = pdp_packet_texture_sub_aspect(p);

    /* set the viewport to the size of the sub texture */
    glViewport(0, 0, w, h);

    /* set orthogonal projection, with a relative frame size of (2asp x 2) */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 2*asp, 0, 2);

    /* set the center of view */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(asp, 1, 0);
    glScalef(1,-1,1);
}

void pdp_texture_setup(void)
{
    t_pdp_conversion_program *program;

    /* setup packet class */
    texture_class = pdp_class_new(pdp_gensym("texture/*/*"), 0);
    texture_class->cleanup = _pdp_tex_cleanup;
    texture_class->wakeup = _pdp_tex_reinit;
    //texture_class->clone = _pdp_tex_clone;
    texture_class->copy = _pdp_tex_copy;
    
    /* init command list */
    pdp_dpd_commandfactory_init(&_tex_cf, sizeof(t_texture_command));



    /* setup programs */
    program = pdp_conversion_program_new(_pdp_packet_texture_convert_bitmap_to_texture, 0);
    pdp_type_register_conversion(pdp_gensym("bitmap/*/*"), pdp_gensym("texture/*/*"), program);

    /* this is a hack to use until the type conversion system has a proper search algo */
    program = pdp_conversion_program_new(_pdp_packet_texture_convert_image_to_texture, 0);
    pdp_type_register_conversion(pdp_gensym("image/*/*"), pdp_gensym("texture/*/*"), program);


    program = pdp_conversion_program_new(_pdp_packet_texture_convert_texture_to_bitmap, 0);
    pdp_type_register_conversion(pdp_gensym("texture/*/*"), pdp_gensym("bitmap/*/*"), program);

}

/* all symbols are C-style */
#ifdef __cplusplus
}
#endif
