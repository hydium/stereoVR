
LOCAL_PATH:= $(call my-dir)

#--------------------------------------------------------
# libvrcubeworld.so
#--------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE			:= vrcubeworldsv
LOCAL_CFLAGS			:= -std=c99 -Werror
LOCAL_SRC_FILES			:= ../../../Src/VrCubeWorld_SurfaceView.c
LOCAL_LDLIBS			:= -llog -landroid -lGLESv3 -lEGL		# include default libraries

LOCAL_SHARED_LIBRARIES	:= vrapi gstreamer_android

include $(BUILD_SHARED_LIBRARY)

$(info FINISHED GSTREAMER)

$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)
