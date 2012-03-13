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

#include "wpad-jni.h"
#include "Device.h"
#include "com_mercury_wpad_Device.h"

using namespace WPAD;

JNIEXPORT void JNICALL Java_com_mercury_wpad_Device_disconnect(JNIEnv *env, jobject obj)
{
  Device *device = devices[obj];
  device->disconnect();
}

JNIEXPORT void JNICALL Java_com_mercury_wpad_Device_setLEDs(JNIEnv *env, jobject obj, jint leds)
{
  Device *device = devices[obj];
  device->setLEDs(leds);
}

JNIEXPORT void JNICALL Java_com_mercury_wpad_Device_setRumble(JNIEnv *env, jobject obj, jboolean rumble)
{
  Device *device = devices[obj];
  device->setRumble(rumble);
}
