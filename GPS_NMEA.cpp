/**
 * @file ?/GPS_NMEA.cpp
 * @version 0.7
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

#include <ctype.h>
#include "Cosa/RTC.hh"
#include "Cosa/Trace.hh"

#include "GPS_NMEA.hh"

/**
 * NMEA messages consist of sentences made up of fields and a checksum.
 * While there are many potential sentences, only 2, $GPRMC and $GPGGA, are
 * needed to provide all the data exposed by the base class GPS.  Sentences
 * may arrive in any order requiring that data is temporarily accumulated
 * here until all sentences needed for an update to GPS have arrived.
 * We know we have associated sentences when the time fields match.
 */

GPS_NMEA::GPS_NMEA(IOStream::Device *device) :
  GPS(),
  m_active(false),
  m_tracing(false),
  m_sentence(SENTENCE_INVALID),
  m_parity(0),
  m_field_number(0),
  m_field_offset(0),
  m_checksum_field(false),
  m_tmp_date(0),
  m_tmp_gprmc_time(0)
#ifndef GPS_TIME_ONLY
  ,
  m_tmp_gpgga_time(1),
  m_tmp_latitude(0),
  m_tmp_longitude(0),
  m_tmp_altitude(0),
  m_tmp_course(0),
  m_tmp_speed(0),
  m_tmp_satellites(0),
  m_tmp_hdop(0)
#endif
{
  m_device = device;
}

bool
GPS_NMEA::begin()
{
  if (m_active)
    return (false);

  m_active = true;

  return (true);
}

void
GPS_NMEA::end()
{
  m_active = false;

  reset();
}

bool
GPS_NMEA::active()
{
  return (m_active);
}

void
GPS_NMEA::reset(void)
{
  m_sentence = SENTENCE_INVALID;
  m_field_number = 0;
  m_field_offset = 0;
  m_checksum_field = false;
  m_tmp_date = 0;
  m_tmp_gprmc_time = 0;
#ifndef GPS_TIME_ONLY
  m_tmp_gpgga_time = 1;
  m_tmp_latitude = 0;
  m_tmp_longitude = 0;
  m_tmp_altitude = 0;
  m_tmp_course = 0;
  m_tmp_speed = 0;
  m_tmp_satellites = 0;
  m_tmp_hdop = 0;
#endif

  GPS::reset();
}

void
GPS_NMEA::begin_tracing()
{
  m_tracing = true;
}

void
GPS_NMEA::end_tracing()
{
  m_tracing = false;
}

#ifndef GPS_INTERRUPT_IMPL
void
GPS_NMEA::consume(void)
{
  while (m_device->available())
    feedchar(m_device->getchar());
}
#endif

#ifdef GPS_INTERRUPT_IMPL
int
GPS_NMEA::putchar(char c)
#else
void
GPS_NMEA::feedchar(char c)
#endif
{
  if (c != '\r' && c != '\n' && (c < ' ' || c > '~'))
    {
      if (m_tracing)
        trace << PSTR("_");
      return;
    }

  if (m_tracing)
    {
#ifdef GPS_INTERRUPT_IMPL
      // bad idea to trace within Irq...
#endif
      if (c == '$')
        trace << PSTR("<- ");
      trace << c;
    }

  if (m_sentence != SENTENCE_INVALID ||
      c == '$')
    {
      switch(c)
        {
        case '$':  // start of sentence
          m_sentence = SENTENCE_OTHER;  // unknown at this point
          m_parity = 0;
          m_field_number = 0;
          m_field_offset = 0;
          m_checksum_field = false;
          break;

        case ',':
          m_parity ^= c;
        case '*':
          m_field[m_field_offset] = '\0';
          process_field();
          m_field_number++;
          m_field_offset = 0;
          m_checksum_field = c == '*';
          break;

        case '\r':
          m_field[m_field_offset] = '\0';
          process_sentence();
          m_sentence = SENTENCE_INVALID;  // anything until next '$' is invalid
          break;

        case '\n':
          break;

        default:
          if (!m_checksum_field)
            m_parity ^= c;

          if (m_field_offset < sizeof(m_field) - 1)
            m_field[m_field_offset++] = c;
          else
            {
              /* Field overflow */
              m_field_offset = 0;
              m_sentence = SENTENCE_INVALID;
            }
        }
    }

#ifdef GPS_INTERRUPT_IMPL
  return (c);
#endif
}

#ifndef GPS_TIME_ONLY
GPS_NMEA::position_t
GPS_NMEA::parse_position(char *p)
{
  position_t result;
  uint32_t scaled;
  uint32_t hundred_thousandths_of_minutes;


  /* Result is millionths of a degree */

  /* DDMM.MMMMM */

  scaled = parse_and_scale(p, 5);

  /* DDMMMMMMM */

  /* Get minutes */
  hundred_thousandths_of_minutes = scaled % 10000000;

  /* Truncate minutes leaving degrees in millionths */
  result = (scaled / 10000000) * 1000000;

  /* Convert minutes to degrees and add back in */
  result += (hundred_thousandths_of_minutes + 3) / 6;

  return result;
}
#endif

int32_t
GPS_NMEA::parse_and_scale(char *p, uint8_t places)
{
  uint32_t result = 0;
  bool negative;


  while (*p && (*p == ' ' || *p == '\t'))
    p++;

  negative = (*p == '-');
  if (negative)
    p++;

  while (isdigit(*p))
    result = (result * 10) + (*(p++) - '0');

  if (*p == '.')
    p++;

  while (places--)
    {
      result *= 10;
      if (isdigit(*p))
        result += (*(p++) - '0');
    }

  if (negative)
    result = -result;

  return result;
}

void
GPS_NMEA::field(uint8_t field_number, char *new_field)
{
  UNUSED(field_number);
  UNUSED(new_field);

  /* May be implemented by subsclasses */
}

void
GPS_NMEA::sentence(bool valid)
{
  UNUSED(valid);

  /* May be implemented by subsclasses */
}

IOStream&
operator<<(IOStream& outs, GPS_NMEA& gps_nmea)
{
  outs << (GPS&)gps_nmea;
  return (outs);
}

void
GPS_NMEA::process_field()
{
  if (m_field_number == 0)
    {
      if (!strcmp((char*)m_field, "GPRMC"))
        m_sentence = SENTENCE_GPRMC;
      else
        {
#ifndef GPS_TIME_ONLY
          if (!strcmp((char*)m_field, "GPGGA"))
            m_sentence = SENTENCE_GPGGA;
          else
#endif
            field(m_field_number, (char*)m_field);  // Subclasses may implement other sentences
        }
      return;
    }

  switch (m_sentence)
    {
    case SENTENCE_GPRMC:
      switch (m_field_number)
        {
        case 1:  // Time
          m_tmp_gprmc_time = parse_and_scale((char*)m_field, 3);
          break;

        case 2: // Validity
          if (m_field[0] != 'A')
            m_sentence = SENTENCE_INVALID;
          break;

        case 3: // Latitude
#ifndef GPS_TIME_ONLY
          m_tmp_latitude = parse_position((char*)m_field);
#endif
          break;

        case 4: // North/South
#ifndef GPS_TIME_ONLY
          if (m_field[0] == 'S')
            m_tmp_latitude = -m_tmp_latitude;
#endif
          break;

        case 5: // Longitude
#ifndef GPS_TIME_ONLY
          m_tmp_longitude = parse_position((char*)m_field);
#endif
          break;

        case 6: // East/West
#ifndef GPS_TIME_ONLY
          if (m_field[0] == 'W')
            m_tmp_longitude = -m_tmp_longitude;
#endif
          break;

        case 7: // Speed
#ifndef GPS_TIME_ONLY
          m_tmp_speed = parse_and_scale((char*)m_field, 2);
#endif
          break;

        case 8: // Course
#ifndef GPS_TIME_ONLY
          m_tmp_course = parse_and_scale((char*)m_field, 2);
#endif
          break;

        case 9: // Date
          m_tmp_date = strtoul((char*)m_field, NULL, 10);
          break;

        case 10: // magnetic variation
        case 11: // E or W
          break;

        case 12: // unknown
          break;

        default:
          m_sentence = SENTENCE_INVALID;
          break;
        }
      break;

#ifndef GPS_TIME_ONLY
    case SENTENCE_GPGGA:
      switch (m_field_number)
        {
        case 1:  // Time
          m_tmp_gpgga_time = parse_and_scale((char*)m_field, 3);
          break;

        case 2: // Latitude
          m_tmp_latitude = parse_position((char*)m_field);
          break;

        case 3: // North/South
          if (m_field[0] == 'S')
            m_tmp_latitude = -m_tmp_latitude;
          break;

        case 4: // Longitude
          m_tmp_longitude = parse_position((char*)m_field);
          break;

        case 5: // East/West
          if (m_field[0] == 'W')
            m_tmp_longitude = -m_tmp_longitude;
          break;

        case 6: // Fix valid?
          if (m_field[0] == '0')
            m_sentence = SENTENCE_INVALID;
          break;

        case 7: // Satellites
          m_tmp_satellites = strtoul((char*)m_field, NULL, 10);
          break;

        case 8: // HDOP
          m_tmp_hdop = parse_and_scale((char*)m_field, 2);
          break;

        case 9: // Altitude
          m_tmp_altitude = parse_and_scale((char*)m_field, 2);
          break;

        case 10: // Altitude units
        case 11: // Geoidal separation
        case 12: // Geoidal separation units
        case 13: // Age of differential data
        case 14: // Differential reference station id
          break;

        default:
          m_sentence = SENTENCE_INVALID;
          break;
        }
#endif

    default:
      break;
    }

  /* Subclass may implement field() to handle other sentences */
  field(m_field_number, (char*)m_field);
}

void
GPS_NMEA::process_sentence()
{
  uint32_t checksum;


  /* Last field is the checksum */
  checksum = strtoul((char*)m_field, NULL, 16);

  if (checksum == m_parity)
    {
#ifdef GPS_TIME_ONLY
      if (m_sentence == SENTENCE_GPRMC)
#else
      if (m_tmp_gprmc_time == m_tmp_gpgga_time &&
          m_tmp_satellites >= GPS_MINIMUM_SATELLITES)
#endif
        {
          m_date = m_tmp_date;
          m_time = m_tmp_gprmc_time;
#ifndef GPS_TIME_ONLY
          m_latitude = m_tmp_latitude;
          m_longitude = m_tmp_longitude;
          m_altitude = m_tmp_altitude;
          m_course = m_tmp_course;
          m_speed = m_tmp_speed;
          m_satellites = m_tmp_satellites;
          m_hdop = m_tmp_hdop;
#endif
          m_last_update = RTC::millis();

          m_tmp_gprmc_time = 0;
        }

      /* Subclass may implement sentence() to handle other sentences */
      sentence(true);
    }
  else
    sentence(false);
}
