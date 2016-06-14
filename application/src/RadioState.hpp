/*
 * RadioState.hpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#ifndef RADIOSTATE_HPP_
#define RADIOSTATE_HPP_


typedef enum {
    RADIO_RECEIVING,
    RADIO_TRANSMITTING
} RadioState;

// By default, we are in receiving mode
static volatile RadioState gRadioState = RADIO_RECEIVING;

#endif /* RADIOSTATE_HPP_ */
