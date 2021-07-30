// Class B position report - 18 "B"

#include "ais.h"

namespace libais {

Ais18::Ais18(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad),
      spare(0),
      sog(0.0),
      position_accuracy(0),
      cog(0.0),
      true_heading(0),
      timestamp(0),
      spare2(0),
      unit_flag(0),
      display_flag(0),
      dsc_flag(0),
      band_flag(0),
      m22_flag(0),
      mode_flag(0),
      raim(false),
      commstate_flag(0),
      sync_state(0),
      slot_timeout_valid(false),
      slot_timeout(0),
      received_stations_valid(false),
      received_stations(0),
      slot_number_valid(false),
      slot_number(0),
      utc_valid(false),
      utc_hour(0),
      utc_min(0),
      utc_spare(0),
      slot_offset_valid(false),
      slot_offset(0),
      slot_increment_valid(false),
      slot_increment(0),
      slots_to_allocate_valid(false),
      slots_to_allocate(0),
      keep_flag_valid(false),
      keep_flag(0),
      commstate_cs_fill_valid(false),
      commstate_cs_fill(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 0 || num_chars != 28) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 18);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 8);
  sog = bits.ToUnsignedInt(46, 10) / 10.;

  position_accuracy = bits[56];
  position = bits.ToAisPoint(57, 55);

  cog = bits.ToUnsignedInt(112, 12) / 10.;
  true_heading = bits.ToUnsignedInt(124, 9);
  timestamp = bits.ToUnsignedInt(133, 6);
  spare2 = bits.ToUnsignedInt(139, 2);
  unit_flag = bits[141];
  display_flag = bits[142];
  dsc_flag = bits[143];
  band_flag = bits[144];
  m22_flag = bits[145];
  mode_flag = bits[146];
  raim = bits[147];
  commstate_flag = bits[148];  // 0 SOTDMA, 1 ITDMA

  if (unit_flag == 0) {
    sync_state = bits.ToUnsignedInt(149, 2);
    if (commstate_flag == 0) {
      // SOTDMA
      slot_timeout = bits.ToUnsignedInt(151, 3);
      slot_timeout_valid = true;

      switch (slot_timeout) {
        case 0:
          slot_offset = bits.ToUnsignedInt(154, 14);
          slot_offset_valid = true;
          break;
        case 1:
          utc_hour = bits.ToUnsignedInt(154, 5);
          utc_min = bits.ToUnsignedInt(159, 7);
          utc_spare = bits.ToUnsignedInt(166, 2);
          utc_valid = true;
          break;
        case 2:  // FALLTHROUGH
        case 4:  // FALLTHROUGH
        case 6:
          slot_number = bits.ToUnsignedInt(154, 14);
          slot_number_valid = true;
          break;
        case 3:  // FALLTHROUGH
        case 5:  // FALLTHROUGH
        case 7:
          received_stations = bits.ToUnsignedInt(154, 14);
          received_stations_valid = true;
          break;
        default:
          assert(false);
      }

    } else {
      // ITDMA
      slot_increment = bits.ToUnsignedInt(151, 13);
      slot_increment_valid = true;

      slots_to_allocate = bits.ToUnsignedInt(164, 3);
      slots_to_allocate_valid = true;

      keep_flag = bits[167];
      keep_flag_valid = true;
    }
  } else {
    // Carrier Sense (CS) with unit_flag of 1.
    commstate_cs_fill = bits.ToUnsignedInt(149, 19);
    commstate_cs_fill_valid = true;
  }

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
