/*
 * Copyright (c) 2025 Antoine Rousseau
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once

#include "z_libpd.h"

extern "C" {
    pthread_mutex_t *jni_oboe_get_libpd_mutex();
}

class PdStreamDuplex : public oboe::FullDuplexStream {
private:
    static const int MAX_NFRAMES = 8192;
    static const int MAX_IN_CHANS = 8;
    static const int MAX_OUT_CHANS = 8;
    static const int PD_BLOCKSIZE = 64;
    pthread_mutex_t &mutex = *jni_oboe_get_libpd_mutex();

    class InputBuffer {
    private:
        static const int BUFSIZE = MAX_NFRAMES * MAX_IN_CHANS;
        float ringbuffer[BUFSIZE];
        float blockbuffer[PD_BLOCKSIZE * MAX_IN_CHANS];
        int queue = 0;
        int tail = 0;
    public:
        bool is_empty() {
            return queue == tail;
        }
        bool is_full() {
            return ((tail + 1) % BUFSIZE) == queue;
        }
        void push(const float *samples, int nsamples) {
            for(int c = 0; c < nsamples; c++) {
                if(is_full()) break;
                ringbuffer[tail++] = samples[c];
                if(tail == BUFSIZE)
                    tail = 0;
            }
        }
        float *pop_block(int nchannels) {
            int nsamples = PD_BLOCKSIZE * nchannels;
            for(int i = 0 ; i < nsamples; i++) {
                float s = 0.0;
                if(!is_empty()) {
                    s = ringbuffer[queue++];
                    if(queue == BUFSIZE)
                        queue = 0;
                }
                blockbuffer[i] = s;
            }
            return blockbuffer;
        }
    } inputBuffer;

    class OutputBuffer {
    private:
        float buffer[PD_BLOCKSIZE * MAX_OUT_CHANS];
        int len = 0;
        int index = 0;
    public:
        bool is_empty() {
            return index == len;
        }
        float *init(int nsamples) {
            len = nsamples;
            index = 0;
            return buffer;
        }
        float pop() {
            return buffer[index++];
        }
    } outputBuffer;

public:
    virtual oboe::DataCallbackResult onBothStreamsReady(
            const void *inputData,
            int   numInputFrames,
            void *outputData,
            int   numOutputFrames) {

        // This code assumes the data format for both streams is Float.
        const float *inputFloats = static_cast<const float *>(inputData);
        float *outputFloats = static_cast<float *>(outputData);

        int32_t inChannels = getInputStream()->getChannelCount();
        int32_t outChannels = getOutputStream()->getChannelCount();
        int32_t numInputSamples = numInputFrames * inChannels;
        int32_t numOutputSamples = numOutputFrames * outChannels;

        // Store input samples in inputBuffer ring buffer
        if(numInputSamples < MAX_NFRAMES) // ignore too big buffers
            inputBuffer.push(inputFloats, numInputSamples);

        int processedSamples = 0;
        while(processedSamples < numOutputSamples) {
            // If outputBuffer is empty, get a new one from Pd in exchange for a block from inputBuffer.
            // When inputBuffer doesn't have enough data, it fills the block with zeroes.
            if(outputBuffer.is_empty()) {
                pthread_mutex_lock(&mutex);
                libpd_process_float(1, 
                    inputBuffer.pop_block(inChannels),
                    outputBuffer.init(PD_BLOCKSIZE * outChannels)
                );
                pthread_mutex_unlock(&mutex);
            }
            // Send next outputBuffer sample to audio output
            outputFloats[processedSamples++] = outputBuffer.pop();
        }

        return oboe::DataCallbackResult::Continue;
    }
};

class PdStreamSimplex {
private:
    static const int MAX_NFRAMES = 8192;
    static const int MAX_OUT_CHANS = 8;
    static const int PD_BLOCKSIZE = 64;
    pthread_mutex_t &mutex = *jni_oboe_get_libpd_mutex();

    class OutputBuffer {
    private:
        float buffer[PD_BLOCKSIZE * MAX_OUT_CHANS];
        int len = 0;
        int index = 0;
    public:
        bool is_empty() {
            return index == len;
        }
        float *init(int nsamples) {
            len = nsamples;
            index = 0;
            return buffer;
        }
        float pop() {
            return buffer[index++];
        }
    } outputBuffer;

public:
    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream *oboeStream,
            void *outputData,
            int   numOutputFrames) {

        float *outputFloats = static_cast<float *>(outputData);

        int32_t outChannels = oboeStream->getChannelCount();
        int32_t numOutputSamples = numOutputFrames * outChannels;
        int processedSamples = 0;
        while(processedSamples < numOutputSamples) {
            // If outputBuffer is empty, get a new one from Pd.
            if(outputBuffer.is_empty()) {
                float dummy;
                pthread_mutex_lock(&mutex);
                libpd_process_float(1, 
                    &dummy,
                    outputBuffer.init(PD_BLOCKSIZE * outChannels)
                );
                pthread_mutex_unlock(&mutex);
            }
            // Send next outputBuffer sample to audio output
            outputFloats[processedSamples++] = outputBuffer.pop();
        }

        return oboe::DataCallbackResult::Continue;
    }
};

