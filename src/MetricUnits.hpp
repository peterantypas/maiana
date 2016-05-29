/*
 * MetricUnits.hpp
 *
 *  Created on: Jan 16, 2016
 *      Author: peter
 */

#ifndef METRICUNITS_HPP_
#define METRICUNITS_HPP_


#include <math.h>
//#include <proj_api.h>

static const double EARTH_RADIUS_IN_METERS        = 6378137.0;
static const double EARTH_CIRCUMFERENCE_IN_METERS = EARTH_RADIUS_IN_METERS * 2.0 * M_PI;
static const double MILES_PER_METER               = 0.000621371192;
static const double METERS_PER_FOOT               = 0.30408;
static const double FEET_PER_METER                = 1.0/METERS_PER_FOOT;
static const double DEG_TO_RAD                    = M_PI/180.0;
static const double RAD_TO_DEG                    = 180.0/M_PI;
static const double METERS_PER_NAUTICAL_MILE      = 1852.0;

enum MetricSystem
  {
    METRIC_SYSTEM_NONE,
    METRIC_SYSTEM_SI,
    METRIC_SYSTEM_IMPERIAL
  };



#endif /* METRICUNITS_HPP_ */
