##
 # =====================================================================================
 #
 #       Filename:  Android.mk
 #
 #    Description:  Recovery WiFi Manager android make files
 #
 #        Version:  1.0
 #        Created:  12/07/2019 11:04:57 AM
 #       Revision:  none
 #       Compiler:  gcc
 #
 #         Author:  Li Jin (lijin), princeofdream@outlook.com
 #   Organization:  XPeng
 #
 # =====================================================================================
##

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

procmgr_objs := src/network_utils.cpp
procmgr_objs += src/service_manager.cpp
procmgr_objs += src/socket_server.cpp
procmgr_objs += src/process_manager.cpp
procmgr_objs += src/messages_manager.cpp
procmgr_objs += src/netlink_event.cpp
procmgr_objs += src/misc_utils.cpp
procmgr_objs += src/network_filter.cpp
procmgr_objs += src/network_manager.cpp
procmgr_objs += src/serv_config.cpp
procmgr_objs += src/proc_config.cpp

$(warning "===================$(ANDROID_TOOLCHAIN)")

LOCAL_MODULE := libprocmgr
LOCAL_MODULE_TAGS := optional
	# prebuilts/ndk/r13/sources/android/support/include
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	bionic \
	$(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := libnetutils libbase libcutils liblog
LOCAL_SHARED_LIBRARIES += libcrypto libssl

CFLAGS := -Wno-unused-parameter
CFLAGS += -Wno-unused-private-field
CFLAGS += -Wno-format
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-int-to-void-pointer-cast
CFLAGS += -Wno-null-conversion
CFLAGS += -Wno-sign-compare
CFLAGS += -Wno-writable-strings
CFLAGS += -Wno-invalid-source-encoding

LOCAL_SRC_FILES := $(procmgr_objs)
LOCAL_CFLAGS := $(CFLAGS)

# LOCAL_MODULE_PATH := $(TARGET_RECOVERY_OUT)/root/sbin/

# LOCAL_FORCE_STATIC_EXECUTABLE := true
# include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)


