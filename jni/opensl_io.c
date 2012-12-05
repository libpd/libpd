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

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define NBUFFERS 4

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

  int internalInputBufferFrames;
  int internalOutputBufferFrames;
  int externalBufferFrames;
  int totalInputBufferFrames;
  int totalOutputBufferFrames;
  short *inputBuffer;
  short *outputBuffer;

  int inputIndex;
  int outputIndex;

  opensl_process_t callback;
  void *context;

  pthread_t renderThread;
  int isRunning;
};

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
  SLresult result = slCreateEngine(&(p->engineObject), 0, NULL, 0, NULL, NULL);
  if (result != SL_RESULT_SUCCESS) return result;
  result = (*p->engineObject)->Realize(p->engineObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return result;
  result = (*p->engineObject)->GetInterface(
      p->engineObject, SL_IID_ENGINE, &(p->engineEngine));
  return result;
}

static void recorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;
  int index = (p->inputIndex + p->internalInputBufferFrames) %
      p->totalInputBufferFrames;
  __sync_bool_compare_and_swap(&(p->inputIndex), p->inputIndex, index);
  (*bq)->Enqueue(bq, p->inputBuffer + index * p->inputChannels,
      p->internalInputBufferFrames * p->inputChannels * sizeof(short));
}

static void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;
  int index = (p->outputIndex + p->internalOutputBufferFrames) %
      p->totalOutputBufferFrames;
  __sync_bool_compare_and_swap(&(p->outputIndex), p->outputIndex, index);
  (*bq)->Enqueue(bq, p->outputBuffer + index * p->outputChannels,
      p->internalOutputBufferFrames * p->outputChannels * sizeof(short));
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
      p->engineEngine, &(p->recorderObject), &audioSrc, &audioSnk, 1, id, req);
  if (SL_RESULT_SUCCESS != result) return result;

  result = (*p->recorderObject)->Realize(p->recorderObject, SL_BOOLEAN_FALSE);
  if (SL_RESULT_SUCCESS != result) return result;

  result = (*p->recorderObject)->GetInterface(p->recorderObject,
      SL_IID_RECORD, &(p->recorderRecord));
  if (SL_RESULT_SUCCESS != result) return result;

  result = (*p->recorderObject)->GetInterface(
      p->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
      &(p->recorderBufferQueue));
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
      p->engineEngine, &(p->outputMixObject), 1, mixIds, mixReq);
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
      p->engineEngine, &(p->playerObject), &audioSrc, &audioSnk,
      1, playIds, playRec);
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->playerObject)->Realize(p->playerObject, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->playerObject)->GetInterface(
      p->playerObject, SL_IID_PLAY, &(p->playerPlay));
  if (result != SL_RESULT_SUCCESS) return result;

  result = (*p->playerObject)->GetInterface(
      p->playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
      &(p->playerBufferQueue));
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
    int internalInputBufferFrames, int internalOutputBufferFrames,
    int externalBufferFrames, opensl_process_t proc, void *context) {
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

  p->externalBufferFrames = externalBufferFrames;
  if (inChans) {
    p->internalInputBufferFrames = internalInputBufferFrames;
    int lcm = lowest_common_multiple(internalInputBufferFrames,
        externalBufferFrames);
    p->totalInputBufferFrames = lcm;
    while (p->totalInputBufferFrames / internalInputBufferFrames < NBUFFERS ||
           p->totalInputBufferFrames / externalBufferFrames < NBUFFERS) {
      p->totalInputBufferFrames += lcm;
    }
    p->totalInputBufferFrames *= 2;
  }
  if (outChans) {
    p->internalOutputBufferFrames = internalOutputBufferFrames;
    int lcm = lowest_common_multiple(internalOutputBufferFrames,
        externalBufferFrames);
    p->totalOutputBufferFrames = lcm;
    while (p->totalOutputBufferFrames / internalOutputBufferFrames < NBUFFERS ||
           p->totalOutputBufferFrames / externalBufferFrames < NBUFFERS) {
      p->totalOutputBufferFrames += lcm;
    }
  }
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
    int inBufSize = p->totalInputBufferFrames * inChans;
    if (!(openSLRecOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->inputBuffer = (short *) calloc(inBufSize, sizeof(short))))) {
      opensl_close(p);
      return NULL;
    }
  }

  if (outChans) {
    int outBufSize = p->totalOutputBufferFrames * outChans;
    if (!(openSLPlayOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->outputBuffer = (short *) calloc(outBufSize, sizeof(short))))) {
      opensl_close(p);
      return NULL;
    }
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
  openSLDestroyEngine(p);
  if (p->inputBuffer) {
    free(p->inputBuffer);
  }
  if (p->outputBuffer) {
    free(p->outputBuffer);
  }
  free(p);
}

static void *render_loop(void *arg) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) arg;
  int renderInputIndex = p->totalInputBufferFrames / 2;
  int renderOutputIndex = 0;
  while (__sync_fetch_and_or(&(p->isRunning), 0)) {
    int invokeCallback = 1;
    if (p->inputChannels) {
      int inputIndex = __sync_fetch_and_or(&(p->inputIndex), 0);
      int d = (inputIndex - renderInputIndex + p->totalInputBufferFrames)
          % p->totalInputBufferFrames;
      if (d < p->externalBufferFrames) {
        invokeCallback = 0;
      }
    }
    if (p->outputChannels) {
      int outputIndex = __sync_fetch_and_or(&(p->outputIndex), 0);
      int d = (outputIndex - renderOutputIndex + p->totalOutputBufferFrames)
          % p->totalOutputBufferFrames;
      if (d < p->externalBufferFrames) {
        invokeCallback = 0;
      }
    }
    if (invokeCallback) {
      p->callback(p->context, p->sampleRate, p->externalBufferFrames,
          p->inputChannels,
          p->inputBuffer + renderInputIndex * p->inputChannels,
          p->outputChannels,
          p->outputBuffer + renderOutputIndex * p->outputChannels);
      if (p->inputChannels) {
        renderInputIndex = (renderInputIndex + p->externalBufferFrames)
            % p->totalInputBufferFrames;
      }
      if (p->outputChannels) {
        renderOutputIndex = (renderOutputIndex + p->externalBufferFrames)
            % p->totalOutputBufferFrames;
      }
    } else {
      usleep(50);  // Seems short, but larger values tend to glitch on Nexus S.
    }
  }
  return NULL;
}

int opensl_is_running(OPENSL_STREAM *p) {
  return __sync_fetch_and_or(&(p->isRunning), 0);
}

int opensl_start(OPENSL_STREAM *p) {
  if (opensl_is_running(p)) return 0;

  p->inputIndex = 0;
  p->outputIndex = 0;
  if (p->inputChannels) {
    memset(p->inputBuffer, 0, sizeof(p->inputBuffer));
  }
  if (p->outputChannels) {
    memset(p->outputBuffer, 0, sizeof(p->outputBuffer));
  }

  p->isRunning = 1;
  if (pthread_create(&(p->renderThread), NULL, render_loop, p)) {
    p->isRunning = 0;
    return -1;
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
  if (__sync_bool_compare_and_swap(&(p->isRunning), 1, 0)) {
    pthread_join(p->renderThread, NULL);
    if (p->recorderRecord) {
      (*p->recorderBufferQueue)->Clear(p->recorderBufferQueue);
      (*p->recorderRecord)->SetRecordState(p->recorderRecord,
          SL_RECORDSTATE_PAUSED);
    }
    if (p->playerPlay) {
      (*p->playerBufferQueue)->Clear(p->playerBufferQueue);
      (*p->playerPlay)->SetPlayState(p->playerPlay, SL_PLAYSTATE_PAUSED);
    }
  }
}
