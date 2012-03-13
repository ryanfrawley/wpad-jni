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

#ifndef __WPAD_DEVICE_MANAGER_H_
#define __WPAD_DEVICE_MANAGER_H_

#include "wpad.h"
#include "Device.h"
#include "Lock.h"
#include <list>

__WPAD_BEGIN_NAMESPACE_

typedef enum
{
  WPAD_DEVICE_MANAGER_STATE_RUNNING,
  WPAD_DEVICE_MANAGER_STATE_SHUTDOWN
} WPAD_DEVICE_MANAGER_STATE;

typedef WPAD_RESULT (wpadEnumDevicesCallback_t)(bdaddr_t *address, void *param);

class DeviceManager
{
public:
  static WPAD_RESULT initialize();
  static WPAD_RESULT shutdown();
  static WPAD_RESULT connectDevice(bdaddr_t *address, Device **device);
  static WPAD_RESULT disconnectDevice(Device *device);
  static WPAD_RESULT enumDevices(wpadEnumDevicesCallback_t callback, int duration, void *param, int *deviceCount);
private:
  static void * update(void *param);
private:
  static std::list<Device *> deviceList;
  static fd_set masterSet;
  static fd_set workingSet;
  static pthread_t updateThread;
  static WPAD_DEVICE_MANAGER_STATE state;
  static Lock lock;
};

__WPAD_END_NAMESPACE_

#endif // __WPAD_DEVICE_MANAGER_H
