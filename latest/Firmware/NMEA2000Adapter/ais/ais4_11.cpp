// AIS message 4 or 11

#include "ais.h"

namespace libais {

Ais4_11::Ais4_11(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), year(0), month(0), day(0), hour(0), minute(0),
      second(0), position_accuracy(0), fix_type(0),
      transmission_ctl(0), spare(0), raim(false), sync_state(0),
      slot_timeout(0), received_stations_valid(false), received_stations(0),
      slot_number_valid(false), slot_number(0), utc_valid(false), utc_hour(0),
      utc_min(0), utc_spare(0), slot_offset_valid(false), slot_offset(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 0 || num_chars != 28) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 4 || message_id == 11);

  bits.SeekTo(38);
  year = bits.ToUnsignedInt(38, 14);
  month = bits.ToUnsignedInt(52, 4);
  day = bits.ToUnsignedInt(56, 5);
  hour = bits.ToUnsignedInt(61, 5);
  minute = bits.ToUnsignedInt(66, 6);
  second = bits.ToUnsignedInt(72, 6);

  position_accuracy = bits[78];
  position = bits.ToAisPoint(79, 55);

  fix_type = bits.ToUnsignedInt(134, 4);
  transmission_ctl = bits[138];
  spare = bits.ToUnsignedInt(139, 9);
  raim = bits[148];

  // SOTDMA commstate
  sync_state = bits.ToUnsignedInt(149, 2);
  slot_timeout = bits.ToUnsignedInt(151, 3);

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

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

#if 0
ostream& operator<< (ostream &o, const Ais4_11 &msg) {
  return o << msg.message_id << ": " << msg.mmsi;
}
#endif


}  // namespace libais
