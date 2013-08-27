# Build the unit tests.
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifneq ($(TARGET_SIMULATOR),true)

LOCAL_MODULE := mtop

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    mtop.c

LOCAL_SHARED_LIBRARIES := \
	libbinder \
	libcutils \
	libstlport \
	libui \
	libutils \

LOCAL_C_INCLUDES := \
    bionic \
    bionic/libstdc++/include \
    external/gtest/include \
    external/stlport/stlport

include $(BUILD_EXECUTABLE)

endif


