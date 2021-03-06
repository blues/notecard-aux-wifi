// Copyright 2022 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "NotecardAuxiliaryWiFi.h"

#include <algorithm>
#include <sstream>
#include <string>

#include <Arduino.h>
#include <Notecard.h>
#include <WiFi.h>

// https://serverfault.com/questions/45439/what-is-the-maximum-length-of-a-wifi-access-points-ssid
#define SSID_LEN_MAX 32
#define WIFI_DISCONNECT_TIMEOUT_MS 10000

using namespace blues;

char
scrubSsidChar (
    char c_
) {
    char result;

    if (c_ < ' ' || c_ >= 0x7f || c_ == '"') {
        result = '.';
    } else {
        result = c_;
    }

    return result;
}

NotecardAuxiliaryWiFi::NotecardAuxiliaryWiFi (
    Notecard & notecard_
) :
    _ap_count(0),
    _last_scan_ms(0),
    _library_initialized(false),
    _notecard(notecard_)
{ }

int
NotecardAuxiliaryWiFi::begin (
    void
) {
    int err;

    if (!_library_initialized) {
        //TODO: Capture previous configuration of Notecard

        // Prepare the Notecard for Wi-Fi triangulation input
        J *req = _notecard.newRequest("card.triangulate");
        if (req != NULL) {
            JAddStringToObject(req, "mode", "wifi");
            JAddBoolToObject(req, "on", true);
            JAddBoolToObject(req, "set", true);
            J *rsp = _notecard.requestAndResponse(req);
            if (NoteResponseError(rsp)) {
                err = -2;
                _notecard.logDebugf("[ERROR][Wi-Fi] %s\r\n", JGetString(rsp, "err"));
            } else {
                err = 0;
            }
            NoteDeleteResponse(rsp);
        } else {
            err = -1;
            _notecard.logDebug("[ERROR][Wi-Fi] No memory available for new Note!\r\n");
        }

        if (!err) {
            //TODO: Capture previous configuration of ESP32

            // Go into station mode to enable scanning
            WiFi.mode(WIFI_STA);

            // Disconnect ESP32 from any/all access points
            if (WiFi.disconnect()) {
                // Block until disconnection completed
                const size_t begin_disconnect_ms = ::millis();
                bool timeout_occured = false;
                while (WiFi.isConnected() && !(timeout_occured = (::millis() >= (begin_disconnect_ms + WIFI_DISCONNECT_TIMEOUT_MS))));
                if (timeout_occured) {
                    err = -3;
                    _notecard.logDebug("[ERROR][Wi-Fi] Timeout while scanning for Wi-Fi!\r\n");
                }
            } else {
                err = 0;
                _notecard.logDebug("[INFO ][Wi-Fi] Library already initialized prior to `begin()`\r\n");
            }
        }
    } else {
        err = 0;
    }

    _library_initialized = !err;
    return err;
}

bool
NotecardAuxiliaryWiFi::cacheIsValid (
    void
) {
    bool valid;

    // Determine whether or not the module has moved since the last time we scanned wifi.
    J *rsp = _notecard.requestAndResponse(_notecard.newRequest("card.triangulate"));
    if (rsp != NULL) {
        uint32_t wifiTextLength = JGetInt(rsp, "length");
        uint32_t wifiScanTimeSecs = JGetInt(rsp, "time");
        uint32_t movementTimeSecs = JGetInt(rsp, "motion");
        if (wifiTextLength > 1 && wifiScanTimeSecs != 0 && movementTimeSecs != 0 && wifiScanTimeSecs > movementTimeSecs) {
            // Triangulation has been performed since last
            // movement and a cached location exists.
            valid = true;
        } else {
            // Movement detected since last triangulation
            valid = false;
        }
        NoteDeleteResponse(rsp);
    } else {
        // ERROR CONDITION
        valid = false;  // invalidate cache as a precaution
    }

    return valid;
}

void
NotecardAuxiliaryWiFi::end (
    void
) {
    //TODO: Restore ESP32 to previous configuration
    //TODO: Restore Notecard to previous configuration
}

int
NotecardAuxiliaryWiFi::enqueueResults (
    const char * ssid_buffer_,
    bool clear_location_on_empty_scan_
) {
    int err;

    if (_ap_count || clear_location_on_empty_scan_) {
        J *req = _notecard.newRequest("card.triangulate");
        if (req != NULL) {
            if (_ap_count) {
                JAddStringToObject(req, "text", ssid_buffer_);
            } else {
                JAddStringToObject(req, "text", "-");
            }
            J *rsp = _notecard.requestAndResponse(req);
            if (NoteResponseError(rsp)) {
                err = -2;
                _notecard.logDebugf("[ERROR][Wi-Fi] %s\r\n", JGetString(rsp, "err"));
            } else {
                err = 0;  // Notecard location data updated accordingly
            }
            NoteDeleteResponse(rsp);
        } else {
            err = -1;
            _notecard.logDebug("[ERROR][Wi-Fi] No memory available for new Note!\r\n");
        }
    } else {
        err = 0;  // No SSIDs were provided and no action was taken
    }

    return err;
}

template<typename fn>
void
NotecardAuxiliaryWiFi::for_each_cwlap (
    fn func_
) {
    for (int i = 0; i < _ap_count; i++) {
        std::ostringstream cwlap_stream;

        // Clean the raw SSID value, because there's some really bad stuff
        // out there - including strings with "quotes" and emoji! ????
        std::string clean_ssid(WiFi.SSID(i).length(), '\0');
        clean_ssid = WiFi.SSID(i).c_str();  // Implmentation of `String::end`
                                            // disallows `std::transform` to be
                                            // used on it directly
        std::transform(clean_ssid.begin(), clean_ssid.end(),
                       clean_ssid.begin(), scrubSsidChar);

        // Compile information as an ESP-AT compatible command
        cwlap_stream << "+CWLAP:("
                     << WiFi.encryptionType(i) << ','
                     << '"' << clean_ssid << '"' << ','
                     << WiFi.RSSI(i) << ','
                     << '"' << WiFi.BSSIDstr(i).c_str() << '"' << ','
                     << WiFi.channel(i) << ")\r\n";
        func_(cwlap_stream.str().c_str());
    }
}

void
NotecardAuxiliaryWiFi::logCachedSsids (
    void
) {
    for_each_cwlap([this](const char * cwlap_){ _notecard.logDebug(cwlap_); });
}

int
NotecardAuxiliaryWiFi::updateTriangulationData (
    bool clear_location_on_empty_scan_,
    bool use_cache_
) {
    int err;

    // Scan networks (if necessary)
    if (!use_cache_ || !cacheIsValid()) {
        _last_scan_ms = millis();

        // Initiate network scan
        int ap_count = WiFi.scanNetworks();
        if (ap_count < 0) {
            _notecard.logDebug("[ERROR][Wi-Fi] AP scan failed!\r\n");
            _ap_count = 0;
            _last_scan_ms = 0;
            err = -1;
        } else {
            _ap_count = ap_count;
            _notecard.logDebugf("[INFO ][Wi-Fi] <%d> access points found.\r\n", _ap_count);
            err = 0;
        }
    }

    // Enumerate the networks and compile triangulation string
    std::ostringstream scan_stream;
    for_each_cwlap([&scan_stream](const char * cwlap_){ scan_stream << cwlap_; });
    scan_stream << "\r\n";

    // Enqueue triangulation information
    err = enqueueResults(scan_stream.str().c_str(), clear_location_on_empty_scan_);

    return err;
}
