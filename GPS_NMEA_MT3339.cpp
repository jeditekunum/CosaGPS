/**
 * @file ?/GPS_NMEA_MT3339.cpp
 * @version 0.6
 *
 * @section License
 * Copyright (C) 2014-2015, jeditekunum
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

#include "Cosa/Trace.hh"
#include "Cosa/RTC.hh"

#include "GPS_NMEA_MT3339.hh"


GPS_NMEA_MT3339::GPS_NMEA_MT3339(IOStream::Device *device) :
  GPS_NMEA(device),
  m_sentence(SENTENCE_UNKNOWN),
  m_first_sentence_received(false),
  m_ending(false)
{
}

bool
GPS_NMEA_MT3339::begin()
{
  reset();

  if (!GPS_NMEA::begin())
    return (false);

  send_cmd(PSTR(""));  // wake

  return (true);
}

void
GPS_NMEA_MT3339::end(void)
{
  if (!m_active)
    return;

  if (RTC::since(m_last_update) > 5000)
    {
      // Ending while device isn't responding
      // Don't defer
      send_cmd(PSTR("$PMTK161,0*28"));
      reset();
      GPS_NMEA::end();
    }
  else
    {
      // Device seems active
      // Defer handling final end until ack comes in

      if (!m_ending)
        send_cmd(PSTR("$PMTK161,0*28"));

      m_ending = true;
    }
}

bool
GPS_NMEA_MT3339::active()
{
  return (m_active && !m_ending);
}

void
GPS_NMEA_MT3339::reset(void)
{
  m_sentence = SENTENCE_UNKNOWN;
  m_first_sentence_received = false;
  m_ending = false;

  GPS_NMEA::reset();
}

void
GPS_NMEA_MT3339::factory_reset(void)
{
  send_cmd(PSTR("$PMTK104*37"));
  reset();
  GPS_NMEA::begin();
}

void
GPS_NMEA_MT3339::field(uint8_t field_number, char *new_field)
{
  // trace << endl << PSTR("field_number=") << field_number << PSTR(" field=") << new_field << endl;

  if (field_number == 0)
    {
      if (!strcmp(new_field, "PMTK001"))
        m_sentence = SENTENCE_ACK;
      else if (!strcmp(new_field, "PMTK705"))
        m_sentence = SENTENCE_VERSION;
      return;
    }

  switch (m_sentence)
    {
    case SENTENCE_ACK:
      switch (field_number)
        {
        case 1: // Command
          m_command = strtoul(new_field, NULL, 10);
          break;

        case 2: // Status
          m_status = strtoul(new_field, NULL, 10);
          break;

        default:
          m_sentence = SENTENCE_UNKNOWN;
          break;
        }

    case SENTENCE_VERSION:
      switch (field_number)
        {
        case 1: // Release
          strncpy(m_release, new_field, sizeof(m_release)-1);
          break;

        case 2: // Version
          m_version = strtoul(new_field, NULL, 10);
          break;
        }

    default:
      break;
    }
}

void
GPS_NMEA_MT3339::sentence(bool valid)
{
  //  trace << endl << PSTR("valid=") << valid << PSTR(" sentence=") << m_sentence << PSTR(" command=") << m_command << endl;

  if (valid)
    {
      switch (m_sentence)
        {
        case SENTENCE_ACK:
          switch (m_command)
            {
            case 161: // standby
              // doesn't matter if it succeeded or not
              reset();
              GPS_NMEA::end();
              break;

            case 314: // select sentences
              break;
            }
          break;

        case SENTENCE_VERSION:
          //          trace << PSTR("GPS Device: Release ") << m_release << PSTR(" Version ") << m_version << endl;
          break;

        default:
          if (!m_first_sentence_received && !m_ending)
            {
              m_first_sentence_received = true;
              select_sentences();
            }
        }
    }

  // reset extended sentence
  m_sentence = SENTENCE_UNKNOWN;
  m_command = 0;
  m_status = 0;
}

void
GPS_NMEA_MT3339::send_cmd(str_P cmd)
{
#ifdef GPS_INTERRUPT_IMPL
      // bad idea to trace within Irq...
#endif
  if (m_tracing)
    trace << endl << PSTR("-> ") << cmd << endl;
  m_device->puts_P(cmd);
  m_device->puts_P((str_P)IOStream::CRLF);
}

void
GPS_NMEA_MT3339::select_sentences(void)
{
  //  send_cmd(PSTR("$PMTK605*31"));  // query release and version

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

#ifndef GPS_TIME_ONLY
  send_cmd(PSTR("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"));
#else
  send_cmd(PSTR("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"));
#endif
}

IOStream&
operator<<(IOStream& outs, GPS_NMEA_MT3339& gps_nmea_mt3339)
{
  outs << (GPS_NMEA&)gps_nmea_mt3339;
  return (outs);
}
