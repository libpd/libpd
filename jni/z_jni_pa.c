/*
 * Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "portaudio.h"

#include "z_jni_shared.c"

#define NTICKS 4

static PaStream *pa_stream = NULL;

static int pa_callback(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags, void *userData) {
  pthread_mutex_lock(&mutex);
  libpd_process_float(NTICKS, inputBuffer, outputBuffer);
  pthread_mutex_unlock(&mutex);
  return 0;
}

JNIEXPORT jboolean JNICALL
Java_org_puredata_core_PdBase_implementsAudio(JNIEnv *env, jclass cls) {
  return 1;
}

JNIEXPORT jstring JNICALL
Java_org_puredata_core_PdBase_audioImplementation(JNIEnv *env, jclass cls) {
  return (*env)->NewStringUTF(env, "PortAudio");
}

JNIEXPORT jint JNICALL
Java_org_puredata_core_PdBase_suggestSampleRate(JNIEnv *env, jclass cls) {
  return -1;
}

JNIEXPORT jint JNICALL
Java_org_puredata_core_PdBase_suggestInputChannels(JNIEnv *env, jclass cls) {
  return -1;
}

JNIEXPORT jint JNICALL
Java_org_puredata_core_PdBase_suggestOutputChannels(JNIEnv *env, jclass cls) {
  return -1;
}

JNIEXPORT jint JNICALL
Java_org_puredata_core_PdBase_openAudio(JNIEnv *env, jclass cls, jint inChans,
                                        jint outChans, jint sRate, jobject x) {
  Java_org_puredata_core_PdBase_closeAudio(env, cls);
  pthread_mutex_lock(&mutex);
  jint err = libpd_init_audio(inChans, outChans, sRate);
  pthread_mutex_unlock(&mutex);
  if (err) return err;
  PaError pa_err = Pa_Initialize();
  if (pa_err != paNoError) return pa_err;
  pa_err = Pa_OpenDefaultStream(&pa_stream, inChans, outChans, paFloat32, sRate,
                                NTICKS * libpd_blocksize(), pa_callback, NULL);
  if (pa_err == paNoError) {
    return 0;
  } else {
    Pa_Terminate();
    return pa_err;
  }
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_closeAudio(JNIEnv *env,
                                                                jclass cls) {
  if (pa_stream) {
    Pa_StopStream(pa_stream);
    Pa_CloseStream(pa_stream);
    pa_stream = NULL;
    Pa_Terminate();
  }
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_startAudio(JNIEnv *env,
                                                                jclass cls) {
  return pa_stream ? Pa_StartStream(pa_stream) : -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_pauseAudio(JNIEnv *env,
                                                                jclass cls) {
  return pa_stream ? Pa_StopStream(pa_stream) : -1;
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_isRunning(JNIEnv *env,
                                                                   jclass cls) {
  return pa_stream && Pa_IsStreamActive(pa_stream);
}
