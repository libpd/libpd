/*
  this tests pd's currently *experimental* multi instance support 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "z_libpd.h"

#define LIBPD_TEST_NINSTANCES   4
#define LIBPD_TEST_NLOOPS       16

typedef struct l_instance
{
    t_pdinstance*       l_pd;
    size_t              l_blocksize;
    size_t              l_samplerate;
    size_t              l_ninputs;
    t_sample*           l_inputs;
    size_t              l_noutputs;
    t_sample*           l_outputs;
    char                l_file[MAXPDSTRING];
    char                l_folder[MAXPDSTRING];
    void*               l_patch;
    
    pthread_t           l_thd;
} t_libpd_instance;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doinit(t_libpd_instance* inst)
{
    inst->l_pd = libpd_new_instance();
    libpd_set_instance(inst->l_pd);
    assert(inst->l_pd && "pd instance can't be allocated.");
    libpd_init_audio((int)inst->l_ninputs, (int)inst->l_noutputs, (int)inst->l_samplerate);
    return NULL;
}

static void libpd_instance_init(t_libpd_instance* inst,
                                size_t blksize, size_t samplerate, size_t nins, size_t nouts)
{
    
    inst->l_blocksize   = blksize;
    inst->l_samplerate  = samplerate;
    inst->l_ninputs     = nins;
    inst->l_noutputs    = nouts;
    inst->l_patch       = NULL;
    
    assert(blksize && nins && nouts && "block size, number of inputs and number of outputs must be positives");
    inst->l_inputs      = (t_sample *)malloc(blksize * nins * sizeof(*inst->l_inputs));
    assert(inst->l_inputs && "inputs can't be allocated.");
    inst->l_outputs      = (t_sample *)malloc(blksize * nouts * sizeof(*inst->l_outputs));
    assert(inst->l_outputs && "outputs can't be allocated.");
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doinit, inst) &&
           "libpd_instance_init thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_dofree(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    if(inst->l_pd) {
        libpd_free_instance(inst->l_pd); }
    return NULL;
}

static void libpd_instance_free(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dofree, inst) &&
           "thread creation error.");
    pthread_join(inst->l_thd, NULL);
    if(inst->l_inputs)
    {
        free(inst->l_inputs);
        inst->l_inputs = NULL;
        inst->l_ninputs = 0;
    }
    if(inst->l_outputs)
    {
        free(inst->l_outputs);
        inst->l_outputs = NULL;
        inst->l_noutputs = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_dodsp_start(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    libpd_start_message(1);
    libpd_add_float(1.f);
    libpd_finish_message("pd", "dsp");
    return NULL;
}

static void libpd_instance_dsp_start(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dodsp_start, inst) &&
           "libpd_instance_dsp_start thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

static void* libpd_instance_dodsp_stop(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    libpd_start_message(1);
    libpd_add_float(0.f);
    libpd_finish_message("pd", "dsp");
    return NULL;
}

static void libpd_instance_dsp_stop(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dodsp_stop, inst) &&
           "libpd_instance_dsp_stop thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doclose(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    assert(inst->l_patch && "patch not loaded so can't be closed");
    libpd_closefile(inst->l_patch);
    return NULL;
}

static void libpd_instance_close(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doclose, inst) &&
           "libpd_instance_close thread creation error.");
    pthread_join(inst->l_thd, NULL);
    inst->l_patch = NULL;
}

static void* libpd_instance_doopen(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    assert((inst->l_patch = libpd_openfile(inst->l_file, inst->l_folder)) &&
           "patch can't be loaded");
    return NULL;
}

static void libpd_instance_open(t_libpd_instance* inst, const char *file, const char *folder)
{
    if(inst->l_patch) {
        libpd_instance_close(inst); }
    strncpy(inst->l_file, file, MAXPDSTRING);
    strncpy(inst->l_folder, folder, MAXPDSTRING);
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doopen, inst) &&
           "libpd_instance_open thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doperform(t_libpd_instance* inst)
{
    size_t i;
    libpd_set_instance(inst->l_pd);
    libpd_process_float((int)(inst->l_blocksize / (size_t)64), inst->l_inputs, inst->l_outputs);
    for(i = 0; i < inst->l_blocksize; ++i) {
        int result   = (int)inst->l_outputs[i];
        int expected = i%2 ? ((i-1)/2)%64 * -1 : (i/2)%64;
        assert(result == expected && "DSP results are wrong"); }
    return NULL;
}

static void libpd_instance_perform(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doperform, inst) &&
           "libpd_instance_perform thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static char* test_file;
static char* test_folder;
static void* multi_instance_run(t_libpd_instance* inst)
{
    size_t i;
    libpd_instance_init(inst, 256, 44100, 2, 2);
    libpd_instance_open(inst, test_file, test_folder);
    libpd_instance_dsp_start(inst);
    for(i = 0; i < LIBPD_TEST_NLOOPS; ++i) {
        libpd_instance_perform(inst); }
    libpd_instance_dsp_stop(inst);
    libpd_instance_close(inst);
    
    libpd_instance_free(inst);
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    size_t i;
    pthread_t threads[LIBPD_TEST_NINSTANCES];
    t_libpd_instance instance[LIBPD_TEST_NINSTANCES];
    
    if (argc < 3) {
        fprintf(stderr, "usage: %s file folder\n", argv[0]);
        return -1;
    }
    test_file = argv[1];
    test_folder = argv[2];

    libpd_init();
#ifndef PDINSTANCE
    assert("PDINSTANCE undefined");
#endif
#ifndef PDTHREADS
    assert("PDTHREADS undefined");
#endif
        
    for(i = 0; i < LIBPD_TEST_NINSTANCES; ++i)
    {
        assert(!pthread_create(threads+i, NULL, (void *)multi_instance_run, instance+i) &&
               "multi_instance_run thread creation error.");
    }
    for(i = 0; i < LIBPD_TEST_NINSTANCES; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
