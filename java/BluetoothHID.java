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

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;


public class BluetoothHID {
	private static final int HID_CONTROL_CHANNEL = 0x11;
	private static final int HID_INTERRUPT_CHANNEL = 0x13;
	
	public static BluetoothSocket getControlSocket(BluetoothDevice device) {
		try {
			return BluetoothConnectionFactory.createBluetoothSocket(BluetoothConnectionFactory.TYPE_L2CAP, -1, false, false, device, HID_CONTROL_CHANNEL, null);
		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	public static BluetoothSocket getInterruptSocket(BluetoothDevice device) {
		try {
			return BluetoothConnectionFactory.createBluetoothSocket(BluetoothConnectionFactory.TYPE_L2CAP, -1, false, false, device, HID_INTERRUPT_CHANNEL, null);
		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}
}
