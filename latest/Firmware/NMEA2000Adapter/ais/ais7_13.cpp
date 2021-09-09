// ACK to ABM or safety ABM.

#include "ais.h"

namespace libais {

Ais7_13::Ais7_13(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0) {

  if (!CheckStatus()) {
    return;
  }
  if (num_bits < 72 || num_bits > 168 || ((num_bits - 40) % 32) != 0) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 7 || message_id == 13);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 2);
  const size_t num_acks = (num_bits - 40) / 32;
  for (size_t i = 0; i < num_acks; i++) {
    dest_mmsi.push_back(bits.ToUnsignedInt(40 + i*32, 30));
    seq_num.push_back(bits.ToUnsignedInt(40 + i*32 + 30, 2));
  }

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
