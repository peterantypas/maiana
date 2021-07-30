// Class A shipdata

#include "ais.h"

namespace libais {

Ais5::Ais5(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), ais_version(0), imo_num(0),
      type_and_cargo(0), dim_a(0), dim_b(0), dim_c(0), dim_d(0),
      fix_type(0), eta_month(0), eta_day(0), eta_hour(0), eta_minute(0),
      draught(0.0), dte(0), spare(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 2 || num_chars != 71) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 5);

  bits.SeekTo(38);
  ais_version = bits.ToUnsignedInt(38, 2);
  imo_num = bits.ToUnsignedInt(40, 30);
  callsign = bits.ToString(70, 42);

  name = bits.ToString(112, 120);

  type_and_cargo = bits.ToUnsignedInt(232, 8);
  dim_a = bits.ToUnsignedInt(240, 9);
  dim_b = bits.ToUnsignedInt(249, 9);
  dim_c = bits.ToUnsignedInt(258, 6);
  dim_d = bits.ToUnsignedInt(264, 6);
  fix_type = bits.ToUnsignedInt(270, 4);
  eta_month = bits.ToUnsignedInt(274, 4);
  eta_day = bits.ToUnsignedInt(278, 5);
  eta_hour = bits.ToUnsignedInt(283, 5);
  eta_minute = bits.ToUnsignedInt(288, 6);
  draught = bits.ToUnsignedInt(294, 8) / 10.;
  destination = bits.ToString(302, 120);
  dte = bits[422];
  spare = bits[423];

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

#if 0

ostream& operator<< (ostream& o, const Ais5 &msg) {
  return o << 5 << ": " << msg.mmsi << " \"" << msg.name << "\" "
           << msg.type_and_cargo << " " << msg.dim_a + msg.dim_b
           << "x" << msg.dim_c + msg.dim_d << "x" << msg.draught << "m";
}
#endif


}  // namespace libais
