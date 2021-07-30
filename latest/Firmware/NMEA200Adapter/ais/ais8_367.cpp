// 8:367:22 Defined by an email from Greg Johnson representing the
// USCG, Fall 2012.  Breaks from the RTCM and IMO Circular 289.
// "Area Notice Message Release Version: 1" 13 Aug 2012
//
// http://www.e-navigation.nl/content/geographic-notice
// http://www.e-navigation.nl/sites/default/files/asm_files/GN%20Release%20Version%201b.pdf

#include <cmath>

#include "ais.h"

namespace libais {

const size_t SUB_AREA_BITS = 96;

static int scale_multipliers[4] = {1, 10, 100, 1000};

Ais8_367_22_Circle::Ais8_367_22_Circle(
    const AisBitset &bits, const size_t offset)
    : precision(0), radius_m(0), spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  position = bits.ToAisPoint(offset + 2, 55);
  precision = bits.ToUnsignedInt(offset + 57, 3);
  radius_m =
      bits.ToUnsignedInt(offset + 60, 12) * scale_multipliers[scale_factor];
  spare = bits.ToUnsignedInt(offset + 72, 21);
}

Ais8_367_22_Rect::Ais8_367_22_Rect(const AisBitset &bits, const size_t offset)
    : precision(0), e_dim_m(0), n_dim_m(0), orient_deg(0), spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  position = bits.ToAisPoint(offset + 2, 55);
  precision = bits.ToUnsignedInt(offset + 57, 3);
  e_dim_m =
      bits.ToUnsignedInt(offset + 60, 8) * scale_multipliers[scale_factor];
  n_dim_m =
      bits.ToUnsignedInt(offset + 68, 8) * scale_multipliers[scale_factor];
  orient_deg = bits.ToUnsignedInt(offset + 76, 9);
  spare = bits.ToUnsignedInt(offset + 85, 8);
}

Ais8_367_22_Sector::Ais8_367_22_Sector(
    const AisBitset &bits, const size_t offset)
    : precision(0),
      radius_m(0),
      left_bound_deg(0),
      right_bound_deg(0),
      spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  position = bits.ToAisPoint(offset + 2, 55);
  precision = bits.ToUnsignedInt(offset + 57, 3);
  radius_m =
      bits.ToUnsignedInt(offset + 60, 12) * scale_multipliers[scale_factor];
  left_bound_deg = bits.ToUnsignedInt(offset + 72, 9);
  right_bound_deg = bits.ToUnsignedInt(offset + 81, 9);
  spare = bits.ToUnsignedInt(offset + 90, 3);
}

// Polyline or polygon.
Ais8_367_22_Poly::Ais8_367_22_Poly(const AisBitset &bits, const size_t offset,
                                   Ais8_366_22_AreaShapeEnum area_shape)
    : shape(area_shape), precision(0), spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  size_t poly_offset = offset + 2;
  for (size_t i = 0; i < 4; i++) {
    const int angle = bits.ToUnsignedInt(poly_offset, 10);
    poly_offset += 10;
    const int dist = bits.ToUnsignedInt(poly_offset, 11) *
                     scale_multipliers[scale_factor];
    poly_offset += 11;
    if (dist == 0) {
      break;
    }
    angles.push_back(angle);
    dists_m.push_back(dist);
  }
  spare = bits.ToUnsignedInt(offset + 86, 7);
}

Ais8_367_22_Text::Ais8_367_22_Text(const AisBitset &bits, const size_t offset) {
  text = string(bits.ToString(offset, 90));
  spare = bits.ToUnsignedInt(offset + 90, 3);
}

Ais8_367_22_SubArea *ais8_367_22_subarea_factory(const AisBitset &bits,
                                                 const size_t offset) {
  const Ais8_366_22_AreaShapeEnum area_shape =
      static_cast<Ais8_366_22_AreaShapeEnum>(bits.ToUnsignedInt(offset, 3));

  switch (area_shape) {
    case AIS8_366_22_SHAPE_CIRCLE:
      return new Ais8_367_22_Circle(bits, offset + 3);
    case AIS8_366_22_SHAPE_RECT:
      return new Ais8_367_22_Rect(bits, offset + 3);
    case AIS8_366_22_SHAPE_SECTOR:
      return new Ais8_367_22_Sector(bits, offset + 3);
    case AIS8_366_22_SHAPE_POLYLINE:  // FALLTHROUGH
    case AIS8_366_22_SHAPE_POLYGON:
      return new Ais8_367_22_Poly(bits, offset + 3, area_shape);
    case AIS8_366_22_SHAPE_TEXT:
      return new Ais8_367_22_Text(bits, offset + 3);
    case AIS8_366_22_SHAPE_RESERVED_6:  // FALLTHROUGH
    case AIS8_366_22_SHAPE_RESERVED_7:  // FALLTHROUGH
      // Leave area as 0 to indicate error.
      break;
    case AIS8_366_22_SHAPE_ERROR:
      break;
    default:
      assert(false);
  }
  return nullptr;
}

Ais8_367_22::Ais8_367_22(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), version(0), link_id(0), notice_type(0),
      month(0), day(0), hour(0), minute(0), duration_minutes(0), spare2(0) {
  assert(dac == 367);
  assert(fi == 22);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits < 216 || num_bits > 1016) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }
  // Is there more than spare bits after the last submessage?
  if ((num_bits - 120) % SUB_AREA_BITS >= 6) {
    // More than just spare.
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);
  version = bits.ToUnsignedInt(56, 6);
  link_id = bits.ToUnsignedInt(62, 10);
  notice_type = bits.ToUnsignedInt(72, 7);
  month = bits.ToUnsignedInt(79, 4);
  day = bits.ToUnsignedInt(83, 5);
  hour = bits.ToUnsignedInt(88, 5);
  minute = bits.ToUnsignedInt(93, 6);
  duration_minutes = bits.ToUnsignedInt(99, 18);
  spare2 = bits.ToUnsignedInt(117, 3);

  const int num_sub_areas = static_cast<int>(
      floor((num_bits - 120) / static_cast<float>(SUB_AREA_BITS)));

  for (int area_idx = 0; area_idx < num_sub_areas; area_idx++) {
    const size_t start = 120 + area_idx * SUB_AREA_BITS;
    Ais8_367_22_SubArea *area = ais8_367_22_subarea_factory(bits, start);
    if (area != nullptr) {
      sub_areas.push_back(area);
    } else {
      status = AIS_ERR_BAD_SUB_SUB_MSG;
      return;
    }
  }

  // TODO(schwehr): Save the spare bits at the end of the message.
  assert(bits.GetRemaining() < 6);
  status = AIS_OK;
}

Ais8_367_22::~Ais8_367_22() {
  // Switch to unique_ptr.
  for (size_t i = 0; i < sub_areas.size(); i++) {
    delete sub_areas[i];
    sub_areas[i] = nullptr;
  }
}

}  // namespace libais
