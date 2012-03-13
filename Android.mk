#######################################################################
#
# Copyright 2012 Ryan Frawley

#
# This file is part of wpad.
#
# wpad is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# wpad is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with wpad. If not, see <http://www.gnu.org/licenses/>.

#
#######################################################################

LOCAL_PATH := $(call my-dir)/jni

# Static library
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libwpad
LOCAL_SRC_FILES := Device.cpp DeviceManager.cpp Lock.cpp

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lbluetooth

include $(BUILD_STATIC_LIBRARY)

# JNI shared lib
#
include $(CLEAR_VARS)

LOCAL_MODULE    := libwpad-jni
LOCAL_SRC_FILES := com_mercury_wpad_Device.cpp com_mercury_wpad_DeviceManager.cpp wpad-jni.cpp

LOCAL_STATIC_LIBRARIES := libwpad

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lbluetooth

include $(BUILD_SHARED_LIBRARY)