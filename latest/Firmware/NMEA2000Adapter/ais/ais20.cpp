// Msg 20 D - data link management

#include "ais.h"

namespace libais{

Ais20::Ais20(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), offset_1(0), num_slots_1(0),
      timeout_1(0), incr_1(0), group_valid_2(false), offset_2(0),
      num_slots_2(0), timeout_2(0), incr_2(0), group_valid_3(false),
      offset_3(0), num_slots_3(0), timeout_3(0), incr_3(0),
      group_valid_4(false), offset_4(0), num_slots_4(0), timeout_4(0),
      incr_4(0), spare2(0) {
  if (!CheckStatus()) {
    return;
  }
  if (num_bits < 72 || num_bits > 160) {
    status = AIS_ERR_BAD_BIT_COUNT;  return;
  }

  // 160, but must be 6 bit aligned
  assert(message_id == 20);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 2);

  offset_1 = bits.ToUnsignedInt(40, 12);
  num_slots_1 = bits.ToUnsignedInt(52, 4);
  timeout_1 = bits.ToUnsignedInt(56, 3);
  incr_1 = bits.ToUnsignedInt(59, 11);

  if (num_bits == 72) {
    spare2 = bits.ToUnsignedInt(70, 2);
    assert(bits.GetRemaining() == 0);
    status = AIS_OK;
    return;
  }

  group_valid_2 = true;
  offset_2 = bits.ToUnsignedInt(70, 12);
  num_slots_2 = bits.ToUnsignedInt(82, 4);
  timeout_2 = bits.ToUnsignedInt(86, 3);
  incr_2 = bits.ToUnsignedInt(89, 11);
  // 100 bits for the message
  // 104 is the next byte boundary
  // 108 is the next 6 bit boundary -> 18 characters
  if (num_bits >= 100 && num_bits <=108) {
    spare2 = bits.ToUnsignedInt(100, bits.GetRemaining());
    status = AIS_OK;
    return;
  }

  group_valid_3 = true;
  offset_3 = bits.ToUnsignedInt(100, 12);
  num_slots_3 = bits.ToUnsignedInt(112, 4);
  timeout_3 = bits.ToUnsignedInt(116, 3);
  incr_3 = bits.ToUnsignedInt(119, 11);
  // 130 bits for the message
  // 136 is the next byte boundary
  // 138 is the next 6 bit boundary -> 23 characters
  if (num_bits >= 130 && num_bits <= 138) {
    // Makes the result 8 bit / 1 byte aligned.
    spare2 = bits.ToUnsignedInt(130, bits.GetRemaining());
    status = AIS_OK;
    return;
  }

  group_valid_4 = true;
  offset_4 = bits.ToUnsignedInt(130, 12);
  num_slots_4 = bits.ToUnsignedInt(142, 4);
  timeout_4 = bits.ToUnsignedInt(146, 3);
  incr_4 = bits.ToUnsignedInt(149, 11);

  spare2 = 0;

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
