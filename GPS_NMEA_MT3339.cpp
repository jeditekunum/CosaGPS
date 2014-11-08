/**
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

//#include "Cosa/Trace.hh"

GPS_NMEA_MT3339::GPS_NMEA_MT3339() :
  GPS_NMEA(),
  m_running(false),
  m_sentence(SENTENCE_UNKNOWN),
  m_field_number(0),
  m_first_sentence_received(false),
  m_in_standby(false)
{
}

bool
GPS_NMEA_MT3339::begin(IOStream::Device* dev)
{
  if (m_running)
    return(false);

  m_running = true;

  reset();

  if (!GPS_NMEA::begin(dev))
    return (false);

  wake();

  return (true);
}

void
GPS_NMEA_MT3339::end(void)
{
  standby();

  m_running = false;

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
  m_sentence = SENTENCE_UNKNOWN;
  m_field_number = 0;
  m_first_sentence_received = false;
  m_in_standby = false;

  GPS_NMEA::reset();
}

void
GPS_NMEA_MT3339::factory_reset(void)
{
  reset();

  send_cmd(PSTR("$PMTK104*37"));

  delay(250);  // won't respond to new commands for awhile
}

void
GPS_NMEA_MT3339::standby(void)
{
  // Defer handling of reset until ack comes in

  if (!m_running)
    return;

  if (!m_in_standby)
    send_cmd(PSTR("$PMTK161,0*28"));

  m_in_standby = true;
}

void
GPS_NMEA_MT3339::wake(void)
{
  if (!m_running)
    return;
  
  m_in_standby = false;
  send_cmd(PSTR(""));
}

void
GPS_NMEA_MT3339::field(char *new_field)
{
  if (m_field_number == 0)
    {
      if (!strcmp(new_field, "PMTK001"))
        {
          m_sentence = SENTENCE_ACK;
          m_field_number++;
        }
      return;
    }

  switch (m_sentence)
    {
    case SENTENCE_ACK:
      switch (m_field_number)
        {
        case 1: // Command
          m_command = strtoul(new_field, NULL, 10);
          m_field_number++;
          break;

        case 2: // Status
          m_status = strtoul(new_field, NULL, 10);
          m_field_number++;
          break;

        default:
          m_sentence = SENTENCE_UNKNOWN;
          break;
        }

    default:
      break;
    }
}

void
GPS_NMEA_MT3339::sentence(bool valid)
{
  if (!m_running)
    return;

  if (valid)
    {
      if (m_sentence == SENTENCE_ACK)
        {
          switch (m_command)
            {
            case 161: // standby
              if (m_status == 3) // ok
                {
                  //  trace << PSTR("standby command ack") << endl;
                  reset();
                  m_in_standby = true;
                }
              else
                {
                  //  trace << PSTR("standby command NACK") << endl;
                  m_in_standby = false;  // because it failed
                }
              break;
            }
        }
      else
        {
          if (!m_first_sentence_received && !m_in_standby)
            {
              m_first_sentence_received = true;
              select_sentences();
          }
        }
    }

  // reset extended sentence
  m_sentence = SENTENCE_UNKNOWN;
  m_field_number = 0;
  m_command = 0;
  m_status = 0;
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

