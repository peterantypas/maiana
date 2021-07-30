// @ - Assigned mode command
// TODO(schwehr): Use valid flags rather than negative numbers.

#include "ais.h"

namespace libais {

Ais16::Ais16(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad),
      spare(0),
      dest_mmsi_a(0),
      offset_a(0),
      inc_a(0),
      dest_mmsi_b(0),
      offset_b(0),
      inc_b(0),
      spare2(0) {
  if (!CheckStatus()) {
    return;
  }
  // 96 or 144 bits
  // 168 bits violates the spec but is common
  if (num_bits != 96 && num_bits != 144 && num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 16);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 2);

  dest_mmsi_a = bits.ToUnsignedInt(40, 30);
  offset_a = bits.ToUnsignedInt(70, 12);
  inc_a = bits.ToUnsignedInt(82, 10);
  if (num_chars == 16) {
    dest_mmsi_b = -1;
    offset_b = -1;
    inc_b = -1;
    spare2 = bits.ToUnsignedInt(92, 4);

    assert(bits.GetRemaining() == 0);
    status = AIS_OK;
    return;
  }

  dest_mmsi_b = bits.ToUnsignedInt(92, 30);
  offset_b = bits.ToUnsignedInt(122, 12);
  inc_b = bits.ToUnsignedInt(134, 10);
  // TODO(schwehr): Actually decode spare2.
  spare2 = -1;

  // Currently crashes with the check.
  // TODO(schwehr): Add assert(bits.GetRemaining() == 0);

  status = AIS_OK;
}

}  // namespace libais
