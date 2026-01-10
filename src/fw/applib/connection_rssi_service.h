/* SPDX-FileCopyrightText: 2024 Google LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

//! @addtogroup Foundation
//! @{
//!   @addtogroup EventService
//!   @{
//!     @addtogroup ConnectionRSSIService
//! \brief Monitor BLE connection signal strength
//!
//! The ConnectionRSSIService allows your app to monitor the Received Signal Strength Indicator
//! (RSSI) of the BLE connection between the Pebble watch and the connected phone. RSSI values
//! are measured in dBm and typically range from -30 (very strong signal) to -90 (weak signal)
//! or lower.
//!
//! You can either subscribe to receive events when the RSSI changes, or query the current
//! value on demand using the peek function.
//!
//! **Important notes:**
//! - RSSI values are relative and can vary significantly based on environment and device
//! - Apps should look for trends (improving/degrading signal) rather than absolute thresholds
//! - The service only monitors the gateway connection (phone), not other BLE peripherals
//! - RSSI is updated approximately every 2 seconds when apps are subscribed
//!
//!     @{

//! Data structure containing the current RSSI state
typedef struct {
  //! RSSI value in dBm. Typical range: -30 (excellent) to -90 (poor) or lower.
  //! A value of 0 indicates no data available.
  int8_t rssi;
  //! Whether a BLE connection to the phone is currently active
  bool is_connected;
} ConnectionRSSIState;

//! Callback type for RSSI change events
//! @param state The current RSSI state \ref ConnectionRSSIState
typedef void (*ConnectionRSSIHandler)(ConnectionRSSIState state);

//! Subscribe to the connection RSSI event service. Once subscribed, the handler gets called
//! whenever the RSSI value changes significantly (typically ±2 dBm or more).
//! @param handler A callback to be executed on RSSI change events
void connection_rssi_service_subscribe(ConnectionRSSIHandler handler);

//! Unsubscribe from the connection RSSI event service. Once unsubscribed, the previously
//! registered handler will no longer be called, and RSSI monitoring will stop if no other
//! apps are subscribed.
void connection_rssi_service_unsubscribe(void);

//! Peek at the last known RSSI state without subscribing to events.
//! @return a \ref ConnectionRSSIState containing the last known RSSI data.
//! If not connected, returns `{.rssi = 0, .is_connected = false}`.
ConnectionRSSIState connection_rssi_service_peek(void);

//!     @} // end addtogroup ConnectionRSSIService
//!   @} // end addtogroup EventService
//! @} // end addtogroup Foundation
