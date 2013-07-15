/*
 * Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "opensl_stream/opensl_stream.h"

#include <stdio.h>

#include "z_jni_shared.c"

static OPENSL_STREAM *streamPtr = NULL;

static void process_callback(void *context, int sRate, int bufFrames,
    int nIn, const short *inBuf, int nOut, short *outBuf) {
  pthread_mutex_lock(&mutex);
  libpd_process_short(bufFrames / libpd_blocksize(), inBuf, outBuf);
  pthread_mutex_unlock(&mutex);
}

JNIEXPORT jstring JNICALL Java_org_puredata_core_PdBase_audioImplementation
(JNIEnv *env , jclass cls) {
  return (*env)->NewStringUTF(env, "OpenSL");
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_implementsAudio
(JNIEnv *env, jclass cls) {
  return 1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_openAudio
(JNIEnv *env, jclass cls, jint inChans, jint outChans, jint sRate,
jobject options) {
  Java_org_puredata_core_PdBase_closeAudio(env, cls);
  pthread_mutex_lock(&mutex);
  jint err = libpd_init_audio(inChans, outChans, sRate);
  pthread_mutex_unlock(&mutex);
  if (err) return err;
  streamPtr = opensl_open(sRate, inChans, outChans, libpd_blocksize(),
      process_callback, NULL);
  return !streamPtr;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_closeAudio
(JNIEnv *env, jclass cls) {
  if (streamPtr) {
    opensl_close(streamPtr);
    streamPtr = NULL;
  }
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_startAudio
(JNIEnv *env, jclass cls) {
  return streamPtr ? opensl_start(streamPtr) : -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_pauseAudio
(JNIEnv *env, jclass cls) {
  if (streamPtr) {
    opensl_pause(streamPtr);
    return 0;
  } else {
    return -1;
  }
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_isRunning
(JNIEnv *env, jclass cls) {
  return streamPtr && opensl_is_running(streamPtr);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestSampleRate
(JNIEnv *env, jclass cls) {
  return -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestInputChannels
(JNIEnv *env, jclass cls) {
  return -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestOutputChannels
(JNIEnv *env, jclass cls) {
  return -1;
}
