ifeq ($(BOARD_HAVE_BLUETOOTH_BCM),true)

LOCAL_PATH:= $(call my-dir)

#
# brcm_patchram_plus.c
#

include $(CLEAR_VARS)

# broadcom bt
ifeq ($(SW_BOARD_HAVE_BLUETOOTH_NAME), ap6210)
LOCAL_CFLAGS += -DUSE_AP6210_BT_MODULE
endif

LOCAL_SRC_FILES := \
	brcm_patchram_plus.c \
	mac.c

LOCAL_MODULE := brcm_patchram_plus

LOCAL_SHARED_LIBRARIES := libcutils liblog

LOCAL_C_FLAGS := \
	-DANDROID

include $(BUILD_EXECUTABLE)

endif
