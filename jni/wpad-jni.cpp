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

JavaVM *jvm;
jclass deviceManagerClass;
jclass deviceClass;
jclass onEnumDevicesListenerClass;
jmethodID deviceConstructor;
jmethodID onEnumDeviceMethod;
jmethodID onDisconnectMethod;
jmethodID onExtensionMethod;
jmethodID onButtonMethod;
std::map<jobject, WPAD::Device *> devices;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
  JNIEnv *env;
  jclass cls;
	if(vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK)
    return JNI_ERR;
	cls = env->FindClass("com/mercury/wpad/DeviceManager");
  if(!cls)
    return JNI_ERR;
  deviceManagerClass = (jclass)env->NewGlobalRef(cls);
  if(!deviceManagerClass)
    return JNI_ERR;
  cls = env->FindClass("com/mercury/wpad/Device");
  if(!cls)
    return JNI_ERR;
  deviceClass = (jclass)env->NewGlobalRef(cls);
  if(!deviceClass)
    return JNI_ERR;
  deviceConstructor = env->GetMethodID(deviceClass, "<init>", "()V");
  onDisconnectMethod = env->GetMethodID(deviceClass, "onDisconnect", "()V");
  onExtensionMethod = env->GetMethodID(deviceClass, "onExtension", "(I)V");
  onButtonMethod = env->GetMethodID(deviceClass, "onButton", "(III)V");
  cls = env->FindClass("com/mercury/wpad/OnEnumDevicesListener");
  if(!cls)
    return JNI_ERR;
  onEnumDevicesListenerClass = (jclass)env->NewGlobalRef(cls);
  if(!onEnumDevicesListenerClass)
    return JNI_ERR;
  onEnumDeviceMethod = env->GetMethodID(onEnumDevicesListenerClass, "onEnumDevice", "(Ljava/lang/String;)Z");
  if(env->RegisterNatives(deviceManagerClass, deviceManagerMethods, sizeof(deviceManagerMethods) / sizeof(deviceManagerMethods[0])) != JNI_OK)
    return JNI_ERR;
  if(env->RegisterNatives(deviceClass, deviceMethods, sizeof(deviceMethods) / sizeof(deviceMethods[0])) != JNI_OK) {
    return JNI_ERR;
  }
  jvm = vm;
	return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
  /*JNIEnv *env;
  jvm->GetEnv((void **)&env, JNI_VERSION_1_4);
  env->DeleteWeakGlobalRef(deviceClass);
  env->DeleteWeakGlobalRef(deviceManagerClass);
  env->DeleteWeakGlobalRef(onEnumDevicesListenerClass);*/
  // Weak global references arn't implemented on Android!
}
