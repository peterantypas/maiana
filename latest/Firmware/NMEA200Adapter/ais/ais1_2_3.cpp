// Since Apr 2010

#include <cmath>

#include "ais.h"

using std::abs;

namespace libais {

Ais1_2_3::Ais1_2_3(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), nav_status(0), rot_over_range(false),
      rot_raw(0), rot(0.0), sog(0.0), position_accuracy(0),
      cog(0.0), true_heading(0), timestamp(0), special_manoeuvre(0), spare(0),
      raim(false), sync_state(0),
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

  assert(message_id >= 1 && message_id <= 3);

  bits.SeekTo(38);
  nav_status = bits.ToUnsignedInt(38, 4);

  rot_raw = bits.ToInt(42, 8);
  rot_over_range = abs(rot_raw) > 126 ? true : false;
  rot = pow((rot_raw/4.733), 2);
  if (rot_raw < 0) rot = -rot;

  sog = (float)bits.ToUnsignedInt(50, 10) / 10.0f;  // Knots.
  position_accuracy = bits[60];
  position = bits.ToAisPoint(61, 55);
  cog = (float)bits.ToUnsignedInt(116, 12) / 10.0f;  // Degrees.
  true_heading = bits.ToUnsignedInt(128, 9);
  timestamp = bits.ToUnsignedInt(137, 6);
  special_manoeuvre = bits.ToUnsignedInt(143, 2);
  spare = bits.ToUnsignedInt(145, 3);
  raim = bits[148];

  sync_state = bits.ToUnsignedInt(149, 2);

  if (message_id == 1 || message_id == 2) {
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

#if 0
ostream& operator<< (ostream &o, const Ais1_2_3 &msg) {
  return o << msg.message_id << ": " << msg.mmsi;
}
#endif

}  // namespace libais
