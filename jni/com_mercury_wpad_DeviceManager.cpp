/*
 * Copyright 2012 Ryan Frawley
 *
 * This file is part of wpad.
 *
 * wpad is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wpad is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wpad. If not, see <http://www.gnu.org/licenses/>.
 */

#include "wpad.h"
#include "DeviceManager.h"
#include "Device.h"
#include "wpad-jni.h"
#include "com_mercury_wpad_DeviceManager.h"

using namespace WPAD;

WPAD_RESULT wpadEnumDevicesCallback(bdaddr_t *address, void *param);

JNIEXPORT void JNICALL Java_com_mercury_wpad_DeviceManager_enumDevices(JNIEnv *env, jclass cls, jobject callback, jint duration)
{
  DeviceManager::enumDevices(wpadEnumDevicesCallback, duration, &callback, NULL);
}

JNIEXPORT jobject JNICALL Java_com_mercury_wpad_DeviceManager_connectDevice(JNIEnv *env, jclass cls, jstring address)
{
  // Convert from string to bdaddr_t
  jboolean isCopy;
  const char *buffer = env->GetStringUTFChars(address, &isCopy);
  bdaddr_t deviceAddress;
  str2ba(buffer, &deviceAddress);
  env->ReleaseStringUTFChars(address, buffer);

  Device *device;
  if(DeviceManager::connectDevice(&deviceAddress, &device) != WPAD_SUCCESS)
    return NULL;
  jobject obj = env->NewObject(deviceClass, deviceConstructor);
  if(obj)
  {
    obj = env->NewGlobalRef(obj);
    devices[obj] = device;
  }
  return obj;
}

WPAD_RESULT wpadEnumDevicesCallback(bdaddr_t *address, void *param)
{
  jobject listenerObject = *(jobject *)param;
  if(!listenerObject)
    return WPAD_INVALID_PARAMETER;
  char buffer[19];
  ba2str(address, buffer);
  JNIEnv *env;
  if(jvm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK)
    return WPAD_FAILURE;
  jstring string = env->NewStringUTF(buffer);
  bool result = env->CallBooleanMethod(listenerObject, onEnumDeviceMethod, string);
  return result ? WPAD_SUCCESS : WPAD_FAILURE;
}

JNIEXPORT void JNICALL Java_com_mercury_wpad_DeviceManager_disconnectDevice(JNIEnv *env, jclass cls, jobject obj)
{
  DeviceManager::disconnectDevice(devices[obj]);
  devices.erase(obj);
  env->DeleteGlobalRef(obj);
}

JNIEXPORT void JNICALL Java_com_mercury_wpad_DeviceManager_initialize(JNIEnv *env, jclass cls)
{
  DeviceManager::initialize();
}

JNIEXPORT void JNICALL Java_com_mercury_wpad_DeviceManager_shutdown(JNIEnv *emv, jclass cls)
{
  DeviceManager::shutdown();
}
