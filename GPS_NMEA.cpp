/**
 * @file ?/GPS_NMEA.cpp
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

#include <ctype.h>

#include "GPS_NMEA.hh"

#include "Cosa/RTC.hh"


/*
 * GPS NMEA (generic)
 */

/*
 * For each update interval we require that both $GPRMC and $GPGGA be
 * seen before we update the real state of the object (GPS::) since the state
 * contains info from both of them.  This is accomplished by comparing
 * the times from each message.
 */

GPS_NMEA::GPS_NMEA() :
  GPS(),
  m_sentence(SENTENCE_INVALID),
  m_parity(0),
  m_field_number(0),
  m_field_offset(0),
  m_checksum_field(false),
  m_tmp_date(0),
  m_tmp_gprmc_time(0),
  m_tmp_gpgga_time(1),
  m_tmp_latitude(0),
  m_tmp_longitude(0),
  m_tmp_altitude(0),
  m_tmp_course(0),
  m_tmp_speed(0),
  m_tmp_satellites(0),
  m_tmp_hdop(0)
{
  m_field[0] = '\0';
}

bool
GPS_NMEA::begin(IOStream::Device* dev)
{
  GPS_NMEA::reset();

  m_dev = dev;

  return true;
}

void
GPS_NMEA::end()
{
  GPS_NMEA::reset();
}

void
GPS_NMEA::begin_monitor(IOStream::Device* monitor_dev)
{
  m_monitor_dev = monitor_dev;
}

void
GPS_NMEA::end_monitor()
{
  m_monitor_dev = NULL;
}

void
GPS_NMEA::consume()
{
  char c;


  while (m_dev->available())
    {
      c = m_dev->getchar();

      if (m_monitor_dev)
        {
          if (c == '$')
            m_monitor_dev->puts_P(PSTR("<- "));
          m_monitor_dev->putchar(c);
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
    }
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
  m_tmp_gpgga_time = 1;
  m_tmp_latitude = 0;
  m_tmp_longitude = 0;
  m_tmp_altitude = 0;
  m_tmp_course = 0;
  m_tmp_speed = 0;
  m_tmp_satellites = 0;
  m_tmp_hdop = 0;

  GPS::reset();
}

void
GPS_NMEA::new_field(char *field)
{
  UNUSED(field);

  /* May be implemented by subsclasses */
}

void
GPS_NMEA::new_sentence(void)
{
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
      if (!strcmp(m_field, "GPRMC"))
        m_sentence = SENTENCE_GPRMC;
      else
        if (!strcmp(m_field, "GPGGA"))
          m_sentence = SENTENCE_GPGGA;
      return;
    }

  switch (m_sentence)
    {
    case SENTENCE_GPRMC:
      switch (m_field_number)
        {
        case 1:  // Time
          m_tmp_gprmc_time = parse_and_scale(m_field, 2);
          break;

        case 2: // Validity
          if (m_field[0] != 'A')
            m_sentence = SENTENCE_INVALID;
          break;

        case 3: // Latitude
          m_tmp_latitude = parse_position();
          break;

        case 4: // North/South
          if (m_field[0] == 'S')
            m_tmp_latitude = -m_tmp_latitude;
          break;

        case 5: // Longitude
          m_tmp_longitude = parse_position();
          break;

        case 6: // East/West
          if (m_field[0] == 'W')
            m_tmp_longitude = -m_tmp_longitude;
          break;

        case 7: // Speed
          m_tmp_speed = parse_and_scale(m_field, 2);
          break;

        case 8: // Course
          m_tmp_course = parse_and_scale(m_field, 2);
          break;

        case 9: // Date
          m_tmp_date = strtol(m_field, NULL, 10);
          break;

        default:
          break;
        }
      break;

    case SENTENCE_GPGGA:
      switch (m_field_number)
        {
        case 1:  // Time
          m_tmp_gpgga_time = parse_and_scale(m_field, 2);
          break;

        case 2: // Latitude
          m_tmp_latitude = parse_position();
          break;

        case 3: // North/South
          if (m_field[0] == 'S')
            m_tmp_latitude = -m_tmp_latitude;
          break;

        case 4: // Longitude
          m_tmp_longitude = parse_position();
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
          m_tmp_satellites = strtol(m_field, NULL, 10);
          break;

        case 8: // HDOP
          m_tmp_hdop = parse_and_scale(m_field, 2);
          break;

        case 9: // Altitude
          m_tmp_altitude = parse_and_scale(m_field, 2);
          break;

        default:
          break;
        }

    default:
      break;
    }

  /* Subclass may implement new_field to handle other sentences */
  new_field(m_field);
}

void
GPS_NMEA::process_sentence()
{
  uint32_t checksum;


  /* Last field is the checksum */
  checksum = strtol(m_field, NULL, 16);

  if (checksum == m_parity)
    {
      if (m_tmp_gprmc_time == m_tmp_gpgga_time &&
          m_tmp_satellites >= GPS_MINIMUM_SATELLITES)
        {
          m_date = m_tmp_date;
          m_time = m_tmp_gprmc_time;
          m_latitude = m_tmp_latitude;
          m_longitude = m_tmp_longitude;
          m_altitude = m_tmp_altitude;
          m_course = m_tmp_course;
          m_speed = m_tmp_speed;
          m_satellites = m_tmp_satellites;
          m_hdop = m_tmp_hdop;

          m_last_update = RTC::millis();

          m_tmp_gprmc_time = 0;
        }
    }

  /* Subclass may implement new_sentence to handle other sentences */
  new_sentence();
}

GPS_NMEA::position_t
GPS_NMEA::parse_position()
{
  position_t result;
  uint32_t scaled;
  uint32_t hundred_thousandths_of_minutes;


  /* Result is millionths of a degree */
  
  /* DDMM.MMMMM */
  
  scaled = parse_and_scale(m_field, 5);

  /* DDMMMMMMM */

  /* Get minutes */
  hundred_thousandths_of_minutes = scaled % 10000000;

  /* Truncate minutes leaving degrees in millionths */
  result = (scaled / 10000000) * 1000000;
  
  /* Convert minutes to degrees and add back in */
  result += (hundred_thousandths_of_minutes + 3) / 6;
 
  return result;
}

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
