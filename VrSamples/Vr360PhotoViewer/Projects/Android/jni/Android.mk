LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)					# clean everything up to prepare for a module

include $(LOCAL_PATH)/../../../../../cflags.mk

LOCAL_MODULE		:= vr360photoviewer

LOCAL_C_INCLUDES 	:= 	$(LOCAL_PATH)/../../../../SampleFramework/Src \
						$(LOCAL_PATH)/../../../../../VrApi/Include \
						$(LOCAL_PATH)/../../../../../1stParty/OVR/Include \
						$(LOCAL_PATH)/../../../../../1stParty/utilities/include \
						$(LOCAL_PATH)/../../../../../3rdParty/stb/src \

LOCAL_SRC_FILES		:= 	../../../Src/main.cpp \
					    ../../../Src/Vr360PhotoViewer.cpp \
#					    ../../../Src/dummy.cpp \
#					    ../gst-build-arm64-v8a/gstreamer_android.so \

# include default libraries
LOCAL_LDLIBS 			:= -llog -landroid -lGLESv3 -lEGL -lz
LOCAL_STATIC_LIBRARIES 	:= sampleframework
LOCAL_SHARED_LIBRARIES	:= vrapi gstreamer_android shared_c

include $(BUILD_SHARED_LIBRARY)



$(call import-module, VrSamples/Vr360PhotoViewer/Projects/Android/VrSamples/GStreamer/jni)
$(call import-module,VrSamples/SampleFramework/Projects/Android/jni)
$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)


#include $(CLEAR_VARS)
#LOCAL_MODULE := gstreamer
#LOCAL_SRC_FILES := ../../../../VrSamples/Vr360PhotoViewer/Projects/Android/gst-build-arm64-v8a/libgstreamer_android.so
#include $(PREBUILT_SHARED_LIBRARY)


#LOCAL_SHARED_LIBRARIES	:= gstreamer_android

#include $(BUILD_SHARED_LIBRARY)



GSTREAMER_ROOT        	  := $(GSTREAMER_ROOT_ANDROID)/arm64
GSTREAMER_NDK_BUILD_PATH  := $(GSTREAMER_ROOT)/share/gst-android/ndk-build/
include $(GSTREAMER_NDK_BUILD_PATH)/plugins.mk
GSTREAMER_PLUGINS         := $(GSTREAMER_PLUGINS_CORE) $(GSTREAMER_PLUGINS_SYS) $(GSTREAMER_PLUGINS_EFFECTS) $(GSTREAMER_PLUGINS_CODECS) $(GSTREAMER_PLUGINS_NET) $(GSTREAMER_PLUGINS_CODECS_RESTRICTED)
GSTREAMER_EXTRA_DEPS      := gstreamer-app-1.0 gstreamer-video-1.0 gobject-2.0
GSTREAMER_EXTRA_LIBS      := -liconv
include $(GSTREAMER_NDK_BUILD_PATH)/gstreamer-1.0.mk

$(info FINISHED GSTREAMER)
