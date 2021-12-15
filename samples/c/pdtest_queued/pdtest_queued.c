/*
  this tests queued messaging in multi instance context
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "z_libpd.h"
#include "util/z_queued.h"

#define NLOOPS       16
#define NINSTANCES   2

typedef struct _instance
{
    t_pdinstance*       i_pd;
    pthread_t           i_thd;
    char                i_terminated;
} t_instance;

//////////////////////////////////////////////////////////////////////////////////////////////

void pdprint1(const char *s) {
    printf("pd1 %s\n", s);
}

void pdprint2(const char *s) {
    printf("pd2 %s\n", s);
}

void pdfloat1(const char *recv, float x)
{
    printf("f1 %s %f\n", recv, x);
}

void pdfloat2(const char *recv, float x)
{
    printf("f2 %s %f\n", recv, x);
}

//////////////////////////////////////////////////////////////////////////////////////////////

static void* instance_perform(t_instance* inst)
{
    size_t i;
    float inbuf[64], outbuf[128];  // one input channel, two output channels
                                   // block size 64, one tick per buffer
    libpd_set_instance(inst->i_pd);
    for(i = 0; i < NLOOPS; ++i) {
        libpd_process_float(1, inbuf, outbuf);
        usleep(1333);
    }
    inst->i_terminated = 1;
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    size_t i;
    char still_running = 1;
    int srate = 48000;
    t_instance instances[NINSTANCES];
    char *filename = "test.pd", *dirname = ".";

    if (argc > 1) {
        filename = argv[1];
    }
    if (argc > 2) {
        dirname = argv[2];
    }

    libpd_init();
#ifndef PDINSTANCE
    assert("PDINSTANCE undefined");
#endif
#ifndef PDTHREADS
    assert("PDTHREADS undefined");
#endif

    /* init the instances */
    for(i = 0; i < NINSTANCES; ++i) {
        instances[i].i_pd = libpd_new_instance();
        libpd_set_instance(instances[i].i_pd);
        libpd_bind("TestChannel");
        libpd_init_audio(1, 2, srate);
        libpd_openfile(filename, dirname);
    }

    /* set the hooks */
    libpd_set_instance(instances[0].i_pd);
    libpd_set_concatenated_queued_printhook(pdprint1);
    libpd_set_queued_floathook(pdfloat1);

    libpd_set_instance(instances[1].i_pd);
    libpd_set_concatenated_queued_printhook(pdprint2);
    libpd_set_queued_floathook(pdfloat2);

    /* run the instances */
    for(i = 0; i < NINSTANCES; ++i) {
        instances[i].i_terminated = 0;
        assert(!pthread_create(
                    &instances[i].i_thd,
                    NULL,
                    (void *)instance_perform,
                    &instances[i]
                ) && "instance_perform thread creation error.");
    }

    /* receive the messages from the instances */
    while(still_running) {
        still_running = 0;
        for(i = 0; i < NINSTANCES; ++i) {
            libpd_set_instance(instances[i].i_pd);
            libpd_queued_receive_pd_messages();
            if (instances[i].i_terminated == 0) still_running = 1;
        }
    }

    /* join the threads free the instances */
    for(i = 0; i < NINSTANCES; ++i)
    {
        pthread_join(instances[i].i_thd, NULL);
        libpd_free_instance(instances[i].i_pd);
    }

    return 0;
}
