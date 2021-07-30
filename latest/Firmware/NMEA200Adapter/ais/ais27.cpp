// 'K' - 27 - Long-range AIS broadcast message

#include "ais.h"

namespace libais {

Ais27::Ais27(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), position_accuracy(0), raim(false),
      nav_status(0), sog(0), cog(0), gnss(false), spare(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 0 || num_bits != 96) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 27);

  bits.SeekTo(38);
  position_accuracy = bits[38];
  raim = bits[39];
  nav_status = bits.ToUnsignedInt(40, 4);
  position = bits.ToAisPoint(44, 35);
  sog = bits.ToUnsignedInt(79, 6);  // Knots.
  cog = bits.ToUnsignedInt(85, 9);  // Degrees.
  // 0 is a current GNSS position.  1 is NOT the current GNSS position
  gnss = !bits[94];
  spare = bits[95];

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
