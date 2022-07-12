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

/*!
 * \brief NotecardAuxiliaryWiFi is a utility class to make external Wi-Fi
 * functionality available to the Blues Wireless Notecard
 */
class NotecardAuxiliaryWiFi {
  public:
    /*!
     * \brief NotecardAuxiliaryWiFi (constructor)
     * \param[in] Notecard object
     * \warning The Notecard object provided MUST be intialized.
     */
    NotecardAuxiliaryWiFi (Notecard & notecard);

    /*!
     * \brief Updates Host Wi-Fi configuration to support Notecard operations
     * \returns Zero (0) upon success, or a number representing a failure case
     */
    int begin (void);

    /*!
     * \brief Releases claimed resources
     */
    void end (void);

    /*!
     * \brief Logs SSID strings collected during scan (as well-formed `+CWLAP`)
     */
    void logCachedSsids (void);

    /*!
     * \brief Scan SSIDs and update Notecard's triangulation cache
     *
     * This method does _NOT_ perform a geospatial triangulation, nor return or
     * update latitude and longitude coordinates. Rather, it updates the
     * `+CWLAP` string cached on the Notecard. This string is submitted to
     * Notehub with each new session, and Notehub will convert the string to
     * a geospatial location and applied it to all Notes in the session.
     *
     * \param[in] clear_cache_on_empty_scan Erase previously cached location
     *                                      data when scan returns empty
     * \param[in] use_cache Use previously cached location data if device has
     *                      not moved since last scan
     * \returns Zero (0) upon success, or a number representing a failure case
     */
    int updateTriangulationData (bool clear_cache_on_empty_scan = false, bool use_cache = true);

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
