#include <stdio.h>
#include <unistd.h>
#include "z_libpd.h"

#define NINSTANCES 2
t_pdinstance *pdinstancevec[NINSTANCES];

void pdprint(const char *s)
{
    int instancenumber = -1, i;
    for (i = 0; i < NINSTANCES; i++)
        if (pdinstancevec[i] == pd_this)
            instancenumber = i; 
    printf("print(%d): %s", instancenumber, s);
}

void pdnoteon(int ch, int pitch, int vel)
{
    int instancenumber = -1, i;
    for (i = 0; i < NINSTANCES; i++)
        if (pdinstancevec[i] == pd_this)
            instancenumber = i; 
    printf("noteon (%d): %d %d %d\n", instancenumber, ch, pitch, vel);
}

float inbuf[64], outbuf[128];  // one input channel, two output channels
                               // block size 64, one tick per buffer

#include <sys/time.h>

static void waituntil(double f)
{
    static struct timeval starttime;
    struct timeval now;
    double fnow;
    if (starttime.tv_sec == 0 && starttime.tv_usec == 0)
        gettimeofday(&starttime, 0);
    while (1)
    {
        gettimeofday(&now, 0);
        fnow = now.tv_sec - starttime.tv_sec +
            1e-6 * (now.tv_usec - starttime.tv_usec);
        if (fnow >= f)
            break;
        usleep(1000);
    }
}

int main(int argc, char **argv)
{
    int i;
    void *file[NINSTANCES];
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s file [folder]\n", argv[0]);
        return (-1);
    }
    // init pd
    float logicaltime;

    libpd_set_printhook((t_libpd_printhook)pdprint);
    libpd_set_noteonhook((t_libpd_noteonhook)pdnoteon);
    libpd_init();
    libpd_init_audio(1, 2, 44100);
    for (i = 0; i < NINSTANCES; i++)
    {
        pdinstancevec[i] = pdinstance_new();
        pd_setinstance(pdinstancevec[i]);

            /* [; pd dsp 1( */
        libpd_start_message(1); // one entry in list
        libpd_add_float(1.0f);
        libpd_finish_message("pd", "dsp");

        file[i] = libpd_openfile(argv[1], (argc > 2 ? argv[2] : "."));

        if (libpd_start_gui("../../../pure-data/"))
            printf("gui startup failed\n");
    }

    for (logicaltime = 0; ; logicaltime += 0.001451)
    {
        for (i = 0; i < NINSTANCES; i++)
        {
            pd_setinstance(pdinstancevec[i]);
            libpd_process_float(1, inbuf, outbuf);
            libpd_poll_gui();
        }
        waituntil(logicaltime);
    }

    for (i = 0; i < NINSTANCES; i++)
    {
        pd_setinstance(pdinstancevec[i]);
        libpd_stop_gui();
        libpd_closefile(file[i]);
    }
    return (0);
}
