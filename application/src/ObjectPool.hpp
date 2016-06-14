/*
 * ObjectPool.hpp
 *
 *  Created on: Mar 5, 2016
 *      Author: peter
 */

#ifndef OBJECTPOOL_HPP_
#define OBJECTPOOL_HPP_

using namespace std;

#include <cstdlib>
#include <cassert>
#include "printf2.h"
#include "_assert.h"


template<typename T> class ObjectPool
{
public:

    ObjectPool<T>(uint32_t size)
    {
        mSize = size;
        mPool = new T[size];
        mState = new bool[size];
        for ( uint32_t i = 0; i < mSize; ++i ) {
            mState[i] = true; // Available
            ASSERT(&mPool[i]);
        }

        printf2("Initialized pool of size %d\r\n", mSize);
    }

    T *get()
    {
        for ( uint32_t i = 0; i < mSize; ++i ) {
            if ( mState[i] ) {
                mState[i] = false;
                return &mPool[i];
            }
        }

        return NULL;
    }

    void put(T* o)
    {
        for ( uint32_t i = 0; i < mSize; ++i ) {
            if ( o == &mPool[i] ) {
                ASSERT(!mState[i]);
                mState[i] = true;
                return;
            }
        }

        ASSERT(false);
    }


private:
    uint32_t mSize;
    T*        mPool;
    bool*     mState;    // Not ideal. How about a bit vector instead?
};



#endif /* OBJECTPOOL_HPP_ */
