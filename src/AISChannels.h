/*
 * channels.h
 *
 *  Created on: May 8, 2016
 *      Author: peter
 */

#ifndef AISCHANNELS_H_
#define AISCHANNELS_H_


#include <inttypes.h>

#define NUM_AIS_CHANNELS 22

/*
 * This unit can utilize the top 525KHz of the VHF band, from 161.500Mhz to 162.025Mhz. Base stations
 * can force all traffic to switch to any of those frequencies via AIS message 22 and scheduled DSC transmissions
 * on marine channel 70 (156.525Mhz). It is rare, but it can happen.
 * Absent this override or when outside the range of any coastal base stations, the default channels are
 * always 87 and 88.
 *
 * Marine VHF channels have an ITU designated number that is interleaved (Rec. ITU-R M.1084). They can never fit
 * the Silabs WDS channel definition scheme, so they must be associated with those channels instead.
 * For now (and maybe forever), given an ITU channel number, we'll scan the AIS_CHANNELS array to find
 * its corresponding RF IC channel ordinal.
 */

typedef struct {
    uint8_t itu;              // The ITU channel #
    uint8_t ordinal;          // A zero-based index as defined by WDS in radio_config.h
    char designation;         // 'A', 'B' or '?'
    double frequency;         // Frequency in MHz, mostly for reference
}
ais_channel;

typedef enum {
    CH_18 = 0,
    CH_78,
    CH_19,
    CH_79,
    CH_20,
    CH_80,
    CH_21,
    CH_81,
    CH_22,
    CH_82,
    CH_23,
    CH_83,
    CH_24,
    CH_84,
    CH_25,
    CH_85,
    CH_26,
    CH_86,
    CH_27,
    CH_87,
    CH_28,
    CH_88
} VHFChannel;

static const ais_channel AIS_CHANNELS[] = {
        {18, 0, '?', 161.500},
        {78, 1, '?', 161.525},
        {19, 2, '?', 161.550},
        {79, 3, '?', 161.575},
        {20, 4, '?', 161.600},
        {80, 5, '?', 161.625},
        {21, 6, '?', 161.650},
        {81, 7, '?', 161.675},
        {22, 8, '?', 161.700},
        {82, 9, '?', 161.725},
        {23, 10, '?', 161.750},
        {83, 11, '?', 161.775},
        {24, 12, '?', 161.800},
        {84, 13, '?', 161.825},
        {25, 14, '?', 161.850},
        {85, 15, '?', 161.875},
        {26, 16, '?', 161.900},
        {86, 17, '?', 161.925},
        {27, 18, '?', 161.950},
        {87, 19, 'A', 161.975}, // Default channel A
        {28, 20, '?', 162.000},
        {88, 21, 'B', 162.025}  // Default channel B
};



#define ITU_TO_ORDINAL(C) (C < 78 ? (C-18)*2 : (C-78)*2+1)


#endif /* AISCHANNELS_H_ */
