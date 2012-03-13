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

#include "Lock.h"

__WPAD_BEGIN_NAMESPACE_

Lock::Lock()
{
  pthread_mutex_init(&mutex, NULL);
}

Lock::~Lock()
{
  pthread_mutex_destroy(&mutex);
}

void Lock::lock()
{
  pthread_mutex_lock(&mutex);
}

void Lock::unlock()
{
  pthread_mutex_unlock(&mutex);
}

__WPAD_END_NAMESPACE_
