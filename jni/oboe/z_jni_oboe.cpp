/*
 * Copyright (c) 2025 Antoine Rousseau
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "logging_macros.h"
#include "z_jni_oboe.h"

static OboeEngine *engine = nullptr;
static bool isRunning = false;

pthread_mutex_t &mutex = *jni_oboe_get_libpd_mutex();

OboeEngine *jni_oboe_get_engine() {
    return engine;
}

extern "C" {

JNIEXPORT jstring JNICALL Java_org_puredata_core_PdBase_audioImplementation
(JNIEnv *env , jclass cls) {
    return env->NewStringUTF("Oboe");
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_implementsAudio
(JNIEnv *env, jclass cls) {
    return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_closeAudio
(JNIEnv *env, jclass cls) {
    if (engine) {
        engine->setEffectOn(false);
        isRunning = false;
        delete engine;
        engine = nullptr;
    }
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_openAudio
(JNIEnv *env, jclass cls, jint inChans, jint outChans, jint sRate,
jobject options) {
    Java_org_puredata_core_PdBase_closeAudio(env, cls);
    pthread_mutex_lock(&mutex);
    // Pd audio parameters will be updated later, in startAudio
    jint err = libpd_init_audio(2, 2, 48000);
    pthread_mutex_unlock(&mutex);
    if (err) return err;
    if (engine == nullptr) {
        engine = new OboeEngine();
    }
    if (engine != nullptr) {
        LOGE("requested sample rate %d", sRate);
        engine->setSampleRate(sRate < 0 ? oboe::kUnspecified : sRate);
        engine->setAudioApi(engine->isAAudioRecommended() ? oboe::AudioApi::AAudio : oboe::AudioApi::OpenSLES);
        engine->setChannelCounts(
            inChans < 0 ? oboe::kUnspecified : inChans,
            outChans < 0 ? oboe::kUnspecified : outChans
        );
    }
    return (engine == nullptr) ? -1 : 0;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_startAudio
(JNIEnv *env, jclass cls) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling startAudio "
            "method");
        return -1;
    }
    isRunning = engine->setEffectOn(true);
    // update Pd audio parameters for the actual devices
    if (isRunning) {
        int inChans, outChans, sRate;
        engine->getAudioParams(inChans, outChans, sRate);
        libpd_init_audio(inChans, outChans, sRate);
        LOGE("final sample rate %d", sRate);
    }
    return isRunning ? 0 : -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_pauseAudio
(JNIEnv *env, jclass cls) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling pauseAudio "
            "method");
        return -1;
    }
    isRunning = false;
    engine->setEffectOn(false);
    return 0;
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_isRunning
(JNIEnv *env, jclass cls) {
  return engine && isRunning;
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


JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_setRecordingDeviceId
(JNIEnv *env, jclass, jint deviceId) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling setRecordingDeviceId "
            "method");
        return;
    }

    engine->setRecordingDeviceId(deviceId < 0 ? oboe::kUnspecified : deviceId);
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_setPlaybackDeviceId
(JNIEnv *env, jclass, jint deviceId) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling setPlaybackDeviceId "
            "method");
        return;
    }
    engine->setPlaybackDeviceId(deviceId < 0 ? oboe::kUnspecified : deviceId);
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_setBufferSizeInFrames
(JNIEnv *env, jclass, jint frames) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling setBufferSizeInFrames "
            "method");
        return;
    }
    engine->setBufferSizeInFrames(frames);
}

} // extern "C"

