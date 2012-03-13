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

#ifndef WPAD_ERROR_H_
#define WPAD_ERROR_H_

#include "WPAD.h"

__WPAD_BEGIN_NAMESPACE_

typedef enum
{
  WPAD_SUCCESS,
  WPAD_FAILURE,
  WPAD_INVALID_PARAMETER,
  WPAD_INVALID_CALL,
  WPAD_SOCKET_ERROR,
  WPAD_CONTROL_SOCKET_ERROR,
  WPAD_INTERRUPT_SOCKET_ERROR,
  WPAD_DEVICE_ERROR,
  WPAD_HCI_ERROR
} WPAD_RESULT;

__WPAD_END_NAMESPACE_

#endif // WPAD_ERROR_H_
