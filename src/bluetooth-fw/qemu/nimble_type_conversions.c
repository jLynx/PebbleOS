/* SPDX-FileCopyrightText: 2025 Google LLC */
/* SPDX-License-Identifier: Apache-2.0 */

// Stub implementations of NimBLE type conversion functions for QEMU

#include <bluetooth/bluetooth_types.h>
#include <stdbool.h>
#include <stdint.h>

// Stub implementation - QEMU doesn't have real NimBLE connection handles
bool pebble_device_to_nimble_conn_handle(const BTDeviceInternal *device, uint16_t *handle) {
  // In QEMU, we don't have real connection handles, so just return a dummy value
  if (handle) {
    *handle = 0;
  }
  // Return true if there's a valid device pointer (simulating a connection)
  return (device != NULL);
}

// Stub implementation - QEMU doesn't support real RSSI readings
int ble_gap_conn_rssi(uint16_t conn_handle, int8_t *out_rssi) {
  // In QEMU, return a simulated RSSI value
  // Typical BLE RSSI values range from -100 to 0 dBm
  // Use -60 as a reasonable "good connection" value
  if (out_rssi) {
    *out_rssi = -60;
  }
  return 0;  // Success
}
