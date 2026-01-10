/* SPDX-FileCopyrightText: 2024 Google LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "connection_rssi_monitor.h"

#include "applib/connection_rssi_service.h"
#include "comm/ble/gap_le_connection.h"
#include "comm/bt_lock.h"
#include "kernel/events.h"
#include "services/common/event_service.h"
#include "services/common/new_timer/new_timer.h"
#include "syscall/syscall.h"
#include "syscall/syscall_internal.h"
#include "system/logging.h"

#include <stdlib.h>
#include <stdint.h>

// Forward declarations for NimBLE functions
// These are defined in bluetooth-fw/nimble but we don't want to pull in all the nimble headers here
struct BTDeviceInternal;
bool pebble_device_to_nimble_conn_handle(const struct BTDeviceInternal *device, uint16_t *handle);
int ble_gap_conn_rssi(uint16_t conn_handle, int8_t *out_rssi);

// -------------------------------------------------------------------------------------------------
// Configuration

//! How often to poll RSSI (in milliseconds)
#define RSSI_POLL_INTERVAL_MS (2000)

//! Minimum RSSI change threshold to generate an event (in dBm)
#define RSSI_CHANGE_THRESHOLD (2)

// -------------------------------------------------------------------------------------------------
// Static State

static TimerID s_rssi_poll_timer = TIMER_INVALID_ID;
static int8_t s_last_rssi = 0;
static bool s_is_monitoring = false;

// -------------------------------------------------------------------------------------------------
// Internal Helpers

static void prv_send_rssi_event(int8_t rssi, bool is_connected) {
  PebbleEvent event = {
    .type = PEBBLE_CONNECTION_RSSI_EVENT,
    .connection_rssi = {
      .rssi = rssi,
      .is_connected = is_connected,
    },
  };
  event_put(&event);
}

static void prv_poll_rssi_callback(void *data) {
  bt_lock();

  // Get the gateway connection (phone connection)
  GAPLEConnection *conn = gap_le_connection_get_gateway();
  if (!conn) {
    // No gateway connection, send disconnected event if we were monitoring
    if (s_is_monitoring) {
      s_is_monitoring = false;
      s_last_rssi = 0;
      bt_unlock();
      prv_send_rssi_event(0, false);
    } else {
      bt_unlock();
    }
    return;
  }

  // Convert device to NimBLE connection handle
  uint16_t conn_handle = 0;
  if (!pebble_device_to_nimble_conn_handle(&conn->device, &conn_handle)) {
    PBL_LOG(LOG_LEVEL_WARNING, "Failed to get NimBLE connection handle for RSSI");
    bt_unlock();
    return;
  }

  // Read RSSI from NimBLE
  int8_t rssi = 0;
  int rc = ble_gap_conn_rssi(conn_handle, &rssi);
  if (rc != 0) {
    PBL_LOG(LOG_LEVEL_WARNING, "Failed to read RSSI from NimBLE: %d", rc);
    bt_unlock();
    return;
  }

  // Store in connection struct
  conn->last_rssi = rssi;

  // Check if RSSI changed significantly
  bool should_send_event = false;
  if (!s_is_monitoring) {
    // First reading, always send event
    should_send_event = true;
    s_is_monitoring = true;
  } else if (abs(rssi - s_last_rssi) >= RSSI_CHANGE_THRESHOLD) {
    // RSSI changed beyond threshold
    should_send_event = true;
  }

  if (should_send_event) {
    s_last_rssi = rssi;
    bt_unlock();
    prv_send_rssi_event(rssi, true);
  } else {
    bt_unlock();
  }
}

// -------------------------------------------------------------------------------------------------
// Event Service Callbacks

static void prv_start_monitoring(PebbleTask task) {
  // Start timer when first app subscribes
  if (!new_timer_scheduled(s_rssi_poll_timer, NULL)) {
    s_is_monitoring = false;  // Reset state
    s_last_rssi = 0;
    new_timer_start(s_rssi_poll_timer, RSSI_POLL_INTERVAL_MS, prv_poll_rssi_callback, NULL,
                    TIMER_START_FLAG_REPEATING);
    PBL_LOG(LOG_LEVEL_DEBUG, "Started RSSI monitoring");
  }
}

static void prv_stop_monitoring(PebbleTask task) {
  // Stop timer when last app unsubscribes
  if (new_timer_scheduled(s_rssi_poll_timer, NULL)) {
    new_timer_stop(s_rssi_poll_timer);
    s_is_monitoring = false;
    s_last_rssi = 0;
    PBL_LOG(LOG_LEVEL_DEBUG, "Stopped RSSI monitoring");
  }
}

// -------------------------------------------------------------------------------------------------
// Syscall Implementation

DEFINE_SYSCALL(ConnectionRSSIState, sys_connection_rssi_peek, void) {
  ConnectionRSSIState state = { .rssi = 0, .is_connected = false };

  bt_lock();
  GAPLEConnection *conn = gap_le_connection_get_gateway();
  if (conn) {
    state.rssi = conn->last_rssi;
    state.is_connected = true;
  }
  bt_unlock();

  return state;
}

// -------------------------------------------------------------------------------------------------
// Public API

void connection_rssi_monitor_init(void) {
  s_rssi_poll_timer = new_timer_create();
  event_service_init(PEBBLE_CONNECTION_RSSI_EVENT, prv_start_monitoring, prv_stop_monitoring);
  PBL_LOG(LOG_LEVEL_DEBUG, "Connection RSSI monitor initialized");
}
