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

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_mercury_wpad_Device */

#ifndef _Included_com_mercury_wpad_Device
#define _Included_com_mercury_wpad_Device
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_mercury_wpad_Device
 * Method:    disconnect
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mercury_wpad_Device_disconnect
  (JNIEnv *, jobject);

/*
 * Class:     com_mercury_wpad_Device
 * Method:    setLEDs
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_mercury_wpad_Device_setLEDs
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_mercury_wpad_Device
 * Method:    setRumble
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_mercury_wpad_Device_setRumble
  (JNIEnv *, jobject, jboolean);

#ifdef __cplusplus
}
#endif
#endif