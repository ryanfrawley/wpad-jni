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

#ifndef __WPAD_DEVICE_H_
#define __WPAD_DEVICE_H_

#include "wpad.h"
#include "Lock.h"
#include <queue>

#define WPAD_CONTROL_CHANNEL_PSM 0x11
#define WPAD_INTERRUPT_CHANNEL_PSM 0x13
#define WPAD_INQUIRY_MAX_RSP 255
#define WPAD_NAME_LENGTH 32

#define WPAD_BUTTON_2 0x1
#define WPAD_BUTTON_1 0x2
#define WPAD_BUTTON_B 0x4
#define WPAD_BUTTON_A 0x8
#define WPAD_BUTTON_MINUS 0x10
#define WPAD_BUTTON_HOME 0x80
#define WPAD_BUTTON_LEFT 0x100
#define WPAD_BUTTON_RIGHT 0x200
#define WPAD_BUTTON_DOWN 0x400
#define WPAD_BUTTON_UP 0x800
#define WPAD_BUTTON_PLUS 0x1000

#define WPAD_LED1 0x10
#define WPAD_LED2 0x20
#define WPAD_LED3 0x40
#define WPAD_LED4 0x80

#define WPAD_OUTPUT 0x52
#define WPAD_OUTPUT_LEDS 0x11
#define WPAD_OUTPUT_RUMBLE 0x11
#define WPAD_OUTPUT_REPORT_MODE 0x12
#define WPAD_OUTPUT_IR_CAMERA_ENABLE 0x13
#define WPAD_OUTPUT_SPEAKER_ENABLE 0x14
#define WPAD_OUTPUT_STATUS_REQUEST 0x15
#define WPAD_OUTPUT_MEMORY_WRITE 0x16
#define WPAD_OUTPUT_MEMORY_READ 0x17
#define WPAD_OUTPUT_SPEAKER_WRITE 0x18
#define WPAD_OUTPUT_SPEAKER_MUTE 0x19
#define WPAD_OUTPUT_IR_CAMERA_ENABLE2 0x1a

#define WPAD_INPUT 0xa1
#define WPAD_INPUT_STATUS 0x20
#define WPAD_INPUT_MEMORY_READ 0x21
#define WPAD_INPUT_FUNCTION_RESULT 0x22

#define WPAD_MEMORY_EEPROM 0x0
#define WPAD_MEMORY_REGISTERS 0x4

#define WPAD_REPORT_TYPE_DEFAULT 0x0
#define WPAD_REPORT_TYPE_CONTINUOUS 0x4

#define WPAD_REPORT_MODE_DEFAULT 0x30

#define WPAD_LIBRARY_UNINITIALIZED 0
#define WPAD_LIBRARY_INITIALIZED 1

#define WPAD_DEVICE_INITIALIZING 1
#define WPAD_DEVICE_CONNECTED 2

#define WPAD_PACKET_SIZE 23

__WPAD_BEGIN_NAMESPACE_

static const uint8_t WPAD_EXTENSION_NUNCHUCK[] = {0x00, 0x00, 0xa4, 0x20, 0x00, 0x00};
static const uint8_t WPAD_EXTENSION_CLASSIC[] = {0x00, 0x00, 0xa4, 0x20, 0x01, 0x01};
static const uint8_t WPAD_EXTENSION_CLASSIC_PRO[] = {0x01, 0x00, 0xa4, 0x20, 0x01, 0x01};
static const uint8_t WPAD_EXTENSION_GUITAR[] = {0x00, 0x00, 0xa4, 0x20, 0x01, 0x03};
static const uint8_t WPAD_EXTENSION_DRUMS[] = {0x01, 0x00, 0xa4, 0x20, 0x01, 0x03};

// TODO: Possibly add async getX functions? (faster, but less accurate)

class Device
{
public:
  WPAD_RESULT disconnect();
  WPAD_RESULT send(uint8_t *buffer, int length);
  WPAD_RESULT setLEDs(int leds);
  //WPAD_RESULT getLEDs(int *leds);
  WPAD_RESULT setRumble(bool enabled);
  //WPAD_RESULT getRumble(bool *enabled);
  //WPAD_RESULT getButtons(int *buttons);
  //WPAD_RESULT setReportType(int reportType);
  //WPAD_RESULT getReportType(int *reportType);
  WPAD_RESULT setReportMode(int reportMode);
  WPAD_RESULT getReportMode(int *reportMode);
  WPAD_RESULT writeMemory(int address, int memoryBank, uint8_t *data, int size);
  WPAD_RESULT readMemory(int address, int memoryBank, uint8_t *data, int size);
private:
  Device();
  ~Device();
  WPAD_RESULT initializeExtension();
private:
  typedef enum
  {
    DISCONNECT,
    SEND
  } CommandFunction;

  struct Command
  {
    CommandFunction function;
    uint8_t buffer[WPAD_PACKET_SIZE];
    int length;
  };

  friend class DeviceManager;
  int controlSocket;
  int interruptSocket;
  int leds;
  int buttons;
  int rumble;
  int reportType;
  int reportMode;
  int extension;
  bdaddr_t address;
  uint8_t buffer[WPAD_PACKET_SIZE];
  std::queue<Command> commandQueue;
  Lock lock;
};

extern int wpad_init_extension(Device *device);

__WPAD_END_NAMESPACE_

#endif // __WPAD_DEVICE_H_
