
#ifdef __gnu_linux__

#include "ext13.h"
#include "m_pd.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <errno.h>
#include <string.h>

#ifndef SOUND_MIXER_READ
# define SOUND_MIXER_READ(x) MIXER_READ(x)
#endif
#ifndef SOUND_MIXER_WRITE
# define SOUND_MIXER_WRITE(x) MIXER_WRITE(x)
#endif


/* -------------------------- ossmixer ------------------------------ */
static t_class *ossmixer_class;

typedef struct _ossmixer
{
    t_object x_obj;
    int channel;
    t_symbol* channel_name;
    t_symbol* device;
    t_outlet *x_out1;
} t_ossmixer;

static void *ossmixer_new(t_floatarg f)
{
    char devicename[FILENAME_MAX];
    int fd = -1;
    t_ossmixer *x = (t_ossmixer *)pd_new(ossmixer_class);
    outlet_new(&x->x_obj, &s_bang);
    x->x_out1 = outlet_new(&x->x_obj, &s_symbol);
    sprintf(devicename,"/dev/mixer%d",(int)f);
//    x->device = gensym(devicename);
    x->device = gensym ("/dev/mixer");
    fd = open(x->device->s_name, O_WRONLY);
    if (fd < 0){
       post ("ossmixer: could not open %s",x->device->s_name);
       x->device = gensym("/dev/mixer");
       close (fd);
    }else{
       post ("ossmixer: device set to %s",x->device->s_name);
    }
    close(fd);
    return (x);
}

static void ossmixer_set_device(t_ossmixer *x, t_floatarg f)
{
   char devicename[FILENAME_MAX];
   int fd = -1;
   sprintf(devicename,"/dev/mixer%d",(int)f);
   x->device = gensym(devicename);
   fd = open(x->device->s_name, O_WRONLY);
   if (fd < 0){
     post ("ossmixer: could not open %s",x->device->s_name);
     x->device = gensym("/dev/mixer");
     close (fd);
   }else{
     post ("ossmixer: device set to %s",devicename);
   }
}

static void ossmixer_bang(t_ossmixer *x)
{
  post ("ossmixer: what should a mixer do with a bang?");

}

static void ossmixer_get(t_ossmixer *x, t_symbol* s)
{
  int vol = -1;
  x->channel = -1;
  if (!strncmp(s->s_name,"main",4))
        x->channel = (int) SOUND_MIXER_VOLUME;
  if (!strncmp(s->s_name,"treble",6))
        x->channel = (int) SOUND_MIXER_TREBLE;
  if (!strncmp(s->s_name,"bass",4))
        x->channel = (int) SOUND_MIXER_BASS;
  if (!strncmp(s->s_name,"synth",5))
        x->channel = (int) SOUND_MIXER_SYNTH;
  if (!strncmp(s->s_name,"pcm",3))
        x->channel = (int) SOUND_MIXER_PCM;
  if (!strncmp(s->s_name,"speaker",7))
        x->channel = (int) SOUND_MIXER_SPEAKER;
  if (!strncmp(s->s_name,"line",4))
        x->channel = (int) SOUND_MIXER_LINE;
  if (!strncmp(s->s_name,"line1",5))
        x->channel = (int) SOUND_MIXER_LINE1;
  if (!strncmp(s->s_name,"line2",5))
        x->channel = (int) SOUND_MIXER_LINE2;
  if (!strncmp(s->s_name,"line3",5))
        x->channel = (int) SOUND_MIXER_LINE3;
  if (!strncmp(s->s_name,"mic",3))
        x->channel = (int) SOUND_MIXER_MIC;
  if (!strncmp(s->s_name,"cd",2))
        x->channel = (int) SOUND_MIXER_CD;
  if (!strncmp(s->s_name,"imix",4))
        x->channel = (int) SOUND_MIXER_IMIX;
  if (!strncmp(s->s_name,"altpcm",6))
         x->channel = (int) SOUND_MIXER_ALTPCM;
  if (!strncmp(s->s_name,"reclev",6))
         x->channel = (int) SOUND_MIXER_RECLEV;
  if (!strncmp(s->s_name,"reclevel",8))
         x->channel = (int) SOUND_MIXER_RECLEV;
  if (!strncmp(s->s_name,"igain",5))
         x->channel = (int) SOUND_MIXER_IGAIN;
  if (!strncmp(s->s_name,"ogain",5))
         x->channel = (int) SOUND_MIXER_OGAIN;

/*
braucht das wer?
         #define SOUND_MIXER_DIGITAL1    17  
         #define SOUND_MIXER_DIGITAL2    18 
         #define SOUND_MIXER_DIGITAL3    19  
         #define SOUND_MIXER_PHONEIN     20  
         #define SOUND_MIXER_PHONEOUT    21    
         #define SOUND_MIXER_VIDEO       22  
         #define SOUND_MIXER_RADIO       23 
         #define SOUND_MIXER_MONITOR     24 
*/         


  if (x->channel > -1){
    int fd = -1;
/*    fd = open("/dev/mixer0", O_RDONLY);*/
    fd = open(x->device->s_name, O_RDONLY);
    if (fd > 0){
      int vol = 50;
      if (ioctl(fd, SOUND_MIXER_READ( x->channel ), &vol)==-1){
        post("ossmixer: undefined mixer channel");
      }else{
        x->channel_name = s;
        vol = vol & 255;
        outlet_symbol (x->x_out1,s);
        outlet_float(x->x_obj.ob_outlet,(t_float)vol);
      }
      close(fd);
    }else{
      post ("ossmixer: could not open device %s",x->device->s_name);
    }
  }
}

static void ossmixer_set(t_ossmixer *x, t_symbol* s, t_float f)
{
  int vol = (int) f;
  if (vol < 0 ){
    post ("ossmixer: minimum volume: 0");
    vol = 0;
  }
  if (vol > 100 ){
    post ("ossmixer: maximum volume: 100");
    vol = 100;
  }
  x->channel = -1;
  
  if (!strncmp(s->s_name,"main",4))
          x->channel = (int) SOUND_MIXER_VOLUME;
  if (!strncmp(s->s_name,"treble",6))
          x->channel = (int) SOUND_MIXER_TREBLE;
  if (!strncmp(s->s_name,"bass",4))
          x->channel = (int) SOUND_MIXER_BASS;
  if (!strncmp(s->s_name,"synth",5))
          x->channel = (int) SOUND_MIXER_SYNTH;
  if (!strncmp(s->s_name,"pcm",3))
          x->channel = (int) SOUND_MIXER_PCM;
  if (!strncmp(s->s_name,"speaker",7))
          x->channel = (int) SOUND_MIXER_SPEAKER;
  if (!strncmp(s->s_name,"line",4))
          x->channel = (int) SOUND_MIXER_LINE;
  if (!strncmp(s->s_name,"line1",5))
          x->channel = (int) SOUND_MIXER_LINE1;
  if (!strncmp(s->s_name,"line2",5))
          x->channel = (int) SOUND_MIXER_LINE2;
  if (!strncmp(s->s_name,"line3",5))
          x->channel = (int) SOUND_MIXER_LINE3;
  if (!strncmp(s->s_name,"mic",3))
          x->channel = (int) SOUND_MIXER_MIC;
  if (!strncmp(s->s_name,"cd",2))
          x->channel = (int) SOUND_MIXER_CD;
  if (!strncmp(s->s_name,"imix",4))
          x->channel = (int) SOUND_MIXER_IMIX;
  if (!strncmp(s->s_name,"altpcm",6))
           x->channel = (int) SOUND_MIXER_ALTPCM;
  if (!strncmp(s->s_name,"reclev",6))
           x->channel = (int) SOUND_MIXER_RECLEV;
  if (!strncmp(s->s_name,"reclevel",8))
           x->channel = (int) SOUND_MIXER_RECLEV;
  if (!strncmp(s->s_name,"igain",5))
           x->channel = (int) SOUND_MIXER_IGAIN;
  if (!strncmp(s->s_name,"ogain",5))
           x->channel = (int) SOUND_MIXER_OGAIN;



  if (x->channel > -1){
    int fd = -1;
    fd = open(x->device->s_name, O_WRONLY);
    if (fd > 0){
      vol =  vol | (vol << 8);
      if (ioctl(fd, SOUND_MIXER_WRITE( x->channel ), &vol)==-1){
        post("ossmixer: undefined mixer channel");
      }else{
         x->channel_name = s;
         vol &= 255;
         outlet_symbol (x->x_out1,s);
         outlet_float(x->x_obj.ob_outlet,(t_float)vol);
      }
      close (fd);
    }else{
      post ("ossmixer: could not open device %s",x->device->s_name);
    }
  }
}
static void ossmixer_get_source(t_ossmixer *x)
{
    int fd = -1;
    int channel = -1;
    fd = open(x->device->s_name, O_WRONLY);
    if (fd > 0){
      if ( ioctl(fd, SOUND_MIXER_READ_RECSRC, &channel) ){
         post ("ossmixer: could not get recording source");
      }else{
         t_symbol* s_ch = gensym("no_source_found");
         if (channel & SOUND_MASK_VOLUME) s_ch = gensym("main");
         if (channel & SOUND_MASK_PCM) s_ch = gensym("pcm");
         if (channel & SOUND_MASK_MIC) s_ch = gensym("mic");
         if (channel & SOUND_MASK_CD) s_ch = gensym("cd");
         if (channel & SOUND_MASK_SYNTH) s_ch = gensym("synth");
         if (channel & SOUND_MASK_LINE) s_ch = gensym("line");
         if (channel & SOUND_MASK_LINE1) s_ch = gensym("line1");
         if (channel & SOUND_MASK_LINE2) s_ch = gensym("line2");
         if (channel & SOUND_MASK_LINE3) s_ch = gensym("line3");
         if (channel & SOUND_MASK_ALTPCM) s_ch = gensym("altpcm");

         outlet_symbol (x->x_out1,s_ch);
         outlet_symbol (x->x_obj.ob_outlet,gensym("source"));
      }
      close (fd);
    }else{
       post ("ossmixer: could not open mixer device");
    }

}

static void ossmixer_set_source(t_ossmixer *x, t_symbol* s)
{
  int channel = -1;
  if (!strncmp(s->s_name,"main",4))
            channel = (int) SOUND_MASK_VOLUME;
  if (!strncmp(s->s_name,"treble",6))
            channel = (int) SOUND_MASK_TREBLE;
  if (!strncmp(s->s_name,"bass",4))
            channel = (int) SOUND_MASK_BASS;
  if (!strncmp(s->s_name,"synth",5))
            channel = (int) SOUND_MASK_SYNTH;
  if (!strncmp(s->s_name,"pcm",3))
            channel = (int) SOUND_MASK_PCM;
  if (!strncmp(s->s_name,"speaker",7))
            channel = (int) SOUND_MASK_SPEAKER;
  if (!strncmp(s->s_name,"line",4))
            channel = (int) SOUND_MASK_LINE;
  if (!strncmp(s->s_name,"line1",5))
            channel = (int) SOUND_MASK_LINE1;
  if (!strncmp(s->s_name,"line2",5))
            channel = (int) SOUND_MASK_LINE2;
  if (!strncmp(s->s_name,"line3",5))
            channel = (int) SOUND_MASK_LINE3;
  if (!strncmp(s->s_name,"mic",3))
            channel = (int) SOUND_MASK_MIC;
  if (!strncmp(s->s_name,"cd",2))
            channel = (int) SOUND_MASK_CD;
  if (!strncmp(s->s_name,"imix",4))
            channel = (int) SOUND_MASK_IMIX;
  if (!strncmp(s->s_name,"altpcm",6))
            channel = (int) SOUND_MASK_ALTPCM;
  if (!strncmp(s->s_name,"reclev",6))
            channel = (int) SOUND_MASK_RECLEV;
  if (!strncmp(s->s_name,"reclevel",8))
            channel = (int) SOUND_MASK_RECLEV;
  if (!strncmp(s->s_name,"igain",5))
            channel = (int) SOUND_MASK_IGAIN;
  if (!strncmp(s->s_name,"ogain",5))
            channel = (int) SOUND_MASK_OGAIN;

  if(channel > -1){
    int fd = -1;
    fd = open(x->device->s_name, O_WRONLY);
    if (fd > 0){
      if ( ioctl(fd, SOUND_MIXER_WRITE_RECSRC, &channel) ){
         post ("ossmixer: could not set recordiing source");
      }else{
        ossmixer_get_source(x);
      }
      close (fd);
    }else{
      post ("ossmixer: could not open mixer device %s",x->device->s_name);
    }
  }else{
    post ("ossmixer: channel unknown");
  }
}

static void ossmixer_float(t_ossmixer *x, t_float f)
{
  ossmixer_set (x, x->channel_name, f);
}


void ossmixer_setup(void)
{
    ossmixer_class = class_new(gensym("ossmixer"), (t_newmethod)ossmixer_new, 0, sizeof(t_ossmixer), 0, A_DEFFLOAT, 0);
    class_addbang(ossmixer_class, ossmixer_bang);
    class_addfloat(ossmixer_class, ossmixer_float);
    class_addmethod(ossmixer_class, (t_method) ossmixer_get, gensym("get"), A_DEFSYM, 0);
    class_addmethod(ossmixer_class, (t_method) ossmixer_set, gensym("set"), A_DEFSYM, A_DEFFLOAT, 0);
    class_addmethod(ossmixer_class, (t_method) ossmixer_get_source, gensym("get_source"), A_DEFSYM, 0);
    class_addmethod(ossmixer_class, (t_method) ossmixer_set_source, gensym("set_source"), A_DEFSYM, 0);
    class_addmethod(ossmixer_class, (t_method) ossmixer_set_device, gensym("set_device"), A_DEFFLOAT, 0);
}


#endif
