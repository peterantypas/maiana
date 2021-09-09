// ? - Interrogation

#include "ais.h"

namespace libais {

Ais15::Ais15(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), mmsi_1(0), msg_1_1(0),
      slot_offset_1_1(0), spare2(0), dest_msg_1_2(0), slot_offset_1_2(0),
      spare3(0), mmsi_2(0), msg_2(0), slot_offset_2(0), spare4(0) {
  if (!CheckStatus()) {
    return;
  }
  // TODO(schwehr): verify the pad makes sense for each size
  if (num_chars != 15 && num_chars != 18 && num_chars != 27) {
    // 88-160 bits
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  // 160 / 6 = 26.66
  assert(message_id == 15);

  bits.SeekTo(38);
  spare = bits.ToUnsignedInt(38, 2);
  mmsi_1 = bits.ToUnsignedInt(40, 30);  // Destination ID 1
  msg_1_1 = bits.ToUnsignedInt(70, 6);
  slot_offset_1_1 = bits.ToUnsignedInt(76, 12);

  // TODO(schwehr): set remaining fields to -1
  if (num_chars <= 15) {
    assert(bits.GetRemaining() == 0);
    status = AIS_OK;
    return;
  }

  spare2 = bits.ToUnsignedInt(88, 2);
  dest_msg_1_2 = bits.ToUnsignedInt(90, 6);
  // Failing test here. Getting bits [96..108], but num_bits is 104 (+4 pad).
  // Context (http://catb.org/gpsd/AIVDM.html):
  // "One station is interrogated for two message types, Length is 110 bits.
  // There is a design error in the standard here; according to the <[ITU1371]>
  // requirement for padding to 8 bits, this should have been 112 with a 4-bit
  // trailing spare field, and decoders should be prepared to handle that length
  // as well."
  slot_offset_1_2 = bits.ToUnsignedInt(96, 12);

  // TODO(schwehr): set remaining fields to -1
  if (num_chars <= 18) {
    // TODO(schwehr): Crashes: assert(bits.GetRemaining() == 0);
    status = AIS_OK;
    return;
  }

  spare3 = bits.ToUnsignedInt(108, 2);
  mmsi_2 = bits.ToUnsignedInt(110, 30);
  msg_2 = bits.ToUnsignedInt(140, 6);
  slot_offset_2 = bits.ToUnsignedInt(146, 12);
  spare4 = bits.ToUnsignedInt(158, 2);

  assert(bits.GetRemaining() == 0);

  status = AIS_OK;
}

}  // namespace libais
