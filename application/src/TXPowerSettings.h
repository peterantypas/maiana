/*
 * power.h
 *
 *  Created on: May 8, 2016
 *      Author: peter
 */

#ifndef TXPOWERSETTINGS_H_
#define TXPOWERSETTINGS_H_



#define MAX_TX_LEVEL 33;
#define MIN_TX_LEVEL 24;

#include <inttypes.h>

/*
 * Table settings derived from Silabs AN900 with additional measurements using a dedicated RF power/SWR meter on a 50 Ohm dummy load.
 */

typedef enum {
    PWR_P33,
    PWR_P32,
    PWR_P30,
    PWR_P27,
    PWR_P24
}
tx_power_level;

typedef struct {
    tx_power_level level;       // For convenience
    uint8_t pa_mode;            // PA Mode
    uint8_t pa_level;           // PA power level (native parameter)
    uint8_t pa_bias_clkduty;    // PA bias clock duty
}
pa_params;



static const pa_params POWER_TABLE[] = {
        {PWR_P33, 0x08, 0x7f, 0x00},
        {PWR_P32, 0x08, 0x0e, 0x00},
        {PWR_P30, 0x09, 0x21, 0x21},
        {PWR_P27, 0x09, 0x1a, 0x1a},
        {PWR_P24, 0x09, 0x14, 0x15}
};


#endif /* TXPOWERSETTINGS_H_ */
