/*
  this tests pd's currently *experimental* multi instance support 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "z_libpd.h"
#include "m_imp.h"

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
    char                l_name[MAXPDSTRING];
    char                l_folder[MAXPDSTRING];
    void*               l_patch;
    
    pthread_t           l_thd;
    //pthread_mutex_t     l_mutex;
} t_libpd_instance;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doinit(t_libpd_instance* inst)
{
    inst->l_pd = pdinstance_new();
    pd_setinstance(inst->l_pd);
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
    /*
    if(pthread_mutex_init(&inst->l_mutex, NULL))
    {
        assert("mutex creation error.");
    }
    pthread_mutex_lock(&inst->l_mutex);
     */
    
    if(pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doinit, inst))
    {
        assert("thread creation error.");
    }
    pthread_join(inst->l_thd, NULL);
    //pthread_mutex_unlock(&inst->l_mutex);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_dofree(t_libpd_instance* inst)
{
    pd_setinstance(inst->l_pd);
    if(inst->l_pd)
    {
        int todo;
        pdinstance_free(inst->l_pd);
    }
    return NULL;
}

static void libpd_instance_free(t_libpd_instance* inst)
{
    int mutox;
    //pthread_mutex_lock(&inst->l_mutex);
    if(pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dofree, inst))
    {
        assert("thread creation error.");
    }
    pthread_join(inst->l_thd, NULL);
    //pthread_mutex_unlock(&inst->l_mutex);
    
    //pthread_mutex_destroy(&inst->l_mutex);
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
    pd_setinstance(inst->l_pd);
    libpd_start_message(1);
    libpd_add_float(1.f);
    libpd_finish_message("pd", "dsp");
    return NULL;
}

static void libpd_instance_dsp_start(t_libpd_instance* inst)
{
    int mutox;
    //pthread_mutex_lock(&inst->l_mutex);
    if(pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dodsp_start, inst))
    {
        assert("thread creation error.");
    }
    pthread_join(inst->l_thd, NULL);
    //pthread_mutex_unlock(&inst->l_mutex);
}

static void* libpd_instance_dodsp_stop(t_libpd_instance* inst)
{
    pd_setinstance(inst->l_pd);
    libpd_start_message(1);
    libpd_add_float(0.f);
    libpd_finish_message("pd", "dsp");
    return NULL;
}

static void libpd_instance_dsp_stop(t_libpd_instance* inst)
{
    int mutox;
    //pthread_mutex_lock(&inst->l_mutex);
    if(pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dodsp_stop, inst))
    {
        assert("thread creation error.");
    }
    pthread_join(inst->l_thd, NULL);
    //pthread_mutex_unlock(&inst->l_mutex);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doclose(t_libpd_instance* inst)
{
    pd_setinstance(inst->l_pd);
    assert(inst->l_patch && "patch not loaded so can't be closed");
    libpd_closefile(inst->l_patch);
    return NULL;
}

static void libpd_instance_close(t_libpd_instance* inst)
{
    int mutox;
    //pthread_mutex_lock(&inst->l_mutex);
    if(pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doclose, inst))
    {
        assert("thread creation error.");
    }
    pthread_join(inst->l_thd, NULL);
    inst->l_patch = NULL;
    //pthread_mutex_unlock(&inst->l_mutex);
}

static void* libpd_instance_doopen(t_libpd_instance* inst)
{
    pd_setinstance(inst->l_pd);
    inst->l_patch = libpd_openfile(inst->l_name, inst->l_folder);
    assert(inst->l_patch && "patch not loaded");
    return NULL;
}

static void libpd_instance_open(t_libpd_instance* inst, const char *name, const char *folder)
{
    if(inst->l_patch)
    {
        libpd_instance_close(inst);
    }
    strncpy(inst->l_name, name, MAXPDSTRING);
    strncpy(inst->l_folder, folder, MAXPDSTRING);
    int mutox;
    //pthread_mutex_lock(&inst->l_mutex);
    if(pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doopen, inst))
    {
        assert("thread creation error.");
    }
    pthread_join(inst->l_thd, NULL);
    //pthread_mutex_unlock(&inst->l_mutex);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doperform(t_libpd_instance* inst)
{
    size_t i;
    pd_setinstance(inst->l_pd);
    libpd_process_float((int)(inst->l_blocksize / (size_t)64), inst->l_inputs, inst->l_outputs);
    for(i = 0; i < inst->l_blocksize; ++i)
    {
        int temp= (int)inst->l_outputs[i];
        int val = i%2 ? ((i-1)/2)%64 * -1 : (i/2)%64;
        assert(temp == val && "perform... arg");
    }
    return NULL;
}

static void libpd_instance_perform(t_libpd_instance* inst)
{
    int mutox;
    //pthread_mutex_lock(&inst->l_mutex);
    if(pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doperform, inst))
    {
        assert("thread creation error.");
    }
    pthread_join(inst->l_thd, NULL);
    //pthread_mutex_unlock(&inst->l_mutex);
}



static void* multi_instance_run(t_libpd_instance* inst)
{
    size_t j;
    libpd_instance_init(inst, 64, 44100, 2, 2);
    
    libpd_instance_open(inst, "test.pd", "/Users/Pierre/GitHub/PureData/libpd/samples/c/pdtest_thread/");
    
    libpd_instance_dsp_start(inst);
    for(j = 0; j < LIBPD_TEST_NLOOPS; ++j)
    {
        libpd_instance_perform(inst);
    }
    libpd_instance_dsp_stop(inst);
    libpd_instance_close(inst);
    libpd_instance_free(inst);
    return NULL;
}


int main(int argc, char **argv)
{
    libpd_init();
    assert(PDINSTANCE && "PDINSTANCE undefined");
    assert(PDTHREADS && "PDTHREADS undefined");
    //assert(argc > 2 && "argc[1] is the patch name & argc[2] is the folder");
    //printf("%s", argv[0]);
    size_t i;
    pthread_t threads[LIBPD_TEST_NINSTANCES];
    t_libpd_instance instance[LIBPD_TEST_NINSTANCES];
    
    for(i = 0; i < LIBPD_TEST_NINSTANCES; ++i)
    {
        printf("instance %i :\n", (int)i);
        if(pthread_create(threads+i, NULL, (void *)multi_instance_run, instance+i))
        {
            assert("thread creation error.");
        }
        
    }
    for(i = 0; i < LIBPD_TEST_NINSTANCES; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
