/*
 * TXErrors.h
 *
 *  Created on: Sep 18, 2021
 *      Author: peter
 */

#ifndef INC_TXERRORS_H_
#define INC_TXERRORS_H_

typedef enum
{
  TX_NO_ERROR = 0,
  TX_ALLOC_ERROR,
  TX_QUEUE_FULL,
  TX_PACKET_TOO_OLD,
} TXErrorCode;



#endif /* INC_TXERRORS_H_ */
