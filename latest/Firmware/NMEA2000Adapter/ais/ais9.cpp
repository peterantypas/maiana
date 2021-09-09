#include "ais.h"

namespace libais {

Ais9::Ais9(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), alt(0), sog(0.0), position_accuracy(0),
      cog(0.0), timestamp(0), alt_sensor(0), spare(0), dte(0), spare2(0),
      assigned_mode(0), raim(false), commstate_flag(0), sync_state(0),
      slot_timeout_valid(false), slot_timeout(0),
      received_stations_valid(false), received_stations(0),
      slot_number_valid(false), slot_number(0),
      utc_valid(false), utc_hour(0), utc_min(0), utc_spare(0),
      slot_offset_valid(false), slot_offset(0),
      slot_increment_valid(false), slot_increment(0),
      slots_to_allocate_valid(false), slots_to_allocate(0),
      keep_flag_valid(false), keep_flag(false) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 0 || num_chars != 28) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 9);

  bits.SeekTo(38);
  alt = bits.ToUnsignedInt(38, 12);
  sog = bits.ToUnsignedInt(50, 10);  // Type 9: Speed over ground is in knots.

  position_accuracy = bits[60];
  position = bits.ToAisPoint(61, 55);

  cog = bits.ToUnsignedInt(116, 12) / 10.;
  timestamp = bits.ToUnsignedInt(128, 6);
  alt_sensor = bits[134];
  spare = bits.ToUnsignedInt(135, 7);
  dte = bits[142];
  spare2 = bits.ToUnsignedInt(143, 3);
  assigned_mode = bits[146];
  raim = bits[147];
  commstate_flag = bits[148];  // 0 SOTDMA, 1 ITDMA

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

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
