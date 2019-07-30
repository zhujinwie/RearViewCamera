#Description:makefile of Helloworld  
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS    := -Wall
LOCAL_MODULE    := myegl_jni
LOCAL_C_INCLUDES :=$(LOCAL_PATH)/include
LOCAL_CPP_EXTENSION := .cpp
LOCAL_LDLIBS += -llog -lGLESv1_CM
LOCAL_SRC_FILES := com_example_rearviewcamera_MyRenderer.cpp
 
include $(BUILD_SHARED_LIBRARY)
