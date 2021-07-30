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


#ifndef CIRCULARQUEUE_HPP_
#define CIRCULARQUEUE_HPP_

#include <stdint.h>
#include <stddef.h>
#include "_assert.h"

template<typename T> class CircularQueue
{
public:

  CircularQueue(size_t size)
  {
    mReadPosition = 0;
    mWritePosition = 0;
    mSize = size;
    mBuffer = new T[mSize];
    ASSERT_VALID_PTR(mBuffer);
  }

  inline bool empty()
  {
    return mReadPosition == mWritePosition;
  }

  bool push(T &Element)
  {
    int nextElement = (mWritePosition + 1) % mSize;
    if ( isSafeToWrite(nextElement) )
      {
        mBuffer[mWritePosition] = Element;
        mWritePosition = nextElement;
        return true;
      }
    else
      return false;
  }

  bool pop(T &Element)
  {
    if ( empty() )
      return false;

    Element = mBuffer[mReadPosition];
    mReadPosition = (mReadPosition + 1) % mSize;
    return true;
  }


private:
  inline bool isSafeToWrite(int nextElement)
  {
    return nextElement != mReadPosition;
  }

private:
  volatile int mReadPosition;
  volatile int mWritePosition;
  size_t mSize;
  volatile T* mBuffer = nullptr;
};

#endif /* CIRCULARQUEUE_HPP_ */
