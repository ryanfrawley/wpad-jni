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

#ifndef __WPAD_LOCK_H_
#define __WPAD_LOCK_H_

#include "wpad.h"

__WPAD_BEGIN_NAMESPACE_

class Lock
{
public:
  Lock();
  ~Lock();
  void lock();
  void unlock();
private:
  pthread_mutex_t mutex;
};

__WPAD_END_NAMESPACE_

#endif // __WPAD_LOCK_H_
