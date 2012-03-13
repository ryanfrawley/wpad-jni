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
import java.util.HashMap;
import java.util.Set;

import android.bluetooth.BluetoothSocket;

public class Wiimote implements IWiimote {
	private static final int OUTPUT = 0x52;
	private static final int OUTPUT_LEDS = 0x11;
	private static final int OUTPUT_RUMBLE = 0x11;
	private static final int OUTPUT_REQUEST_STATUS = 0x15;
	private static final int OUTPUT_WRITE_MEMORY = 0x16;
	private static final int OUTPUT_READ_MEMORY = 0x17;
	
	private static final int MEMORY_EEPROM = 0x0;
	private static final int MEMORY_REGISTERS = 0x4;
	
	private static final int MAX_PACKET_SIZE = 23;
	
	private static final int REPORT_BUTTONS = 0x30;
	private static final int REPORT_BUTTONS_EXT = 0x32;
	
	private static final byte[] EXTENSION_BYTES_NUNCHUCK = {0x00, 0x00, (byte)0xa4, 0x20, 0x00, 0x00};
	private static final byte[] EXTENSION_BYTES_CLASSIC = {0x00, 0x00, (byte)0xa4, 0x20, 0x01, 0x01};
	private static final byte[] EXTENSION_BYTES_GUITAR = {0x00, 0x00, (byte)0xa4, 0x20, 0x01, 0x03};
	private static final byte[] EXTENSION_BYTES_DRUMS = {0x01, 0x00, (byte)0xa4, 0x20, 0x01, 0x03};
	private static final byte[] EXTENSION_BYTES_DJ = {0x03, 0x00, (byte)0xa4, 0x20, 0x01, 0x03};
	private static final byte[] EXTENSION_BYTES_BALANCE_BOARD = {0x00, 0x00, (byte)0xa4, 0x20, 0x04, 0x02};
	private static final byte[] EXTENSION_BYTES_MOTION_PLUS = {0x00, 0x00, (byte)0xa4, 0x20, 0x04, 0x05};
	private static final byte[] EXTENSION_BYTES_MOTION_PLUS_NUNCHUCK = {0x00, 0x00, (byte)0xa4, 0x20, 0x05, 0x05};
	private static final byte[] EXTENSION_BYTES_MOTION_PLUS_CLASSIC = {0x00, 0x00, (byte)0xa4, 0x20, 0x07, 0x05};
	
	private static class ExtensionHashMap extends HashMap<byte[], Integer> {
		ExtensionHashMap() {
			put(EXTENSION_BYTES_NUNCHUCK, EXTENSION_NUNCHUCK);
			put(EXTENSION_BYTES_CLASSIC, EXTENSION_CLASSIC);
			put(EXTENSION_BYTES_GUITAR, EXTENSION_GUITAR);
			put(EXTENSION_BYTES_DRUMS, EXTENSION_DRUMS);
			put(EXTENSION_BYTES_DJ, EXTENSION_DJ);
			put(EXTENSION_BYTES_BALANCE_BOARD, EXTENSION_BALANCE_BOARD);
			put(EXTENSION_BYTES_MOTION_PLUS, EXTENSION_MOTION_PLUS);
			put(EXTENSION_BYTES_MOTION_PLUS_NUNCHUCK, null);
			put(EXTENSION_BYTES_MOTION_PLUS_CLASSIC, null);
		}
	};
	
	private static final ExtensionHashMap mExtensionMap = new ExtensionHashMap();
	
	private BluetoothSocket mOutput;
	private BluetoothSocket mInput;
	
	private int mLEDs;
	private int mRumble;
	private int mBattery;
	private int mButtons;
	private int mButtonsEx;
	private boolean mStatusRequest;
	private int mExtension;
	private int mReportMode;
	
	private WiimoteActionListener mListener;
	
	public Wiimote(BluetoothSocket output, BluetoothSocket input) {
		mOutput = output;
		mInput = input;
	}
	
	public void disconnect() {
		try {
			mOutput.close();
			mInput.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void setActionListener(WiimoteActionListener listener) {
		mListener = listener;
	}
	
	public void setLEDs(int leds) {
		byte buffer[] = {
				OUTPUT,
				OUTPUT_LEDS,
				(byte)(leds + mRumble)
		};
		send(buffer);
		mLEDs = leds;
	}
	
	public void setRumble(int rumble) {
		byte buffer[] = {
				OUTPUT,
				OUTPUT_RUMBLE,
				(byte)(mLEDs + rumble)
		};
		send(buffer);
		mRumble = rumble;
	}
	
	public void setReportMode(int reportMode) {
		byte buffer[] = {
				OUTPUT,
				0x12,
				0x00,
				(byte)reportMode
		};
		send(buffer);
		mReportMode = reportMode;
	}
	
	private void send(byte[] buffer) {
		try {
			mOutput.getOutputStream().write(buffer);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public int getBatteryLevel() {
		return mBattery;
	}
	
	public void requestStatus() {
		if(mStatusRequest)
			return;
		byte[] buffer = {
				OUTPUT,
				OUTPUT_REQUEST_STATUS,
				0x00
		};
		send(buffer);
	}
	
	public int getExtension() {
		return mExtension;
	}
	
	public void writeMemory(int address, int memoryBank, byte[] data, int size) {
		byte buffer[] = new byte[MAX_PACKET_SIZE];
		buffer[0] = OUTPUT;
		buffer[1] = OUTPUT_WRITE_MEMORY;
		buffer[2] = (byte)memoryBank;
		buffer[3] = (byte)(address >> 0x10);
		buffer[4] = (byte)(address >> 0x8);
		buffer[5] = (byte)address;
		buffer[6] = (byte)size;
		for(int i = 0; i < size; i++)
			buffer[7 + i] = data[i];
		for(int i = 7 + size; i < MAX_PACKET_SIZE; i++)
			buffer[i] = 0x0;
		send(buffer);
	}
	
	public void readMemory(int address, int memoryBank, int size) {
		byte buffer[] = {
				OUTPUT,
				OUTPUT_READ_MEMORY,
				(byte)memoryBank,
				(byte)(address >> 0x10),
				(byte)(address >> 0x8),
				(byte)address,
				(byte)(size >> 0x8),
				(byte)size
		};
		send(buffer);
	}
	
	public boolean update() {
		byte buffer[] = new byte[MAX_PACKET_SIZE];
		int length = 0;
		try {
			length = mInput.getInputStream().read(buffer);
		} catch (IOException e) {
			if(mListener != null)
				mListener.onDisconnect(this);
			return false;
		}
		if(length == 0) { // Controller disconnected
			try {
				mInput.close();
				mOutput.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			if(mListener != null)
				mListener.onDisconnect(this);
			return false;
		}
		switch(buffer[1]) {
		case 0x20: // Status report
			mButtons = ((buffer[2] & 0xff) << 8) + (buffer[3] & 0xff);
			mBattery = buffer[7] & 0xff;
			if(mListener != null)
				mListener.onBatteryLevelChanged(this, mBattery);
			int extension = buffer[4] & 0x2;
			if(extension > 0) {
				if(mExtension == EXTENSION_NONE) { // Extension was plugged in
					mReportMode = REPORT_BUTTONS_EXT;
					setReportMode(mReportMode);
					// Initialize extension
					byte[] b = {
							0x55
					};
					writeMemory(0xa400f0, MEMORY_REGISTERS, b, 1);
					try {
						Thread.sleep(50);
					} catch (InterruptedException e) {
					}
					b[0] = 0x00;
					writeMemory(0xa400fb, MEMORY_REGISTERS, b, 1);
					try {
						Thread.sleep(50);
					} catch (InterruptedException e) {
					}
					readMemory(0xa400fa, MEMORY_REGISTERS, 6);
				}
			} else if(mExtension != EXTENSION_NONE) { // Extension was unplugged
				mReportMode = REPORT_BUTTONS;
				setReportMode(mReportMode);
				mExtension = EXTENSION_NONE;
				mListener.onExtensionDisconnected(this);
			}
			break;
		case 0x21: // Read memory input
			int address = ((buffer[5] & 0xff) << 0x8) + (buffer[6] & 0xff); // Two least significant bytes of the address
			if(address == 0x00fa) { // Extension controller
				byte ext[] = new byte[6];
				for(int i = 0; i < 6; i++)
					ext[i] = buffer[i + 7];
				Set<byte[]> keys = mExtensionMap.keySet();
				for(byte[] key : keys) {
					boolean match = true;
					for(int i = 0; i < 6; i++) {
						if(key[i] != ext[i]) {
							match = false;
							break;
						}
					}
					if(match) {
						mExtension = mExtensionMap.get(key);
					}
				}
				if(mExtension == EXTENSION_NONE)
					mExtension = EXTENSION_UNKNOWN;
				mListener.onExtensionConnected(this, mExtension);
				mButtonsEx = 0;
			}
			break;
		case 0x22:
			break;
		case 0x30:
			int buttons = ((buffer[2] & 0xff) << 0x8) + (buffer[3] & 0xff);
			boolean pressed = false;
			if(buttons > mButtons)
				pressed = true;
			int mask = mButtons ^ buttons;
			mButtons = buttons;
			if(pressed)
				mListener.onButtonPressed(this, mask);
			else
				mListener.onButtonReleased(this, mask);
			break;
		case 0x31:
			break;
		case 0x32:
			int buttons1 = ((buffer[2] & 0xff) << 0x8) + (buffer[3] & 0xff);
			boolean pressed1 = false;
			boolean isExtensionEvent = false;
			if(buttons1 == mButtons)
				isExtensionEvent = true;
			if(isExtensionEvent) {
				switch(mExtension) {
				case EXTENSION_NUNCHUCK:
					buttons1 = buffer[9] & 0x3;
					break;
				case EXTENSION_CLASSIC:
				case EXTENSION_CLASSIC_PRO:
					int row1 = buffer[8] & 0xff;
					int row2 = buffer[9] & 0xff;
					buttons1 = 0;
					// Classic controllers have the bits set to 0 when pressed
					row1 = ~row1;
					row2 = ~row2;
					if((row1 & 0x2) > 0)
						buttons1 |= CLASSIC_BUTTON_RT;
					if((row1 & 0x4) > 0)
						buttons1 |= CLASSIC_BUTTON_PLUS;
					if((row1 & 0x8) > 0) 
						buttons1 |= CLASSIC_BUTTON_HOME;
					if((row1 & 0x10) > 0)
						buttons1 |= CLASSIC_BUTTON_MINUS;
					if((row1 & 0x20) > 0)
						buttons1 |= CLASSIC_BUTTON_LT;
					if((row1 & 0x40) > 0)
						buttons1 |= CLASSIC_BUTTON_DPAD_DOWN;
					if((row1 & 0x80) > 0)
						buttons1 |= CLASSIC_BUTTON_DPAD_RIGHT;
					if((row2 & 0x1) > 0)
						buttons1 |= CLASSIC_BUTTON_DPAD_UP;
					if((row2 & 0x2) > 0)
						buttons1 |= CLASSIC_BUTTON_DPAD_LEFT;
					if((row2 & 0x4) > 0)
						buttons1 |= CLASSIC_BUTTON_ZR;
					if((row2 & 0x8) > 0)
						buttons1 |= CLASSIC_BUTTON_X;
					if((row2 & 0x10) > 0)
						buttons1 |= CLASSIC_BUTTON_A;
					if((row2 & 0x20) > 0)
						buttons1 |= CLASSIC_BUTTON_Y;
					if((row2 & 0x40) > 0)
						buttons1 |= CLASSIC_BUTTON_B;
					if((row2 & 0x80) > 0)
						buttons1 |= CLASSIC_BUTTON_ZL;
				}
				if(buttons1 > mButtonsEx)
					pressed1 = true;
				int mask1 = mButtonsEx ^ buttons1;
				mButtonsEx = buttons1;
				if(pressed1)
					mListener.onButtonPressedEx(this, mask1);
				else
					mListener.onButtonReleasedEx(this, mask1);
			} else {
				if(buttons1 > mButtons)
					pressed1 = true;
				int mask1 = mButtons ^ buttons1;
				mButtons = buttons1;
				if(pressed1)
					mListener.onButtonPressed(this, mask1);
				else
					mListener.onButtonReleased(this, mask1);
			}
			break;
		case 0x33:
			break;
		case 0x34:
			break;
		case 0x35:
			break;
		case 0x36:
			break;
		case 0x37:
			break;
		case 0x38:
			break;
		case 0x39:
			break;
		case 0x3a:
			break;
		case 0x3b:
			break;
		case 0x3c:
			break;
		case 0x3d:
			break;
		case 0x3e:
			break;
		case 0x3f:
			break;
		default:
			break;
		}
		return true;
	}
}
