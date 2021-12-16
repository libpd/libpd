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

typedef struct _queued_instance
{
    t_pdinstance*   i_pd;
    pthread_t       i_thd;
    char            i_terminated;
    int             i_id;
} t_queued_instance;


///////////////////////////////////////////////////////////////////////////////

void pdprint(const char *s)
{
    int id = ((t_queued_instance*)libpd_get_instancedata())->i_id;
    printf("pd_%d %s\n", id, s);
}

void pdfloat(const char *recv, float x)
{
    int id = ((t_queued_instance*)libpd_get_instancedata())->i_id;
    printf("pd_%d %s %f\n", id, recv, x);
}

///////////////////////////////////////////////////////////////////////////////

static void* instance_perform(t_queued_instance* inst)
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

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    t_queued_instance instances[NINSTANCES];
    size_t i;
    char still_running = 1;
    int srate = 48000;
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
        /* create pd instance: */
        instances[i].i_pd = libpd_new_instance();
        /* set the new instance as the current one: */
        libpd_set_instance(instances[i].i_pd); 
        /* set id=1 for the first instance, id=2 for the second one: */
        instances[i].i_id = i + 1;
        /* allow to find the "queued_instance" from the pd instance: */
        libpd_set_instancedata(&instances[i]);
        /* set the hooks: */
        libpd_set_concatenated_queued_printhook(pdprint);
        libpd_set_queued_floathook(pdfloat);
        /* receive "TestChannel" messages from Pd: */
        libpd_bind("TestChannel");

        libpd_init_audio(1, 2, srate);
        libpd_openfile(filename, dirname);
    }

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
