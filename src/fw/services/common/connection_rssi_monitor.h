/* SPDX-FileCopyrightText: 2024 Google LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "applib/connection_rssi_service.h"

//! Initialize the connection RSSI monitor service.
//! This service periodically polls BLE RSSI for the gateway connection
//! and generates PEBBLE_CONNECTION_RSSI_EVENT events.
void connection_rssi_monitor_init(void);
