// Msg 19 C - Extended Class B equipment position report

#include "ais.h"

namespace libais {

Ais19::Ais19(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), sog(0.0), position_accuracy(0),
      cog(0.0), true_heading(0), timestamp(0), spare2(0), type_and_cargo(0),
      dim_a(0), dim_b(0), dim_c(0), dim_d(0), fix_type(0), raim(false), dte(0),
      assigned_mode(0), spare3(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 0 || num_chars != 52) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 19);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 8);
  sog = bits.ToUnsignedInt(46, 10) / 10.;

  position_accuracy = bits[56];
  position = bits.ToAisPoint(57, 55);

  cog = bits.ToUnsignedInt(112, 12) / 10.;
  true_heading = bits.ToUnsignedInt(124, 9);
  timestamp = bits.ToUnsignedInt(133, 6);
  spare2 = bits.ToUnsignedInt(139, 4);

  name = bits.ToString(143, 120);

  type_and_cargo = bits.ToUnsignedInt(263, 8);
  dim_a = bits.ToUnsignedInt(271, 9);
  dim_b = bits.ToUnsignedInt(280, 9);
  dim_c = bits.ToUnsignedInt(289, 6);
  dim_d = bits.ToUnsignedInt(295, 6);

  fix_type = bits.ToUnsignedInt(301, 4);
  raim = bits[305];
  dte = bits[306];
  assigned_mode = bits[307];
  spare3 = bits.ToUnsignedInt(308, 4);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
