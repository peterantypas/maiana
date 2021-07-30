// Safety related broadcast message (SRBM)

#include "ais.h"

namespace libais {

Ais14::Ais14(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), spare2(0) {
  if (!CheckStatus()) {
    return;
  }
  if (num_bits < 46 || num_bits > 1008) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 14);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 2);

  const int num_char = (num_bits - 40) / 6;
  text = bits.ToString(40, num_char * 6);
  if (bits.GetRemaining() > 0) {
    spare2 = bits.ToUnsignedInt(40 + num_char * 6, bits.GetRemaining());
  }

  status = AIS_OK;
}

}  // namespace libais
