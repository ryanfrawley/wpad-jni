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

package com.mercury.inputmethod.wpad;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.ParcelUuid;

abstract class BluetoothConnectionFactory {

	static final int TYPE_RFCOMM = 1;
	static final int TYPE_SCO = 2;
	static final int TYPE_L2CAP = 3;
	static final Constructor<BluetoothSocket> BLUETOOTH_SOCKET_CONSTRUCTOR;

	static {
		BLUETOOTH_SOCKET_CONSTRUCTOR = obtainConstructor();
	}

	static final BluetoothSocket createBluetoothSocket(int type, int fd,
			boolean auth, boolean encrypt, BluetoothDevice device, int port,
			ParcelUuid uuid) throws IOException {
		Constructor<BluetoothSocket> constructor = obtainConstructor();
		BluetoothSocket tmp = null;

		try {
			tmp = constructor.newInstance(type, fd, auth, encrypt, device,
					port, uuid);
		} catch (IllegalArgumentException e) {
		} catch (InstantiationException e) {
		} catch (IllegalAccessException e) {
		} catch (InvocationTargetException e) {
		}

		return tmp;
	}

	private static final Constructor<BluetoothSocket> obtainConstructor() {
		Class<BluetoothSocket> cls = BluetoothSocket.class;
		Constructor<BluetoothSocket> tmp = null;

		try {
			tmp = cls.getDeclaredConstructor(int.class, int.class,
					boolean.class, boolean.class, BluetoothDevice.class,
					int.class, ParcelUuid.class);
			if (!tmp.isAccessible()) {
				tmp.setAccessible(true);
			}
		} catch (SecurityException e) {
		} catch (NoSuchMethodException e) {
		}

		return tmp;
	}
}
