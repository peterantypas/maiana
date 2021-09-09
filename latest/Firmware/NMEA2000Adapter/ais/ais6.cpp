// Address Binary Message (ABM) 6

#include <cmath>
#include <iomanip>

#include "ais.h"

namespace libais {

Ais6::Ais6(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), seq(0), mmsi_dest(0), retransmit(false),
      spare(0), dac(0), fi(0) {

  if (!CheckStatus()) {
    return;
  }

  // TODO(olafsinram): 46 or rather 56??
  const int payload_len = num_bits - 46;  // in bits w/o DAC/FI
  if (num_bits < 88 || payload_len < 0 || payload_len > 952) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  assert(message_id == 6);

  bits.SeekTo(38);
  seq = bits.ToUnsignedInt(38, 2);
  mmsi_dest = bits.ToUnsignedInt(40, 30);
  retransmit = !bits[70];
  spare = bits[71];
  dac = bits.ToUnsignedInt(72, 10);
  fi = bits.ToUnsignedInt(82, 6);
}

// http://www.e-navigation.nl/content/monitoring-aids-navigation
Ais6_0_0::Ais6_0_0(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad),
      sub_id(1),
      voltage(0.0),
      current(0.0),
      dc_power_supply(true),
      light_on(true),
      battery_low(false),
      off_position(false),
      spare2(0) {
  assert(dac == 0);
  assert(fi == 0);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 136) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  sub_id = bits.ToUnsignedInt(88, 16);
  voltage = bits.ToUnsignedInt(104, 12) / 10.0;
  current = bits.ToUnsignedInt(116, 10) / 10.0;
  dc_power_supply = bits[126];
  light_on = bits[127];
  battery_low = bits[128];
  off_position = bits[129];

  spare2 = bits.ToUnsignedInt(130, 6);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

Ais6_1_0::Ais6_1_0(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), ack_required(false), msg_seq(0),
      spare2(0) {
  assert(dac == 1);
  assert(fi == 0);

  if (!CheckStatus()) {
    return;
  }

  // ITU-1371-5 says 112 to 920 because there must be at least one character.
  // TODO(schwehr): Are there any examples with no characters in the wild?
  if (num_bits < 112 || num_bits > 920) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  ack_required = bits[88];
  msg_seq = bits.ToUnsignedInt(89, 11);

  const size_t text_size = 6 * ((num_bits - 100) / 6);
  const size_t spare2_size = num_bits - 100 - text_size;
  text = bits.ToString(100, text_size);

  if (!spare2_size)
    spare2 = 0;
  else
    spare2 = bits.ToUnsignedInt(100 + text_size, spare2_size);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

Ais6_1_1::Ais6_1_1(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), ack_dac(0), msg_seq(0), spare2(0) {
  assert(dac == 1);
  assert(fi == 1);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 112) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  ack_dac = bits.ToUnsignedInt(88, 10);
  msg_seq = bits.ToUnsignedInt(98, 11);
  spare2 = bits.ToUnsignedInt(109, 3);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

Ais6_1_2::Ais6_1_2(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), req_dac(0), req_fi(0) {
  assert(dac == 1);
  assert(fi == 2);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 104) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  req_dac = bits.ToUnsignedInt(88, 10);
  req_fi = bits.ToUnsignedInt(98, 6);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// IFM 3: Capability interrogation - OLD ITU 1371-1
Ais6_1_3::Ais6_1_3(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), req_dac(0), spare2(0), spare3(0), spare4(0) {
  assert(dac == 1);
  assert(fi == 3);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 104 && num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  req_dac = bits.ToUnsignedInt(88, 10);
  if (num_bits == 104) {
    // Invalid but often used.
    spare2 = bits.ToUnsignedInt(98, 6);
    assert(bits.GetRemaining() == 0);
    status = AIS_OK;
    return;
  }
  spare2 = bits.ToUnsignedInt(98, 32);
  spare3 = bits.ToUnsignedInt(130, 32);
  spare4 = bits.ToUnsignedInt(162, 6);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// IFM 4: Capability reply - OLD ITU 1371-4
// TODO(schwehr): WTF?  10 + 128 + 6 == 80  Is this 168 or 232 bits?
Ais6_1_4::Ais6_1_4(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), ack_dac(0), capabilities(),
      cap_reserved(), spare2(0), spare3(0), spare4(0), spare5(0) {
  assert(dac == 1);
  assert(fi == 4);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 352) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  ack_dac = bits.ToUnsignedInt(88, 10);
  constexpr int kNumFI = 64;
  for (size_t cap_num = 0; cap_num < kNumFI; cap_num++) {
    size_t start = 98 + cap_num * 2;
    capabilities[cap_num] = bits[start];
    cap_reserved[cap_num] = bits[start + 1];
  }
  spare2 = bits.ToUnsignedInt(226, 32);
  spare3 = bits.ToUnsignedInt(258, 32);
  spare4 = bits.ToUnsignedInt(290, 32);
  // This last one is not a full 32 bits to cover the 126 bits of spare.
  spare5 = bits.ToUnsignedInt(322, 30);


  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// IMO 1371-5 Ack
Ais6_1_5::Ais6_1_5(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), ack_dac(0), ack_fi(0), seq_num(0),
      ai_available(false), ai_response(0), spare(0), spare2(0) {
  assert(dac == 1);
  assert(fi == 5);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  ack_dac = bits.ToUnsignedInt(88, 10);
  ack_fi = bits.ToUnsignedInt(98, 6);
  seq_num = bits.ToUnsignedInt(104, 11);
  ai_available = static_cast<bool>(bits[115]);
  ai_response = bits.ToUnsignedInt(116, 3);
  spare = bits.ToUnsignedInt(119, 32);
  spare2 = bits.ToUnsignedInt(151, 17);

  assert(bits.GetRemaining() == 0);

  status = AIS_OK;
}

// IMO Circ 289 - Dangerous cargo
// See also Circ 236
Ais6_1_12::Ais6_1_12(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), utc_month_dep(0), utc_day_dep(0),
      utc_hour_dep(0), utc_min_dep(0), utc_month_next(0),
      utc_day_next(0), utc_hour_next(0), utc_min_next(0),
      un(0), value(0), value_unit(0), spare2(0) {
  assert(dac == 1);
  assert(fi == 12);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  // TODO(schwehr): add in the offset of the dest mmsi

#if 0
  bits.SeekTo(56);
  last_port = bits.ToString(56, 30);
  utc_month_dep = bits.ToUnsignedInt(86, 4);
  utc_day_dep = bits.ToUnsignedInt(90, 5);
  utc_hour_dep = bits.ToUnsignedInt(95, 5);
  utc_min_dep = bits.ToUnsignedInt(100, 6);
  next_port = bits.ToString(106, 30);
  utc_month_next = bits.ToUnsignedInt(136, 4);  // estimated arrival
  utc_day_next = bits.ToUnsignedInt(140, 5);
  utc_hour_next = bits.ToUnsignedInt(145, 5);
  utc_min_next = bits.ToUnsignedInt(150, 6);
  main_danger = bits.ToString(156, 120);
  imo_cat = bits.ToString(276, 24);
  un = bits.ToUnsignedInt(300, 13);
  value = bits.ToUnsignedInt(313, 10);  // TODO(schwehr): units
  value_unit = bits.ToUnsignedInt(323, 2);
  spare = bits.ToUnsignedInt(325, 3);
  // 360
#endif

  // TODO(schwehr): Add assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// 6_1_13 Does not exist

// IMO Circ 289 - Tidal Window
// See also Circ 236
Ais6_1_14::Ais6_1_14(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), utc_month(0), utc_day(0) {
  // TODO(schwehr): untested - no sample of the correct length yet
  assert(dac == 1);
  assert(fi == 14);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 376) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  utc_month = bits.ToUnsignedInt(88, 4);
  utc_day = bits.ToUnsignedInt(92, 5);

  for (size_t window_num = 0; window_num < 3; window_num++) {
    Ais6_1_14_Window w;
    const size_t start = 97 + window_num * 93;
    // Reversed order for lng/lat.
    double y = bits.ToInt(start, 27) / 600000.0;
    double x = bits.ToInt(start + 27, 28) / 600000.0;
    w.position = AisPoint(x, y);
    w.utc_hour_from = bits.ToUnsignedInt(start + 55, 5);
    w.utc_min_from = bits.ToUnsignedInt(start + 60, 6);
    w.utc_hour_to = bits.ToUnsignedInt(start + 66, 5);
    w.utc_min_to = bits.ToUnsignedInt(start + 71, 6);
    w.cur_dir = bits.ToUnsignedInt(start + 77, 9);
    w.cur_speed  = bits.ToUnsignedInt(start + 86, 7) / 10.;

    windows.push_back(w);
  }

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// IMO Circ 289 - Clearance time to enter port
Ais6_1_18::Ais6_1_18(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), link_id(0), utc_month(0), utc_day(0),
      utc_hour(0), utc_min(0), spare2() {
  assert(dac == 1);
  assert(fi == 18);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  link_id = bits.ToUnsignedInt(88, 10);
  utc_month = bits.ToUnsignedInt(98, 4);
  utc_day = bits.ToUnsignedInt(102, 5);
  utc_hour = bits.ToUnsignedInt(107, 5);
  utc_min = bits.ToUnsignedInt(112, 6);
  port_berth = bits.ToString(118, 120);
  dest = bits.ToString(238, 30);
  position = bits.ToAisPoint(268, 49);
  spare2[0] = bits.ToUnsignedInt(317, 32);
  spare2[1] = bits.ToUnsignedInt(349, 11);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// IMO Circ 289 - Berthing data
Ais6_1_20::Ais6_1_20(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), link_id(0), length(0), depth(0.0),
      mooring_position(0), utc_month(0), utc_day(0), utc_hour(0), utc_min(0),
      services_known(false), services() {
  assert(dac == 1);
  assert(fi == 20);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  link_id = bits.ToUnsignedInt(88, 10);
  length = bits.ToUnsignedInt(98, 9);
  depth = bits.ToUnsignedInt(107, 8);
  mooring_position = bits.ToUnsignedInt(115, 3);
  utc_month = bits.ToUnsignedInt(118, 4);
  utc_day = bits.ToUnsignedInt(122, 5);
  utc_hour = bits.ToUnsignedInt(127, 5);
  utc_min = bits.ToUnsignedInt(132, 6);
  services_known = bits[138];
  for (size_t serv_num = 0; serv_num < 26; serv_num++) {
    // TODO(schwehr): const int val = bits.ToUnsignedInt(139 + 2*serv_num, 2);
    services[serv_num]
        = static_cast<int>(bits.ToUnsignedInt(139 + 2*serv_num, 2));
  }
  name = bits.ToString(191, 120);
  position = bits.ToAisPoint(311, 49);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

Ais6_1_25_Cargo::Ais6_1_25_Cargo()
    : code_type(0), imdg_valid(false), imdg(0), spare_valid(false), spare(0),
      un_valid(false), un(0), bc_valid(false), bc(0), marpol_oil_valid(false),
      marpol_oil(0), marpol_cat_valid(false), marpol_cat(0) {}

// IMO Circ 289 - Dangerous cargo indication 2
// See also Circ 236
Ais6_1_25::Ais6_1_25(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), amount_unit(0), amount(0) {
  assert(dac == 1);
  assert(fi == 25);

  if (!CheckStatus()) {
    return;
  }

  // TODO(schwehr): verify multiple of the size of cargos + header
  //   or padded to a slot boundary
  // Allowing a message with no payloads
  // TODO(schwehr): (num_bits - 100) % 17 != 0) is okay
  if (num_bits < 100 || num_bits > 576) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }
  if ((num_bits - 100) % 17 != 0) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  amount_unit = bits.ToUnsignedInt(88, 2);
  amount = bits.ToUnsignedInt(90, 10);
  const size_t total_cargos = static_cast<int>(floor((num_bits - 100) / 17.));
  for (size_t cargo_num = 0; cargo_num < total_cargos; cargo_num++) {
    Ais6_1_25_Cargo cargo;
    const size_t start = 100 + 17*cargo_num;
    cargo.code_type = bits.ToUnsignedInt(start, 4);

    // TODO(schwehr): Is this the correct behavior?
    switch (cargo.code_type) {
      // No 0
      case 1:  // IMDG Code in packed form
        cargo.imdg = bits.ToUnsignedInt(start + 4, 7);
        cargo.imdg_valid = true;
        cargo.spare = bits.ToUnsignedInt(start + 11, 6);
        cargo.spare_valid = true;
        break;
      case 2:  // IGC Code
        cargo.un = bits.ToUnsignedInt(start + 4, 13);
        cargo.un_valid = true;
        break;
      case 3:  // BC Code
        cargo.bc = bits.ToUnsignedInt(start + 4, 3);
        cargo.bc_valid = true;
        cargo.imdg = bits.ToUnsignedInt(start + 7, 7);
        cargo.imdg_valid = true;
        cargo.spare = bits.ToUnsignedInt(start + 14, 3);
        cargo.spare_valid = true;
        break;
      case 4:  // MARPOL Annex I
        cargo.marpol_oil = bits.ToUnsignedInt(start + 4, 4);
        cargo.marpol_oil_valid = true;
        cargo.spare = bits.ToUnsignedInt(start + 8, 9);
        cargo.spare_valid = true;
        break;
      case 5:  // MARPOL Annex II IBC
        cargo.marpol_cat = bits.ToUnsignedInt(start + 4, 3);
        cargo.marpol_cat_valid = true;
        cargo.spare = bits.ToUnsignedInt(start + 7, 10);
        cargo.spare_valid = true;
        break;
      // 6: Regional use
      // 7: 7-15 reserved for future
      default:
        break;  // Just push in an all blank record?
    }
    cargos.push_back(cargo);
  }

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// TODO(schwehr): 6_1_28 - Modify 8_1_28 once that is debugged

Ais6_1_32_Window::Ais6_1_32_Window()
    : from_utc_hour(0), from_utc_min(0), to_utc_hour(0), to_utc_min(0),
      cur_dir(0), cur_speed(0.0) {}

// IMO Circ 289 - Tidal window
// See also Circ 236
Ais6_1_32::Ais6_1_32(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), utc_month(0), utc_day(0) {
  assert(dac == 1);
  assert(fi == 32);

  if (!CheckStatus()) {
    return;
  }

  // TODO(schwehr): might get messages with not all windows
  if (num_bits != 350) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(88);
  utc_month = bits.ToUnsignedInt(88, 4);
  utc_day = bits.ToUnsignedInt(92, 5);

  for (size_t window_num = 0; window_num < 3; window_num++) {
    Ais6_1_32_Window w;
    const size_t start = 97 + 88*window_num;
    w.position = bits.ToAisPoint(start, 49);
    w.from_utc_hour = bits.ToUnsignedInt(start + 49, 5);
    w.from_utc_min = bits.ToUnsignedInt(start + 54, 6);
    w.to_utc_hour = bits.ToUnsignedInt(start + 60, 5);
    w.to_utc_min = bits.ToUnsignedInt(start + 65, 6);
    w.cur_dir = bits.ToUnsignedInt(start + 71, 9);
    w.cur_speed = bits.ToUnsignedInt(start + 80, 8) / 10.;
    windows.push_back(w);
  }

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// IFM 40: people on board - OLD ITU 1371-4
Ais6_1_40::Ais6_1_40(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad), persons(0), spare2(0) {
  assert(dac == 1);
  assert(fi == 40);

  if (num_bits != 104) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  AisBitset bs;
  const AIS_STATUS r = bits.ParseNmeaPayload(nmea_payload, pad);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  bits.SeekTo(88);
  persons = bits.ToUnsignedInt(88, 13);
  spare2 = bits.ToUnsignedInt(101, 3);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
