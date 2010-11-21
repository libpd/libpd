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
    if (a.a_type == A_FLOAT) {
      obj = (*env)->NewObject(env, floatClass, floatInit, a.a_w.w_float);
    } else if (a.a_type == A_SYMBOL) {
      obj = (*env)->NewStringUTF(env, a.a_w.w_symbol->s_name);
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
(JNIEnv *env, jclass cls, jint inChans, jint outChans,
jint srate, jint ticksPerBuffer) {
  CACHE_ENV
  return libpd_init_audio((int) inChans, (int) outChans,
               (int) srate, (int) ticksPerBuffer);
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

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process___3S_3S
(JNIEnv *env, jclass cls, jshortArray inBuffer, jshortArray outBuffer) {
  if (inBuffer == NULL || outBuffer == NULL) {
    return -10;
  }
  CACHE_ENV
  short *pIn = (*env)->GetShortArrayElements(env, inBuffer, NULL);
  short *pOut = (*env)->GetShortArrayElements(env, outBuffer, NULL);
  jint err = libpd_process_short(pIn, pOut);
  (*env)->ReleaseShortArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseShortArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process___3F_3F
(JNIEnv *env, jclass cls, jfloatArray inBuffer, jfloatArray outBuffer) {
  if (inBuffer == NULL || outBuffer == NULL) {
    return -10;
  }
  CACHE_ENV
  float *pIn = (*env)->GetFloatArrayElements(env, inBuffer, NULL);
  float *pOut = (*env)->GetFloatArrayElements(env, outBuffer, NULL);
  jint err = libpd_process_float(pIn, pOut);
  (*env)->ReleaseFloatArrayElements(env, inBuffer, pIn, 0);
  (*env)->ReleaseFloatArrayElements(env, outBuffer, pOut, 0);
  return err;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_process___3D_3D
(JNIEnv *env, jclass cls, jdoubleArray inBuffer, jdoubleArray outBuffer) {
  if (inBuffer == NULL || outBuffer == NULL) {
    return -10;
  }
  CACHE_ENV
  double *pIn = (*env)->GetDoubleArrayElements(env, inBuffer, NULL);
  double *pOut = (*env)->GetDoubleArrayElements(env, outBuffer, NULL);
  jint err = libpd_process_double(pIn, pOut);
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
(JNIEnv *env, jclass cls) {
  return libpd_start_message();
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

