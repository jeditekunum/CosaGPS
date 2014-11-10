/**
 * @file ?/GPS_NMEA.hh
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

#ifndef COSA_GPS_NMEA_HH
#define COSA_GPS_NMEA_HH

#include "Cosa/IOStream.hh"

#include "GPS.hh"

/**
 * GPS NMEA (generic)
 *
 * GPS_NMEA can be used with any NMEA compliant GPS device.
 */

class GPS_NMEA : public GPS
#ifdef GPS_INTERRUPT_IMPL
               , public IOStream::Device
#endif
{
public:
  /**
   * Construct GPS_NMEA
   */
  GPS_NMEA(IOStream::Device *device = (IOStream::Device *)NULL);

  /**
   * Begin
   */
  virtual bool begin();

  /**
   * End
   */
  virtual void end();

  /**
   * Reset
   */
  virtual void reset();

  /**
   * Begin tracing GPS data stream
   */
  virtual void begin_tracing();

  /**
   * End tracing GPS data stream
   */
  virtual void end_tracing();

#ifndef GPS_INTERRUPT_IMPL
  virtual void consume();
  virtual void feedchar(char c);
#endif

protected:
  /* Active?  Begin -> active, End -> not active */
  bool m_active;

  /* Are we tracing GPS data stream? */
  bool m_tracing;

  /* GPS device */
  IOStream::Device *m_device;

#ifdef GPS_INTERRUPT_IMPL
  /**
   * Written to by serial driver as soon as character is received.
   * Invoked from Irq handler.
   */
  virtual int putchar(char c);
#endif

#ifndef GPS_TIME_ONLY
  /* Parse position */
  position_t parse_position(char *p);
#endif

  /* Parse and scale */
  int32_t parse_and_scale(char *p, uint8_t places);

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

private:
  /* Kind of sentence */
  enum sentence_t {
    SENTENCE_INVALID,
    SENTENCE_OTHER,
    SENTENCE_GPRMC
#ifndef GPS_TIME_ONLY
    ,
    SENTENCE_GPGGA
#endif
  } __attribute__((packed));

  GPS_VOLATILE sentence_t m_sentence;

  /* Parsing state */
  GPS_VOLATILE uint8_t m_parity;
  GPS_VOLATILE uint8_t m_field_number;
  GPS_VOLATILE char m_field[12];
  GPS_VOLATILE uint8_t m_field_offset;
  GPS_VOLATILE bool m_checksum_field;

  /* Process field buffer */
  void process_field();

  /* Process sentence */
  void process_sentence();

  /* Temporary data */
  GPS_VOLATILE date_t m_tmp_date;
  GPS_VOLATILE gps_time_t m_tmp_gprmc_time;
#ifndef GPS_TIME_ONLY
  GPS_VOLATILE gps_time_t m_tmp_gpgga_time;
  GPS_VOLATILE position_t m_tmp_latitude;
  GPS_VOLATILE position_t m_tmp_longitude;
  GPS_VOLATILE altitude_t m_tmp_altitude;
  GPS_VOLATILE course_t m_tmp_course;
  GPS_VOLATILE speed_t m_tmp_speed;
  GPS_VOLATILE satellites_t m_tmp_satellites;
  GPS_VOLATILE hdop_t m_tmp_hdop;
#endif
};
#endif
