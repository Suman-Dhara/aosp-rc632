#  Copyright (C) 2018 FORTUNA IMPLEX PVT. LTD.
#  Written by SUMAN DHARA @ Embedded System R&D Team <dhara_suman@yahoo.in> 
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	rc632hw.c

#ifeq ($(strip $(TARGET_ARCH)),arm)
LOCAL_MODULE_RELATIVE_PATH := hw
#else
#LOCAL_MODULE_RELATIVE_PATH := hw
#LOCAL_C_INCLUDES := hardware/libhardware
#endif

#LOCAL_MODULE := rc632hw.$(TARGET_DEVICE)
LOCAL_MODULE := rc632hw.s5p4418_drone
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE_TAGS := optional 
include $(BUILD_SHARED_LIBRARY)
