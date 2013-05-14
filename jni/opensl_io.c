/*
 * Copyright 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * Based on sample code by Victor Lazzarini, available at
 * http://audioprograming.wordpress.com/2012/03/03/android-audio-streaming-with-opensl-es-and-the-ndk/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "opensl_io.h"

#include <android/log.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, "opensl_io", __VA_ARGS__)
#define LOGW(...) \
  __android_log_print(ANDROID_LOG_WARN, "opensl_io", __VA_ARGS__)

#define NBUFFERS 3

struct _opensl_stream {
  int sampleRate;
  int inputChannels;
  int outputChannels;

  SLObjectItf engineObject;
  SLEngineItf engineEngine;

  SLObjectItf outputMixObject;

  SLObjectItf playerObject;
  SLPlayItf playerPlay;
  SLAndroidSimpleBufferQueueItf playerBufferQueue;

  SLObjectItf recorderObject;
  SLRecordItf recorderRecord;
  SLAndroidSimpleBufferQueueItf recorderBufferQueue;

  int internalBufferFrames;
  int externalBufferFrames;
  int totalBufferFrames;
  short *inputBuffer;
  short *outputBuffer;

  int inputIndex;
  int outputIndex;
  int isRunning;
  int isDone;

  opensl_process_t callback;
  void *context;

  pthread_t renderThread;
  sem_t semWake;
  sem_t semReady;
};

static void *render_loop(void *arg) {
  setpriority(PRIO_PROCESS, gettid(), -20);
  OPENSL_STREAM *p = (OPENSL_STREAM *) arg;
  int renderInputIndex = p->totalBufferFrames / 2;
  int renderOutputIndex = 0;
  int samplesToCompute = p->totalBufferFrames / 4;
  int samplesComputed = 0;
  while (!__sync_fetch_and_or(&p->isDone, 0)) {
    while (samplesToCompute >= p->externalBufferFrames) {
      p->callback(p->context, p->sampleRate, p->externalBufferFrames,
          p->inputChannels,
          p->inputBuffer + renderInputIndex * p->inputChannels,
          p->outputChannels,
          p->outputBuffer + renderOutputIndex * p->outputChannels);
      renderInputIndex = (renderInputIndex + p->externalBufferFrames)
          % p->totalBufferFrames;
      renderOutputIndex = (renderOutputIndex + p->externalBufferFrames)
          % p->totalBufferFrames;
      if (p->outputChannels) {
        for(samplesComputed += p->externalBufferFrames;
            samplesComputed >= p->internalBufferFrames;
            samplesComputed -= p->internalBufferFrames) {
          sem_post(&p->semReady);
        }
      }
      samplesToCompute -= p->externalBufferFrames;
    }
    sem_wait(&p->semWake);
    samplesToCompute += p->internalBufferFrames;
  }
  return NULL;
}

static void recorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;
  (*bq)->Enqueue(bq, p->inputBuffer + p->inputIndex * p->inputChannels,
      p->internalBufferFrames * p->inputChannels * sizeof(short));
  p->inputIndex = (p->inputIndex + p->internalBufferFrames) %
      p->totalBufferFrames;
  if (!p->outputChannels) {
    sem_post(&p->semWake);
  }
}

static void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;
  if (sem_trywait(&p->semReady)) {
    LOGW("Underrun!");
  }
  (*bq)->Enqueue(bq, p->outputBuffer + p->outputIndex * p->outputChannels,
      p->internalBufferFrames * p->outputChannels * sizeof(short));
  p->outputIndex = (p->outputIndex + p->internalBufferFrames) %
      p->totalBufferFrames;
  sem_post(&p->semWake);
}

static SLuint32 convertSampleRate(SLuint32 sr) {
  switch(sr) {
  case 8000:
    return SL_SAMPLINGRATE_8;
  case 11025:
    return SL_SAMPLINGRATE_11_025;
  case 12000:
    return SL_SAMPLINGRATE_12;
  case 16000:
    return SL_SAMPLINGRATE_16;
  case 22050:
    return SL_SAMPLINGRATE_22_05;
  case 24000:
    return SL_SAMPLINGRATE_24;
  case 32000:
    return SL_SAMPLINGRATE_32;
  case 44100:
    return SL_SAMPLINGRATE_44_1;
  case 48000:
    return SL_SAMPLINGRATE_48;
  case 64000:
    return SL_SAMPLINGRATE_64;
  case 88200:
    return SL_SAMPLINGRATE_88_2;
  case 96000:
    return SL_SAMPLINGRATE_96;
  case 192000:
    return SL_SAMPLINGRATE_192;
  }
  return -1;
}

static SLresult openSLCreateEngine(OPENSL_STREAM *p) {
  SLresult result = slCreateEngine(&p->engineObject, 0, NULL, 0, NULL, NULL);
  if (result != SL_RESULT_SUCCESS) return result;
  result = (*p->engineObject)->Realize(p->engineObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return result;
  result = (*p->engineObject)->GetInterface(
      p->engineObject, SL_IID_ENGINE, &p->engineEngine);
  return result;
}

static SLresult openSLRecOpen(OPENSL_STREAM *p, SLuint32 sr) {
  // enforce (temporary?) bounds on channel numbers)
  if (p->inputChannels < 0 || p->inputChannels > 2) {
    return SL_RESULT_PARAMETER_INVALID;
  }

  SLDataLocator_IODevice loc_dev =
      {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
       SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
  SLDataSource audioSrc = {&loc_dev, NULL};  // source: microphone

  int mics;
  if (p->inputChannels > 1) {
    // Yes, we're using speaker macros for mic config.  It's okay, really.
    mics = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
  } else {
    mics = SL_SPEAKER_FRONT_CENTER;
  }
  SLDataLocator_AndroidSimpleBufferQueue loc_bq =
      {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
  SLDataFormat_PCM format_pcm =
      {SL_DATAFORMAT_PCM, p->inputChannels, sr, SL_PCMSAMPLEFORMAT_FIXED_16,
       SL_PCMSAMPLEFORMAT_FIXED_16, mics, SL_BYTEORDER_LITTLEENDIAN};
  SLDataSink audioSnk = {&loc_bq, &format_pcm};  // sink: buffer queue

  // create audio recorder (requires the RECORD_AUDIO permission)
  const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
  const SLboolean req[1] = {SL_BOOLEAN_TRUE};
  SLresult result = (*p->engineEngine)->CreateAudioRecorder(
      p->engineEngine, &p->recorderObject, &audioSrc, &audioSnk, 1, id, req);
  if (SL_RESULT_SUCCESS != result) return result;

  result = (*p->recorderObject)->Realize(p->recorderObject, SL_BOOLEAN_FALSE);
  if (SL_RESULT_SUCCESS != result) return result;

  result = (*p->recorderObject)->GetInterface(p->recorderObject,
      SL_IID_RECORD, &p->recorderRecord);
  if (SL_RESULT_SUCCESS != result) return result;

  result = (*p->recorderObject)->GetInterface(
      p->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
      &p->recorderBufferQueue);
  if (SL_RESULT_SUCCESS != result) return result;

  result = (*p->recorderBufferQueue)->RegisterCallback(
      p->recorderBufferQueue, recorderCallback, p);
  return result;
}

static SLresult openSLPlayOpen(OPENSL_STREAM *p, SLuint32 sr) {
  // enforce (temporary?) bounds on channel numbers)
  if (p->outputChannels < 0 || p->outputChannels > 2) {
    return SL_RESULT_PARAMETER_INVALID;
  }

  int speakers;
  if (p->outputChannels > 1) {
    speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
  } else {
    speakers = SL_SPEAKER_FRONT_CENTER;
  }
  SLDataFormat_PCM format_pcm =
      {SL_DATAFORMAT_PCM, p->outputChannels, sr,
       SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
       speakers, SL_BYTEORDER_LITTLEENDIAN};
  SLDataLocator_AndroidSimpleBufferQueue loc_bufq =
      {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
  SLDataSource audioSrc = {&loc_bufq, &format_pcm};  // source: buffer queue

  const SLInterfaceID mixIds[] = {SL_IID_VOLUME};
  const SLboolean mixReq[] = {SL_BOOLEAN_FALSE};
  SLresult result = (*p->engineEngine)->CreateOutputMix(
      p->engineEngine, &p->outputMixObject, 1, mixIds, mixReq);
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->outputMixObject)->Realize(
      p->outputMixObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return result;

  SLDataLocator_OutputMix loc_outmix =
      {SL_DATALOCATOR_OUTPUTMIX, p->outputMixObject};
  SLDataSink audioSnk = {&loc_outmix, NULL};  // sink: mixer (volume control)

  // create audio player
  const SLInterfaceID playIds[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
  const SLboolean playRec[] = {SL_BOOLEAN_TRUE};
  result = (*p->engineEngine)->CreateAudioPlayer(
      p->engineEngine, &p->playerObject, &audioSrc, &audioSnk,
      1, playIds, playRec);
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->playerObject)->Realize(p->playerObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->playerObject)->GetInterface(
      p->playerObject, SL_IID_PLAY, &p->playerPlay);
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->playerObject)->GetInterface(
      p->playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
      &p->playerBufferQueue);
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->playerBufferQueue)->RegisterCallback(
      p->playerBufferQueue, playerCallback, p);
  return result;
}

static void openSLDestroyEngine(OPENSL_STREAM *p) {
  if (p->playerObject) {
    (*p->playerObject)->Destroy(p->playerObject);
  }
  if (p->recorderObject) {
    (*p->recorderObject)->Destroy(p->recorderObject);
  }
  if (p->outputMixObject) {
    (*p->outputMixObject)->Destroy(p->outputMixObject);
  }
  if (p->engineObject) {
    (*p->engineObject)->Destroy(p->engineObject);
  }
}

static int lowest_common_multiple(int a, int b) {
  int lcm = a;
  while (lcm % b) {
    lcm += a;
  }
  return lcm;
}

OPENSL_STREAM *opensl_open(
    int sRate, int inChans, int outChans,
    int internalBufferFrames, int externalBufferFrames,
    opensl_process_t proc, void *context) {
  if (!proc || (!inChans && !outChans)) {
    return NULL;
  }

  SLuint32 srmillihz = convertSampleRate(sRate);
  if (srmillihz < 0) {
    return NULL;
  }

  OPENSL_STREAM *p = (OPENSL_STREAM *) calloc(1, sizeof(OPENSL_STREAM));
  if (!p) {
    return NULL;
  }

  sem_init(&p->semWake, 0, 0);
  sem_init(&p->semReady, 0, 0);

  p->inputIndex = 0;
  p->outputIndex = 0;
  p->isRunning = 0;
  p->isDone = 0;

  p->inputBuffer = NULL;
  p->outputBuffer = NULL;
  p->internalBufferFrames = internalBufferFrames;
  p->externalBufferFrames = externalBufferFrames;
  p->totalBufferFrames = lowest_common_multiple(
      NBUFFERS * 4 * internalBufferFrames, NBUFFERS * 4 * externalBufferFrames);

  p->callback = proc;
  p->context = context;
  p->inputChannels = inChans;
  p->outputChannels = outChans;
  p->sampleRate = sRate;

  if (openSLCreateEngine(p) != SL_RESULT_SUCCESS) {
    opensl_close(p);
    return NULL;
  }

  if (inChans) {
    int inBufSize = p->totalBufferFrames * inChans;
    if (!(openSLRecOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->inputBuffer = (short *) calloc(inBufSize, sizeof(short))))) {
      opensl_close(p);
      return NULL;
    }
    memset(p->inputBuffer, 0, sizeof(p->inputBuffer));
  }

  if (outChans) {
    int outBufSize = p->totalBufferFrames * outChans;
    if (!(openSLPlayOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->outputBuffer = (short *) calloc(outBufSize, sizeof(short))))) {
      opensl_close(p);
      return NULL;
    }
    memset(p->outputBuffer, 0, sizeof(p->outputBuffer));
  }

  if (pthread_create(&p->renderThread, NULL, &render_loop, p)) {
    opensl_close(p);
    return NULL;
  }

  return p;
}

void opensl_close(OPENSL_STREAM *p) {
  opensl_pause(p);  // Overkill?
  if (p->recorderRecord) {
    (*p->recorderBufferQueue)->Clear(p->recorderBufferQueue);
    (*p->recorderRecord)->SetRecordState(p->recorderRecord,
        SL_RECORDSTATE_STOPPED);
  }
  if (p->playerPlay) {
    (*p->playerBufferQueue)->Clear(p->playerBufferQueue);
    (*p->playerPlay)->SetPlayState(p->playerPlay, SL_PLAYSTATE_STOPPED);
  }
  __sync_bool_compare_and_swap(&p->isDone, 0, 1);
  sem_post(&p->semWake);
  pthread_join(p->renderThread, NULL);
  openSLDestroyEngine(p);
  free(p->inputBuffer);
  free(p->outputBuffer);
  sem_destroy(&p->semReady);
  sem_destroy(&p->semWake);
  free(p);
}

int opensl_is_running(OPENSL_STREAM *p) {
  return __sync_fetch_and_or(&p->isRunning, 0);
}

int opensl_start(OPENSL_STREAM *p) {
  if (!__sync_bool_compare_and_swap(&p->isRunning, 0, 1)) {
    return 0;  // Already running.
  }

  // Wait for the rendering thread to spin up if we want output.
  if (p->outputChannels) {
    sem_wait(&p->semReady);
    sem_post(&p->semReady);
  }

  if (p->recorderRecord) {
    recorderCallback(p->recorderBufferQueue, p);
    if ((*p->recorderRecord)->SetRecordState(p->recorderRecord,
            SL_RECORDSTATE_RECORDING) != SL_RESULT_SUCCESS) {
      opensl_pause(p);
      return -1;
    }
  }
  if (p->playerPlay) {
    playerCallback(p->playerBufferQueue, p);
    if ((*p->playerPlay)->SetPlayState(p->playerPlay,
           SL_PLAYSTATE_PLAYING) != SL_RESULT_SUCCESS) {
      opensl_pause(p);
      return -1;
    }
  }

  return 0;
}

void opensl_pause(OPENSL_STREAM *p) {
  if (__sync_bool_compare_and_swap(&p->isRunning, 1, 0)) {
    if (p->recorderRecord) {
      (*p->recorderBufferQueue)->Clear(p->recorderBufferQueue);
      (*p->recorderRecord)->SetRecordState(p->recorderRecord,
          SL_RECORDSTATE_PAUSED);
    }
    if (p->playerPlay) {
      (*p->playerBufferQueue)->Clear(p->playerBufferQueue);
      (*p->playerPlay)->SetPlayState(p->playerPlay,
        SL_PLAYSTATE_PAUSED);
    }
  }
}
