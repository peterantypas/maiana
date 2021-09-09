// : UTC and date query

#include "ais.h"

namespace libais {

Ais10::Ais10(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), dest_mmsi(0), spare2(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 0 || num_chars != 12) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 10);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 2);
  dest_mmsi = bits.ToUnsignedInt(40, 30);
  spare2 = bits.ToUnsignedInt(70, 2);

  assert(bits.GetRemaining() == 0);

  status = AIS_OK;
}

#if 0
ostream& operator<< (ostream &o, const Ais10 &msg) {
  return o << msg.message_id << ": " << msg.mmsi
           << " dest=" << msg.dest_mmsi
           << " " << msg.spare << " " << msg.spare2;
}
#endif

}  // namespace libais
