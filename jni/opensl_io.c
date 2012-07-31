/*
 * Copyright 2012 Google Inc.
 * Author: Peter Brinkmann (brinkmann@google.com)
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

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/system_properties.h>

static int sdk_version() {
  static int sdk = 0;
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    char value[PROP_VALUE_MAX];
    int len = __system_property_get("ro.build.version.sdk", value);
    if (len > 0) {
      sdk = atoi(value);
    }
  }
  return sdk;
}

// This method reflects the state of low latency support; it needs
// to be updated whenever low latency latency becomes available on
// a new device.
static int supports_low_output_latency(int srate) {
  static int initialized = 0;
  static int is_jb_gn = 0;
  if (!initialized) {
    initialized = 1;
    char value[PROP_VALUE_MAX];
    int len = __system_property_get("ro.product.model", value);
    if (len > 0 && !strcmp("Galaxy Nexus", value)) {
      is_jb_gn = 1;
    }
  }
  return is_jb_gn && sdk_version() > 15 && srate == 44100;
}

int opensl_suggest_sample_rate() {
  return 44100;
}

int opensl_suggest_input_channels() {
  return 1;
}

int opensl_suggest_output_channels() {
  return 2;
}

struct _opensl_stream {
  int inputChannels;
  int outputChannels;
  int sampleRate;

  SLObjectItf engineObject;
  SLEngineItf engineEngine;

  SLObjectItf outputMixObject;

  SLObjectItf playerObject;
  SLPlayItf playerPlay;
  SLAndroidSimpleBufferQueueItf playerBufferQueue;

  SLObjectItf recorderObject;
  SLRecordItf recorderRecord;
  SLAndroidSimpleBufferQueueItf recorderBufferQueue;

  int bufferFrames;
  int nInBufs;
  int nOutBufs;
  short *inputBuffer[16];
  short *outputBuffer[16];

  int inputIndex;
  int outputIndex;
  int initialReadIndex;
  int readIndex;

  opensl_process_t callback;
  void *context;
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

void recorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;

  (*bq)->Enqueue(bq, p->inputBuffer[p->inputIndex],
      p->bufferFrames * p->inputChannels * sizeof(short));
  if (!p->outputChannels) {
    // Audio processing occurs in the output callback unless there is no output.
    p->callback(p->context, p->sampleRate, p->bufferFrames, p->inputChannels,
        p->inputBuffer[(p->inputIndex + p->nInBufs / 2) % p->nInBufs], 0, NULL);
  }
  p->inputIndex = (p->inputIndex + 1) % p->nInBufs;
}

void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
  OPENSL_STREAM *p = (OPENSL_STREAM *) context;

  p->callback(p->context, p->sampleRate, p->bufferFrames,
      p->inputChannels, p->inputBuffer[p->readIndex],
      p->outputChannels, p->outputBuffer[p->outputIndex]);
  (*bq)->Enqueue(bq, p->outputBuffer[p->outputIndex],
      p->bufferFrames * p->outputChannels * sizeof(short));
  p->outputIndex = (p->outputIndex + 1) % p->nOutBufs;
  p->readIndex = (p->readIndex + 1) % p->nInBufs;
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

OPENSL_STREAM *opensl_open(
    int sRate, int inChans, int outChans,
    opensl_process_t proc, void *context) {
  if (!proc || (!inChans && !outChans)) {
    return NULL;
  }

  SLuint32 srmillihz = convertSampleRate(sRate);
  if (srmillihz < 0) {
    return NULL;
  }

  OPENSL_STREAM *p = (OPENSL_STREAM *) calloc(sizeof(OPENSL_STREAM), 1);
  if (!p) {
    return NULL;
  }

  p->callback = proc;
  p->context = context;
  p->inputChannels = inChans;
  p->outputChannels = outChans;
  p->sampleRate = sRate;

  // Now we set up input and output buffers. The values below are chosen
  // conservatively and probably leave some room for improvement. More
  // experimentation is necessary. Feedback is welcome.
  if (supports_low_output_latency(sRate)) {
    p->bufferFrames = 384;
    p->nInBufs = 16;
    p->nOutBufs = 4;
    p->initialReadIndex = p->nInBufs - 2;
  } else {
    if (sdk_version() < 14) {
      p->bufferFrames = (sRate >= 44100 && inChans > 0) ? 2048 : 1024;
    } else {
      p->bufferFrames = (sRate >= 44100 && inChans > 0) ? 1024 : 512;
    }
    p->nInBufs = 16;
    p->nOutBufs = 4;
    p->initialReadIndex = p->nInBufs / 2;
  }

  if (openSLCreateEngine(p) != SL_RESULT_SUCCESS) {
    opensl_close(p);
    return NULL;
  }

  if (inChans) {
    int inBufSize = p->bufferFrames * inChans;
    if (openSLRecOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->inputBuffer[0] = (short *) calloc(p->nInBufs * inBufSize, sizeof(short)))) {
      int i;
      for (i = 1; i < p->nInBufs; i++) {
        p->inputBuffer[i] = p->inputBuffer[0] + i * inBufSize;
      }
    } else {
      opensl_close(p);
      return NULL;
    }
  }

  if (outChans) {
    int outBufSize = p->bufferFrames * outChans;
    if (openSLPlayOpen(p, srmillihz) == SL_RESULT_SUCCESS &&
        (p->outputBuffer[0] =
            (short *) calloc(p->nOutBufs * outBufSize, sizeof(short)))) {
      int i;
      for (i = 1; i < p->nOutBufs; i++) {
        p->outputBuffer[i] = p->outputBuffer[0] + i * outBufSize;
      }
    } else {
      opensl_close(p);
      return NULL;
    }
  }

  return p;
}

int opensl_buffer_size(OPENSL_STREAM *p) {
  return p->bufferFrames;
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
  if (p->inputBuffer[0]) {
    // Only need to free p->inputBuffer[0] since we allocated them all at once.
    free(p->inputBuffer[0]);
  }
  if (p->outputBuffer[0]) {
    // Only need to free p->outputBuffer[0] since we allocated them all at once.
    free(p->outputBuffer[0]);
  }
  free(p);
}

int opensl_start(OPENSL_STREAM *p) {
  opensl_pause(p);

  p->inputIndex = 0;
  p->outputIndex = 0;
  p->readIndex = p->initialReadIndex;
  memset(p->inputBuffer[0], 0,
      p->nInBufs * p->inputChannels * p->bufferFrames * sizeof(short));

  if (p->recorderRecord) {
    recorderCallback(p->recorderBufferQueue, p);
    if ((*p->recorderRecord)->SetRecordState(p->recorderRecord,
            SL_RECORDSTATE_RECORDING) != SL_RESULT_SUCCESS) {
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
