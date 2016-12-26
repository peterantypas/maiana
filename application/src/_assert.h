/*
 * _assert.h
 *
 *  Created on: May 28, 2016
 *      Author: peter
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#include "globals.h"
#include <cassert>
#include "printf2.h"

#ifdef DEV_MODE
#define ASSERT(b) {if (!(b)) {printf2_now("\r\nAssertion failed at %s:%d\r\n", __FILE__, __LINE__); while(1);}}
#else
#define ASSERT(b) assert(b);
#endif


#endif /* ASSERT_H_ */
// Local Variables:
// mode: c++
// End:
