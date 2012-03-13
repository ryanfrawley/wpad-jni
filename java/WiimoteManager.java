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

public class WiimoteManager {	
	public static Wiimote connectWiimote(BluetoothDevice device) {
		if(device.getName().equals("Nintendo RVL-CNT-01")) {
			BluetoothSocket controlSocket = BluetoothHID.getControlSocket(device);
			BluetoothSocket interruptSocket = BluetoothHID.getInterruptSocket(device);
			if(controlSocket == null || interruptSocket == null)
				return null;
			try {
				controlSocket.connect();
			} catch (IOException e) {
				return null;
			}
			try {
				interruptSocket.connect();
			} catch (IOException e) {
				return null;
			}
			return new Wiimote(controlSocket, interruptSocket);
		}
		return null;
	}
}
