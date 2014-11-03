+/**
 * @file ?/GPS_NMEA_MT3339.cpp
 * @version 0.1
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

#include "GPS_NMEA_MT3339.hh"


GPS_NMEA_MT3339::GPS_NMEA_MT3339() :
  GPS_NMEA(),
  m_factory_reset_in_progress(false)
{
}

bool
GPS_NMEA_MT3339::begin(IOStream::Device* dev)
{
  GPS_NMEA_MT3339::reset();

  if (!GPS_NMEA::begin(dev))
    return (false);

  /* Clear any lingering garbage (will also wake if in standby) */
  send_cmd(PSTR(""));

  select_sentences();

  return (true);
}

void
GPS_NMEA_MT3339::end(void)
{
  GPS_NMEA_MT3339::reset();

  GPS_NMEA::end();
}

void
GPS_NMEA_MT3339::begin_monitor(IOStream::Device* monitor_dev)
{
  GPS_NMEA::begin_monitor(monitor_dev);
}

void
GPS_NMEA_MT3339::end_monitor(void)
{
  GPS_NMEA::end_monitor();
}

void
GPS_NMEA_MT3339::reset(void)
{
  m_factory_reset_in_progress = false;
  GPS_NMEA::reset();
}

void
GPS_NMEA_MT3339::factory_reset(void)
{
  GPS_NMEA_MT3339::reset();

  m_factory_reset_in_progress = true;

  send_cmd(PSTR("$PMTK104*37"));

  delay(250);  // won't respond to new commands for awhile
}

void
GPS_NMEA_MT3339::standby(void)
{
  send_cmd(PSTR("$PMTK161,0*28"));

  GPS_NMEA::reset();
}

void
GPS_NMEA_MT3339::wake(void)
{
  send_cmd(PSTR(""));
}

void
GPS_NMEA_MT3339::new_field(char *field)
{
  UNUSED(field);
}

void
GPS_NMEA_MT3339::new_sentence(void)
{
  if (m_factory_reset_in_progress)
    {
      m_factory_reset_in_progress = false;
      select_sentences();
    }
}

void
GPS_NMEA_MT3339::send_cmd(str_P cmd)
{
  if (m_monitor_dev)
    {
      m_monitor_dev->puts_P((str_P)IOStream::LF);
      m_monitor_dev->puts_P(PSTR("-> "));
      m_monitor_dev->puts_P(cmd);
      m_monitor_dev->puts_P((str_P)IOStream::LF);
    }
  m_dev->puts_P(cmd);
  m_dev->puts_P((str_P)IOStream::CRLF);
}

void
GPS_NMEA_MT3339::select_sentences(void)
{
  /*
   * Set the desired sentences..
   *
   * 0 NMEA_SEN_GLL
   * 1 NMEA_SEN_RMC
   * 2 NMEA_SEN_VTG
   * 3 NMEA_SEN_GGA
   * 4 NMEA_SEN_GSA
   * 5 NMEA_SEN_GSV
   * 6-17 reserved
   * 18 NMEA_SEN_MCHN
   */

  send_cmd(PSTR("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"));
}

IOStream& 
operator<<(IOStream& outs, GPS_NMEA_MT3339& gps_nmea_mt3339)
{
  outs << (GPS_NMEA&)gps_nmea_mt3339;
  return (outs);
}

