/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
*/


#ifndef OBJECTPOOL_HPP_
#define OBJECTPOOL_HPP_

using namespace std;


#include "printf_serial.h"
#include "_assert.h"
#include "CircularQueue.hpp"
#include "Utils.hpp"


template<typename T> class ObjectPool
{
public:

  ObjectPool<T>(uint32_t size)
    : mQueue(size)
  {
    mSize = size;
    mUtilization = 0;
    mMaxUtilization = 0;

    //printf_serial_now("ObjectPool @%p\r\n", this);
    for ( uint32_t i = 0; i < mSize; ++i )
      {
        T *p = new T();
        ASSERT_VALID_PTR(p);
        mQueue.push(p);
        //printf_serial_now("\t@%p\r\n", p);
      }
  }

  T *get()
  {
    T *result = nullptr;
    mQueue.pop(result);
    return result;
  }

  void put(T* o)
  {
    mQueue.push(o);
  }

  uint32_t maxUtilization()
  {
    return mMaxUtilization;
  }

  uint32_t utilization()
  {
    return mUtilization;
  }

  uint32_t size()
  {
    return mSize;
  }

private:
  uint32_t          mSize;
  uint32_t          mUtilization;
  uint32_t          mMaxUtilization;
  CircularQueue<T*> mQueue;
};

#endif /* OBJECTPOOL_HPP_ */
