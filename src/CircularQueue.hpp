/*
 * CircularQueue.hpp
 *
 *  Created on: Mar 5, 2016
 *      Author: peter
 */

#ifndef CIRCULARQUEUE_HPP_
#define CIRCULARQUEUE_HPP_


#include "_assert.h"

//#define NUMQELEMENTS 100

template<typename T> class CircularQueue
{
public:
    CircularQueue(size_t size)
    {
        mReadPosition = 0;
        mWritePosition = 0;
        mSize = size;
        mBuffer = new T[mSize];
    }

    bool empty()
    {
        return mReadPosition == mWritePosition;
    }

    bool push(T &Element)
    {
        int nextElement = (mWritePosition + 1) % mSize;
        if ( isSafeToWrite(nextElement) ) {
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

        int nextElement = (mReadPosition + 1) % mSize;

        Element = mBuffer[mReadPosition];
        mReadPosition = nextElement;
        return true;
    }

    T top()
    {
        ASSERT(!empty());
        return mBuffer[mReadPosition];
    }

private:
    bool isSafeToWrite(int nextElement)
    {
        return nextElement != mReadPosition;
    }

private:
    volatile int mReadPosition;
    volatile int mWritePosition;
    size_t mSize;
    T* mBuffer;
};

#endif /* CIRCULARQUEUE_HPP_ */
