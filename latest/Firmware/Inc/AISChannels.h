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

#ifndef AISCHANNELS_H_
#define AISCHANNELS_H_


#include <inttypes.h>

/**
 * Marine VHF channels have an ITU designated number that is interleaved (Rec. ITU-R M.1084). They can never fit
 * the Silicon Labs WDS channel definition scheme, so they must be associated with those channels instead.
 * For now (and maybe forever), given an ITU channel number, we'll scan the AIS_CHANNELS array to find
 * its corresponding RFIC channel ordinal.
 */

typedef struct {
  uint8_t itu;              // The ITU channel #
  uint8_t ordinal;          // A zero-based index as defined by WDS in radio_config.h
  char designation;         // 'A', 'B' or '?'
  float frequency;          // Frequency in MHz, mostly for reference
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
#define ORDINAL_TO_ITU(O) (AIS_CHANNELS[O].itu)


#endif /* AISCHANNELS_H_ */

/* -*-c++-*- */
