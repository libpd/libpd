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
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, "opensl_io", __VA_ARGS__)
#define LOGW(...) \
  __android_log_print(ANDROID_LOG_WARN, "opensl_io", __VA_ARGS__)

#define OUTPUT_BUFFERS 2
#define STARTUP_INTERVALS 8

struct _opensl_stream {
  SLObjectItf engineObject;
  SLEngineItf engineEngine;

  SLObjectItf outputMixObject;

  SLObjectItf playerObject;
  SLPlayItf playerPlay;
  SLAndroidSimpleBufferQueueItf playerBufferQueue;

  SLObjectItf recorderObject;
  SLRecordItf recorderRecord;
  SLAndroidSimpleBufferQueueItf recorderBufferQueue;

  void *context;
  opensl_process_t callback;

  int sampleRate;
  int inputChannels;
  int outputChannels;

  int callbackBufferFrames;
  int inputBufferFrames;
  int outputBufferFrames;

  double thresholdMillis;

  short *inputBuffer;
  short *outputBuffer;
  short *dummyBuffer;

  long inputIndex;
  long outputIndex;
  long readIndex;

  int isRunning;

  struct timespec inputTime;
  int inputIntervals;
  long previousInputIndex;
  int inputOffset;

  struct timespec outputTime;
  int outputIntervals;
  long previousOutputIndex;
  int outputOffset;
};

static void updateIntervals(
    struct timespec *previousTime, double thresholdMillis,
    int *intervals, int *offset, long *previousIndex, long index) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  if (previousTime->tv_sec + previousTime->tv_nsec > 0) {
    // If a significant amount of time has passed since the previous
    // invocation, we take that as evidence that we're at the beginning of a
    // new internal OpenSL buffer.
    double dt = (t.tv_sec - previousTime->tv_sec) * 1e3 +
                (t.tv_nsec - previousTime->tv_nsec) * 1e-6;
    if (dt > thresholdMillis) {
      if (*intervals > STARTUP_INTERVALS / 2) {
        int currentOffset = index - *previousIndex;
        if (currentOffset > *offset) {
          *offset = currentOffset;
        }
      }
      *previousIndex = index;
      __sync_bool_compare_and_swap(intervals, *intervals, *intervals + 1);
    }
  }
  previousTime->tv_sec = t.tv_sec;
  previousTime->tv_nsec = t.tv_nsec;
}

static void recorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;
  if (p->outputChannels) {
    if (p->inputIntervals < STARTUP_INTERVALS) {
      updateIntervals(&p->inputTime, p->thresholdMillis, &p->inputIntervals,
          &p->inputOffset, &p->previousInputIndex, p->inputIndex);
    }
  } else {
    p->callback(p->context, p->sampleRate, p->callbackBufferFrames,
        p->inputChannels, p->inputBuffer +
        (p->inputIndex % p->inputBufferFrames) * p->inputChannels,
        0, NULL);
  }
  __sync_bool_compare_and_swap(&p->inputIndex, p->inputIndex,
      p->inputIndex + p->callbackBufferFrames);
  (*bq)->Enqueue(bq, p->inputBuffer +
      (p->inputIndex % p->inputBufferFrames) * p->inputChannels,
      p->callbackBufferFrames * p->inputChannels * sizeof(short));
}

static void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;
  if (p->inputChannels) {
    if (p->outputIntervals < STARTUP_INTERVALS) {
      updateIntervals(&p->outputTime, p->thresholdMillis, &p->outputIntervals,
          &p->outputOffset, &p->previousOutputIndex, p->outputIndex);
    }
    if (p->readIndex < 0 &&
        p->outputIntervals == STARTUP_INTERVALS &&
        __sync_fetch_and_or(&p->inputIntervals, 0) == STARTUP_INTERVALS) {
      int offset = p->inputOffset + p->outputOffset +
          OUTPUT_BUFFERS * p->callbackBufferFrames;
      LOGI("Offsets: %d %d %d", p->inputOffset, p->outputOffset, offset);
      p->readIndex = __sync_fetch_and_or(&p->inputIndex, 0) - offset;
    }
  }
  short *currentOutputBuffer = p->outputBuffer +
      (p->outputIndex % p->outputBufferFrames) * p->outputChannels;
  if (p->readIndex >= 0) {  // Synthesize audio with input if available.
    int margin = __sync_fetch_and_or(&p->inputIndex, 0) - p->readIndex;
    if (margin <= p->callbackBufferFrames) {
      LOGI("Low safety margin: %d", margin);
    }
    p->callback(p->context, p->sampleRate, p->callbackBufferFrames,
        p->inputChannels, p->inputBuffer +
        (p->readIndex % p->inputBufferFrames) * p->inputChannels,
        p->outputChannels, currentOutputBuffer);
    p->readIndex += p->callbackBufferFrames;
  } else {  // Synthesize audio with empty input when input is not yet availabe.
    p->callback(p->context, p->sampleRate, p->callbackBufferFrames,
        p->inputChannels, p->dummyBuffer,
        p->outputChannels, currentOutputBuffer);
  }
  (*bq)->Enqueue(bq, currentOutputBuffer,
      p->callbackBufferFrames * p->outputChannels * sizeof(short));
  p->outputIndex += p->callbackBufferFrames;
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
      {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
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
      {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, OUTPUT_BUFFERS};
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

OPENSL_STREAM *opensl_open(
    int sampleRate, int inChans, int outChans, int callbackBufferFrames,
    opensl_process_t proc, void *context) {
  if (!proc) {
    return NULL;
  }
  if (inChans == 0 && outChans == 0) {
    return NULL;
  }

  SLuint32 srmillihz = convertSampleRate(sampleRate);
  if (srmillihz < 0) {
    return NULL;
  }

  OPENSL_STREAM *p = (OPENSL_STREAM *) calloc(1, sizeof(OPENSL_STREAM));
  if (!p) {
    return NULL;
  }

  p->callback = proc;
  p->context = context;
  p->isRunning = 0;

  p->inputChannels = inChans;
  p->outputChannels = outChans;
  p->sampleRate = sampleRate;

  p->thresholdMillis = 750.0 * callbackBufferFrames / sampleRate;

  p->inputBuffer = NULL;
  p->outputBuffer = NULL;
  p->dummyBuffer = NULL;

  p->callbackBufferFrames = callbackBufferFrames;
  p->inputBufferFrames =
      (sampleRate / callbackBufferFrames / 4) * callbackBufferFrames;
  p->outputBufferFrames = OUTPUT_BUFFERS * callbackBufferFrames;

  if (openSLCreateEngine(p) != SL_RESULT_SUCCESS) {
    opensl_close(p);
    return NULL;
  }

  if (inChans) {
    int inBufSize = p->inputBufferFrames * inChans;
    if (!(openSLRecOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->inputBuffer = (short *) calloc(inBufSize, sizeof(short))) &&
        (p->dummyBuffer = (short *) calloc(callbackBufferFrames * inChans,
             sizeof(short))))) {
      opensl_close(p);
      return NULL;
    }
  }

  if (outChans) {
    int outBufSize = p->outputBufferFrames * outChans;
    if (!(openSLPlayOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->outputBuffer = (short *) calloc(outBufSize, sizeof(short))))) {
      opensl_close(p);
      return NULL;
    }
  }

  LOGI("Created OPENSL_STREAM(%d, %d, %d, %d)",
       sampleRate, inChans, outChans, callbackBufferFrames);
  LOGI("numBuffers: %d", OUTPUT_BUFFERS);
  return p;
}

void opensl_close(OPENSL_STREAM *p) {
  opensl_pause(p);
  openSLDestroyEngine(p);
  free(p->inputBuffer);
  free(p->outputBuffer);
  free(p->dummyBuffer);
  free(p);
}

int opensl_is_running(OPENSL_STREAM *p) {
  return p->isRunning;
}

int opensl_start(OPENSL_STREAM *p) {
  if (p->isRunning) {
    return 0;  // Already running.
  }

  p->inputIndex = 0;
  p->outputIndex = 0;
  p->readIndex = -1;

  p->inputTime.tv_sec = 0;
  p->inputTime.tv_nsec = 0;
  p->inputIntervals = 0;
  p->previousInputIndex = 0;
  p->inputOffset = 0;

  p->outputTime.tv_sec = 0;
  p->outputTime.tv_nsec = 0;
  p->outputIntervals = 0;
  p->previousOutputIndex = 0;
  p->outputOffset = 0;

  if (p->playerPlay) {
    LOGI("Starting player queue.");
    int i;
    for (i = 0; i < OUTPUT_BUFFERS; ++i) {
      playerCallback(p->playerBufferQueue, p);
    }
    if ((*p->playerPlay)->SetPlayState(p->playerPlay,
           SL_PLAYSTATE_PLAYING) != SL_RESULT_SUCCESS) {
      opensl_pause(p);
      return -1;
    }
  }
  if (p->recorderRecord) {
    memset(p->inputBuffer, 0, sizeof(p->inputBuffer));
    LOGI("Starting recorder queue.");
    recorderCallback(p->recorderBufferQueue, p);
    if ((*p->recorderRecord)->SetRecordState(p->recorderRecord,
            SL_RECORDSTATE_RECORDING) != SL_RESULT_SUCCESS) {
      opensl_pause(p);
      return -1;
    }
  }

  p->isRunning = 1;
  return 0;
}

void opensl_pause(OPENSL_STREAM *p) {
  if (!p->isRunning) {
    return;
  }
  if (p->playerPlay) {
    (*p->playerPlay)->SetPlayState(p->playerPlay,
        SL_PLAYSTATE_STOPPED);
    (*p->playerBufferQueue)->Clear(p->playerBufferQueue);
  }
  if (p->recorderRecord) {
    (*p->recorderRecord)->SetRecordState(p->recorderRecord,
        SL_RECORDSTATE_STOPPED);
    (*p->recorderBufferQueue)->Clear(p->recorderBufferQueue);
  }
  p->isRunning = 0;
}
