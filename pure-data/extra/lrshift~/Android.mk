LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lrshift~
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := lrshift~.c
LOCAL_SHARED_LIBRARIES := pdnative

include $(BUILD_SHARED_LIBRARY)

