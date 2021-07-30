// < - ASRM

#include "ais.h"

namespace libais {

Ais12::Ais12(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), seq_num(0), dest_mmsi(0), retransmitted(false),
      spare(0), spare2(0) {
  if (!CheckStatus()) {
    return;
  }
  // WARNING: The ITU 1371 specifications says the maximum number of bits is
  // 1008, but it appears that the maximum should be 1192.
  if (num_bits < 72 || num_bits > 1192)  {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 12);

  bits.SeekTo(38);
  seq_num = bits.ToUnsignedInt(38, 2);
  dest_mmsi = bits.ToUnsignedInt(40, 30);
  retransmitted = bits[70];
  spare = bits[71];
  const int num_txt = (num_bits - 72) / 6;
  const int num_txt_bits = num_txt * 6;
  text = bits.ToString(72, num_txt_bits);
  if (bits.GetRemaining() > 0) {
    spare2 = bits.ToUnsignedInt(72 + num_txt_bits, bits.GetRemaining());
  }

  status = AIS_OK;
}

}  // namespace libais
