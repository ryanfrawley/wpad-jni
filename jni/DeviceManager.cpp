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

#include "DeviceManager.h"
#include "wpad-jni.h"
#include "com_mercury_wpad_DeviceManager.h"

__WPAD_BEGIN_NAMESPACE_

static const char WPAD_DEVICE_NAME[] = "Nintendo RVL-CNT-01";
static const uint8_t WPAD_DEVICE_CLASS[] = {0x04, 0x25, 0x00};

std::list<Device *> DeviceManager::deviceList;
fd_set DeviceManager::masterSet;
fd_set DeviceManager::workingSet;
pthread_t DeviceManager::updateThread;
WPAD_DEVICE_MANAGER_STATE DeviceManager::state;
Lock DeviceManager::lock;

WPAD_RESULT DeviceManager::initialize()
{
  lock.lock();
  if(updateThread)
  {
    lock.unlock();
    return WPAD_INVALID_CALL;
  }
  state = WPAD_DEVICE_MANAGER_STATE_RUNNING;
  pthread_create(&updateThread, NULL, &DeviceManager::update, NULL);
  lock.unlock();
  return WPAD_SUCCESS;
}

WPAD_RESULT DeviceManager::shutdown()
{
  lock.lock();
  if(!updateThread)
  {
    lock.unlock();
    return WPAD_INVALID_CALL;
  }
  state = WPAD_DEVICE_MANAGER_STATE_SHUTDOWN;
  for(std::list<Device *>::const_iterator it = deviceList.begin(); it != deviceList.end(); ++it)
    (*it)->disconnect();
  lock.unlock();
  pthread_join(updateThread, NULL);
  lock.lock();
  updateThread = NULL;
  lock.unlock();
  return WPAD_SUCCESS;
}

WPAD_RESULT DeviceManager::connectDevice(bdaddr_t *address, Device **device)
{
  if(!address || !device)
    return WPAD_INVALID_PARAMETER;

  for(std::list<Device *>::const_iterator it = deviceList.begin(); it != deviceList.end(); ++it)
  {
    if(memcmp(&(*it)->address, address, sizeof(bdaddr_t)) == 0)
      return WPAD_INVALID_PARAMETER;
  }

  // Control socket
  int controlSocket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  if(controlSocket < 0)
    return WPAD_SOCKET_ERROR;

  sockaddr_l2 l2addr;
  l2addr.l2_family = AF_BLUETOOTH;
  bacpy(&l2addr.l2_bdaddr, address);
  l2addr.l2_psm = htobs(WPAD_CONTROL_CHANNEL_PSM);
  if(connect(controlSocket, (struct sockaddr *)&l2addr, sizeof(l2addr)) < 0)
  {
    close(controlSocket);
    return WPAD_CONTROL_SOCKET_ERROR;
  }

  // Interrupt socket
  int interruptSocket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  if(interruptSocket < 0)
  {
    close(controlSocket);
    return WPAD_SOCKET_ERROR;
  }
  l2addr.l2_family = AF_BLUETOOTH;
  l2addr.l2_psm = htobs(WPAD_INTERRUPT_CHANNEL_PSM);
  if(connect(interruptSocket, (struct sockaddr *)&l2addr, sizeof(l2addr)) < 0)
  {
    close(controlSocket);
    close(interruptSocket);
    return WPAD_INTERRUPT_SOCKET_ERROR;
  }

  Device *controller = new Device();
  controller->controlSocket = controlSocket;
  controller->interruptSocket = interruptSocket;
  controller->reportType = WPAD_REPORT_TYPE_DEFAULT;
  controller->reportMode = WPAD_REPORT_MODE_DEFAULT;
  FD_SET(controller->interruptSocket, &masterSet);
  lock.lock();
  deviceList.push_front(controller);
  lock.unlock();
  *device = controller;
  return WPAD_SUCCESS;
}

WPAD_RESULT DeviceManager::disconnectDevice(Device *device)
{
  return device->disconnect();
}

WPAD_RESULT DeviceManager::enumDevices(wpadEnumDevicesCallback_t callback, int duration, void *param, int *deviceCount)
{
  int deviceId = hci_get_route(NULL);
  if(deviceId < 0)
    return WPAD_HCI_ERROR;
  int sock = hci_open_dev(deviceId);
  if(sock < 0)
	  return WPAD_SOCKET_ERROR;
  inquiry_info *inqInfo = (inquiry_info *)malloc(WPAD_INQUIRY_MAX_RSP * sizeof(inquiry_info));
  int count = hci_inquiry(deviceId, duration, WPAD_INQUIRY_MAX_RSP, NULL, &inqInfo, IREQ_CACHE_FLUSH);
  if(count < 0)
    return WPAD_HCI_ERROR;
  for(int i = 0; i < count; i++)
  {
    bdaddr_t address;
    sockaddr_l2 l2addr;
    if(memcmp(&inqInfo[i].dev_class[0], WPAD_DEVICE_CLASS, sizeof(WPAD_DEVICE_CLASS)) != 0)
		  continue;
#ifdef WPAD_CHECK_DEVICE_NAME
    char name[WPAD_NAME_LENGTH];
	  memset(name, 0, sizeof(name));
	  if(hci_read_remote_name(sock, &inqInfo[i].bdaddr, sizeof(name), name, 0) < 0)
		  strcpy(name, "unknown");
	  if(strncmp(WPAD_DEVICE_NAME, name, strlen(WPAD_DEVICE_NAME)) != 0)
		  continue;
#endif // WPAD_CHECK_DEVICE_NAME
    bacpy(&address, &inqInfo[i].bdaddr);
    if(callback(&address, param) != WPAD_SUCCESS)
    {
      break;
    }
  }
  free(inqInfo);
  if(deviceCount)
    *deviceCount = count;
  return WPAD_SUCCESS;
}

void * DeviceManager::update(void *param)
{
  bool terminate = false;
  for(;;)
  {
    lock.lock();
    if(state != WPAD_DEVICE_MANAGER_STATE_RUNNING)
      terminate = true;
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    memcpy(&workingSet, &masterSet, sizeof(masterSet));
    int count = select(FD_SETSIZE, &workingSet, NULL, NULL, &tv);
    for(std::list<Device *>::const_iterator it = deviceList.begin(); it != deviceList.end();)
    {
      bool disconnect = false;
      Device *device = *it;
      device->lock.lock();
      if(!terminate)
      {
        if(device->commandQueue.size() > 0)
        {
          Device::Command command = device->commandQueue.front();
          device->commandQueue.pop();
          switch(command.function)
          {
          case Device::SEND:
            send(device->controlSocket, command.buffer, command.length, NULL);
            usleep(20000);
            break;
          case Device::DISCONNECT:
            disconnect = true;
            break;
          }
        }
        if(FD_ISSET(device->interruptSocket, &workingSet))
        {
          int result = recv(device->interruptSocket, device->buffer, sizeof(device->buffer), 0);
          if(result == -1 || result == 0)
            disconnect = true;
          else
          {
            switch(device->buffer[1])
            {
            case 0x30:
              {
                jobject obj = NULL;
                for(std::map<jobject, Device *>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                {
                  if(it->second == device)
                  {
                    obj = it->first;
                    break;
                  }
                }
                if(obj)
                {
                  JNIEnv *env;
                  jvm->AttachCurrentThread(&env, NULL);
                  int buttons = (device->buffer[2] << 8) + device->buffer[3];
                  if((buttons & WPAD_BUTTON_2) && !(device->buttons & WPAD_BUTTON_2))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_2, 1);
                  if((buttons & WPAD_BUTTON_1) && !(device->buttons & WPAD_BUTTON_1))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_1, 1);
                  if((buttons & WPAD_BUTTON_B) && !(device->buttons & WPAD_BUTTON_B))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_B, 1);
                  if((buttons & WPAD_BUTTON_A) && !(device->buttons & WPAD_BUTTON_A))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_A, 1);
                  if((buttons & WPAD_BUTTON_MINUS) && !(device->buttons & WPAD_BUTTON_MINUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_MINUS, 1);
                  if((buttons & WPAD_BUTTON_HOME) && !(device->buttons & WPAD_BUTTON_HOME))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_HOME, 1);
                  if((buttons & WPAD_BUTTON_LEFT) && !(device->buttons & WPAD_BUTTON_LEFT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_LEFT, 1);
                  if((buttons & WPAD_BUTTON_RIGHT) && !(device->buttons & WPAD_BUTTON_RIGHT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_RIGHT, 1);
                  if((buttons & WPAD_BUTTON_DOWN) && !(device->buttons & WPAD_BUTTON_DOWN))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_DOWN, 1);
                  if((buttons & WPAD_BUTTON_UP) && !(device->buttons & WPAD_BUTTON_UP))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_UP, 1);
                  if((buttons & WPAD_BUTTON_PLUS) && !(device->buttons & WPAD_BUTTON_PLUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_PLUS, 1);
                  
                  if(!(buttons & WPAD_BUTTON_2) && (device->buttons & WPAD_BUTTON_2))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_2, 0);
                  if(!(buttons & WPAD_BUTTON_1) && (device->buttons & WPAD_BUTTON_1))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_1, 0);
                  if(!(buttons & WPAD_BUTTON_B) && (device->buttons & WPAD_BUTTON_B))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_B, 0);
                  if(!(buttons & WPAD_BUTTON_A) && (device->buttons & WPAD_BUTTON_A))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_A, 0);
                  if(!(buttons & WPAD_BUTTON_MINUS) && (device->buttons & WPAD_BUTTON_MINUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_MINUS, 0);
                  if(!(buttons & WPAD_BUTTON_HOME) && (device->buttons & WPAD_BUTTON_HOME))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_HOME, 0);
                  if(!(buttons & WPAD_BUTTON_LEFT) && (device->buttons & WPAD_BUTTON_LEFT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_LEFT, 0);
                  if(!(buttons & WPAD_BUTTON_RIGHT) && (device->buttons & WPAD_BUTTON_RIGHT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_RIGHT, 0);
                  if(!(buttons & WPAD_BUTTON_DOWN) && (device->buttons & WPAD_BUTTON_DOWN))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_DOWN, 0);
                  if(!(buttons & WPAD_BUTTON_UP) && (device->buttons & WPAD_BUTTON_UP))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_UP, 0);
                  if(!(buttons & WPAD_BUTTON_PLUS) && (device->buttons & WPAD_BUTTON_PLUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_PLUS, 0);
                  jvm->DetachCurrentThread();
                  device->buttons = buttons;
                }
              //device->buttons = (device->buffer[2] << 8) + device->buffer[3];
              //LOGE("Device(0x%x) buttons changed to: 0x%x", device, device->buttons);
              }
              break;
            case 0x32:
              {
                jobject obj = NULL;
                for(std::map<jobject, Device *>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                {
                  if(it->second == device)
                  {
                    obj = it->first;
                    break;
                  }
                }
                if(obj)
                {
                  JNIEnv *env;
                  jvm->AttachCurrentThread(&env, NULL);
                  static int xlast = 0;
                  static int ylast = 0;
                  if(device->extension == 1)
                  {
                    int x = device->buffer[4];
                    int y = device->buffer[5];
                    if(xlast != x || ylast != y)
                    {
                      env->CallVoidMethod(obj, onButtonMethod, 0x5000, x);
                      env->CallVoidMethod(obj, onButtonMethod, 0x5001, y);
                      xlast = x;
                      ylast = y;
                    }
				          }
                  int buttons = (device->buffer[2] << 8) + device->buffer[3];
                  if((buttons & WPAD_BUTTON_2) && !(device->buttons & WPAD_BUTTON_2))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_2, 1);
                  if((buttons & WPAD_BUTTON_1) && !(device->buttons & WPAD_BUTTON_1))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_1, 1);
                  if((buttons & WPAD_BUTTON_B) && !(device->buttons & WPAD_BUTTON_B))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_B, 1);
                  if((buttons & WPAD_BUTTON_A) && !(device->buttons & WPAD_BUTTON_A))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_A, 1);
                  if((buttons & WPAD_BUTTON_MINUS) && !(device->buttons & WPAD_BUTTON_MINUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_MINUS, 1);
                  if((buttons & WPAD_BUTTON_HOME) && !(device->buttons & WPAD_BUTTON_HOME))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_HOME, 1);
                  if((buttons & WPAD_BUTTON_LEFT) && !(device->buttons & WPAD_BUTTON_LEFT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_LEFT, 1);
                  if((buttons & WPAD_BUTTON_RIGHT) && !(device->buttons & WPAD_BUTTON_RIGHT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_RIGHT, 1);
                  if((buttons & WPAD_BUTTON_DOWN) && !(device->buttons & WPAD_BUTTON_DOWN))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_DOWN, 1);
                  if((buttons & WPAD_BUTTON_UP) && !(device->buttons & WPAD_BUTTON_UP))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_UP, 1);
                  if((buttons & WPAD_BUTTON_PLUS) && !(device->buttons & WPAD_BUTTON_PLUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_PLUS, 1);
                  
                  if(!(buttons & WPAD_BUTTON_2) && (device->buttons & WPAD_BUTTON_2))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_2, 0);
                  if(!(buttons & WPAD_BUTTON_1) && (device->buttons & WPAD_BUTTON_1))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_1, 0);
                  if(!(buttons & WPAD_BUTTON_B) && (device->buttons & WPAD_BUTTON_B))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_B, 0);
                  if(!(buttons & WPAD_BUTTON_A) && (device->buttons & WPAD_BUTTON_A))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_A, 0);
                  if(!(buttons & WPAD_BUTTON_MINUS) && (device->buttons & WPAD_BUTTON_MINUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_MINUS, 0);
                  if(!(buttons & WPAD_BUTTON_HOME) && (device->buttons & WPAD_BUTTON_HOME))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_HOME, 0);
                  if(!(buttons & WPAD_BUTTON_LEFT) && (device->buttons & WPAD_BUTTON_LEFT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_LEFT, 0);
                  if(!(buttons & WPAD_BUTTON_RIGHT) && (device->buttons & WPAD_BUTTON_RIGHT))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_RIGHT, 0);
                  if(!(buttons & WPAD_BUTTON_DOWN) && (device->buttons & WPAD_BUTTON_DOWN))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_DOWN, 0);
                  if(!(buttons & WPAD_BUTTON_UP) && (device->buttons & WPAD_BUTTON_UP))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_UP, 0);
                  if(!(buttons & WPAD_BUTTON_PLUS) && (device->buttons & WPAD_BUTTON_PLUS))
                    env->CallVoidMethod(obj, onButtonMethod, 0, WPAD_BUTTON_PLUS, 0);

				          if(device->extension == 2)
				          {
					          static bool cA = false;
					          static bool cB = false;
					          static bool cX = false;
					          static bool cY = false;
					          static bool cUp = false;
					          static bool cDown = false;
					          static bool cLeft = false;
					          static bool cRight = false;
					          static bool cSelect = false;
					          static bool cHome = false;
					          static bool cStart = false;
					          static bool cL = false;
					          static bool cR = false;
					          static bool cZL = false;
					          static bool cZR = false;

					          if(!cA && !(device->buffer[9] & 0x10))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 1, 1);
					          if(!cB && !(device->buffer[9] & 0x40))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 2, 1);
					          if(!cX && !(device->buffer[9] & 0x8))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 3, 1);
					          if(!cY && !(device->buffer[9] & 0x20))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 4, 1);
					          if(!cUp && !(device->buffer[9] & 0x1))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 5, 1);
					          if(!cDown && !(device->buffer[8] & 0x40))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 6, 1);
					          if(!cLeft && !(device->buffer[9] & 0x2))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 7, 1);
					          if(!cRight && !(device->buffer[8] & 0x80))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 8, 1);
					          if(!cSelect && !(device->buffer[8] & 0x10))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 9, 1);
					          if(!cHome && !(device->buffer[8] & 0x8))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 10, 1);
					          if(!cStart && !(device->buffer[8] & 0x4))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 11, 1);
					          if(!cL && !(device->buffer[8] & 0x20))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 12, 1);
					          if(!cR && !(device->buffer[8] & 0x2))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 13, 1);
					          if(!cZL && !(device->buffer[9] & 0x80))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 14, 1);
					          if(!cZR && !(device->buffer[9] & 0x4))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 15, 1);

					          if(cA && (device->buffer[9] & 0x10))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 1, 0);
					          if(cB && (device->buffer[9] & 0x40))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 2, 0);
					          if(cX && (device->buffer[9] & 0x8))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 3, 0);
					          if(cY && (device->buffer[9] & 0x20))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 4, 0);
					          if(cUp && (device->buffer[9] & 0x1))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 5, 0);
					          if(cDown && (device->buffer[8] & 0x40))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 6, 0);
					          if(cLeft && (device->buffer[9] & 0x2))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 7, 0);
					          if(cRight && (device->buffer[8] & 0x80))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 8, 0);
					          if(cSelect && (device->buffer[8] & 0x10))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 9, 0);
					          if(cHome && (device->buffer[8] & 0x8))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 10, 0);
					          if(cStart && (device->buffer[8] & 0x4))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 11, 0);
					          if(cL && (device->buffer[8] & 0x20))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 12, 0);
					          if(cR && (device->buffer[8] & 0x2))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 13, 0);
					          if(cZL && (device->buffer[9] & 0x80))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 14, 0);
					          if(cZR && (device->buffer[9] & 0x4))
						          env->CallVoidMethod(obj, onButtonMethod, 2, 15, 0);

					          cA = !(device->buffer[9] & 0x10);
					          cB = !(device->buffer[9] & 0x40);
					          cX = !(device->buffer[9] & 0x8);
					          cY = !(device->buffer[9] & 0x20);
					          cUp = !(device->buffer[9] & 0x1);
					          cDown = !(device->buffer[8] & 0x40);
					          cLeft = !(device->buffer[9] & 0x2);
					          cRight = !(device->buffer[8] & 0x80);
					          cSelect = !(device->buffer[8] & 0x10);
					          cHome = !(device->buffer[8] & 0x8);
					          cStart = !(device->buffer[8] & 0x4);
					          cL = !(device->buffer[8] & 0x20);
					          cR = !(device->buffer[8] & 0x2);
					          cZL = !(device->buffer[9] & 0x80);
					          cZR = !(device->buffer[9] & 0x4);
				          }

                  jvm->DetachCurrentThread();
                  device->buttons = buttons;
                }
              //device->buttons = (device->buffer[2] << 8) + device->buffer[3];
              //LOGE("Device(0x%x) buttons changed to: 0x%x", device, device->buttons);
              }
              break;
            case 0x20:
              device->lock.unlock();
              device->setReportMode(device->buffer[4] & 0x2 ? 0x32 : 0x30);
              if(device->buffer[4] & 0x2) {
                wpad_init_extension(device);
                device->lock.lock();
              }
              else
              {
                device->lock.lock();
                jobject obj = NULL;
                for(std::map<jobject, Device *>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                {
                  if(it->second == device)
                  {
                    obj = it->first;
                    break;
                  }
                }
                if(obj)
                {
                  JNIEnv *env;
                  jvm->AttachCurrentThread(&env, NULL);
                  env->CallVoidMethod(obj, onExtensionMethod, -1);
                  jvm->DetachCurrentThread();
                }
              }
              break;
            case 0x21:
              switch((device->buffer[5] << 8) + device->buffer[6])
              {
              case 0x00fa:
                {
                  for(int i = 0; i < 6; i++)
                    LOGE("0x%x", device->buffer[i + 7]);
                  if(memcmp(&device->buffer[7], &WPAD_EXTENSION_NUNCHUCK, 6) == 0)
                    device->extension = 1;
                  else if(memcmp(&device->buffer[7], &WPAD_EXTENSION_CLASSIC, 6) == 0)
                    device->extension = 2;
				  else if(memcmp(&device->buffer[7], &WPAD_EXTENSION_CLASSIC_PRO, 6) == 0)
                    device->extension = 2;
                  else if(memcmp(&device->buffer[7], &WPAD_EXTENSION_GUITAR, 6) == 0)
                    device->extension = 3;
                  else if(memcmp(&device->buffer[7], &WPAD_EXTENSION_DRUMS, 6) == 0)
                    device->extension = 4;
                  else
                    device->extension = 0;
                  jobject obj = NULL;
                  for(std::map<jobject, Device *>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                  {
                    if(it->second == device)
                    {
                      obj = it->first;
                      break;
                    }
                  }
                  if(obj)
                  {
                    JNIEnv *env;
                    jvm->AttachCurrentThread(&env, NULL);
                    env->CallVoidMethod(obj, onExtensionMethod, device->extension);
                    jvm->DetachCurrentThread();
                  }
                }
                break;
              }
              break;
            default:
              LOGE("Unimplemented packet 0x%x", device->buffer[1]);
              break;
            }
          }
          count--;
        }
      }
      if(disconnect || terminate)
      {
        FD_CLR(device->interruptSocket, &masterSet);
        close(device->controlSocket);
        close(device->interruptSocket);
        //LOGE("Disconnected device(0x%x)", device);
        jobject obj = NULL;
        for(std::map<jobject, Device *>::const_iterator it = devices.begin(); it != devices.end(); ++it)
        {
          if(it->second == device)
          {
            obj = it->first;
            break;
          }
        }
        if(obj) {
          JNIEnv *env;
          jvm->AttachCurrentThread(&env, NULL);
          env->CallVoidMethod(obj, onDisconnectMethod);
          jvm->DetachCurrentThread();
        }
        std::list<Device *>::const_iterator next = it;
        next++;
        device->lock.unlock();
        deviceList.remove(*it);
        it = next;
        delete device;
        continue;
      }
      device->lock.unlock();
      ++it;
    }
    lock.unlock();
    if(terminate)
      break;
    sched_yield();
  }
  return NULL;
}

__WPAD_END_NAMESPACE_
