// Copyright 2022 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#ifndef NOTECARD_AUXILIARY_WIFI_H
#define NOTECARD_AUXILIARY_WIFI_H

#include <stddef.h>
#include <stdint.h>

class Notecard;

namespace blues {

// Wi-Fi triangulation support.  This module periodically polls the notecard to
// see if it has moved.  If so, it determines the surrounding access points and
// sends the list to the Notecard, where the list will be sent to the Notehub
// when the next session begins.
// https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html#wifista

class NotecardAuxiliaryWiFi {
  public:
    NotecardAuxiliaryWiFi (Notecard & notecard);

    int begin (void);
    void end (void);
    void logCachedSsids (void);
    int updateTriangulationData (bool clear_location_on_empty_scan = false, bool use_cache = true);

  private:
    bool cacheIsValid (void);
    int enqueueResults (const char * ssid_buffer, bool clear_location_on_empty_scan);
    template<typename fn> void for_each_cwlap (fn func);

    size_t _ap_count;
    uint32_t _last_scan_ms;
    bool _library_initialized;
    Notecard & _notecard;
};

} // namespace blues

#endif // NOTECARD_AUXILIARY_WIFI_H
