/**
 * @file ?/GPS_NMEA_MT3339.hh
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

#ifndef COSA_GPS_NMEA_MT3339_HH
#define COSA_GPS_NMEA_MT3339_HH

#include "GPS_NMEA.hh"


/**
 * GPS NMEA MT3339
 */

class GPS_NMEA_MT3339 : public GPS_NMEA {
public:
  /**
   * Construct GPS_NMEA_MT3339
   */
  GPS_NMEA_MT3339(IOStream::Device *device = (IOStream::Device *)NULL);

  /**
   * Begin
   */
  virtual bool begin();

  /**
   * End
   */
  virtual void end();

  /**
   * Active?
   */
  virtual bool active();

  /**
   * Reset
   */
  virtual void reset();

  /**
   * Factory reset
   */
  virtual void factory_reset();

protected:
  virtual void field(uint8_t field_number, char* new_field);
  virtual void sentence(bool valid);

  /**
   * Print latest gps_nmea_mtk information to
   * given stream.
   * @param[in] outs output stream
   * @param[in] gps_nmea_mtk to print
   * @return stream.
   */
  friend IOStream& operator<<(IOStream& outs, GPS_NMEA_MT3339& gps_nmea_mtk);

private:
  /* Kind of sentence (extended) */
  enum sentence_t {
    SENTENCE_UNKNOWN,
    SENTENCE_ACK,
    SENTENCE_VERSION
  } __attribute__((packed));

  GPS_VOLATILE sentence_t m_sentence;

  GPS_VOLATILE uint16_t m_command;
  GPS_VOLATILE uint8_t m_status;
  GPS_VOLATILE char m_release[32];
  GPS_VOLATILE uint16_t m_version;

  GPS_VOLATILE bool m_first_sentence_received;
  GPS_VOLATILE bool m_ending;
  void send_cmd(str_P);
  void select_sentences();
};
#endif
