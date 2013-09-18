#ifdef __gnu_linux__

#include "ext13.h"
#include "m_pd.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <errno.h>
#include <string.h>



/* -------------------------- cdplayer ------------------------------ */
static t_class *cdplayer_class;

typedef struct _cdplayer
{
    t_object x_obj;
    t_symbol* device;
} t_cdplayer;

static void *cdplayer_new()
{
    char* devicename;
    int fd;
    t_cdplayer *x = (t_cdplayer *)pd_new(cdplayer_class);

    outlet_new(&x->x_obj, &s_bang);
    sprintf(devicename,"/dev/cdrom");
    x->device = gensym(devicename);
/*
    fd = open(x->device->s_name, O_RDONLY);
    if (fd < 0){
       post ("cdplayer: could not open %s",x->device->s_name);
    } else close(fd);
*/
    return (x);
}

static void cdplayer_play(t_cdplayer *x, t_floatarg f)
{
    struct cdrom_tochdr header;
    struct cdrom_ti index;
    int cdrom;
    int childpid;
    int maxTrack;
    int t = (int)f;

    childpid = fork();
    if (childpid < 0) {
        error ("cdplayer: could not fork!");
    }
    if (!childpid){
       cdrom = open(x->device->s_name,O_RDONLY);    // Open device
       if (cdrom > -1){
          ioctl(cdrom,CDROMREADTOCHDR,(void *) &header); // Get start and end tracks
          maxTrack = header.cdth_trk1;
          if (t < 1){
            post ("track number must be 0 or higher");
          }
          if (t > maxTrack){
            post ("track number too high");
          }
           
          index.cdti_trk0=t;   // Set first track
          index.cdti_ind0=0;                  // Start of track
    
          index.cdti_trk1=t;   // Set final track
          index.cdti_ind1=99;                // End of track
      
          ioctl(cdrom,CDROMPLAYTRKIND,(void *) &index); // Play the tracks
          exit (0);
       }else{
         error ("cdplayer: could not open %s",x->device->s_name);
       }
       close (cdrom);
    }
}

static void cdplayer_pause(t_cdplayer *x)
{
    int cdrom;
    int childpid;
    childpid = fork();
    if (childpid < 0) {
       error ("cdplayer: could not fork!");
    }
    if (!childpid){
       cdrom = open(x->device->s_name,O_RDONLY);
       if (cdrom > -1){
         ioctl(cdrom,CDROMPAUSE,0);
         close (cdrom);
         exit (0);
       }else{
         error ("cdplayer: could not open %s",x->device->s_name);
       }
    }
}

static void cdplayer_resume(t_cdplayer *x)
{                         
    int cdrom;
    int childpid;
    childpid = fork();
    if (childpid < 0) {
       error ("cdplayer: could not fork!");
    }
    if (!childpid){
       cdrom = open(x->device->s_name,O_RDONLY);
       if (cdrom > -1){
         ioctl(cdrom,CDROMRESUME,0);
         close (cdrom);
         exit (0);
       }else{
         error ("cdplayer: could not open %s",x->device->s_name);
       }
    }
}

static void cdplayer_stop(t_cdplayer *x)
{
    int cdrom;
    int childpid;
    childpid = fork();
    if (childpid < 0) {
       error ("cdplayer: could not fork!");
    }
    if (!childpid){
       cdrom = open(x->device->s_name,O_RDONLY);
       if (cdrom > -1){
         ioctl(cdrom,CDROMSTOP,0);
         close (cdrom);
         exit (0);
       }else{
         error ("cdplayer: could not open %s",x->device->s_name);
       }
    }
}

static void cdplayer_eject(t_cdplayer *x)
{
    int cdrom;
    int childpid;
    childpid = fork();
    if (childpid < 0) {
       error ("cdplayer: could not fork!");
    }
    if (!childpid){
       cdrom = open(x->device->s_name,O_RDONLY);
       if (cdrom > -1){
         ioctl(cdrom,CDROMEJECT,0);
         close (cdrom);
         exit (0);
       }else{
         error ("cdplayer: could not open %s",x->device->s_name);
       }
    }
}

static void cdplayer_float(t_cdplayer *x, t_float f)
{
    cdplayer_play(x,f);
}


void cdplayer_setup(void)
{
    cdplayer_class = class_new(gensym("cdplayer"), (t_newmethod)cdplayer_new, 0, sizeof(t_cdplayer), 0, 0);
    class_addfloat(cdplayer_class, cdplayer_float);
    class_addmethod(cdplayer_class, (t_method) cdplayer_play, gensym("play"), A_DEFFLOAT,0);
    class_addmethod(cdplayer_class, (t_method) cdplayer_pause, gensym("pause"), 0);
    class_addmethod(cdplayer_class, (t_method) cdplayer_resume, gensym("resume"), 0);
    class_addmethod(cdplayer_class, (t_method) cdplayer_stop, gensym("stop"), 0);
    class_addmethod(cdplayer_class, (t_method) cdplayer_eject, gensym("eject"), 0);
}

#endif
