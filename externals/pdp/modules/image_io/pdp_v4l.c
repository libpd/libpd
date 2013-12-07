/*
 *   Pure Data Packet module.
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


#include "pdp_config.h"
#include "pdp.h"
#include "pdp_llconv.h"
#include "pdp_imageproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/types.h>

#ifdef HAVE_LIBV4L1_VIDEODEV_H
# include <libv4l1-videodev.h>
#else
# warning trying to use deprecated V4L-1 API
# include <linux/videodev.h>
#endif

#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>

// dont open any more after a set number 
// of failed attempts
// this is to prevent locks on auto-open
// is reset when manually opened or closed
#define PDP_XV_RETRIES 10

//include it anyway
//#ifdef HAVE_PWCV4L
#include "pwc-ioctl.h"
//#endif


#define DEVICENO 0
#define NBUF 2
#define COMPOSITEIN 1




typedef struct pdp_v4l_struct
{
  t_object x_obj;
  t_float x_f;
  
  t_outlet *x_outlet0;

  int x_format; // 0 means autodetect  

  bool x_initialized;
  bool x_auto_open;

  unsigned int x_width;
  unsigned int x_height;
  int x_channel;
  unsigned int x_norm;
  int x_freq;

  unsigned int x_framerate;

  struct video_tuner x_vtuner;
  struct video_picture x_vpicture;
  struct video_buffer x_vbuffer;
  struct video_capability x_vcap;
  struct video_channel x_vchannel;
  struct video_audio x_vaudio;
  struct video_mbuf x_vmbuf;
  struct video_mmap x_vmmap[NBUF];
  struct video_window x_vwin;
  int x_tvfd;
  int x_frame;
  unsigned char *x_videobuf;
  int x_skipnext;
  int x_mytopmargin, x_mybottommargin;
  int x_myleftmargin, x_myrightmargin;

    t_symbol *x_device;
    t_symbol *x_image_type;
    //int x_pdp_image_type;
    int x_v4l_palette;

    pthread_t x_thread_id;
    int x_continue_thread;
    int x_frame_ready;
    int x_only_new_frames;
    int x_last_frame;


    int x_open_retry;

    u32 x_minwidth;
    u32 x_maxwidth;
    u32 x_minheight;
    u32 x_maxheight;


} t_pdp_v4l;





static void pdp_v4l_audio(t_pdp_v4l *x, t_floatarg f)
{
    int i = 0;
    if (x->x_initialized){
	fprintf(stderr,"  audios  : %d\n",x->x_vcap.audios);
	x->x_vaudio.audio = 0;
        ioctl(x->x_tvfd,VIDIOCGAUDIO, &x->x_vaudio);

	fprintf(stderr,"    %d (%s): ",i,x->x_vaudio.name);
	if (x->x_vaudio.flags & VIDEO_AUDIO_MUTABLE)
	    fprintf(stderr,"muted=%s ",
		    (x->x_vaudio.flags & VIDEO_AUDIO_MUTE) ? "yes":"no");
	if (x->x_vaudio.flags & VIDEO_AUDIO_VOLUME)
	    fprintf(stderr,"volume=%d ",x->x_vaudio.volume);
	if (x->x_vaudio.flags & VIDEO_AUDIO_BASS)
	    fprintf(stderr,"bass=%d ",x->x_vaudio.bass);
	if (x->x_vaudio.flags & VIDEO_AUDIO_TREBLE)
	    fprintf(stderr,"treble=%d ",x->x_vaudio.treble);
	fprintf(stderr,"\n");
	
    }
}


static void pdp_v4l_close(t_pdp_v4l *x)
{
  /* close the v4l device and dealloc buffer */

    void *dummy;

    /* terminate thread if there is one */
    if(x->x_continue_thread){
	x->x_continue_thread = 0;
	pthread_join (x->x_thread_id, &dummy);
    }


    if (x->x_tvfd >= 0)
    {
        close(x->x_tvfd);
        x->x_tvfd = -1;
    }

    if (x->x_initialized){
	munmap(x->x_videobuf, x->x_vmbuf.size);
	x->x_initialized = false;
    }

}

static void pdp_v4l_close_manual(t_pdp_v4l *x)
{
    x->x_open_retry = PDP_XV_RETRIES;
    pdp_v4l_close(x);

}

static void pdp_v4l_close_error(t_pdp_v4l *x)
{
    pdp_v4l_close(x);
    if(x->x_open_retry) x->x_open_retry--;
}

static void pdp_v4l_pwc_agc(t_pdp_v4l *x, float gain){
    gain *= (float)(1<<16);
    int g = (int)gain;
    if (g < 0) g = -1;            // automatic
    if (g > 1<<16) g = 1<<16 - 1; // fixed

    //post("pdp_v4l: setting agc to %d", g);
    if (ioctl(x->x_tvfd, VIDIOCPWCSAGC, &g)){
	post("pdp_v4l: pwc: VIDIOCPWCSAGC");
	//goto closit;
    }
}

static void pdp_v4l_pwc_init(t_pdp_v4l *x)
{
    struct pwc_probe probe;
    int isPhilips = 0;

#ifdef HAVE_PWCV4L
    /* skip test */
    isPhilips = 1;
#else
    /* test for pwc */
    if (ioctl(x->x_tvfd, VIDIOCPWCPROBE, &probe) == 0)
       if (!strcmp(x->x_vcap.name, probe.name))
         isPhilips = 1;

#endif
    
    /* don't do pwc specific stuff */
    if (!isPhilips) return;

    post("pdp_v4l: detected pwc");


    if(ioctl(x->x_tvfd, VIDIOCPWCRUSER)){
	perror("pdp_v4l: pwc: VIDIOCPWCRUSER");
	goto closit;
    }

    /* this is a workaround:
       we disable AGC after restoring user prefs
       something is wrong with newer cams (like Qickcam 4000 pro)
    */

    if (1){
	pdp_v4l_pwc_agc(x, 1.0);
    }

 
    if (ioctl(x->x_tvfd, VIDIOCGWIN, &x->x_vwin)){
	perror("pdp_v4l: pwc: VIDIOCGWIN");
	goto closit;
    }


    
    if (x->x_vwin.flags & PWC_FPS_MASK){
	//post("pdp_v4l: pwc: camera framerate: %d", (x->x_vwin.flags & PWC_FPS_MASK) >> PWC_FPS_SHIFT);
	//post("pdp_v4l: pwc: setting camera framerate to %d", x->x_framerate);
	x->x_vwin.flags &= PWC_FPS_MASK;
	x->x_vwin.flags |= (x->x_framerate << PWC_FPS_SHIFT);
	if (ioctl(x->x_tvfd, VIDIOCSWIN, &x->x_vwin)){
	    perror("pdp_v4l: pwc: VIDIOCSWIN");
	    goto closit;
	}
	if (ioctl(x->x_tvfd, VIDIOCGWIN, &x->x_vwin)){
	    perror("pdp_v4l: pwc: VIDIOCGWIN");
	    goto closit;
	}
	post("pdp_v4l: camera framerate set to %d fps", (x->x_vwin.flags & PWC_FPS_MASK) >> PWC_FPS_SHIFT);

    }
    
    
    return;
  


 closit:
    pdp_v4l_close_error(x);
    return;

}

static void pdp_v4l_sync_frame(t_pdp_v4l* x){
    /* grab frame */
    if (ioctl(x->x_tvfd, VIDIOCSYNC, &x->x_vmmap[x->x_frame].frame) < 0){
	perror("pdp_v4l: VIDIOCSYNC");
	pdp_v4l_close(x);
	return;
    }
} 

static void pdp_v4l_capture_frame(t_pdp_v4l* x){
    if (ioctl(x->x_tvfd, VIDIOCMCAPTURE, &x->x_vmmap[x->x_frame]) < 0){
	if (errno == EAGAIN)
	    post("pdp_v4l: can't sync (no video source?)\n");
	else 
	    perror("pdp_v4l: VIDIOCMCAPTURE");
	if (ioctl(x->x_tvfd, VIDIOCMCAPTURE, &x->x_vmmap[x->x_frame]) < 0)
	    perror("pdp_v4l: VIDIOCMCAPTURE2");
	
	post("pdp_v4l: frame %d %d, format %d, width %d, height %d",
	     x->x_frame, x->x_vmmap[x->x_frame].frame, x->x_vmmap[x->x_frame].format,
	     x->x_vmmap[x->x_frame].width, x->x_vmmap[x->x_frame].height);
	
	pdp_v4l_close(x);
	return;
    }
}


static void *pdp_v4l_thread(void *voidx)
{
    t_pdp_v4l *x = ((t_pdp_v4l *)voidx);


    /* flip buffers */
    x->x_frame ^= 0x1;

    /* capture with a double buffering scheme */
    while (x->x_continue_thread){

	/* schedule capture command for next frame */
	pdp_v4l_capture_frame(x);

	/* wait until previous capture is ready */
	x->x_frame ^= 0x1;
	pdp_v4l_sync_frame(x);

	/* setup pointers for main thread */
	x->x_frame_ready = 1;
	x->x_last_frame = x->x_frame;

    }

    return 0;
}

static void pdp_v4l_setlegaldim(t_pdp_v4l *x, int xx, int yy);

static void pdp_v4l_open(t_pdp_v4l *x, t_symbol *name)
{
  /* open a v4l device and allocate a buffer */

    unsigned int size;
    int i;

    unsigned int width, height;


    /* if already opened -> close */
    if (x->x_initialized) pdp_v4l_close(x);


    /* exit if retried too much */
    if (!x->x_open_retry){
	post("pdp_v4l: retry count reached zero for %s", name->s_name);
	post("pdp_v4l: try to open manually");
	return;
    }

    post("pdp_v4l: opening %s", name->s_name);

    x->x_device = name;

    if ((x->x_tvfd = open(name->s_name, O_RDWR)) < 0)
    {
      post("pdp_v4l: error:");
        perror(name->s_name);
        goto closit;
    }


    if (ioctl(x->x_tvfd, VIDIOCGCAP, &x->x_vcap) < 0)
    {
        perror("get capabilities");
        goto closit;
    }

    post("pdp_v4l: cap: name %s type %d channels %d maxw %d maxh %d minw %d minh %d",
        x->x_vcap.name, x->x_vcap.type,  x->x_vcap.channels,  x->x_vcap.maxwidth,  x->x_vcap.maxheight,
            x->x_vcap.minwidth,  x->x_vcap.minheight);

    x->x_minwidth = pdp_imageproc_legalwidth(x->x_vcap.minwidth);
    x->x_maxwidth = pdp_imageproc_legalwidth_round_down(x->x_vcap.maxwidth);
    x->x_minheight = pdp_imageproc_legalheight(x->x_vcap.minheight);
    x->x_maxheight = pdp_imageproc_legalheight_round_down(x->x_vcap.maxheight);

 
    if (ioctl(x->x_tvfd, VIDIOCGPICT, &x->x_vpicture) < 0)
    {
        perror("VIDIOCGCAP");
        goto closit;
    }
    
    post("pdp_v4l: picture: brightness %d depth %d palette %d",
            x->x_vpicture.brightness, x->x_vpicture.depth, x->x_vpicture.palette);

    /* get channel info */
    for (i = 0; i < x->x_vcap.channels; i++)
    {
        x->x_vchannel.channel = i;
        if (ioctl(x->x_tvfd, VIDIOCGCHAN, &x->x_vchannel) < 0)
        {
            perror("VDIOCGCHAN");
            goto closit;
        }
        post("pdp_v4l: channel %d name %s type %d flags %d",
            x->x_vchannel.channel, x->x_vchannel.name, 
            x->x_vchannel.type, x->x_vchannel.flags);
    }

    /* switch to the desired channel */
    if (x->x_channel < 0) x->x_channel = 0;
    if (x->x_channel >= x->x_vcap.channels) x->x_channel = x->x_vcap.channels - 1;

    x->x_vchannel.channel = x->x_channel;
    if (ioctl(x->x_tvfd, VIDIOCGCHAN, &x->x_vchannel) < 0)
    {
        perror("pdp_v4l: warning: VDIOCGCHAN");
        post("pdp_v4l: cant change to channel %d",x->x_channel);

        // ignore error
        // goto closit;
    }
    else{
	post("pdp_v4l: switched to channel %d", x->x_channel);
    }


    /* set norm */
    x->x_vchannel.norm = x->x_norm;
    if (ioctl(x->x_tvfd, VIDIOCSCHAN, &x->x_vchannel) < 0)
    {
        perror("pdp_v4l: warning: VDIOCSCHAN");
        post("pdp_v4l: cant change to norm %d",x->x_norm);
        
        // ignore error
        // goto closit;
    }
    else {
	post("pdp_v4l: set norm to %u", x->x_norm);
    }

    if (x->x_freq > 0){
	if (ioctl(x->x_tvfd, VIDIOCSFREQ, &x->x_freq) < 0)
	    perror ("couldn't set frequency :");
    }


   

    /* get mmap numbers */
    if (ioctl(x->x_tvfd, VIDIOCGMBUF, &x->x_vmbuf) < 0)
    {
        perror("pdp_v4l: VIDIOCGMBUF");
        goto closit;
    }
    post("pdp_v4l: buffer size %d, frames %d, offset %d %d", x->x_vmbuf.size,
        x->x_vmbuf.frames, x->x_vmbuf.offsets[0], x->x_vmbuf.offsets[1]);
    if (!(x->x_videobuf = (unsigned char *)
        mmap(0, x->x_vmbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, x->x_tvfd, 0)))
    {
        perror("pdp_v4l: mmap");
        goto closit;
    }

    pdp_v4l_setlegaldim(x, x->x_width, x->x_height);
    width = x->x_width;
    height = x->x_height;

    for (i = 0; i < NBUF; i++)
    {
      //x->x_vmmap[i].format = VIDEO_PALETTE_YUV420P;
      //x->x_vmmap[i].format = VIDEO_PALETTE_UYVY;
      x->x_vmmap[i].width = width;
      x->x_vmmap[i].height = height;
      x->x_vmmap[i].frame  = i;
    }


/* fallthrough macro for case statement */
#define TRYPALETTE(palette)							\
	x->x_v4l_palette = palette;						\
	for (i = 0; i < NBUF; i++) x->x_vmmap[i].format = x->x_v4l_palette;	\
	if (ioctl(x->x_tvfd, VIDIOCMCAPTURE, &x->x_vmmap[x->x_frame]) < 0)	\
	    {									\
		if (errno == EAGAIN)						\
		    post("pdp_v4l: can't sync (no video source?)");		\
		if (x->x_format) break; /* only break if not autodetecting */	\
	    }									\
	else{									\
	    post("pdp_v4l: using " #palette);					\
	    goto capture_ok;							\
	}

    switch(x->x_format){
    default:
    case 0:
    case 1: TRYPALETTE(VIDEO_PALETTE_YUV420P);
    case 2: TRYPALETTE(VIDEO_PALETTE_YUV422);
    case 3: TRYPALETTE(VIDEO_PALETTE_RGB24);
    case 4: TRYPALETTE(VIDEO_PALETTE_RGB32);
    }

    // none of the formats are supported
    perror("pdp_v4l: VIDIOCMCAPTURE: format not supported");
    goto closit;


 capture_ok:

    post("pdp_v4l: frame %d %d, format %d, width %d, height %d",
        x->x_frame, x->x_vmmap[x->x_frame].frame, x->x_vmmap[x->x_frame].format,
        x->x_vmmap[x->x_frame].width, x->x_vmmap[x->x_frame].height);

    x->x_width = width;
    x->x_height = height;

    post("pdp_v4l: Opened video connection (%dx%d)",x->x_width,x->x_height);


    /* do some pwc specific inits */
    pdp_v4l_pwc_init(x);


    x->x_initialized = true;

    /* create thread */
    x->x_continue_thread = 1;
    x->x_frame_ready = 0;
    pthread_create(&x->x_thread_id, 0, pdp_v4l_thread, x);

    return;
 closit:
    pdp_v4l_close_error(x);

}

static void pdp_v4l_open_manual(t_pdp_v4l *x, t_symbol *name)
{
    x->x_open_retry = PDP_XV_RETRIES;
    pdp_v4l_open(x, name);
}


static void pdp_v4l_channel(t_pdp_v4l *x, t_float f)
{
    int channel = (float)f;

    if (x->x_initialized){
	pdp_v4l_close(x);
        x->x_channel = channel;
	pdp_v4l_open(x, x->x_device);
    }
    else
	x->x_channel = channel;
}

static void pdp_v4l_norm(t_pdp_v4l *x, t_symbol *s)
{
    unsigned int norm;

    if (gensym("PAL") == s) norm = VIDEO_MODE_PAL;
    else if (gensym("NTSC") == s) norm = VIDEO_MODE_NTSC;
    else if (gensym("SECAM") == s) norm = VIDEO_MODE_SECAM;
    else norm = VIDEO_MODE_AUTO;
    


    if (x->x_initialized){
	pdp_v4l_close(x);
        x->x_norm = norm;
	pdp_v4l_open(x, x->x_device);
    }
    else
	x->x_norm = norm;
}

static void pdp_v4l_freq(t_pdp_v4l *x, t_float f)
{
        int freq = (int)f;

	x->x_freq = freq;
	if (x->x_freq > 0){
	    if (ioctl(x->x_tvfd, VIDIOCSFREQ, &x->x_freq) < 0)
		perror ("couldn't set frequency :");
	    //else {post("pdp_v4l: tuner frequency: %f MHz", f / 16.0f);}
	}

}

static void pdp_v4l_freqMHz(t_pdp_v4l *x, t_float f)
{
    pdp_v4l_freq(x, f*16.0f);
}


static void pdp_v4l_bang(t_pdp_v4l *x)
{
   
  /* if initialized, grab a frame and output it */

    unsigned int w,h,nbpixels,packet_size,plane1,plane2;
    unsigned char *newimage;
    int object,length,pos,i,encoding;
    t_pdp* header;
    t_image* image;
    short int * data;


    static short int gain[4] = {0x7fff, 0x7fff, 0x7fff, 0x7fff};

    if (!(x->x_initialized)){
	post("pdp_v4l: no device opened");

	if (x->x_auto_open){
	  post("pdp_v4l: attempting auto open");
	  pdp_v4l_open(x, x->x_device);
	  if (!(x->x_initialized)){
	    post("pdp_v4l: auto open failed");
	    return;
	  }
	}

	else return;
    }


    /* do nothing if there is no frame ready */
    if((!x->x_frame_ready) && (x->x_only_new_frames)) return;
    x->x_frame_ready = 0;

    /* get the address of the "other" frame */
    newimage = x->x_videobuf + x->x_vmbuf.offsets[x->x_last_frame];

    /* create new packet */
    w = x->x_width;
    h = x->x_height;

    //nbpixels = w * h;

/*
    switch(x->x_pdp_image_type){
    case PDP_IMAGE_GREY:   
        packet_size = nbpixels << 1; 
        break;
    case PDP_IMAGE_YV12:   
	packet_size = (nbpixels + (nbpixels >> 1)) << 1;
	break;
    default:
	packet_size = 0;
	post("pdp_v4l: internal error");
    }
*/

    //packet_size = (nbpixels + (nbpixels >> 1)) << 1;


    //plane1 = nbpixels;
    //plane2 = nbpixels + (nbpixels>>2);

    object = pdp_packet_new_image(PDP_IMAGE_YV12, w, h);
    header = pdp_packet_header(object);
    image = pdp_packet_image_info(object);

    if (!header){
	post("pdp_v4l: ERROR: can't allocate packet");
	return;
    }

    data = (short int *) pdp_packet_data(object);
    newimage = x->x_videobuf + x->x_vmbuf.offsets[x->x_frame ^ 0x1];


    /* convert data to pdp packet */

    switch(x->x_v4l_palette){
    case  VIDEO_PALETTE_YUV420P:
	pdp_llconv(newimage, RIF_YUV__P411_U8, data, RIF_YVU__P411_S16, w, h); 
	break;
	
	/* long live standards. v4l's rgb is in fact ogl's bgr */
    case  VIDEO_PALETTE_RGB24:
	pdp_llconv(newimage, RIF_BGR__P____U8, data, RIF_YVU__P411_S16, w, h); 
	break;

    case  VIDEO_PALETTE_RGB32:
	pdp_llconv(newimage, RIF_BGRA_P____U8, data, RIF_YVU__P411_S16, w, h); 
	break;

    case  VIDEO_PALETTE_YUV422:
	pdp_llconv(newimage, RIF_YUYV_P____U8, data, RIF_YVU__P411_S16, w, h); 
	break;


    default:
	post("pdp_v4l: unsupported palette");
	break;
    }

/*
    if (PDP_IMAGE_YV12 == x->x_pdp_image_type){
	pixel_unpack_u8s16_y(&newimage[0], data, nbpixels>>3, x->x_state_data->gain);
	pixel_unpack_u8s16_uv(&newimage[plane1], &data[plane2], nbpixels>>5, x->x_state_data->gain);
	pixel_unpack_u8s16_uv(&newimage[plane2], &data[plane1], nbpixels>>5, x->x_state_data->gain);
    }
*/
    //x->x_v4l_palette = VIDEO_PALETTE_YUV420P;
    //x->x_v4l_palette = VIDEO_PALETTE_RGB24;

/*

    else if(PDP_IMAGE_GREY == x->x_pdp_image_type){
	pixel_unpack_u8s16_y(&newimage[0], data, nbpixels>>3, x->x_state_data->gain);
    }
*/
    //post("pdp_v4l: mark unused %d", object);

    pdp_packet_pass_if_valid(x->x_outlet0, &object);

}


static void pdp_v4l_setlegaldim(t_pdp_v4l *x, int xx, int yy)
{

    unsigned int w,h;

    w  = pdp_imageproc_legalwidth((int)xx);
    h  = pdp_imageproc_legalheight((int)yy);
    
    w = (w < x->x_maxwidth) ? w : x->x_maxwidth;
    w = (w > x->x_minwidth) ? w : x->x_minwidth;

    h = (h < x->x_maxheight) ? h : x->x_maxheight;
    h = (h > x->x_minheight) ? h : x->x_minheight;

    x->x_width = w;
    x->x_height = h;
}

static void pdp_v4l_dim(t_pdp_v4l *x, t_floatarg xx, t_floatarg yy)
{
  if (x->x_initialized){
    pdp_v4l_close(x);
    pdp_v4l_setlegaldim(x, (int)xx, (int)yy);
    pdp_v4l_open(x, x->x_device);
    
  }
  else{
    pdp_v4l_setlegaldim(x, (int)xx, (int)yy);
  }
}

static void pdp_v4l_format(t_pdp_v4l *x, t_symbol *s)
{
    if      (s == gensym("YUV420P")) x->x_format = 1;
    else if (s == gensym("YUV422"))  x->x_format = 2;
    else if (s == gensym("RGB24"))   x->x_format = 3;
    else if (s == gensym("RGB32"))   x->x_format = 4;
    else if (s == gensym("auto"))    x->x_format = 0;
    else {
	post("pdp_v4l: format %s unknown, using autodetect", s->s_name);
	x->x_format = 0;
    }

    if (x->x_initialized){
	pdp_v4l_close(x);
	pdp_v4l_open(x, x->x_device);
    }
}


static void pdp_v4l_free(t_pdp_v4l *x)
{
    pdp_v4l_close(x);
}

t_class *pdp_v4l_class;



void *pdp_v4l_new(t_symbol *vdef, t_symbol *format)
{
    t_pdp_v4l *x = (t_pdp_v4l *)pd_new(pdp_v4l_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything);

    x->x_initialized = false;


    x->x_tvfd = -1;
    x->x_frame = 0;
    x->x_last_frame = 0;

    x->x_framerate = 27;

    x->x_auto_open = true;
    if (vdef != gensym("")){
	x->x_device = vdef;
    }
    else{
	x->x_device = gensym("/dev/video0");
    }

    if (format != gensym("")){
	pdp_v4l_format(x, format);
    }
    else {
	x->x_format = 0; // default is autodetect
    }

    x->x_continue_thread = 0;
    x->x_only_new_frames = 1;

    x->x_width = 320;
    x->x_height = 240;

//    pdp_v4l_type(x, gensym("yv12"));


    x->x_open_retry = PDP_XV_RETRIES;

    x->x_channel = 0;
    x->x_norm = 0; // PAL
    x->x_freq = -1; //don't set freq by default

    x->x_minwidth = pdp_imageproc_legalwidth(0);
    x->x_maxwidth = pdp_imageproc_legalwidth_round_down(0x7fffffff);
    x->x_minheight = pdp_imageproc_legalheight(0);
    x->x_maxheight = pdp_imageproc_legalheight_round_down(0x7fffffff);


    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_v4l_setup(void)
{


    pdp_v4l_class = class_new(gensym("pdp_v4l"), (t_newmethod)pdp_v4l_new,
    	(t_method)pdp_v4l_free, sizeof(t_pdp_v4l), 0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);


    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_audio, gensym("audio"), A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_close_manual, gensym("close"), A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_open_manual, gensym("open"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_channel, gensym("channel"), A_FLOAT, A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_norm, gensym("norm"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_dim, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_freq, gensym("freq"), A_FLOAT, A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_freqMHz, gensym("freqMHz"), A_FLOAT, A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_pwc_agc, gensym("gain"), A_FLOAT, A_NULL);
    class_addmethod(pdp_v4l_class, (t_method)pdp_v4l_format, gensym("captureformat"), A_SYMBOL, A_NULL);

    

}

#ifdef __cplusplus
}
#endif
