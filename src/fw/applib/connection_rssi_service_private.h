/* SPDX-FileCopyrightText: 2024 Google LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "applib/connection_rssi_service.h"
#include "applib/event_service_client.h"

//! Private state structure for connection RSSI service
typedef struct __attribute__((packed)) ConnectionRSSIServiceState {
  ConnectionRSSIHandler handler;
  EventServiceInfo event_info;
} ConnectionRSSIServiceState;

//! Initialize the connection RSSI service state
//! @param state Pointer to the state structure to initialize
void connection_rssi_service_state_init(ConnectionRSSIServiceState *state);
