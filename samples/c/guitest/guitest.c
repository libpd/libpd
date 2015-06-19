#include <stdio.h>
#include "z_libpd.h"

void pdprint(const char *s)
{
    printf("%s", s);
}

void pdnoteon(int ch, int pitch, int vel)
{
    printf("noteon: %d %d %d\n", ch, pitch, vel);
}

float inbuf[64], outbuf[128];  // one input channel, two output channels
                               // block size 64, one tick per buffer

static void runawhile(int num_ticks)
{
    int ticksleft = num_ticks*1000;
    while (ticksleft > 0)
    {
        // fill inbuf here
        libpd_process_float(1, inbuf, outbuf);
        // use outbuf here
        sys_pollgui();
        usleep(1451);   /* 1 tick is about 1.45 msec */
        ticksleft -= 1;
    }
}

extern int sys_verbose;

int main(int argc, char **argv) {
    if (argc < 3) {
    fprintf(stderr, "usage: %s file folder\n", argv[0]);
    return -1;
    }

    // init pd
    int srate = 44100, foo;
    libpd_set_printhook((t_libpd_printhook)pdprint);
    libpd_set_noteonhook((t_libpd_noteonhook)pdnoteon);
    libpd_init();
    libpd_init_audio(1, 2, srate);

    // compute audio    [; pd dsp 1(
    libpd_start_message(1); // one entry in list
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");

    // open patch       [; pd open file folder(
    void *file = libpd_openfile(argv[1], argv[2]);

    // now run pd
    for (foo = 0; foo < 2; foo++)  /* note: doesn't yet work the second time */
    {
        printf("running nogui for 1000 ticks...\n");

        runawhile(1);

        printf("starting gui..\n");
        if (libpd_start_gui("../../../pure-data/"))
            printf("gui startup failed\n");

        printf("running for 2000 more ticks...\n");
        runawhile(2);

        libpd_stop_gui();
    }

    printf("Closing and exiting\n");
    libpd_closefile(file);

    return 0;
}
