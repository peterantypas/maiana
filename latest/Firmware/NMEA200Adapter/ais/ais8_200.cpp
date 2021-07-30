// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
// http://www.unece.org/fileadmin/DAM/trans/doc/finaldocs/sc3/ECE-TRANS-SC3-176e.pdf

#include "ais.h"

namespace libais {

// Inland ship static and voyage related data
Ais8_200_10::Ais8_200_10(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), length(0.0), beam(0.0), ship_type(0),
      haz_cargo(0), draught(0.0), loaded(0), speed_qual(0), course_qual(0),
      heading_qual(0), spare2(0) {
  assert(dac == 200);
  assert(fi == 10);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);
  eu_id = bits.ToString(56, 48);
  length = bits.ToUnsignedInt(104, 13) / 10.;  // m
  beam = bits.ToUnsignedInt(117, 10) / 10.;  // m
  ship_type = bits.ToUnsignedInt(127, 14);
  haz_cargo = bits.ToUnsignedInt(141, 3);
  draught = bits.ToUnsignedInt(144, 11) / 10.;  // m
  loaded = bits.ToUnsignedInt(155, 2);
  speed_qual = bits[157];
  course_qual = bits[158];
  heading_qual = bits[159];
  spare2 = bits.ToUnsignedInt(160, 8);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// ETA report
Ais8_200_21::Ais8_200_21(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), eta_month(0), eta_day(0), eta_hour(0),
      eta_minute(0), tugboats(0), air_draught(0.0) // TODO : add missing fields
{
  assert(dac == 200);
  assert(fi == 21);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits != 248) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  // UN country code                    12 bits
  country = bits.ToString(88, 12);

  // UN location code                   18 bits
  location = bits.ToString(100, 18);

  // Fairway section number             30 bits
  section = bits.ToString(118, 30);

  // Terminal code                      30 bits
  terminal = bits.ToString(148, 30);

  // Fairway hectometre                 30 bits
  hectometre = bits.ToString(178, 30);

  // ETA at lock/bridge/terminal        20 bits
  eta_month  = bits.ToUnsignedInt(208, 4);
  eta_day    = bits.ToUnsignedInt(212, 5);
  eta_hour   = bits.ToUnsignedInt(217, 5);
  eta_minute = bits.ToUnsignedInt(223, 6);

  // Number of assisting tugboats        3 bits
  tugboats = bits.ToUnsignedInt(229, 3);

  // Maximum present static air draught 12 bits
  air_draught = bits.ToUnsignedInt(231, 12) / 10.;  // m

  // Spare                               5 bits
  spare2 = bits.ToUnsignedInt(243, 5);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// RTA report
Ais8_200_22::Ais8_200_22(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), rta_month(0), rta_day(0), rta_hour(0),
      rta_minute(0) // TODO : add missing fields
{
  assert(dac == 200);
  assert(fi == 22);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits != 232) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  // UN country code                    12 bits
  country = bits.ToString(88, 12);

  // UN location code                   18 bits
  location = bits.ToString(100, 18);

  // Fairway section number             30 bits
  section = bits.ToString(118, 30);

  // Terminal code                      30 bits
  terminal = bits.ToString(148, 30);

  // Fairway hectometre                 30 bits
  hectometre = bits.ToString(178, 30);

  // RTA at lock/bridge/terminal        20 bits
  rta_month  = bits.ToUnsignedInt(208, 4);
  rta_day    = bits.ToUnsignedInt(212, 5);
  rta_hour   = bits.ToUnsignedInt(217, 5);
  rta_minute = bits.ToUnsignedInt(223, 6);

  // Lock/bridge/terminal status         2 bits
  lock_status = bits.ToUnsignedInt(229, 2);

  // Spare                               2 bits
  spare2 = bits.ToUnsignedInt(231, 2);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
Ais8_200_23::Ais8_200_23(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), utc_year_start(0), utc_month_start(0),
      utc_day_start(0), utc_year_end(0), utc_month_end(0), utc_day_end(0),
      utc_hour_start(0), utc_min_start(0), utc_hour_end(0), utc_min_end(0),
      type(0), min(0), max(0), classification(0), wind_dir(0), spare2(0) {
  assert(dac == 200);
  assert(fi == 23);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits != 256) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);

  // TODO(schwehr): Figure out the correct bits & test against actual messages.

  // The total for the bit column in table 2.11 of ECE/TRANS/SC.3/2006/10
  // add up to 256.  However, start date and end date fields are lists with
  // totals of 17 bits each, but within the details, the spec refers to 18 bits
  // for each.  It is likely the year should be one less bit.
  utc_year_start = bits.ToUnsignedInt(56, 8);
  utc_month_start = bits.ToUnsignedInt(65, 4);
  utc_day_start = bits.ToUnsignedInt(69, 5);

  utc_year_end = bits.ToUnsignedInt(73, 8);
  utc_month_end = bits.ToUnsignedInt(82, 4);
  utc_day_end = bits.ToUnsignedInt(86, 5);

  utc_hour_start = bits.ToUnsignedInt(90, 5);
  utc_min_start = bits.ToUnsignedInt(95, 6);
  utc_hour_end = bits.ToUnsignedInt(101, 5);
  utc_min_end = bits.ToUnsignedInt(106, 6);

  position1 = bits.ToAisPoint(112, 55);
  position2 = bits.ToAisPoint(167, 55);

  type = bits.ToUnsignedInt(222, 4);
  // TODO(schwehr): Handle the sign bit for min and max.
  min = bits.ToUnsignedInt(226, 9);
  max = bits.ToUnsignedInt(235, 9);
  classification = bits.ToUnsignedInt(244, 2);
  wind_dir = bits.ToUnsignedInt(246, 4);
  spare2 = bits.ToUnsignedInt(250, 6);

  // TODO(schwehr): The above bit counts can't work.
  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}


// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
// Water level
Ais8_200_24::Ais8_200_24(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(dac == 200);
  assert(fi == 24);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);
  country = bits.ToString(56, 12);
  for (size_t i = 0; i < 4; i++) {
    size_t start = 68 + 25*i;
    gauge_ids[i] = bits.ToUnsignedInt(start, 11);
    const int sign = bits[start + 11] ? 1 : -1;  // 0 negative, 1 pos
    // ERROR: the spec has a bit listing mistake
    levels[i] = sign * bits.ToUnsignedInt(start + 12, 13);
  }

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
Ais8_200_40::Ais8_200_40(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), form(0), dir(0), stream_dir(0), status_raw(0),
      spare2(0) {
  assert(dac == 200);
  assert(fi == 40);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);
  position = bits.ToAisPoint(56, 55);
  form = bits.ToUnsignedInt(111, 4);
  dir = bits.ToUnsignedInt(115, 9);  // degrees
  stream_dir = bits.ToUnsignedInt(124, 3);
  status_raw = bits.ToUnsignedInt(127, 30);
  // TODO(schwehr): Convert the status_raw to the 9 signal lights.
  // Appears to be a base 10 setup where each of 9 digits range from 0 to 7
  // for the light level.
  spare2 = bits.ToUnsignedInt(157, 11);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}


// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
//
// TODO(schwehr): Search the logs for the various possible sizes and make
//   tests based on those messages.
Ais8_200_55::Ais8_200_55(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), crew(0), passengers(0), yet_more_personnel(0) {
  assert(dac == 200);
  assert(fi == 55);

  if (!CheckStatus()) {
    return;
  }

  // The specification says that there are 51 spare bits, but it is possible
  // that some transmitters may leave off the spare bits.
  if (num_bits != 88 && num_bits != 136 && num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);
  crew = bits.ToUnsignedInt(56, 8);
  passengers = bits.ToUnsignedInt(64, 13);
  yet_more_personnel = bits.ToUnsignedInt(77, 8);

  if (num_bits == 88) {
    spare2[0] = bits.ToUnsignedInt(85, 3);
  } else if (num_bits == 136) {
    spare2[0] = bits.ToUnsignedInt(85, 32);
    spare2[1] = bits.ToUnsignedInt(117, 19);
  } else {
    spare2[0] = bits.ToUnsignedInt(85, 32);
    spare2[1] = bits.ToUnsignedInt(117, 32);
    spare2[2] = bits.ToUnsignedInt(149, 19);
  }

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
