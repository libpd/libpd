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


//#include <quicktime/lqt.h>
//#include <quicktime/colormodels.h>
#include <lqt/lqt.h>
#include <lqt/colormodels.h>

#include "pdp.h"
#include "pdp_llconv.h"


#if PD_MAJOR_VERSION==0 && PD_MINOR_VERSION>=43
#include "s_stuff.h" // need to get sys_libdir for libquicktime plugins
#endif

#define min(x,y) ((x<y)?(x):(y))



#define FREE(x) {if (x) {pdp_dealloc(x); x=0;} else post("free null pointer");}


/* debug macro */
//#define DEBUG_MSG_ENABLED

#ifdef DEBUG_MSG_ENABLED

#define DEBUG_MSG(EXP)\
fprintf (stderr, "mark start: [" #EXP "], on line %d\n", __LINE__);\
 EXP \
fprintf (stderr, "mark end:   [" #EXP "], on line %d\n", __LINE__);

#else
#define DEBUG_MSG(EXP) EXP 
#endif

typedef struct pdp_qt_struct
{
    t_object x_obj;
    t_float x_f;

    float x_gain;


    t_symbol *x_name;   // this is our name
    int x_istilde;      // 0==pdp_qt / 1==pdp_qt~
    int x_syncaudio;


    /* clock object */
    t_clock *x_clock;
    int x_counter;
    int x_queue_id;

    /* audio outlets */
    t_outlet *x_outleft;
    t_outlet *x_outright;

    /* message outlets */
    t_outlet *x_outlet0;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;

    /* pdp data */
    int x_packet0;

    /* toggles */
    int x_loop;
    int x_autoplay;

    /* qt data */
    unsigned char ** x_qt_rows; // pointer array to rows / colour planes
    float ** x_qt_audiochans;   // pointer array to audio channel buffers
    unsigned char * x_qt_frame;
    quicktime_t *x_qt;
    int x_qt_cmodel;

    //t_pdp_qt_data *x_state_data;

    /* audio data */
    int x_chunk_current;
    float *x_chunk_buf;
    float *x_chunk[2][2];
    int x_chunk_used[2]; // marks if chunk is used or not
    int x_chunk_size;
    int x_chunk_pos;

    /* global state */
    int x_initialized;
    int x_frame;
    int x_frame_thread;
    int x_process_in_thread;


    /* audio info */
    int x_audio_tracks;   // ==0 means audio not available
    int x_audio_channels;
    long x_audio_samplerate;
    long x_audio_length;

    /* video info */
    int x_video_tracks;   // ==0 means video not available
    float x_video_framerate;
    long x_video_length;
    unsigned int x_video_width;
    unsigned int x_video_height;


} t_pdp_qt;


static void pdp_qt_bang(t_pdp_qt *x);

static void pdp_qt_close(t_pdp_qt *x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();

    /* disable clock */
    clock_unset(x->x_clock);
    pdp_procqueue_finish(q, x->x_queue_id);

    if (x->x_initialized){
	/* close file */
	quicktime_close(x->x_qt);
	x->x_initialized = 0;


	/* free video data */
	if (x->x_video_tracks){
	    FREE(x->x_qt_frame);
            FREE(x->x_qt_rows);
	    x->x_video_tracks = 0;
	    //x->x_qt_rows = 0;
	    //x->x_qt_frame = 0;
	}

	/* free audio data */
	if (x->x_audio_tracks){
	    x->x_chunk_used[0] = 0;
	    x->x_chunk_used[1] = 0;
	    FREE(x->x_chunk_buf);
            FREE(x->x_qt_audiochans);
	    x->x_audio_tracks = 0;
	    //x->x_qt_audiochans = 0;
	    //x->x_chunk_buf = 0;
	    x->x_chunk[0][0] = 0;
	    x->x_chunk[0][1] = 0;
	    x->x_chunk[1][0] = 0;
	    x->x_chunk[1][1] = 0;
	}


    }


}

void  pdp_qt_create_pdp_packet(t_pdp_qt *x)
{
    t_pdp *header;
    t_image *image;


    /* round to next legal size */
    /* if size is illegal, image distortion will occur */
    u32 w  = pdp_imageproc_legalwidth(x->x_video_width);
    u32 h  = pdp_imageproc_legalheight(x->x_video_height);

    
    int nbpixels = w * h;
    int packet_size = (nbpixels + (nbpixels >> 1)) << 1;


    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = pdp_packet_new_image_YCrCb(w, h);
    header = pdp_packet_header(x->x_packet0);
    image = pdp_packet_image_info(x->x_packet0);

    if (!header){
	post("%s: ERROR: can't create new packet", x->x_name->s_name);
	return;
    }
	

    //header->info.image.encoding = (x->x_qt_cmodel == BC_RGB888) ? PDP_IMAGE_GREY : PDP_IMAGE_YV12;
    //image->encoding = PDP_IMAGE_YV12;
    //image->width = w;
    //image->height =  h;
}




static void pdp_qt_open(t_pdp_qt *x, t_symbol *name)
{
    unsigned int size;
    unsigned int i;
    unsigned int chunk_bytesize;

    post("%s: opening %s", x->x_name->s_name, name->s_name);


    /* close previous one */
    pdp_qt_close(x);


    /* check if qt file */
    if(0 == quicktime_check_sig(name->s_name)){
        pd_error(x,"%s: %s not a quicktime file", x->x_name->s_name, name->s_name);
	goto exit;
    }

    /* open */
    DEBUG_MSG(x->x_qt = quicktime_open(name->s_name, 1, 0);)
    if (!(x->x_qt)){
        pd_error(x,"%s: can't open %s", x->x_name->s_name, name->s_name);
	goto exit;
    }

    /* check video */
    x->x_video_tracks = 0;
    if (quicktime_has_video(x->x_qt)) {
	x->x_video_framerate = quicktime_frame_rate   (x->x_qt, 0);
	x->x_video_length    = quicktime_video_length (x->x_qt, 0);
	x->x_video_width     = quicktime_video_width  (x->x_qt, 0);
	x->x_video_height    = quicktime_video_height (x->x_qt, 0);
	post("%s: video stream found (%dx%d pixels, %0.00f fps, %d frames, %s codec)", 
	     x->x_name->s_name, x->x_video_width, x->x_video_height, x->x_video_framerate, 
	     x->x_video_length, quicktime_video_compressor(x->x_qt, 0));
	x->x_video_tracks = quicktime_video_tracks(x->x_qt);

    }
	

    /* check audior */
    x->x_audio_tracks = 0;
    if (quicktime_has_audio(x->x_qt)) {
	x->x_audio_tracks     = quicktime_audio_tracks   (x->x_qt);
	//x->x_audio_channels   = quicktime_track_channels (x->x_qt, 0);
	x->x_audio_channels   = lqt_total_channels       (x->x_qt);
	x->x_audio_samplerate = quicktime_sample_rate    (x->x_qt, 0);
	x->x_audio_length     = quicktime_audio_length   (x->x_qt, 0);
	x->x_chunk_size = (int)((float)x->x_audio_samplerate / x->x_video_framerate);
	post("%s: audio stream found (%d channels, %d Hz, %d samples, chunksize %d)", 
	     x->x_name->s_name, x->x_audio_channels, x->x_audio_samplerate, x->x_audio_length, x->x_chunk_size);
    }

    /* check if video codec is supported */
    if (x->x_video_tracks){
	if (!quicktime_supported_video(x->x_qt,0)) {
	    post("%s: WARNING: unsupported video codec in %s",x->x_name->s_name);
	    x->x_video_tracks = 0;
	}
    }

    /* check if audio codec is supported */
    if (x->x_audio_tracks){
	if (!quicktime_supported_audio(x->x_qt,0)) {
	    pd_error(x,"%s: unsupported audio codec in %s", x->x_name->s_name, name->s_name);
	    x->x_audio_tracks = 0;
	}
    }



    /* check which colormodel to use */
    if (x->x_video_tracks){

	if (quicktime_reads_cmodel(x->x_qt,BC_YUV420P,0)){
	    post("%s: using colormodel YUV420P", x->x_name->s_name);
	    x->x_qt_cmodel = BC_YUV420P;
	}
	else if (quicktime_reads_cmodel(x->x_qt,BC_YUV422,0)){
	    post("%s: using colormodel YUV422", x->x_name->s_name);
	    x->x_qt_cmodel = BC_YUV422;
	}
	else if (quicktime_reads_cmodel(x->x_qt,BC_RGB888,0)){
	    post("%s: using colormodel RGB888", x->x_name->s_name);
	    x->x_qt_cmodel = BC_RGB888;
	}
	else {
	    post("%s: WARNING: can't find a usable colour model for %", x->x_name->s_name);
	    x->x_video_tracks = 0;
	}

    }



    /* no video == errors */
    if (!x->x_video_tracks) {
        pd_error(x,"%s: no usable video stream found in %s", 
                 x->x_name->s_name, name->s_name);
	goto exit_close;
    }


    /* initialize video data structures */
    if (x->x_video_tracks){

	/* allocate enough space for all supported colormodels (24bpp)*/
	x->x_frame = 0;
	x->x_qt_frame = (unsigned char*)pdp_alloc(x->x_video_width * x->x_video_height * 3);
	x->x_qt_rows =  (unsigned char **)pdp_alloc(sizeof(unsigned char *) * x->x_video_height);
	size = x->x_video_width * x->x_video_height;

	switch(x->x_qt_cmodel){
	case BC_YUV420P:
	    /* planar with u&v 2x2 subsampled */
	    x->x_qt_rows[0] = &x->x_qt_frame[0];
	    x->x_qt_rows[2] = &x->x_qt_frame[size];
	    x->x_qt_rows[1] = &x->x_qt_frame[size + (size>>2)];
	    break;

	case BC_YUV422:
	    /* packed with u&v 2x subsampled (lines) */
	    /* later on we will convert this to planar */
	    for(i=0; i< x->x_video_height; i++) x->x_qt_rows[i] = &x->x_qt_frame[i * x->x_video_width * 2];
	    break;

	case BC_RGB888:
	    /* packed rgb */
	    /* later on we will convert this to planar */
	    for(i=0; i< x->x_video_height; i++) x->x_qt_rows[i] = &x->x_qt_frame[i * x->x_video_width * 3];
	    break;

	default:
	    post("%s: error on init: unkown colour model",x->x_name->s_name);
	    break;
	}
    
	DEBUG_MSG(quicktime_set_cmodel(x->x_qt, x->x_qt_cmodel);)
	outlet_float(x->x_outlet2, (float)quicktime_video_length(x->x_qt,0));
    
    }

    /* initialize audio data structures */
    if (x->x_audio_tracks){
	x->x_chunk_pos = 0;
	x->x_chunk_current = 0;

	chunk_bytesize = sizeof(float)*x->x_chunk_size;
	x->x_chunk_buf =  (float *)pdp_alloc(chunk_bytesize * 4);
	memset(x->x_chunk_buf, 0, chunk_bytesize * 4);
	x->x_chunk[0][0] = x->x_chunk_buf;
	x->x_chunk[0][1] = x->x_chunk_buf + x->x_chunk_size ;
	x->x_chunk[1][0] = x->x_chunk_buf + x->x_chunk_size * 2;
	x->x_chunk[1][1] = x->x_chunk_buf + x->x_chunk_size * 3;
	x->x_chunk_used[0] = 0;
	x->x_chunk_used[1] = 0;
	x->x_syncaudio = x->x_istilde; //sync on audio if this is a tilde object

	DEBUG_MSG(if (x->x_audio_channels == 0) exit(1);)
	x->x_qt_audiochans = (float **)pdp_alloc(x->x_audio_channels * sizeof(float **));
	memset(x->x_qt_audiochans, 0, x->x_audio_channels * sizeof(float **));
    }
    else {
	x->x_syncaudio = 0;
    }


    /* everything went well */
    x->x_initialized = 1;

    /* start playback if outplay is on */
    if(x->x_autoplay)  clock_delay(x->x_clock, 1000.0L / (double)x->x_video_framerate);

    /* brag about success */
    post("%s: %s opened", x->x_name->s_name, name->s_name);

    return;

    /* error exits */

 exit_close:
    DEBUG_MSG(quicktime_close(x->x_qt);)

 exit:
    x->x_initialized = 0;
    x->x_audio_tracks = 0;
    x->x_video_tracks = 0;
    return;
    
}


//static void pdp_qt_setposition(t_pdp_qt *x, int pos)
//{
//    x->x_frame = pos;
//    DEBUG_MSG(if(x->x_video_tracks) quicktime_set_video_position(x->x_qt, pos, 0);)
//    DEBUG_MSG(if(x->x_audio_tracks) quicktime_set_audio_position(x->x_qt, pos * x->x_chunk_size, 0);)
//    
//}


static void pdp_qt_bangaudio(t_pdp_qt *x)
{
    int lefterr=0;
    int righterr=0;
    int err=0;
    int sample = 0;
    int remaining = 0;
    int readamount = 0;



    if (!x->x_initialized){
	//post("pdp_qt: no qt file opened");
	return;
    }


    if (!x->x_audio_tracks){
	//post("pdp_qt: no audio stream present");
	return;
    }



    //DEBUG_MSG(sample = quicktime_audio_position(x->x_qt,0);)


    // if the active chunk is unused, clear it and mark it used
    if (!x->x_chunk_used[x->x_chunk_current]){
	//post("%s: clearing unused active chunk",x->x_name->s_name);



	//probably this is the !@#%&*(*)&!$() bug
	//memset(x->x_chunk[0][x->x_chunk_current], 0, sizeof(float)*2*x->x_chunk_size);
	//memset(x->x_chunk[1][x->x_chunk_current], 0, sizeof(float)*2*x->x_chunk_size);

	memset(x->x_chunk[0][x->x_chunk_current], 0, sizeof(float) * x->x_chunk_size);
	memset(x->x_chunk[1][x->x_chunk_current], 0, sizeof(float) * x->x_chunk_size);




	x->x_chunk_used[x->x_chunk_current] = 1;
    }

    // compute the remaining time
    DEBUG_MSG(remaining = (int ) ( quicktime_audio_length(x->x_qt, 0) - quicktime_audio_position(x->x_qt, 0) );)
    readamount = min(remaining, x->x_chunk_size);
    if (!readamount) return;


    // if the inactive chunk is unused, fill it with the current frame's audio data and mark it used
    if (!x->x_chunk_used[!x->x_chunk_current]){
	switch(x->x_audio_channels){
	case 1:
	    x->x_qt_audiochans[0] = x->x_chunk[0][!x->x_chunk_current];
	    x->x_qt_audiochans[1] = 0;
	    DEBUG_MSG(err = lqt_decode_audio(x->x_qt, NULL, x->x_qt_audiochans, readamount);)
            break;
	default:
	    x->x_qt_audiochans[0] = x->x_chunk[0][!x->x_chunk_current];
	    x->x_qt_audiochans[1] = x->x_chunk[1][!x->x_chunk_current];
	    DEBUG_MSG(err = lqt_decode_audio(x->x_qt, NULL, x->x_qt_audiochans, readamount);)
            break;
	}
	x->x_chunk_used[!x->x_chunk_current] = 1;
    }
    // if it is used, something went wrong with sync
    else{
	//post("%s: dropping audio chunk %d.",x->x_name->s_name, x->x_frame_thread);
    }


    if (err) post("%s: error decoding audio",x->x_name->s_name, x->x_frame_thread);

    // ensure audio pointer points to next frame's data
    //DEBUG_MSG(quicktime_set_audio_position(x->x_qt, sample + readamount, 0);)
    
}




static void pdp_qt_bangvideo(t_pdp_qt *x)
{
  unsigned int w, h, nbpixels, packet_size, i,j;
  unsigned int *source, *dest;
  unsigned int uoffset, voffset;
  short int* data;
  t_pdp* header;

  // check if we want greyscale output or not
  //int grey = (x->x_qt_cmodel == BC_RGB888);

  static short int gain[4] = {0x7fff, 0x7fff, 0x7fff, 0x7fff};

    if ((!x->x_initialized) || (!x->x_video_tracks)){
	//post("pdp_qt: no qt file opened");
	return;
    }

    w = x->x_video_width;
    h = x->x_video_height;
    nbpixels = x->x_video_width * x->x_video_height;

    // create a new packet
    pdp_qt_create_pdp_packet(x);

    header = pdp_packet_header(x->x_packet0);

    if (!header) {
	post("%s: ERROR: no packet available", x->x_name->s_name);
	return;
    }

    data = (short int *) pdp_packet_data(x->x_packet0);


    DEBUG_MSG(lqt_decode_video(x->x_qt, x->x_qt_rows, 0);)


    switch(x->x_qt_cmodel){
    case BC_YUV420P:
	pdp_llconv(x->x_qt_frame, RIF_YVU__P411_U8, data, RIF_YVU__P411_S16, x->x_video_width, x->x_video_height); 
	break;

    case BC_YUV422:
	pdp_llconv(x->x_qt_frame, RIF_YUYV_P____U8, data, RIF_YVU__P411_S16, x->x_video_width, x->x_video_height); 
	break;

    case BC_RGB888:
	pdp_llconv(x->x_qt_frame, RIF_RGB__P____U8, data, RIF_YVU__P411_S16, x->x_video_width, x->x_video_height); 
	break;

    default:
	post("%s: error on decode: unkown colour model",x->x_name->s_name);
	break;
    }



}

static void pdp_qt_sendpacket(t_pdp_qt *x)
{

    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet0);

    //if (x->x_packet0 != -1){
	//pdp_packet_mark_unused(x->x_packet0);
	//outlet_pdp(x->x_outlet0, x->x_packet0);
	//x->x_packet0 = -1;
    //}
}


static void pdp_qt_thread_bang(t_pdp_qt *x)
{
    // set audio position
    if(x->x_video_tracks) quicktime_set_video_position(x->x_qt, x->x_frame_thread, 0);

    // bang video
    pdp_qt_bangvideo(x);

    // if it's a tilde object, bang audio
    if (x->x_istilde && x->x_audio_tracks){
	quicktime_set_audio_position(x->x_qt, x->x_frame_thread * x->x_chunk_size, 0);
	pdp_qt_bangaudio(x);
    }

}


static void pdp_qt_bang(t_pdp_qt *x)
{
    int length, pos;

    t_pdp_procqueue *q = pdp_queue_get_queue();

    /* return if not initialized */
    if (!x->x_initialized) return;

    //length = quicktime_video_length(x->x_qt,0);
    //pos = quicktime_video_position(x->x_qt,0);
    length = x->x_video_length;
    pos = x->x_frame;


    /* check bounds */
    if (x->x_loop){
	pos = x->x_frame % length;
	if (pos < 0) pos += length;
    }
    else{
	if (pos < 0) pos = 0;
	if (pos >= length) pos = length - 1;
    }

    /* store next frame for access in thread */
    x->x_frame_thread = pos;


    // if autoplay is on and we do not have audio synchro
    // set clock to play next frame
    if (x->x_autoplay && !x->x_syncaudio) clock_delay(x->x_clock, 1000.0L / (double)x->x_video_framerate);


    // make sure prev decode is finished don't drop frames in this one
    pdp_procqueue_finish(q, x->x_queue_id);
    x->x_queue_id = -1;

    /* only decode new stuff if previous is done */
    if (-1 == x->x_queue_id){
	// send the current frame number to outlet
	outlet_float(x->x_outlet1, (float)pos);

	//pdp_qt_setposition(x, pos);

	// start process method
	if (x->x_process_in_thread) pdp_procqueue_add(q, x, pdp_qt_thread_bang, pdp_qt_sendpacket, &x->x_queue_id);
	else {
	    pdp_qt_thread_bang(x);
	    pdp_qt_sendpacket(x);
	}
    }
    // advance frame
    x->x_frame = pos + 1;


    // send the packet
    //pdp_qt_sendpacket(x);
}



//static void pdp_qt_getaudiochunk(t_pdp_qt *x, int channel)
//{
//    if (!x->x_audio_tracks) return;
//    quicktime_decode_audio(x->x_qt, NULL, x->x_chunk[channel][x->x_chunk_current], x->x_chunk_size<<1, channel);
//			  
//}

static void pdp_qt_loop(t_pdp_qt *x, t_floatarg loop)
{
    int loopi = (int)loop;
    x->x_loop = !(loopi == 0);
}

static void pdp_qt_autoplay(t_pdp_qt *x, t_floatarg play)
{
    int playi = (int)play;
    x->x_autoplay = !(playi == 0);


    // reset clock if autoplay is off
    if (!x->x_autoplay) clock_unset(x->x_clock);

    
}



static void pdp_qt_frame_cold(t_pdp_qt *x, t_floatarg frameindex)
{
    int frame = (int)frameindex;
    //int length;


    x->x_frame = frame;

    //if (!(x->x_initialized)) return;

    //length = quicktime_video_length(x->x_qt,0);

    //frame = (frame >= length) ? length-1 : frame;
    //frame = (frame < 0) ? 0 : frame;

    //pdp_qt_setposition(x, frame);
}

static void pdp_qt_frame(t_pdp_qt *x, t_floatarg frameindex)
{
    pdp_qt_frame_cold(x, frameindex);
    pdp_qt_bang(x);
}

static void pdp_qt_stop(t_pdp_qt *x)
{
    pdp_qt_autoplay(x, 0);
}

static void pdp_qt_continue(t_pdp_qt *x)
{
    pdp_qt_autoplay(x, 1);
    pdp_qt_bang(x);
}


static void pdp_qt_play(t_pdp_qt *x){
    pdp_qt_frame_cold(x, 0);
    pdp_qt_continue(x);
}




static void pdp_qt_importaudio(t_pdp_qt *x, t_symbol *array, t_floatarg channel)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    int c = (int)channel;
    t_garray *g;
    int vecsize;
    int sample;
    float *f;
    int i;

    /* if there's no audio, there's nothing to export */
    if (!x->x_audio_tracks) return;

    /* check audio channel */
    if ((c < 0) || (c >= x->x_audio_channels)) return;

    /* check if array exists */
    if (!(g = (t_garray *)pd_findbyclass(array, garray_class))){
	pd_error(x, "%s: no such table", array->s_name);
            return;
    }

    post("%s: importing audio channel %d into array %s", x->x_name->s_name, c, array->s_name);


    // make sure decode is finished
    pdp_procqueue_finish(q, x->x_queue_id);
    x->x_queue_id = -1;


    /* resize array */
    garray_resize(g, x->x_audio_length);

    /* for sanity's sake let's clear the save-in-patch flag here */
    garray_setsaveit(g, 0);
    garray_getfloatarray(g, &vecsize, &f);

    /* if the resize failed, garray_resize reported the error */
    if (vecsize != x->x_audio_length){
	pd_error(x, "array resize failed");
	return;
    }

    /* save pointer in file */
    DEBUG_MSG(sample = quicktime_audio_position(x->x_qt, 0);)
    DEBUG_MSG(quicktime_set_audio_position(x->x_qt, 0, 0);)

    /* transfer the audio file to the end of the array */
    DEBUG_MSG(quicktime_decode_audio(x->x_qt, NULL, f, vecsize, c);)
 
    /* restore pointer in file */
	DEBUG_MSG(quicktime_set_audio_position(x->x_qt, sample, 0);)


}



static t_int *pdp_qt_perform(t_int *w)
{
    t_pdp_qt *x    =  (t_pdp_qt *)w[1];
    t_float  *out0 =  (t_float *)w[2];
    t_float  *out1 =  (t_float *)w[3];
    t_int    n     =  (t_int)w[4];

    t_int xfer_samples;
    if (!x->x_initialized || !x->x_audio_tracks) goto zero;

    while(1){
	// check current chunk
	if (!x->x_chunk_used[x->x_chunk_current]) goto zero;


	// transfer from chunk to output
	xfer_samples = min(n, x->x_chunk_size - x->x_chunk_pos);

	//x->x_audio_channels = 1;

	if (x->x_audio_channels == 1){
	    memcpy(out0, x->x_chunk[0][x->x_chunk_current] + x->x_chunk_pos, sizeof(float)*xfer_samples);
	    memcpy(out1, x->x_chunk[0][x->x_chunk_current] + x->x_chunk_pos, sizeof(float)*xfer_samples);
	}
	else {
	    memcpy(out0, x->x_chunk[0][x->x_chunk_current] + x->x_chunk_pos, sizeof(float)*xfer_samples);
	    memcpy(out1, x->x_chunk[1][x->x_chunk_current] + x->x_chunk_pos, sizeof(float)*xfer_samples);
	}
	out0 += xfer_samples;
	out1 += xfer_samples;
	n -= xfer_samples;
	x->x_chunk_pos += xfer_samples;


	// check if chunk is finished, if so mark unused, swap buffers and set clock
	if (x->x_chunk_size ==  x->x_chunk_pos){
	    x->x_chunk_used[x->x_chunk_current] = 0;
	    x->x_chunk_pos = 0;
	    x->x_chunk_current ^= 1;
	    if (x->x_autoplay) clock_delay(x->x_clock, 0L);
	}

	// if chunk is not finished, the output buffer is full
	else{
	    goto exit;
	}
	    
    }

 
 zero:
    // fill the rest of the output with zeros
    memset(out0, 0, sizeof(float)*n); 
    memset(out1, 0, sizeof(float)*n); 

 exit:
    return(w+5);
}

static void pdp_qt_dsp(t_pdp_qt *x, t_signal **sp)
{
    dsp_add(pdp_qt_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void pdp_qt_process_in_thread(t_pdp_qt *x, t_float f)
{

    int t = (f != 0.0f);

    x->x_process_in_thread = t;

    post("pdp_qt: thread processing switched %d", t ? "on" : "off");

}

static void pdp_qt_tick(t_pdp_qt *x)
{

    // bang audio/video
    pdp_qt_bang(x);
}

static void pdp_qt_free(t_pdp_qt *x)
{
    clock_unset(x->x_clock);
    pdp_qt_close(x);
    clock_free(x->x_clock);
    //free (x->x_state_data);

}



t_class *pdp_qt_class;
t_class *pdp_qt_tilde_class;


void pdp_qt_init_common(t_pdp_qt *x)
{

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("frame_cold"));

    /* add common outlets */
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);

    /* init */
    x->x_gain = 1.0f;
    x->x_process_in_thread = 0;
    x->x_packet0 = -1;
    x->x_queue_id = -1;
    x->x_initialized = 0;
    x->x_audio_tracks = 0;
    x->x_video_tracks = 0;
    x->x_loop = 0;
    x->x_autoplay = 0;
    x->x_chunk[0][0] = 0;
    x->x_chunk[0][1] = 0;
    x->x_chunk[1][0] = 0;
    x->x_chunk[1][1] = 0;

    /* initialize clock object */
    x->x_clock = clock_new(x, (t_method)pdp_qt_tick);




}

void *pdp_qt_new(void)
{
    t_pdp_qt *x = (t_pdp_qt *)pd_new(pdp_qt_class);
    x->x_name = gensym("pdp_qt");
    x->x_istilde = 0;
    pdp_qt_init_common(x);
    return (void *)x;
}

void *pdp_qt_tilde_new(void)
{
    t_pdp_qt *x = (t_pdp_qt *)pd_new(pdp_qt_tilde_class);
    x->x_name = gensym("pdp_qt");
    x->x_istilde = 1;

    pdp_qt_init_common(x);

    /* add outlets to the right so pdp_qt~ can replace pdp_qt without breaking a patch */
    x->x_outleft  = outlet_new(&x->x_obj, &s_signal);
    x->x_outright = outlet_new(&x->x_obj, &s_signal);

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_qt_setup_common(t_class *class)
{
    class_addmethod(class, (t_method)pdp_qt_bang, gensym("bang"), A_NULL);
    class_addmethod(class, (t_method)pdp_qt_close, gensym("close"), A_NULL);
    class_addmethod(class, (t_method)pdp_qt_open, gensym("open"), A_SYMBOL, A_NULL);
    class_addmethod(class, (t_method)pdp_qt_autoplay, gensym("autoplay"), A_DEFFLOAT, A_NULL);
    class_addmethod(class, (t_method)pdp_qt_stop, gensym("stop"), A_NULL);
    class_addmethod(class, (t_method)pdp_qt_play, gensym("play"), A_NULL);
    class_addmethod(class, (t_method)pdp_qt_continue, gensym("cont"), A_NULL);
    class_addmethod(class, (t_method)pdp_qt_loop, gensym("loop"), A_DEFFLOAT, A_NULL);
    class_addfloat (class, (t_method)pdp_qt_frame);
    class_addmethod(class, (t_method)pdp_qt_frame_cold, gensym("frame_cold"), A_FLOAT, A_NULL);
    class_addmethod(class, (t_method)pdp_qt_process_in_thread, gensym("thread"), A_FLOAT, A_NULL);
    class_addmethod(class, (t_method)pdp_qt_importaudio, gensym("importaudio"), A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(class, (t_method)pdp_qt_importaudio, gensym("dump"), A_SYMBOL, A_DEFFLOAT, A_NULL);
}

void pdp_qt_setup(void)
{

    /* plain class */
    pdp_qt_class = class_new(gensym("pdp_qt"), (t_newmethod)pdp_qt_new,
    	(t_method)pdp_qt_free, sizeof(t_pdp_qt), 0, A_NULL);
    pdp_qt_setup_common(pdp_qt_class);
 

    /* tilde class */
    pdp_qt_tilde_class = class_new(gensym("pdp_qt~"), (t_newmethod)pdp_qt_tilde_new,
    	(t_method)pdp_qt_free, sizeof(t_pdp_qt), 0, A_NULL);
    pdp_qt_setup_common(pdp_qt_tilde_class);

    class_addmethod(pdp_qt_tilde_class, (t_method)pdp_qt_dsp, gensym("dsp"), 0);

#ifdef __APPLE__
    /* this is necessary for pdp_qt to find the embedded libquicktime plugins */
    char buf[FILENAME_MAX];
    char realpath_buf[FILENAME_MAX];
    strncpy(buf, sys_libdir->s_name, FILENAME_MAX - 20);
    strcat(buf, "/../lib/libquicktime1");
    if(realpath(buf, realpath_buf))
    {
        if(sys_verbose)
            post("[pdp_qt]: setting LIBQUICKTIME_PLUGIN_DIR to:\n   %s", realpath_buf);
        setenv("LIBQUICKTIME_PLUGIN_DIR", realpath_buf, 0); // 0 means don't overwrite existing value
    }
#endif
}

#ifdef __cplusplus
}
#endif


