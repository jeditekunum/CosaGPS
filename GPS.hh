/**
 * @file ?/GPS.hh
 * @version 0.6
 *
 * @section License
 * Copyright (C) 2014, jediunix
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 */

#ifndef COSA_GPS_HH
#define COSA_GPS_HH

#include "Cosa/IOStream.hh"
#include "Cosa/Time.hh"

#define GPS_TIME_ONLY
//#define GPS_INTERRUPT_IMPL

#define GPS_FEET_PER_CENTIMETER 0.0328084

#define GPS_MILES_PER_HOUR_PER_KNOT 1.15077945
#define GPS_METERS_PER_SECOND_PER_KNOT 0.51444444
#define GPS_KILOMETER_PER_HOUR_PER_KNOT 1.852

#define GPS_MINIMUM_SATELLITES 4

#ifdef GPS_INTERRUPT_IMPL
#define GPS_VOLATILE volatile
#else
#define GPS_VOLATILE
#endif


/**
 * GPS abstraction
 */

class GPS {
public:
  typedef uint32_t last_update_t;
  typedef uint32_t date_t;
  typedef uint32_t gps_time_t;
#ifndef GPS_TIME_ONLY
  typedef int32_t  position_t;
  typedef int32_t  altitude_t;
  typedef uint32_t course_t;
  typedef uint32_t speed_t;
  typedef uint8_t  satellites_t;
  typedef uint32_t hdop_t;
#endif

  /**
   * Construct GPS
   */
  GPS() :
    m_last_update(0),
    m_date(0),
    m_time(0)
#ifndef GPS_TIME_ONLY
    ,
    m_latitude(0),
    m_longitude(0),
    m_altitude(0),
    m_course(0),
    m_speed(0),
    m_satellites(0),
    m_hdop(0)
#endif
  {}

  /**
   * Reset
   */
  virtual void reset();

  /**
   * Is data valid? Data returned from methods is not valid until the
   * first GPS data is received.
   * @return valid
   */
  bool valid()
    __attribute__((always_inline))
  {
    return (m_last_update != 0);
  }

  /**
   * Get last update
   * @return last_update in milliseconds (RTC::millis)
   */
  last_update_t last_update()
    __attribute__((always_inline))
  {
    return m_last_update;
  }

  /**
   * Get date
   * @return date in DDMMYY
   */
  date_t date()
    __attribute__((always_inline))
  {
    return m_date;
  }

  /**
   * Get time
   * @return time in HHMMSSmmm
   */
  gps_time_t time()
    __attribute__((always_inline))
  {
    return m_time;
  }

  /**
   * Get clock (time since Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   * @return clock
   */
  clock_t clock();

#ifndef GPS_TIME_ONLY
   /**
   * Get latitude
   * @return latitude in millionths of a degree
   */
  position_t latitude()
    __attribute__((always_inline))
  {
    return m_latitude;
  }
    
  /**
   * Get latitude
   * @return latitude in degrees
   */
  float f_latitude()
    __attribute__((always_inline))
  {
    return m_latitude / 1000000.0;
  }
    
  /**
   * Get longitude
   * @return longitude in millionths of a degree
   */
  position_t longitude()
    __attribute__((always_inline))
  {
    return m_longitude;
  }

  /**
   * Get longitude
   * @return longitude in degrees
   */
  float f_longitude()
    __attribute__((always_inline))
  {
    return m_longitude / 1000000.0;
  }
    
  /**
   * Get altitude
   * @return altitude in centimeters
   */
  altitude_t altitude()
    __attribute__((always_inline))
  {
    return m_altitude;
  }

  /**
   * Get altitude
   * @return altitude in meters
   */
  float f_altitude()
    __attribute__((always_inline))
  {
    return m_altitude / 100.0;
  }

  /**
   * Get altitude
   * @return altitude in feet
   */
  float f_altitude_ft()
    __attribute__((always_inline))
  {
    return m_altitude * GPS_FEET_PER_CENTIMETER;
  }

  /**
   * Get course
   * @return course in 100ths of a degree
   */
  course_t course()
    __attribute__((always_inline))
  {
    return m_course;
  }

  /**
   * Get course
   * @return course in degrees
   */
  float f_course()
    __attribute__((always_inline))
  {
    return m_course / 100.0;
  }

  /**
   * Get speed
   * @return speed in 100ths of a knot
   */
  speed_t speed()
    __attribute__((always_inline))
  {
    return m_speed;
  }

  /**
   * Get speed
   * @return speed in knots
   */
  float f_speed()
    __attribute__((always_inline))
  {
    return m_speed / 100.0;
  }

  /**
   * Get speed
   * @return speed in mph
   */
  float f_speed_mph()
    __attribute__((always_inline))
  {
    return m_speed * GPS_MILES_PER_HOUR_PER_KNOT;
  }

  /**
   * Get speed
   * @return speed in mps
   */
  float f_speed_mps()
    __attribute__((always_inline))
  {
    return m_speed * GPS_METERS_PER_SECOND_PER_KNOT;
  }

  /**
   * Get speed
   * @return speed in kmph
   */
  float f_speed_kmph()
    __attribute__((always_inline))
  {
    return m_speed * GPS_KILOMETER_PER_HOUR_PER_KNOT;
  }

  /**
   * Get satellites
   * @return satellites
   */
  satellites_t satellites()
    __attribute__((always_inline))
  {
    return m_satellites;
  }

  /**
   * Get hdop
   * @return hdop in 100ths
   */
  hdop_t hdop()
    __attribute__((always_inline))
  {
    return m_hdop;
  }

  /**
   * Get hdop
   * @return hdop
   */
  float f_hdop()
    __attribute__((always_inline))
  {
    return m_hdop / 100.0;
  }
#endif

protected:
  /* Last time updated received */
  GPS_VOLATILE last_update_t m_last_update;

  /* Date DDMMYY */
  GPS_VOLATILE date_t m_date;

  /* Time HHMMSSmmm */
  GPS_VOLATILE gps_time_t m_time;

#ifndef GPS_TIME_ONLY
  /* Latitude in millionths of a degree */
  GPS_VOLATILE position_t m_latitude;

  /* Longitude in millionths of a degree */
  GPS_VOLATILE position_t m_longitude;

  /* Altitude in centimeters */
  GPS_VOLATILE altitude_t m_altitude;

  /* Course in 100th of a degree */
  GPS_VOLATILE course_t m_course;

  /* Speed in 100ths of a knot */
  GPS_VOLATILE speed_t m_speed;

  /* Satellies used in last update */
  GPS_VOLATILE satellites_t m_satellites;

  /* Horizontal dilution of precision in 100ths */
  GPS_VOLATILE hdop_t m_hdop;
#endif

  /**
   * Print latest gps information to
   * given stream.
   * @param[in] outs output stream
   * @param[in] gps to print
   * @return stream
   */
  friend IOStream& operator<<(IOStream& outs, GPS& gps);
};
#endif
