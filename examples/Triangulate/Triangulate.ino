// Copyright 2022 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

/*
 * Wi-Fi Triangulation using NotecardAuxiliaryWiFi
 *
 * This example utilizes NotecardAuxiliaryWiFi to periodically poll the Notecard
 * to see if it has detected motion. If so, it pings the surrounding Wi-Fi
 * access points and sends a list to the Notecard, which will be sent to the
 * Notehub when the next session begins. Triangulation results can be found in
 * the `_geolocate.qo` event for the associated device.
 *
 * This example is designed to demonstrate the capability and usage of the
 * NotecardAuxiliaryWiFi library in the fewest lines of code (quick).
 *
 * Review "Using Cell Tower & Wi-Fi Triangulation", on the Notecard developer
 * site, to understand how to properly configure and utilize this feature.
 * /notecard/notecard-walkthrough/time-and-location-requests/#using-cell-tower-and-wi-fi-triangulation
 *
 * NOTE: This sample is ONLY intended to compile on the ESP32 architecture.
 * Any Notecard/Notecarrier combination can be used, but the sample was created
 * and tested using the Notecarrier-F kit.
 * https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html#wifista
 */

#include <Notecard.h>
#include <NotecardAuxiliaryWiFi.h>

#ifndef PRODUCT_UID
#define PRODUCT_UID ""
#endif

#define productUID PRODUCT_UID

#define serialDebug Serial

using namespace blues;

Notecard notecard;
NotecardAuxiliaryWiFi aux_wifi(notecard);

void setup() {
  // Initialize Debug Output
  serialDebug.begin(115200);
  while (!serialDebug) {
    ; // wait for serial port to connect. Needed for native USB
  }
  notecard.setDebugOutputStream(serialDebug);

  // Initialize Notecard
  notecard.begin();

  // Initialize Auxiliary Wi-Fi
  aux_wifi.begin();

  // Associate the Notecard with your Notehub account
  // _**NOTE:** Wi-Fi location data is only updated with each newly established
  // session with Notehub.io. Therefore, "minimal mode" has been selected,
  // because a new session is established with each manual sync._
  J *cmd = notecard.newCommand("hub.set");
  JAddStringToObject(cmd, "product", productUID);
  JAddStringToObject(cmd, "mode", "minimum");
  notecard.sendRequest(cmd);
}

void loop() {
  // Update Notecard Triangulation Data
  aux_wifi.updateTriangulationData();

  // Log SSIDs submitted to Notecard in the
  // Arduino IDE Serial Monitor (Ctrl+Shift+M)
  aux_wifi.logCachedSsids();

  // Manually sync to send the latest results, provided to the
  // Notecard, on to Notehub.io for final geolocation processing
  notecard.sendRequest(notecard.newCommand("hub.sync"));

  // Wait three (3) minutes between loops
  ::delay(180000);
}
