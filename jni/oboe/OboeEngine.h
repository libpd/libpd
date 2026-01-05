/*
 * Copyright (c) 2025 Antoine Rousseau
 * (derived from 'LiveEffect' oboe sample)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#ifndef OBOE_ENGINE_H
#define OBOE_ENGINE_H

#include <jni.h>
#include <oboe/Oboe.h>
#include <string>
#include <thread>
#include <atomic>

#include "PdStreams.h"

class OboeEngine : public oboe::AudioStreamCallback {
public:
    OboeEngine();

    void setRecordingDeviceId(int32_t deviceId);
    void setPlaybackDeviceId(int32_t deviceId);
    bool setAudioApi(oboe::AudioApi);
    bool isAAudioRecommended(void);
    void setChannelCounts(int numInputs, int numOutputs);
    void getAudioParams(int &numInputs, int &numOutputs, int &sampleRate);
    void setBufferSizeInFrames(int frames);
    void setSampleRate(int srate);
    int getLatencyMillis();
    int getXRuns();

    /* optional user audio callback */
    void setAudioCallback(void (*callback)(void*), void *userData = nullptr);

    /**
     * @param isOn
     * @return true if it succeeds
     */
    bool setEffectOn(bool isOn);

    /*
     * oboe::AudioStreamDataCallback interface implementation
     */
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream,
                                          void *audioData, int32_t numFrames) override;

    /*
     * oboe::AudioStreamErrorCallback interface implementation
     */
    void onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) override;
    void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

private:
    bool mIsEffectOn = false;
    int32_t mRecordingDeviceId = oboe::kUnspecified;
    int32_t mPlaybackDeviceId = oboe::kUnspecified;
    const oboe::AudioFormat mFormat = oboe::AudioFormat::Float; // for easier processing
    oboe::AudioApi mAudioApi = oboe::AudioApi::AAudio;
    int32_t mSampleRate = oboe::kUnspecified;
    int32_t mRequestedSampleRate = oboe::kUnspecified;
    int32_t mBufferSizeInFrames = oboe::kUnspecified;
    int32_t mInputChannelCount = oboe::ChannelCount::Stereo;
    int32_t mOutputChannelCount = oboe::ChannelCount::Stereo;
    std::atomic<int> mCalculatedLatencyMillis = 0;
    std::atomic<int> mXRuns = 0;

    std::unique_ptr<PdStreamDuplex> mDuplexStream;
    std::unique_ptr<PdStreamSimplex> mSimplexStream;
    std::shared_ptr<oboe::AudioStream> mRecordingStream;
    std::shared_ptr<oboe::AudioStream> mPlayStream;

    void (*mAudioCallback)(void *data) = nullptr;
    void *mAudioCallbackUserData = nullptr;

    oboe::Result openStreams();

    void closeStreams();

    void closeStream(std::shared_ptr<oboe::AudioStream> &stream);

    oboe::AudioStreamBuilder *setupCommonStreamParameters(
        oboe::AudioStreamBuilder *builder);
    oboe::AudioStreamBuilder *setupRecordingStreamParameters(
        oboe::AudioStreamBuilder *builder, int32_t sampleRate);
    oboe::AudioStreamBuilder *setupPlaybackStreamParameters(
        oboe::AudioStreamBuilder *builder);
    void warnIfNotLowLatency(std::shared_ptr<oboe::AudioStream> &stream);
};

#endif  // OBOE_ENGINE_H
