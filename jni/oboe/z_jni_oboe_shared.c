
#include "z_jni_shared.c"

// export mutex to C++
pthread_mutex_t *jni_oboe_get_libpd_mutex() {
    return &mutex;
}
