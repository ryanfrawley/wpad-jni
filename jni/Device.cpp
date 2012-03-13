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

#include "Device.h"

__WPAD_BEGIN_NAMESPACE_

Device::Device():
  controlSocket(NULL),
  interruptSocket(NULL),
  leds(0),
  buttons(0),
  rumble(0),
  reportType(WPAD_REPORT_TYPE_DEFAULT),
  reportMode(WPAD_REPORT_MODE_DEFAULT)
{
  memset(buffer, 0, sizeof(buffer));
  memcpy(&address, BDADDR_ANY, sizeof(BDADDR_ANY));
}

Device::~Device()
{
}

WPAD_RESULT Device::disconnect()
{
  if(!controlSocket || !interruptSocket)
    return WPAD_INVALID_CALL;
  Command command;
  command.function = DISCONNECT;
  lock.lock();
  commandQueue.push(command);
  lock.unlock();
  return WPAD_SUCCESS;
}

WPAD_RESULT Device::send(uint8_t *buffer, int length)
{
  if(!buffer || length <= 0)
    return WPAD_INVALID_PARAMETER;
  Command command;
  command.function = SEND;
  memcpy(&command.buffer, buffer, length);
  command.length = length;
  lock.lock();
  commandQueue.push(command);
  lock.unlock();
  return WPAD_SUCCESS;
}

WPAD_RESULT Device::setLEDs(int leds)
{
  lock.lock();
  this->leds = leds & (WPAD_LED1 | WPAD_LED2 | WPAD_LED3 | WPAD_LED4);
  uint8_t buffer[3];
  buffer[0] = WPAD_OUTPUT;
  buffer[1] = WPAD_OUTPUT_LEDS;
  buffer[2] = this->leds + rumble;
  lock.unlock();
  if(send(buffer, sizeof(buffer)) != WPAD_SUCCESS)
    return WPAD_FAILURE;
  return WPAD_SUCCESS;
}

WPAD_RESULT Device::setRumble(bool rumble)
{
  lock.lock();
  this->rumble = rumble;
  uint8_t buffer[3];
  buffer[0] = WPAD_OUTPUT;
  buffer[1] = WPAD_OUTPUT_RUMBLE;
  buffer[2] = leds + this->rumble;
  lock.unlock();
  if(send(buffer, sizeof(buffer)) != WPAD_SUCCESS)
    return WPAD_FAILURE;
  return WPAD_SUCCESS;
}

WPAD_RESULT Device::writeMemory(int address, int memoryBank, uint8_t *data, int size)
{
  if(!data || size <= 0)
    return WPAD_INVALID_PARAMETER;
  uint8_t buffer[WPAD_PACKET_SIZE];
  buffer[0] = WPAD_OUTPUT;
  buffer[1] = WPAD_OUTPUT_MEMORY_WRITE;
  buffer[2] = memoryBank;
  buffer[3] = (address >> 16) & 0xff;
  buffer[4] = 0x00; // The Wii remote ignores this byte
  buffer[5] = address & 0xff;
  buffer[6] = size;
  memcpy(&buffer[7], data, size);
  memset(&buffer[7] + size, 0, sizeof(buffer) - size - 7);
  if(send(buffer, sizeof(buffer)) != WPAD_SUCCESS)
    return WPAD_FAILURE;
  return WPAD_SUCCESS;
}

WPAD_RESULT Device::readMemory(int address, int memoryBank, uint8_t *data, int size)
{
  if(!data || size <= 0)
    return WPAD_INVALID_PARAMETER;
  uint8_t buffer[8];
  buffer[0] = WPAD_OUTPUT;
  buffer[1] = WPAD_OUTPUT_MEMORY_READ;
  buffer[2] = memoryBank;
  buffer[3] = (address >> 16) & 0xff;
  buffer[4] = 0x00; // The Wii remote ignores this byte
  buffer[5] = address & 0xff;
  buffer[6] = (size >> 8) & 0xff;
  buffer[7] = size & 0xff;
  if(send(buffer, sizeof(buffer)) != WPAD_SUCCESS)
    return WPAD_FAILURE;
  return WPAD_SUCCESS;
}

int wpad_init_extension(Device *device)
{
  uint8_t buffer[6];
  buffer[0] = 0x55;
  device->writeMemory(0xa400f0, WPAD_MEMORY_REGISTERS, buffer, 1);
  buffer[0] = 0x00;
  device->writeMemory(0xa400fb, WPAD_MEMORY_REGISTERS, buffer, 1);
  device->readMemory(0xa400fa, WPAD_MEMORY_REGISTERS, buffer, 6);
}

WPAD_RESULT Device::setReportMode(int reportMode)
{
  lock.lock();
  if(this->reportMode == reportMode)
  {
    lock.unlock();
    return WPAD_SUCCESS;
  }
  this->reportMode = reportMode;
  uint8_t buffer[4];
  buffer[0] = WPAD_OUTPUT;
  buffer[1] = WPAD_OUTPUT_REPORT_MODE;
  buffer[2] = reportType;
  buffer[3] = this->reportMode;
  lock.unlock();
  send(buffer, sizeof(buffer));
  return WPAD_SUCCESS;
}
/*
void debug_print_buffer(uint8_t *buffer, int size)
{
  char buff[1024] = {0};
  char temp[128];
  int i;
  for(i = 0; i < size; i++)
  {
    sprintf(temp, "0x%x ", buffer[i]);
    strcat(buff, temp);
  }
  LOGE("Buffer: %s", buff);
}*/

__WPAD_END_NAMESPACE_
