/*
 * Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "opensl_io.h"

#include "z_jni_shared.c"

static OPENSL_STREAM *streamPtr = NULL;
static int isRunning = 0;
static int nticks;

static void process_callback(void *context, int sRate, int bufFrames,
    int nIn, const short *inBuf, int nOut, short *outBuf) {
  pthread_mutex_lock(&mutex);
  libpd_process_short(*((int *)context), inBuf, outBuf);
  pthread_mutex_unlock(&mutex);
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_implementsAudio
(JNIEnv *env, jclass cls) {
  return 1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_openAudio
(JNIEnv *env, jclass cls, jint inChans, jint outChans, jint sRate) {
  Java_org_puredata_core_PdBase_closeAudio(env, cls);
  pthread_mutex_lock(&mutex);
  jint err = libpd_init_audio(inChans, outChans, sRate);
  pthread_mutex_unlock(&mutex);
  if (err) return err;
  int blkSize = libpd_blocksize();
  int bufSize = opensl_suggest_buffer_size(sRate, inChans, outChans);
  if (bufSize % blkSize) {
    bufSize = (bufSize / blkSize) * blkSize + blkSize;
  }
  nticks = bufSize / blkSize;
  streamPtr = opensl_open(sRate, inChans, outChans,
          bufSize, process_callback, &nticks);
  return !streamPtr;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_closeAudio
(JNIEnv *env, jclass cls) {
  if (streamPtr) {
    opensl_close(streamPtr);
    streamPtr = NULL;
    isRunning = 0;
  }
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_startAudio
(JNIEnv *env, jclass cls) {
  if (streamPtr) {
    isRunning = 1;
    return opensl_start(streamPtr);
  } else {
    return -1;
  }
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_pauseAudio
(JNIEnv *env, jclass cls) {
  if (streamPtr) {
    isRunning = 0;
    return opensl_pause(streamPtr);
  } else {
    return -1;
  }
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_isRunning
(JNIEnv *env, jclass cls) {
  return isRunning;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestSampleRate
(JNIEnv *env, jclass cls) {
  return opensl_suggest_sample_rate();
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestInputChannels
(JNIEnv *env, jclass cls) {
  return opensl_suggest_input_channels();
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestOutputChannels
(JNIEnv *env, jclass cls) {
  return opensl_suggest_output_channels();
}

