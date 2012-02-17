/*
 * Copyright (c) 2010 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include <stdio.h>
#include <pthread.h>
#include <jni.h>
#include "z_jni.h"
#include "z_libpd.h"

pthread_key_t __envkey; // thread local storage, for safely
                        // caching env pointer

#define GET_ENV   JNIEnv *env = (JNIEnv *) pthread_getspecific(__envkey);
// gets valid env pointer if possible, NULL otherwise

#define CACHE_ENV pthread_setspecific(__envkey, env);
// cache env pointer in a thread-safe manner

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

static void java_printhook(const char *msg) {
  if (messageHandler == NULL || msg == NULL) return;
  GET_ENV   // safely retrieve cached env pointer, if possible
  if (env == NULL) return;
  jstring jmsg = (*env)->NewStringUTF(env, msg);
  (*env)->CallVoidMethod(env, messageHandler, printMethod, jmsg);
}

void java_sendBang(const char *source) {
  if (messageHandler == NULL || source == NULL) return;
  GET_ENV
  if (env == NULL) return;
  jstring jsource = (*env)->NewStringUTF(env, source);
  (*env)->CallVoidMethod(env, messageHandler, bangMethod, jsource);
}

void java_sendFloat(const char *source, float x) {
  if (messageHandler == NULL || source == NULL) return;
  GET_ENV
  if (env == NULL) return;
  jstring jsource = (*env)->NewStringUTF(env, source);
  (*env)->CallVoidMethod(env, messageHandler, floatMethod, jsource, x);
}

void java_sendSymbol(const char *source, const char *sym) {
  if (messageHandler == NULL || source == NULL || sym == NULL) return;
  GET_ENV
  if (env == NULL) return;
  jstring jsource = (*env)->NewStringUTF(env, source);
  jstring jsym = (*env)->NewStringUTF(env, sym);
  (*env)->CallVoidMethod(env, messageHandler, symbolMethod, jsource, jsym);
}

static jobjectArray makeJavaArray(JNIEnv *env, int argc, t_atom *argv) {
  jobjectArray jarray = (*env)->NewObjectArray(env, argc, objClass, NULL);
  int i;
  for (i = 0; i < argc; i++) {
    t_atom a = argv[i];
    jobject obj = NULL;
    if (libpd_is_float(a)) {
      obj = (*env)->NewObject(env, floatClass, floatInit, libpd_get_float(a));
    } else if (libpd_is_symbol(a)) {
      obj = (*env)->NewStringUTF(env, libpd_get_symbol(a));
    }
    (*env)->SetObjectArrayElement(env, jarray, i, obj);
  }
  return jarray;
}

void java_sendList(const char *source, int argc, t_atom *argv) {
  if (messageHandler == NULL || source == NULL) return;
  GET_ENV
  if (env == NULL) return;
  jstring jsource = (*env)->NewStringUTF(env, source);
  jobjectArray jarray = makeJavaArray(env, argc, argv);
  (*env)->CallVoidMethod(env, messageHandler, listMethod, jsource, jarray);
}

void java_sendMessage(const char *source, const char *msg,
              int argc, t_atom *argv) {
  if (messageHandler == NULL || source == NULL || msg == NULL) return;
  GET_ENV
  if (env == NULL) return;
  jstring jsource = (*env)->NewStringUTF(env, source);
  jstring jmsg = (*env)->NewStringUTF(env, msg);
  jobjectArray jarray = makeJavaArray(env, argc, argv);
  (*env)->CallVoidMethod(env, messageHandler, anyMethod,
             jsource, jmsg, jarray);
}

void java_sendNoteOn(int channel, int pitch, int velocity) {
  if (midiHandler == NULL) return;
  GET_ENV
  if (env == NULL) return;
  (*env)->CallVoidMethod(env, midiHandler,
            noteOnMethod, channel, pitch, velocity);
}

void java_sendControlChange(int channel, int controller, int value) {
  if (midiHandler == NULL) return;
  GET_ENV
  if (env == NULL) return;
  (*env)->CallVoidMethod(env, midiHandler,
            controlChangeMethod, channel, controller, value);
}

void java_sendProgramChange(int channel, int value) {
  if (midiHandler == NULL) return;
  GET_ENV
  if (env == NULL) return;
  (*env)->CallVoidMethod(env, midiHandler,
            programChangeMethod, channel, value);
}

void java_sendPitchBend(int channel, int value) {
  if (midiHandler == NULL) return;
  GET_ENV
  if (env == NULL) return;
  (*env)->CallVoidMethod(env, midiHandler,
            pitchBendMethod, channel, value);
}

void java_sendAftertouch(int channel, int value) {
  if (midiHandler == NULL) return;
  GET_ENV
  if (env == NULL) return;
  (*env)->CallVoidMethod(env, midiHandler,
            aftertouchMethod, channel, value);
}

void java_sendPolyAftertouch(int channel, int pitch, int value) {
  if (midiHandler == NULL) return;
  GET_ENV
  if (env == NULL) return;
  (*env)->CallVoidMethod(env, midiHandler,
            polyAftertouchMethod, channel, pitch, value);
}

void java_sendMidiByte(int port, int value) {
  if (midiHandler == NULL) return;
  GET_ENV
  if (env == NULL) return;
  (*env)->CallVoidMethod(env, midiHandler,
            midiByteMethod, port, value);
}

static void deleteHandlerRef(JNIEnv *env) {
  if (messageHandler == NULL) return;
  (*env)->DeleteGlobalRef(env, messageHandler);
  messageHandler = NULL;
  printMethod = NULL;
  bangMethod = NULL;
  floatMethod = NULL;
  symbolMethod = NULL;
  listMethod = NULL;
  anyMethod = NULL;
}

static void deleteMidiHandlerRef(JNIEnv *env) {
  if (midiHandler == NULL) return;
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

JNIEXPORT jint JNICALL JNI_OnLoad
(JavaVM *jvm, void *ignored) {
  pthread_key_create(&__envkey, NULL);
  return JNI_VERSION_1_4;
}

#define LIBPD_CLASS_REF(c) (*env)->NewGlobalRef(env, (*env)->FindClass(env, c));

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_initialize
(JNIEnv *env, jclass cls) {
  CACHE_ENV
  objClass = LIBPD_CLASS_REF("java/lang/Object");
  floatClass = LIBPD_CLASS_REF("java/lang/Float");
  floatInit = (*env)->GetMethodID(env, floatClass, "<init>", "(F)V");

  libpd_printhook = (t_libpd_printhook) java_printhook;
  libpd_banghook = (t_libpd_banghook) java_sendBang;
  libpd_floathook = (t_libpd_floathook) java_sendFloat;
  libpd_symbolhook = (t_libpd_symbolhook) java_sendSymbol;
  libpd_listhook = (t_libpd_listhook) java_sendList;
  libpd_messagehook = (t_libpd_messagehook) java_sendMessage;

  libpd_noteonhook = (t_libpd_noteonhook) java_sendNoteOn;
  libpd_controlchangehook = (t_libpd_controlchangehook) java_sendControlChange;
  libpd_programchangehook = (t_libpd_programchangehook) java_sendProgramChange;
  libpd_pitchbendhook = (t_libpd_pitchbendhook) java_sendPitchBend;
  libpd_aftertouchhook = (t_libpd_aftertouchhook) java_sendAftertouch;
  libpd_polyaftertouchhook =
            (t_libpd_polyaftertouchhook) java_sendPolyAftertouch;
  libpd_midibytehook = (t_libpd_midibytehook) java_sendMidiByte;

  libpd_init();
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_clearSearchPath
(JNIEnv *env, jclass cls) {
  CACHE_ENV
  libpd_clear_search_path();
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_addToSearchPath
(JNIEnv *env, jclass cls, jstring jpath) {
  if (jpath == NULL) {
    return;
  }
  CACHE_ENV
  const char *cpath = (char *) (*env)->GetStringUTFChars(env, jpath, NULL);
  libpd_add_to_search_path(cpath);
  (*env)->ReleaseStringUTFChars(env, jpath, cpath);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_openAudio
(JNIEnv *env, jclass cls, jint inChans, jint outChans, jint srate) {
  CACHE_ENV
  return libpd_init_audio((int) inChans, (int) outChans, (int) srate);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_processRaw
(JNIEnv *env, jclass cls, jfloatArray inBuffer, jfloatArray outBuffer) {
  if (inBuffer == NULL || outBuffer == NULL) {
    return -10;
  }
  CACHE_ENV
  float *pIn = (*env)->GetFloatArrayElements(env, inBuffer, NULL);
  float *pOut = (*env)->GetFloatArrayElements(env, outBuffer, NULL);
  jint err = libpd_process_raw(pIn, pOut);
  (*env)->ReleaseFloatArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseFloatArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process__I_3S_3S
(JNIEnv *env, jclass cls, jint ticks,
    jshortArray inBuffer, jshortArray outBuffer) {
  if (inBuffer == NULL || outBuffer == NULL) {
    return -10;
  }
  CACHE_ENV
  short *pIn = (*env)->GetShortArrayElements(env, inBuffer, NULL);
  short *pOut = (*env)->GetShortArrayElements(env, outBuffer, NULL);
  jint err = libpd_process_short((int) ticks, pIn, pOut);
  (*env)->ReleaseShortArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseShortArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process__I_3F_3F
(JNIEnv *env, jclass cls, jint ticks,
    jfloatArray inBuffer, jfloatArray outBuffer) {
  if (inBuffer == NULL || outBuffer == NULL) {
    return -10;
  }
  CACHE_ENV
  float *pIn = (*env)->GetFloatArrayElements(env, inBuffer, NULL);
  float *pOut = (*env)->GetFloatArrayElements(env, outBuffer, NULL);
  jint err = libpd_process_float((int) ticks, pIn, pOut);
  (*env)->ReleaseFloatArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseFloatArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process__I_3D_3D
(JNIEnv *env, jclass cls, jint ticks,
    jdoubleArray inBuffer, jdoubleArray outBuffer) {
  if (inBuffer == NULL || outBuffer == NULL) {
    return -10;
  }
  CACHE_ENV
  double *pIn = (*env)->GetDoubleArrayElements(env, inBuffer, NULL);
  double *pOut = (*env)->GetDoubleArrayElements(env, outBuffer, NULL);
  jint err = libpd_process_double((int) ticks, pIn, pOut);
  (*env)->ReleaseDoubleArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseDoubleArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_setReceiver
(JNIEnv *env, jclass cls, jobject handler) {
  CACHE_ENV
  deleteHandlerRef(env);
  if (handler == NULL) {
    return;
  }
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

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_setMidiReceiver
(JNIEnv *env, jclass cls, jobject handler) {
  CACHE_ENV
  deleteMidiHandlerRef(env);
  if (handler == NULL) {
    return;
  }
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

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_exists
(JNIEnv *env, jclass cls, jstring jsym) {
  if (jsym == NULL) return 0;
  CACHE_ENV
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  jboolean flag = libpd_exists(csym);
  (*env)->ReleaseStringUTFChars(env, jsym, csym);
  return flag;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendBang
(JNIEnv *env, jclass cls, jstring jrecv) {
  if (jrecv == NULL) return -2;
  CACHE_ENV
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  int err = libpd_bang(crecv);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendFloat
(JNIEnv *env, jclass cls, jstring jrecv, jfloat x) {
  if (jrecv == NULL) return -2;
  CACHE_ENV
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  int err = libpd_float(crecv, x);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendSymbol
(JNIEnv *env, jclass cls, jstring jrecv, jstring jsym) {
  if (jrecv == NULL) return -2;
  CACHE_ENV
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  jint err = libpd_symbol(crecv, csym);
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
  if (jsym == NULL) {
    return;
  }
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  libpd_add_symbol(csym);
  (*env)->ReleaseStringUTFChars(env, jsym, csym);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_finishList
(JNIEnv *env, jclass cls, jstring jrecv) {
  if (jrecv == NULL) {
    return -10;
  }
  CACHE_ENV
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  jint err = libpd_finish_list(crecv);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_finishMessage
(JNIEnv *env, jclass cls, jstring jrecv, jstring jmsg) {
  if (jrecv == NULL || jmsg == NULL) {
    return -10;
  }
  CACHE_ENV
  const char *crecv = (char *) (*env)->GetStringUTFChars(env, jrecv, NULL);
  const char *cmsg = (char *) (*env)->GetStringUTFChars(env, jmsg, NULL);
  jint err = libpd_finish_message(crecv, cmsg);
  (*env)->ReleaseStringUTFChars(env, jrecv, crecv);
  (*env)->ReleaseStringUTFChars(env, jmsg, cmsg);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_blockSize
(JNIEnv *env, jclass cls) {
  return libpd_blocksize();
}

JNIEXPORT jlong JNICALL Java_org_puredata_core_PdBase_bindSymbol
(JNIEnv *env, jclass cls, jstring jsym) {
  if (jsym == NULL) return 0;
  CACHE_ENV
  const char *csym = (char *) (*env)->GetStringUTFChars(env, jsym, NULL);
  jlong ptr = (jlong) libpd_bind(csym);
  (*env)->ReleaseStringUTFChars(env, jsym, csym);
  return ptr;
  // very naughty, returning a pointer to Java
  // using long integer in case we're on a 64bit CPU
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_unbindSymbol
(JNIEnv *env, jclass cls, jlong ptr) {
  CACHE_ENV
  libpd_unbind((void *)ptr);
  // even naughtier, using a pointer from Java
  // using long integer in case we're on a 64bit CPU
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendNoteOn
(JNIEnv *env, jclass cls, jint channel, jint pitch, jint velocity) {
  CACHE_ENV
  return libpd_noteon(channel, pitch, velocity);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendControlChange
(JNIEnv *env, jclass cls, jint channel, jint controller, jint value) {
  CACHE_ENV
  return libpd_controlchange(channel, controller, value);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendProgramChange
(JNIEnv *env, jclass cls, jint channel, jint value) {
  CACHE_ENV
  return libpd_programchange(channel, value);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendPitchBend
(JNIEnv *env, jclass cls, jint channel, jint value) {
  CACHE_ENV
  return libpd_pitchbend(channel, value);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendAftertouch
(JNIEnv *env, jclass cls, jint channel, jint value) {
  CACHE_ENV
  return libpd_aftertouch(channel, value);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendPolyAftertouch
(JNIEnv *env, jclass cls, jint channel, jint pitch, jint value) {
  CACHE_ENV
  return libpd_polyaftertouch(channel, pitch, value);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendMidiByte
(JNIEnv *env, jclass cls, jint port, jint value) {
  CACHE_ENV
  return libpd_midibyte(port, value);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendSysex
(JNIEnv *env, jclass cls, jint port, jint value) {
  CACHE_ENV
  return libpd_sysex(port, value);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_sendSysRealTime
(JNIEnv *env, jclass cls, jint port, jint value) {
  CACHE_ENV
  return libpd_sysrealtime(port, value);
}

JNIEXPORT jlong JNICALL Java_org_puredata_core_PdBase_openFile
(JNIEnv *env, jclass cls, jstring jpatch, jstring jdir) {
  if (jpatch == NULL || jdir == NULL) return 0;
  CACHE_ENV
  const char *cpatch = (char *) (*env)->GetStringUTFChars(env, jpatch, NULL);
  const char *cdir = (char *) (*env)->GetStringUTFChars(env, jdir, NULL);
  jlong ptr = (jlong) libpd_openfile(cpatch, cdir);
  (*env)->ReleaseStringUTFChars(env, jpatch, cpatch);
  (*env)->ReleaseStringUTFChars(env, jdir, cdir);
  return ptr;
  // very naughty, returning a pointer to Java
  // using long integer in case we're on a 64bit CPU
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_closeFile
(JNIEnv *env, jclass cls, jlong ptr) {
  CACHE_ENV
  libpd_closefile((void *)ptr);
  // even naughtier, using a pointer from Java
  // using long integer in case we're on a 64bit CPU
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_getDollarZero
(JNIEnv *env, jclass cls, jlong ptr) {
  CACHE_ENV
  return libpd_getdollarzero((void *)ptr);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_arraySize
(JNIEnv *env, jclass cls, jstring jname) {
  if (jname == NULL) return -2;
  CACHE_ENV
  const char *cname = (char *) (*env)->GetStringUTFChars(env, jname, NULL);
  int result = libpd_arraysize(cname);
  (*env)->ReleaseStringUTFChars(env, jname, cname);
  return result;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_readArrayNative
(JNIEnv *env, jclass cls, jfloatArray jdest, jint destOffset,
jstring jsrc, jint srcOffset, jint n) {
  if (jdest == NULL || jsrc == NULL) return -3;
  CACHE_ENV
  float *pdest = (*env)->GetFloatArrayElements(env, jdest, NULL);
  const char *csrc = (char *) (*env)->GetStringUTFChars(env, jsrc, NULL);
  int result = libpd_read_array(pdest + destOffset, csrc, srcOffset, n);
  (*env)->ReleaseStringUTFChars(env, jsrc, csrc);
  (*env)->ReleaseFloatArrayElements(env, jdest, pdest, 0);
  return result;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_writeArrayNative
(JNIEnv *env, jclass cls, jstring jdest, jint destOffset,
jfloatArray jsrc, jint srcOffset, jint n) {
  if (jdest == NULL || jsrc == NULL) return -3;
  CACHE_ENV
  float *psrc = (*env)->GetFloatArrayElements(env, jsrc, NULL);
  const char *cdest = (char *) (*env)->GetStringUTFChars(env, jdest, NULL);
  int result = libpd_write_array(cdest, destOffset, psrc + srcOffset, n);
  (*env)->ReleaseStringUTFChars(env, jdest, cdest);
  (*env)->ReleaseFloatArrayElements(env, jsrc, psrc, 0);
  return result;
}
