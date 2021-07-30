// 'J' - Multi slot binary message with comm state
// TODO(schwehr): handle payload
// BAD: the comm-state is after the veriable payload.  This is a bad design.

// See also: http://www.e-navigation.nl/asm

#include "ais.h"

namespace libais {

Ais26::Ais26(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), use_app_id(false), dest_mmsi_valid(false),
      dest_mmsi(0), dac(0), fi(0), commstate_flag(0), sync_state(0),
      slot_timeout_valid(false), slot_timeout(0),
      received_stations_valid(false), received_stations(0),
      slot_number_valid(false), slot_number(0),
      utc_valid(false), utc_hour(0), utc_min(0), utc_spare(0),
      slot_offset_valid(false),  slot_offset(0),
      slot_increment_valid(false), slot_increment(0),
      slots_to_allocate_valid(false), slots_to_allocate(0),
      keep_flag_valid(false), keep_flag(false) {
  if (!CheckStatus()) {
    return;
  }

  // TODO(schwehr): Check for off by one.
  const size_t comm_flag_offset = num_bits - 20;

  if (num_bits < 52 || num_bits > 1064) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 26);

  bits.SeekTo(38);
  const bool addressed = bits[38];
  use_app_id = bits[39];
  if (addressed) {
    dest_mmsi_valid = true;
    dest_mmsi = bits.ToUnsignedInt(40, 30);
    if (use_app_id) {
      if (num_bits < 86) {
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
      }
      dac = bits.ToUnsignedInt(70, 10);
      fi = bits.ToUnsignedInt(80, 6);
    }
    // TODO(schwehr): Handle the payload.
  } else {
    // broadcast
    if (use_app_id) {
      dac = bits.ToUnsignedInt(40, 10);
      fi = bits.ToUnsignedInt(50, 6);
    }
    // TODO(schwehr): Handle the payload.
  }

  bits.SeekTo(comm_flag_offset);
  commstate_flag = bits[comm_flag_offset];
  sync_state = bits.ToUnsignedInt(comm_flag_offset + 1, 2);  // SOTDMA and TDMA.

  if (!commstate_flag) {
    // SOTDMA
    slot_timeout = bits.ToUnsignedInt(comm_flag_offset + 3, 3);
    slot_timeout_valid = true;
    switch (slot_timeout) {
    case 0:
      slot_offset = bits.ToUnsignedInt(comm_flag_offset + 6, 14);
      slot_offset_valid = true;
      break;
    case 1:
      utc_hour = bits.ToUnsignedInt(comm_flag_offset + 6, 5);
      utc_min = bits.ToUnsignedInt(comm_flag_offset + 11, 7);
      utc_spare = bits.ToUnsignedInt(comm_flag_offset + 18, 2);
      utc_valid = true;
      break;
    case 2:  // FALLTHROUGH
    case 4:  // FALLTHROUGH
    case 6:
      slot_number = bits.ToUnsignedInt(comm_flag_offset + 6, 14);
      slot_number_valid = true;
      break;
    case 3:  // FALLTHROUGH
    case 5:  // FALLTHROUGH
    case 7:
      received_stations = bits.ToUnsignedInt(comm_flag_offset + 6, 14);
      received_stations_valid = true;
      break;
    default:
      assert(false);
    }
  } else {
    // ITDMA
    slot_increment = bits.ToUnsignedInt(comm_flag_offset + 3, 13);
    slot_increment_valid = true;

    slots_to_allocate = bits.ToUnsignedInt(comm_flag_offset + 16, 3);
    slots_to_allocate_valid = true;

    keep_flag = bits[comm_flag_offset + 19];
    keep_flag_valid = true;
  }

  // TODO(schwehr): Add assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
