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

#ifndef __WPAD_JNI_H_
#define __WPAD_JNI_H_

#include "wpad.h"
#include "Device.h"
#include "com_mercury_wpad_Device.h"
#include "com_mercury_wpad_DeviceManager.h"
#include <map>

extern JavaVM *jvm;
extern jclass deviceManagerClass;
extern jclass deviceClass;
extern jclass onEnumDevicesListenerClass;
extern jmethodID deviceConstructor;
extern jmethodID onEnumDeviceMethod;
extern jmethodID onDisconnectMethod;
extern jmethodID onExtensionMethod;
extern jmethodID onButtonMethod;
extern std::map<jobject, WPAD::Device *> devices;

static JNINativeMethod deviceMethods[] =
{
  {"disconnect", "()V", (void *)&Java_com_mercury_wpad_Device_disconnect},
  {"setLEDs", "(I)V", (void *)&Java_com_mercury_wpad_Device_setLEDs},
  {"setRumble", "(Z)V", (void *)&Java_com_mercury_wpad_Device_setRumble}
};

static JNINativeMethod deviceManagerMethods[] =
{
  {"enumDevices", "(Lcom/mercury/wpad/OnEnumDevicesListener;I)V", (void *)&Java_com_mercury_wpad_DeviceManager_enumDevices},
  {"connectDevice", "(Ljava/lang/String;)Lcom/mercury/wpad/Device;", (void *)&Java_com_mercury_wpad_DeviceManager_connectDevice},
  {"disconnectDevice", "(Lcom/mercury/wpad/Device;)V", (void *)&Java_com_mercury_wpad_DeviceManager_disconnectDevice},
  {"initialize", "()V", (void *)Java_com_mercury_wpad_DeviceManager_initialize},
  {"shutdown", "()V", (void *)Java_com_mercury_wpad_DeviceManager_shutdown},
};

#endif // __WPAD_JNI_H_
