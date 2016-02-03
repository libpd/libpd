/*
 * Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "z_jni.h"
#include "z_jni_native_hooks.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>

#include "z_libpd.h"
#include "z_queued.h"
#include "z_print_util.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static JNIEnv *cached_env = NULL;

static jobject messageHandler = NULL;
static jmethodID printMethod = NULL;
static jmethodID bangMethod = NULL;
static jmethodID floatMethod = NULL;
static jmethodID symbolMethod = NULL;
static jmethodID listMethod = NULL;
static jmethodID anyMethod = NULL;
static jclass objClass = NULL;
static jclass floatClass = NULL;
static jmethodID floatInit = NULL;

static jobject midiHandler = NULL;
static jmethodID noteOnMethod = NULL;
static jmethodID controlChangeMethod = NULL;
static jmethodID programChangeMethod = NULL;
static jmethodID pitchBendMethod = NULL;
static jmethodID aftertouchMethod = NULL;
static jmethodID polyAftertouchMethod = NULL;
static jmethodID midiByteMethod = NULL;

#define LIBPD_CLASS_REF(c) (*env)->NewGlobalRef(env, (*env)->FindClass(env, c));

int libpd_sync_init_audio(
    int input_channels, int output_channels, int sample_rate) {
  pthread_mutex_lock(&mutex);
  int err = libpd_init_audio(input_channels, output_channels, sample_rate);
  pthread_mutex_unlock(&mutex);
  return err;
}

int libpd_sync_process_raw(const float *inBuf, float *outBuf) {
  pthread_mutex_lock(&mutex);
  int err = libpd_process_raw(inBuf, outBuf);
  pthread_mutex_unlock(&mutex);
  return err;
}

static jobjectArray makeJavaArray(JNIEnv *env, int argc, t_atom *argv) {
  jobjectArray jarray = (*env)->NewObjectArray(env, argc, objClass, NULL);
  int i;
  for (i = 0; i < argc; i++) {
    t_atom *a = &argv[i];
    jobject obj = NULL;
    if (libpd_is_float(a)) {
      obj = (*env)->NewObject(env, floatClass, floatInit, libpd_get_float(a));
    } else if (libpd_is_symbol(a)) {
      obj = (*env)->NewStringUTF(env, libpd_get_symbol(a));
    }
    (*env)->SetObjectArrayElement(env, jarray, i, obj);
    if (obj != NULL) {
      (*env)->DeleteLocalRef(env, obj);  // The reference in the array remains.
    }
  }
  return jarray;
}

static void java_printhook(const char *msg) {
  if (!messageHandler || !msg || !cached_env) return;
  jstring jmsg = (*cached_env)->NewStringUTF(cached_env, msg);
  (*cached_env)->CallVoidMethod(cached_env, messageHandler, printMethod, jmsg);
  (*cached_env)->DeleteLocalRef(cached_env, jmsg);
}

void java_sendBang(const char *source) {
  if (!messageHandler || !source || !cached_env) return;
  jstring jsource = (*cached_env)->NewStringUTF(cached_env, source);
  (*cached_env)->CallVoidMethod(cached_env, messageHandler, bangMethod, jsource);
  (*cached_env)->DeleteLocalRef(cached_env, jsource);
}

void java_sendFloat(const char *source, float x) {
  if (!messageHandler || !source || !cached_env) return;
  jstring jsource = (*cached_env)->NewStringUTF(cached_env, source);
  (*cached_env)->CallVoidMethod(cached_env, messageHandler, floatMethod, jsource, x);
  (*cached_env)->DeleteLocalRef(cached_env, jsource);
}

void java_sendSymbol(const char *source, const char *sym) {
  if (!messageHandler || !source || !sym || !cached_env) return;
  jstring jsource = (*cached_env)->NewStringUTF(cached_env, source);
  jstring jsym = (*cached_env)->NewStringUTF(cached_env, sym);
  (*cached_env)->CallVoidMethod(cached_env,
      messageHandler, symbolMethod, jsource, jsym);
  (*cached_env)->DeleteLocalRef(cached_env, jsym);
  (*cached_env)->DeleteLocalRef(cached_env, jsource);
}

void java_sendList(const char *source, int argc, t_atom *argv) {
  if (!messageHandler || !source || !cached_env) return;
  jstring jsource = (*cached_env)->NewStringUTF(cached_env, source);
  jobjectArray jarray = makeJavaArray(cached_env, argc, argv);
  (*cached_env)->CallVoidMethod(cached_env,
      messageHandler, listMethod, jsource, jarray);
  (*cached_env)->DeleteLocalRef(cached_env, jarray);
  (*cached_env)->DeleteLocalRef(cached_env, jsource);
}

void java_sendMessage(const char *source, const char *msg, int argc, t_atom *argv) {
  if (!messageHandler || !source || !msg || !cached_env) return;
  jstring jsource = (*cached_env)->NewStringUTF(cached_env, source);
  jstring jmsg = (*cached_env)->NewStringUTF(cached_env, msg);
  jobjectArray jarray = makeJavaArray(cached_env, argc, argv);
  (*cached_env)->CallVoidMethod(cached_env,
      messageHandler, anyMethod, jsource, jmsg, jarray);
  (*cached_env)->DeleteLocalRef(cached_env, jarray);
  (*cached_env)->DeleteLocalRef(cached_env, jmsg);
  (*cached_env)->DeleteLocalRef(cached_env, jsource);
}

void java_sendNoteOn(int channel, int pitch, int velocity) {
  if (!midiHandler || !cached_env) return;
  (*cached_env)->CallVoidMethod(cached_env, midiHandler,
            noteOnMethod, channel, pitch, velocity);
}

void java_sendControlChange(int channel, int controller, int value) {
  if (!midiHandler || !cached_env) return;
  (*cached_env)->CallVoidMethod(cached_env, midiHandler,
            controlChangeMethod, channel, controller, value);
}

void java_sendProgramChange(int channel, int value) {
  if (!midiHandler || !cached_env) return;
  (*cached_env)->CallVoidMethod(cached_env, midiHandler,
            programChangeMethod, channel, value);
}

void java_sendPitchBend(int channel, int value) {
  if (!midiHandler || !cached_env) return;
  (*cached_env)->CallVoidMethod(cached_env, midiHandler,
            pitchBendMethod, channel, value);
}

void java_sendAftertouch(int channel, int value) {
  if (!midiHandler || !cached_env) return;
  (*cached_env)->CallVoidMethod(cached_env, midiHandler,
            aftertouchMethod, channel, value);
}

void java_sendPolyAftertouch(int channel, int pitch, int value) {
  if (!midiHandler || !cached_env) return;
  (*cached_env)->CallVoidMethod(cached_env, midiHandler,
            polyAftertouchMethod, channel, pitch, value);
}

void java_sendMidiByte(int port, int value) {
  if (!midiHandler || !cached_env) return;
  (*cached_env)->CallVoidMethod(cached_env, midiHandler,
            midiByteMethod, port, value);
}

JNIEXPORT jint JNICALL JNI_OnLoad
(JavaVM *jvm, void *ignored) {
  return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_initialize
(JNIEnv *env, jclass cls) {
  libpd_queued_init();

  objClass = LIBPD_CLASS_REF("java/lang/Object");
  floatClass = LIBPD_CLASS_REF("java/lang/Float");
  floatInit = (*env)->GetMethodID(env, floatClass, "<init>", "(F)V");

  libpd_set_queued_printhook(libpd_print_concatenator);
  libpd_set_concatenated_printhook(java_printhook);

  libpd_set_queued_banghook(java_sendBang);
  libpd_set_queued_floathook(java_sendFloat);
  libpd_set_queued_symbolhook(java_sendSymbol);
  libpd_set_queued_listhook(java_sendList);
  libpd_set_queued_messagehook(java_sendMessage);

  libpd_set_queued_noteonhook(java_sendNoteOn);
  libpd_set_queued_controlchangehook(java_sendControlChange);
  libpd_set_queued_programchangehook(java_sendProgramChange);
  libpd_set_queued_pitchbendhook(java_sendPitchBend);
  libpd_set_queued_aftertouchhook(java_sendAftertouch);
  libpd_set_queued_polyaftertouchhook(java_sendPolyAftertouch);
  libpd_set_queued_midibytehook(java_sendMidiByte);
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_pollPdMessageQueue
(JNIEnv *env, jclass cls) {
  cached_env = env;
  libpd_queued_receive_pd_messages();
  cached_env = NULL;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_pollMidiQueueInternal
(JNIEnv *env, jclass cls) {
  cached_env = env;
  libpd_queued_receive_midi_messages();
  cached_env = NULL;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_clearSearchPath
(JNIEnv *env, jclass cls) {
  pthread_mutex_lock(&mutex);
  libpd_clear_search_path();
  pthread_mutex_unlock(&mutex);
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_addToSearchPath
(JNIEnv *env, jclass cls, jstring jpath) {
  if (!jpath) return;
  const char *cpath = (char *) (*env)->GetStringUTFChars(env, jpath, NULL);
  pthread_mutex_lock(&mutex);
  libpd_add_to_search_path(cpath);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jpath, cpath);
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_exists
(JNIEnv *env, jclass cls, jstring jsym) {
  if (!jsym) return 0;
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  pthread_mutex_lock(&mutex);
  jboolean flag = libpd_exists(csym);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jsym, csym);
  return flag;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendBang
(JNIEnv *env, jclass cls, jstring jrecv) {
  if (!jrecv) return -2;
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  pthread_mutex_lock(&mutex);
  int err = libpd_bang(crecv);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendFloat
(JNIEnv *env, jclass cls, jstring jrecv, jfloat x) {
  if (!jrecv) return -2;
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  pthread_mutex_lock(&mutex);
  int err = libpd_float(crecv, x);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendSymbol
(JNIEnv *env, jclass cls, jstring jrecv, jstring jsym) {
  if (!jrecv) return -2;
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  pthread_mutex_lock(&mutex);
  jint err = libpd_symbol(crecv, csym);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  (*env)->ReleaseStringUTFChars(env, jsym, csym);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_startMessage
(JNIEnv *env, jclass cls, jint length) {
  return libpd_start_message(length);
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_addFloat
(JNIEnv *env, jclass cls, jfloat x) {
  libpd_add_float(x);
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_addSymbol
(JNIEnv *env, jclass cls, jstring jsym) {
  if (!jsym) return;
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  libpd_add_symbol(csym);
  (*env)->ReleaseStringUTFChars(env, jsym, csym);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_finishList
(JNIEnv *env, jclass cls, jstring jrecv) {
  if (!jrecv) return -10;
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  pthread_mutex_lock(&mutex);
  jint err = libpd_finish_list(crecv);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_finishMessage
(JNIEnv *env, jclass cls, jstring jrecv, jstring jmsg) {
  if (!jrecv || !jmsg) return -10;
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  const char *cmsg = (char *) (*env)->GetStringUTFChars(env, jmsg, NULL);
  pthread_mutex_lock(&mutex);
  jint err = libpd_finish_message(crecv, cmsg);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  (*env)->ReleaseStringUTFChars(env, jmsg, cmsg);
  return err;
}

JNIEXPORT jlong JNICALL Java_org_puredata_core_PdBase_openFile
(JNIEnv *env, jclass cls, jstring jpatch, jstring jdir) {
  if (!jpatch || !jdir) return 0;
  const char *cpatch = (char *) (*env)->GetStringUTFChars(env, jpatch, NULL);
  const char *cdir = (char *) (*env)->GetStringUTFChars(env, jdir, NULL);
  pthread_mutex_lock(&mutex);
  jlong ptr = (jlong) libpd_openfile(cpatch, cdir);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jpatch, cpatch);
  (*env)->ReleaseStringUTFChars(env, jdir, cdir);
  return ptr;
  // very naughty, returning a pointer to Java
  // using long integer in case we're on a 64bit CPU
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_closeFile
(JNIEnv *env, jclass cls, jlong ptr) {
  pthread_mutex_lock(&mutex);
  libpd_closefile((void *)ptr);
  pthread_mutex_unlock(&mutex);
  // even naughtier, using a pointer from Java
  // using long integer in case we're on a 64bit CPU
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_getDollarZero
(JNIEnv *env, jclass cls, jlong ptr) {
  pthread_mutex_lock(&mutex);
  int dz = libpd_getdollarzero((void *)ptr);
  pthread_mutex_unlock(&mutex);
  return dz;
}

JNIEXPORT jlong JNICALL Java_org_puredata_core_PdBase_bindSymbol
(JNIEnv *env, jclass cls, jstring jsym) {
  if (!jsym) return 0;
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  pthread_mutex_lock(&mutex);
  jlong ptr = (jlong) libpd_bind(csym);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jsym, csym);
  return ptr;
  // very naughty, returning a pointer to Java
  // using long integer in case we're on a 64bit CPU
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_unbindSymbol
(JNIEnv *env, jclass cls, jlong ptr) {
  pthread_mutex_lock(&mutex);
  libpd_unbind((void *)ptr);
  pthread_mutex_unlock(&mutex);
  // even naughtier, using a pointer from Java
  // using long integer in case we're on a 64bit CPU
}

static void deleteHandlerRef(JNIEnv *env) {
  if (!messageHandler) return;
  (*env)->DeleteGlobalRef(env, messageHandler);
  messageHandler = NULL;
  printMethod = NULL;
  bangMethod = NULL;
  floatMethod = NULL;
  symbolMethod = NULL;
  listMethod = NULL;
  anyMethod = NULL;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_setReceiver
(JNIEnv *env, jclass cls, jobject handler) {
  deleteHandlerRef(env);
  if (!handler) return;
  messageHandler = (*env)->NewGlobalRef(env, handler);
  jclass class_handler = (*env)->GetObjectClass(env, messageHandler);
  printMethod = (*env)->GetMethodID(env, class_handler,
    "print", "(Ljava/lang/String;)V");
  bangMethod = (*env)->GetMethodID(env, class_handler,
    "receiveBang", "(Ljava/lang/String;)V");
  floatMethod = (*env)->GetMethodID(env, class_handler,
    "receiveFloat", "(Ljava/lang/String;F)V");
  symbolMethod = (*env)->GetMethodID(env, class_handler,
    "receiveSymbol", "(Ljava/lang/String;Ljava/lang/String;)V");
  listMethod = (*env)->GetMethodID(env, class_handler,
    "receiveList", "(Ljava/lang/String;[Ljava/lang/Object;)V");
  anyMethod = (*env)->GetMethodID(env, class_handler, "receiveMessage",
    "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)V");
  // no global ref necessary for method IDs
}

static void deleteMidiHandlerRef(JNIEnv *env) {
  if (!midiHandler) return;
  (*env)->DeleteGlobalRef(env, midiHandler);
  midiHandler = NULL;
  noteOnMethod = NULL;
  controlChangeMethod = NULL;
  programChangeMethod = NULL;
  pitchBendMethod = NULL;
  aftertouchMethod = NULL;
  polyAftertouchMethod = NULL;
  midiByteMethod = NULL;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_setMidiReceiverInternal
(JNIEnv *env, jclass cls, jobject handler) {
  deleteMidiHandlerRef(env);
  if (!handler) return;
  midiHandler = (*env)->NewGlobalRef(env, handler);
  jclass class_handler = (*env)->GetObjectClass(env, midiHandler);
  noteOnMethod = (*env)->GetMethodID(env, class_handler,
    "receiveNoteOn", "(III)V");
  controlChangeMethod = (*env)->GetMethodID(env, class_handler,
    "receiveControlChange", "(III)V");
  programChangeMethod = (*env)->GetMethodID(env, class_handler,
    "receiveProgramChange", "(II)V");
  pitchBendMethod = (*env)->GetMethodID(env, class_handler,
    "receivePitchBend", "(II)V");
  aftertouchMethod = (*env)->GetMethodID(env, class_handler,
    "receiveAftertouch", "(II)V");
  polyAftertouchMethod = (*env)->GetMethodID(env, class_handler,
    "receivePolyAftertouch", "(III)V");
  midiByteMethod = (*env)->GetMethodID(env, class_handler,
    "receiveMidiByte", "(II)V");
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_blockSize
(JNIEnv *env, jclass cls) {
  pthread_mutex_lock(&mutex);
  int n = libpd_blocksize();
  pthread_mutex_unlock(&mutex);
  return n;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_processRaw
(JNIEnv *env, jclass cls, jfloatArray inBuffer, jfloatArray outBuffer) {
  if (!inBuffer || !outBuffer) return -10;
  float *pIn = (*env)->GetFloatArrayElements(env, inBuffer, NULL);
  float *pOut = (*env)->GetFloatArrayElements(env, outBuffer, NULL);
  pthread_mutex_lock(&mutex);
  jint err = libpd_process_raw(pIn, pOut);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseFloatArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseFloatArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process__I_3S_3S
(JNIEnv *env, jclass cls, jint ticks,
    jshortArray inBuffer, jshortArray outBuffer) {
  if (!inBuffer || !outBuffer) return -10;
  short *pIn = (*env)->GetShortArrayElements(env, inBuffer, NULL);
  short *pOut = (*env)->GetShortArrayElements(env, outBuffer, NULL);
  pthread_mutex_lock(&mutex);
  jint err = libpd_process_short((int) ticks, pIn, pOut);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseShortArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseShortArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process__I_3F_3F
(JNIEnv *env, jclass cls, jint ticks,
    jfloatArray inBuffer, jfloatArray outBuffer) {
  if (!inBuffer || !outBuffer) return -10;
  float *pIn = (*env)->GetFloatArrayElements(env, inBuffer, NULL);
  float *pOut = (*env)->GetFloatArrayElements(env, outBuffer, NULL);
  pthread_mutex_lock(&mutex);
  jint err = libpd_process_float((int) ticks, pIn, pOut);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseFloatArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseFloatArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process__I_3D_3D
(JNIEnv *env, jclass cls, jint ticks,
    jdoubleArray inBuffer, jdoubleArray outBuffer) {
  if (!inBuffer || !outBuffer) return -10;
  double *pIn = (*env)->GetDoubleArrayElements(env, inBuffer, NULL);
  double *pOut = (*env)->GetDoubleArrayElements(env, outBuffer, NULL);
  pthread_mutex_lock(&mutex);
  jint err = libpd_process_double((int) ticks, pIn, pOut);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseDoubleArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseDoubleArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_arraySize
(JNIEnv *env, jclass cls, jstring jname) {
  if (!jname) return -2;
  const char *cname = (char *) (*env)->GetStringUTFChars(env, jname, NULL);
  pthread_mutex_lock(&mutex);
  int result = libpd_arraysize(cname);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jname, cname);
  return result;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_readArrayNative
(JNIEnv *env, jclass cls, jfloatArray jdest, jint destOffset,
jstring jsrc, jint srcOffset, jint n) {
  if (!jdest || !jsrc) return -3;
  float *pdest = (*env)->GetFloatArrayElements(env, jdest, NULL);
  const char *csrc = (char *) (*env)->GetStringUTFChars(env, jsrc, NULL);
  pthread_mutex_lock(&mutex);
  int result = libpd_read_array(pdest + destOffset, csrc, srcOffset, n);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jsrc, csrc);
  (*env)->ReleaseFloatArrayElements(env, jdest, pdest, 0);
  return result;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_writeArrayNative
(JNIEnv *env, jclass cls, jstring jdest, jint destOffset,
jfloatArray jsrc, jint srcOffset, jint n) {
  if (!jdest || !jsrc) return -3;
  float *psrc = (*env)->GetFloatArrayElements(env, jsrc, NULL);
  const char *cdest = (char *) (*env)->GetStringUTFChars(env, jdest, NULL);
  pthread_mutex_lock(&mutex);
  int result = libpd_write_array(cdest, destOffset, psrc + srcOffset, n);
  pthread_mutex_unlock(&mutex);
  (*env)->ReleaseStringUTFChars(env, jdest, cdest);
  (*env)->ReleaseFloatArrayElements(env, jsrc, psrc, 0);
  return result;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendNoteOn
(JNIEnv *env, jclass cls, jint channel, jint pitch, jint velocity) {
  pthread_mutex_lock(&mutex);
  int err = libpd_noteon(channel, pitch, velocity);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendControlChange
(JNIEnv *env, jclass cls, jint channel, jint controller, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_controlchange(channel, controller, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendProgramChange
(JNIEnv *env, jclass cls, jint channel, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_programchange(channel, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendPitchBend
(JNIEnv *env, jclass cls, jint channel, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_pitchbend(channel, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendAftertouch
(JNIEnv *env, jclass cls, jint channel, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_aftertouch(channel, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendPolyAftertouch
(JNIEnv *env, jclass cls, jint channel, jint pitch, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_polyaftertouch(channel, pitch, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendMidiByte
(JNIEnv *env, jclass cls, jint port, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_midibyte(port, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendSysex
(JNIEnv *env, jclass cls, jint port, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_sysex(port, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendSysRealTime
(JNIEnv *env, jclass cls, jint port, jint value) {
  pthread_mutex_lock(&mutex);
  int err = libpd_sysrealtime(port, value);
  pthread_mutex_unlock(&mutex);
  return err;
}

// -----------------------------------------------------------------------------
// Audio glue needs to be supplied in another file.
// -----------------------------------------------------------------------------
