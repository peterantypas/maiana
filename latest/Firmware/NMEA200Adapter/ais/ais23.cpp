// Msg 23 - G - Channel Management

#include "ais.h"

namespace libais {

Ais23::Ais23(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), station_type(0), type_and_cargo(0),
      spare2(3), txrx_mode(0), interval_raw(0), quiet(0), spare3(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 2 || num_chars != 27) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 23);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 2);

  position1 = bits.ToAisPoint(40, 35);
  position2 = bits.ToAisPoint(75, 35);

  station_type = bits.ToUnsignedInt(110, 4);
  type_and_cargo = bits.ToUnsignedInt(114, 8);
  spare2 = bits.ToUnsignedInt(122, 22);

  txrx_mode = bits.ToUnsignedInt(144, 2);
  interval_raw = bits.ToUnsignedInt(146, 4);
  quiet = bits.ToUnsignedInt(150, 4);
  spare3 = bits.ToUnsignedInt(154, 6);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
