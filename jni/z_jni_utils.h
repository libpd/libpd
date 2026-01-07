/*
 * Java-native interface utilities
 */

#ifndef _Z_JNI_UTILS_H
#define _Z_JNI_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

// key/value callback type
typedef void(*foreach_callback_t)(const char* key, const char *value);

// Gets all key/value pairs from a java HashMap<String, String>,
// and call a user function for each.
void map_foreach(JNIEnv *env, jobject map, foreach_callback_t callback);

#ifdef __cplusplus
}
#endif
#endif
