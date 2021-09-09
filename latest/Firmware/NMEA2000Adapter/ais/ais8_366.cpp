// USCG binary application messages (BBM).

#include "ais.h"

namespace libais {

// US Coast Guard (USCG) blue force encrypted position report.
//
// Also known as: Blue Force Tracking (BFT) or encrypted AIS (EAIS).
// Messages use AES 256 or Blowfish, but the decryption is not done here.
//
// "NAIS Performance Specification" says:
//
// Technical Characteristics for USCG Encrypted Automatic
// Identification System (EAIS) Very High Frequency Data Link (VDL)
// Standard (v4.0)
//
// (FIPS) 140-2 and 197 for data communications encryption
Ais8_366_56::Ais8_366_56(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(dac == 366);
  assert(fi == 56);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits < 56 || num_bits > 1192) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);
  int num_full_bytes = bits.GetRemaining() / 8;

  for (int i = 0; i < num_full_bytes; i++) {
    encrypted.push_back(bits.ToUnsignedInt(56 + i * 8, 8));
  }

  if (bits.GetRemaining() > 0) {
    assert(bits.GetRemaining() < 8);
    encrypted.push_back(
        bits.ToUnsignedInt(bits.GetPosition(), bits.GetRemaining()));
  }

  status = AIS_OK;
}

}  // namespace libais
