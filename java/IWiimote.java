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

public interface IWiimote {
	public static final int RUMBLE_ENABLE = 0x1;
	public static final int RUMBLE_DISBLE = 0x0;
	
	public static final int LED1 = 0x10;
	public static final int LED2 = 0x20;
	public static final int LED3 = 0x40;
	public static final int LED4 = 0x80;
	
	public static final int EXTENSION_UNKNOWN = -1;
	public static final int EXTENSION_NONE = 0;
	public static final int EXTENSION_NUNCHUCK = 1;
	public static final int EXTENSION_CLASSIC = 2;
	public static final int EXTENSION_CLASSIC_PRO = 3;
	public static final int EXTENSION_MOTION_PLUS = 4;
	public static final int EXTENSION_GUITAR = 5;
	public static final int EXTENSION_DRUMS = 6;
	public static final int EXTENSION_BALANCE_BOARD = 7;
	public static final int EXTENSION_DJ = 8;
	
	// Wiimote buttons
	public static final int BUTTON_DPAD_LEFT = 0x100;
	public static final int BUTTON_DPAD_RIGHT = 0x200;
	public static final int BUTTON_DPAD_DOWN = 0x400;
	public static final int BUTTON_DPAD_UP = 0x800;
	public static final int BUTTON_PLUS = 0x1000;
	public static final int BUTTON_TWO = 0x1;
	public static final int BUTTON_ONE = 0x2;
	public static final int BUTTON_B = 0x4;
	public static final int BUTTON_A = 0x8;
	public static final int BUTTON_MINUS = 0x10;
	public static final int BUTTON_HOME = 0x80;
	
	// Nunchuck buttons
	public static final int NUNCHUCK_BUTTON_C = 0x1;
	public static final int NUNCHUCK_BUTTON_Z = 0x2;
	
	// Classic controller buttons
	public static final int CLASSIC_BUTTON_X = 0x1;
	public static final int CLASSIC_BUTTON_Y = 0x2;
	public static final int CLASSIC_BUTTON_LT = 0x4;
	public static final int CLASSIC_BUTTON_RT = 0x8;
	public static final int CLASSIC_BUTTON_ZR = 0x10;
	public static final int CLASSIC_BUTTON_ZL = 0x20;
	public static final int CLASSIC_BUTTON_HOME = 0x40;
	public static final int CLASSIC_BUTTON_MINUS = 0x80;
	public static final int CLASSIC_BUTTON_PLUS = 0x100;
	public static final int CLASSIC_BUTTON_DPAD_UP = 0x200;
	public static final int CLASSIC_BUTTON_DPAD_DOWN = 0x400;
	public static final int CLASSIC_BUTTON_DPAD_LEFT = 0x800;
	public static final int CLASSIC_BUTTON_DPAD_RIGHT = 0x1000;
	public static final int CLASSIC_BUTTON_A = 0x2000;
	public static final int CLASSIC_BUTTON_B = 0x4000;
	
	public void setLEDs(int leds);
	public void setRumble(int rumble);
	public int getBatteryLevel();
	public int getExtension();
	
	public boolean update();
}
