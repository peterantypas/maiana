#include "ais.h"

namespace libais {

AisBitset::AisBitset() : num_bits(0), num_chars(0), current_position(0) {}

AIS_STATUS AisBitset::ParseNmeaPayload(const char *nmea_payload, int pad) {
  assert(nmea_payload);
  assert(pad >= 0 && pad < 6);

  InitNmeaOrd();

  num_bits = 0;
  current_position = 0;
  reset();

  num_chars = strlen(nmea_payload);

  const size_t max_chars = size()/6;
  if (static_cast<size_t>(num_chars) > max_chars) {
#ifdef LIBAIS_DEBUG
    std::cerr << "ERROR: message longer than max allowed size (" << max_chars
              << "): found " << num_chars << " characters in "
              << nmea_payload << std::endl;
#endif
    num_chars = 0;  // Don't leave an impression that there are any valid chars.
    return AIS_ERR_MSG_TOO_LONG;
  }

  size_t bit = 0;
  for (size_t idx = 0; nmea_payload[idx] != '\0' && idx < max_chars; idx++) {
    int c = static_cast<int>(nmea_payload[idx]);
    if (c < 48 || c > 119 || (c >= 88 && c <= 95)) {
      // Make it clear that nothing valuable is in here.
      reset();
      num_chars = 0;
      return AIS_ERR_BAD_NMEA_CHR;
    }
    for (size_t offset = 0; offset < 6; offset++) {
      set(bit++, nmea_ord_[c].test(offset));
    }
  }

  num_bits = num_chars * 6 - pad;

  return AIS_OK;
}

const AisBitset& AisBitset::SeekRelative(int d) const {
  assert((current_position + d) >= 0 && (current_position + d) < num_bits);
  current_position += d;
  return *this;
}

const AisBitset& AisBitset::SeekTo(size_t pos) const {
  assert(pos < num_bits);
  current_position = pos;
  return *this;
}

bool AisBitset::operator[](size_t pos) const {
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(pos < num_chars * 6);

  assert(current_position == pos);

  current_position = pos + 1;
  return bitset<MAX_BITS>::operator[](pos);
}

unsigned int AisBitset::ToUnsignedInt(const size_t start,
                                      const size_t len) const {
  assert(len <= 32);
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(start + len <= num_chars * 6);

  assert(current_position == start);

  unsigned int result = 0;
  size_t end = start + len;
  for (size_t i = start; i < end; ++i) {
    result <<= 1;
    if (test(i))
      result |= 1;
  }

  current_position = end;
  return result;
}

int AisBitset::ToInt(const size_t start, const size_t len)  const {
  assert(len <= 32);
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(start + len <= num_chars * 6);

  assert(current_position == start);

  // Converting the sub-bitset to a signed number, per "Two's complement":
  // - If negative, invert all the bits, then add 1.
  bool is_positive = (len == 32 || !test(start));
  int result = 0;
  size_t end = start + len;
  for (size_t i = start; i < end; ++i) {
    result <<= 1;
    if (test(i) == is_positive)
      result |= 1;
  }
  current_position = end;
  return is_positive ? result : -(result + 1);
}

string AisBitset::ToString(const size_t start, const size_t len) const {
  assert(len % 6 == 0);
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(start + len <= num_chars * 6);

  assert(current_position == start);

  const size_t num_char = len / 6;
  string result(num_char, '@');
  for (size_t char_idx = 0; char_idx < num_char; char_idx++) {
    const int char_num = ToUnsignedInt(start + char_idx*6, 6);
    result[char_idx] = bits_to_char_tbl_[char_num];
  }
  return result;
}

const AisPoint AisBitset::ToAisPoint(const size_t start,
                                     const size_t point_size) const {
  int lng_bits;
  int lat_bits;
  double divisor;
  switch (point_size) {
    case 35:
      lng_bits = 18;
      lat_bits = 17;
      divisor = 600.0;
      break;
    case 49:
      lng_bits = 25;
      lat_bits = 24;
      divisor = 60000.0;  // 1/1000th minute
      break;
    case 55:
      lng_bits = 28;
      lat_bits = 27;
      divisor = 600000.;
      break;
    default:
      //std::cerr << "Unsupported point AIS size: " << point_size << std::endl;
      assert(false);
      return AisPoint(-1, -1);
  }
  double lng_deg = ToInt(start, lng_bits);
  double lat_deg = ToInt(start + lng_bits, lat_bits);
  return AisPoint(lng_deg / divisor, lat_deg / divisor);
}


// static private

void AisBitset::InitNmeaOrd() {
  if (nmea_ord_initialized_) {
    return;
  }

  for (int c = 48; c < 88; c++) {
    int val = c - 48;
    nmea_ord_[c] = Reverse(bitset<6>(val));
  }
  for (int c = 88; c < 128; c++) {
    int val = c - 48;
    val -= 8;
    nmea_ord_[c] = Reverse(bitset<6>(val));
  }
  nmea_ord_initialized_ = true;
}

bitset<6> AisBitset::Reverse(const bitset<6> &bits) {
  bitset<6> out;
  for (size_t i = 0; i < 6; i++)
    out[5 - i] = bits[i];
  return out;
}

bool AisBitset::nmea_ord_initialized_ = false;
bitset<6> AisBitset::nmea_ord_[128];
// For decoding str bits inside of a binary message.
const char AisBitset::bits_to_char_tbl_[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";

}  // namespace libais
