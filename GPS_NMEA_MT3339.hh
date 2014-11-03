/**
 * @file ?/GPS_NMEA_MT3339.hh
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
  GPS_NMEA_MT3339();

  /**
   * Begin
   */
  virtual bool begin(IOStream::Device *);

  /**
   * End
   */
  virtual void end();

  /**
   * Begin monitoring GPS data stream
   */
  virtual void begin_monitor(IOStream::Device*);

  /**
   * End monitoring
   */
  virtual void end_monitor();

  /**
   * Reset
   */
  virtual void reset();

  /**
   * Factory reset
   */
  virtual void factory_reset();

  /**
   * Standby
   */
  virtual void standby();

  /**
   * Wake (exit standby)
   */
  virtual void wake();

protected:
  virtual void new_field(char*);
  virtual void new_sentence();

  /**
   * Print latest gps_nmea information to
   * given stream.
   * @param[in] outs output stream
   * @param[in] gps_nmea_mtk to print
   * @return stream.
   */
  friend IOStream& operator<<(IOStream& outs, GPS_NMEA_MT3339& gps_nmea_mtk);

private:
  bool m_factory_reset_in_progress;
  void send_cmd(str_P);
  void select_sentences();
};
#endif
