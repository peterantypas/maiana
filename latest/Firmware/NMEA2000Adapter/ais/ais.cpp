// General helper functions for working with AIS and NMEA.

#include "ais.h"

extern "C" {
int LibAisVersionMajor() {
  return LIBAIS_VERSION_MAJOR;
}

int LibAisVersionMinor() {
  return LIBAIS_VERSION_MINOR;
}
}  // extern "C"

namespace libais {

string GetNthField(const string &str, const size_t n,
                   const string &delim_str) {
  assert(!delim_str.empty());
  if (str.empty())
    return "";

  size_t prev = 0;
  size_t off = str.find(delim_str);
  size_t count = 0;

  for (; off != string::npos && count != n;
       off = str.find(delim_str, off + 1), count++) {
    prev = off + delim_str.size();
  }

  if (count == n)
    return str.substr(prev, off-prev);

  return "";
}

int GetPad(const string &nmea_str) {
  // -1 on error
  const string chksum_block(GetNthField(nmea_str, 6, ","));
  if (chksum_block.size() != 4 || chksum_block[1] != '*')
    return -1;
  const char pad = chksum_block[0];
  if (pad < '0' || pad > '5')
    return -1;
  return static_cast<int>(pad - '0');
}

string GetBody(const string &nmea_str) {
  return GetNthField(nmea_str, 5, ",");
}

// Note: Needs to be kept in sync with enum AIS_STATUS list in ais.h.
const char * const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES] = {
  "AIS_UNINITIALIZED",
  "AIS_OK",
  "AIS_ERR_BAD_BIT_COUNT",
  "AIS_ERR_BAD_NMEA_CHR",
  "AIS_ERR_BAD_PTR",
  "AIS_ERR_UNKNOWN_MSG_TYPE",
  "AIS_ERR_MSG_NOT_IMPLEMENTED",
  "AIS_ERR_MSG_SUB_NOT_IMPLEMENTED",
  "AIS_ERR_EXPECTED_STRING",
  "AIS_ERR_BAD_MSG_CONTENT",
  "AIS_ERR_MSG_TOO_LONG",
  "AIS_ERR_BAD_SUB_MSG",
  "AIS_ERR_BAD_SUB_SUB_MSG",
};

AisMsg::AisMsg(const char *nmea_payload, const size_t pad)
    : message_id(0), repeat_indicator(0), mmsi(0), status(AIS_UNINITIALIZED),
      num_chars(0), num_bits(0), bits() {
  assert(nmea_payload);
  assert(pad < 6);

  const AIS_STATUS r = bits.ParseNmeaPayload(nmea_payload, pad);
  if (r != AIS_OK) {
    status = r;
    return;
  }
  num_bits = bits.GetNumBits();
  num_chars = bits.GetNumChars();

  if (bits.GetNumBits() < 38) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  message_id = bits.ToUnsignedInt(0, 6);
  repeat_indicator = bits.ToUnsignedInt(6, 2);
  mmsi = bits.ToUnsignedInt(8, 30);
}

bool AisMsg::CheckStatus() const {
  if (status == AIS_OK || status == AIS_UNINITIALIZED) {
    return true;
  }
#ifdef LIBAIS_DEBUG
  std::cerr << "AisMsg::CheckStatus failed: "
            << AIS_STATUS_STRINGS[status]
            << std::endl;
#endif
  return false;
}

AisPoint::AisPoint() : lng_deg(0), lat_deg(0) {
}

AisPoint::AisPoint(double lng_deg_, double lat_deg_)
    : lng_deg(lng_deg_), lat_deg(lat_deg_) {
}

#if 0
ostream& operator<< (ostream &o, const AisPoint &position) {
  return o << " (" << position.lng_deg << ", " << position.lat_deg << ")";
}
#endif

}  // namespace libais
