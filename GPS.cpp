/**
 * @file ?/GPS.cpp
 * @version 0.5
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

#include "GPS.hh"

void
GPS::reset(void)
{
  m_last_update = 0;
  m_date = 0;
  m_time = 0;
  m_latitude = 0;
  m_longitude = 0;
  m_altitude = 0;
  m_course = 0;
  m_speed = 0;
  m_satellites = 0;
  m_hdop = 0;
}

clock_t
GPS::clock(void)
{
  uint32_t tmp;
  time_t t;

  tmp = m_time / 100;  // get rid of 100ths

  t.seconds = tmp % 100;
  tmp /= 100;
  t.minutes = tmp % 100;
  t.hours = tmp / 100;

  t.day = 0; // unknown

  tmp = m_date;
  t.year = tmp % 100;
  tmp /= 100;
  t.month = tmp % 100;
  t.date = tmp / 100;

  return (clock_t(t));
}

IOStream& 
operator<<(IOStream& outs, GPS& gps)
{
  outs
    << PSTR("GPS::V=") << (gps.valid() ? PSTR("t") : PSTR("f"))
    << PSTR(",U=") << gps.last_update()
    << PSTR(",D=") << gps.date()
    << PSTR(",T=") << gps.time()
    << PSTR(",LA=") << gps.f_latitude()
    << PSTR(",LO=") << gps.f_longitude()
    << PSTR(",A=") << gps.f_altitude()
    << PSTR(",C=") << gps.f_course()
    << PSTR(",SP=") << gps.f_speed()
    << PSTR(",SA=") << gps.satellites()
    << PSTR(",H=") << gps.f_hdop()
    ;
  return (outs);
}
