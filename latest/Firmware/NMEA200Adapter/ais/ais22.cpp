// Msg 22 - F - Channel Management

#include "ais.h"

namespace libais {

Ais22::Ais22(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), chan_a(0), chan_b(0), txrx_mode(0),
      power_low(false), pos_valid(false), dest_valid(false), dest_mmsi_1(0),
      dest_mmsi_2(0), chan_a_bandwidth(0), chan_b_bandwidth(0), zone_size(0),
      spare2(0) {
  if (!CheckStatus()) {
    return;
  }
  if (pad != 0 || num_chars != 28) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 22);

  spare = bits.SeekTo(38).ToUnsignedInt(38, 2);

  chan_a = bits.ToUnsignedInt(40, 12);
  chan_b = bits.ToUnsignedInt(52, 12);
  txrx_mode = bits.ToUnsignedInt(64, 4);
  power_low = bits[68];

  // WARNING: OUT OF ORDER DECODE
  bits.SeekTo(139);
  bool addressed = bits[139];

  bits.SeekTo(69);
  if (!addressed) {
    // geographic position
    // TODO(schwehr): For all implementations in libais, set the valid flag
    //   after setting all of the data members.
    pos_valid = true;
    // TODO(schwehr): Confirm this is correct!
    position1 = bits.ToAisPoint(69, 35);
    position2 = bits.ToAisPoint(104, 35);
  } else {
    dest_valid = true;
    dest_mmsi_1 = bits.ToUnsignedInt(69, 30);
    // 5 spare bits
    bits.SeekRelative(5);
    dest_mmsi_2 = bits.ToUnsignedInt(104, 30);
    // 5 spare bits
    bits.SeekRelative(5);
  }

  // OUT OF ORDER: addressed is earlier before
  bits.SeekRelative(1);
  chan_a_bandwidth = bits[140];
  chan_b_bandwidth = bits[141];
  zone_size = bits.ToUnsignedInt(142, 3);

  spare2 = bits.ToUnsignedInt(145, 23);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
