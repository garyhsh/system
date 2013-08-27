#
# libbluedroid
#

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)


# realtek rtl8723as combo bt
ifeq ($(SW_BOARD_HAVE_BLUETOOTH_RTK), true)
LOCAL_CFLAGS += -DSW_BOARD_HAVE_BLUETOOTH_RTK
endif

# realtek rtl8723au combo bt
ifeq ($(SW_BOARD_HAVE_BLUETOOTH_NAME), rtl8723au)
LOCAL_CFLAGS += -DUSE_RTK_RTL8723AU
endif

# broadcom bt
ifeq ($(BOARD_HAVE_BLUETOOTH_BCM), true)
LOCAL_CFLAGS += -DSW_BOARD_HAVE_BLUETOOTH_BCM
endif

LOCAL_SRC_FILES := \
	bluetooth.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	system/bluetooth/bluez-clean-headers

LOCAL_SHARED_LIBRARIES := \
	libcutils

LOCAL_MODULE := libbluedroid

include $(BUILD_SHARED_LIBRARY)
