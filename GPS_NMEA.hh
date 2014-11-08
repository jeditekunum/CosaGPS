/**
 * @file ?/GPS_NMEA.hh
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

#ifndef COSA_GPS_NMEA_HH
#define COSA_GPS_NMEA_HH

#include "GPS.hh"


/**
 * GPS NMEA (generic)
 */

class GPS_NMEA : public GPS {
public:
  /**
   * Construct GPS_NMEA
   */
  GPS_NMEA();

  /**
   * Begin
   */
  virtual bool begin(IOStream::Device *);

  /**
   * End
   */
  virtual void end();

  /**
   * Begin monitoring data stream
   */
  virtual void begin_monitor(IOStream::Device*);

  /**
   * End monitoring
   */
  virtual void end_monitor();

  /**
   * Consume any data
   */
  virtual void consume();

  /**
   * Reset
   */
  virtual void reset();

protected:
  /* Where GPS device is connected */
  IOStream::Device* m_dev;

  /**
   * GPS_NMEA processes only two sentences, $GPRMC and $GPGGA.  A subclass may
   * handle other sentences by implementing field/sentence.  The argument
   * to sentence is true if checksum was valid, false if not.
   */
  virtual void field(char* new_field);
  virtual void sentence(bool valid);

  /**
   * Print latest gps_nmea information to
   * given stream.
   * @param[in] outs output stream
   * @param[in] gps_nmea to print
   * @return stream.
   */
  friend IOStream& operator<<(IOStream& outs, GPS_NMEA& gps_nmea);

  /* GPS data monitoring stream */
  IOStream::Device* m_monitor_dev;

private:
  /* Kind of sentence */
  enum sentence_t {
    SENTENCE_INVALID,
    SENTENCE_OTHER,
    SENTENCE_GPRMC,
    SENTENCE_GPGGA
  } __attribute__((packed));

  sentence_t m_sentence;

  /* Parsing state */
  uint8_t m_parity;
  uint8_t m_field_number;
  char m_field[12];
  uint8_t m_field_offset;
  bool m_checksum_field;

  /* Process field buffer */
  void process_field();

  /* Process sentence */
  void process_sentence();

  /* Parse degrees */
  position_t parse_position();

  /* Parse and scale */
  int32_t parse_and_scale(char *p, uint8_t places);

  /* Temporary data */
  date_t m_tmp_date;
  gps_time_t m_tmp_gprmc_time;
  gps_time_t m_tmp_gpgga_time;
  position_t m_tmp_latitude;
  position_t m_tmp_longitude;
  altitude_t m_tmp_altitude;
  course_t m_tmp_course;
  speed_t m_tmp_speed;
  satellites_t m_tmp_satellites;
  hdop_t m_tmp_hdop;
};
#endif
