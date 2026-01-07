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

static void option_set(const char* key, const char *value) {
    if (!strcmp(key, "inputDeviceId")) {
        int id = atoi(value);
        engine->setRecordingDeviceId(id < 0 ? oboe::kUnspecified : id);
    } else if (!strcmp(key, "outputDeviceId")) {
        int id = atoi(value);
        engine->setPlaybackDeviceId(id < 0 ? oboe::kUnspecified : id);
    } else if (!strcmp(key, "bufferSizeInFrames")) {
        int size = atoi(value);
        engine->setBufferSizeInFrames(size < 0 ? oboe::kUnspecified : size);
    }
}

JNIEXPORT jstring JNICALL Java_org_puredata_core_PdBase_audioRuntimeInfo
(JNIEnv *env, jclass cls, jstring key) {
    const char* keyChars = (char *) env->GetStringUTFChars(key, NULL);
    std::string keyString{keyChars};
    env->ReleaseStringUTFChars(key, keyChars);
    std::string valueString{"unknown"};
    if (engine != nullptr) {
        if (keyString == "latencyMillis") {
            valueString = std::to_string(engine->getLatencyMillis());
        } else if (keyString == "xRuns") {
            valueString = std::to_string(engine->getXRuns());
        } else if (keyString == "sampleRate") {
            int inChans, outChans, sRate = 0;
            engine->getAudioParams(inChans, outChans, sRate);
            valueString = std::to_string(sRate);
        }
    }
    return env->NewStringUTF(valueString.c_str());
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_openAudio
(JNIEnv *env, jclass cls, jint inChans, jint outChans, jint sRate,
jobject options) {
    Java_org_puredata_core_PdBase_closeAudio(env, cls);
    if (engine == nullptr) {
        engine = new OboeEngine();
    }
    if (engine != nullptr) {
        engine->setSampleRate(sRate < 0 ? oboe::kUnspecified : sRate);
        engine->setAudioApi(engine->isAAudioRecommended() ? oboe::AudioApi::AAudio : oboe::AudioApi::OpenSLES);
        engine->setChannelCounts(
            inChans < 0 ? oboe::kUnspecified : inChans,
            outChans < 0 ? oboe::kUnspecified : outChans
        );
        map_foreach(env, options, option_set);
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
    // init Pd audio parameters with the final hardware settings
    if (isRunning) {
        int inChans, outChans, sRate;
        engine->getAudioParams(inChans, outChans, sRate);
        libpd_init_audio(inChans, outChans, sRate);
    }
    return isRunning ? 0 : -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_pauseAudio
(JNIEnv *env, jclass cls) {
    if (engine == nullptr) {
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

} // extern "C"

