/* SPDX-FileCopyrightText: 2024 Google LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "connection_rssi_service.h"
#include "connection_rssi_service_private.h"

#include "event_service_client.h"
#include "kernel/events.h"
#include "syscall/syscall.h"
#include "system/passert.h"

#include "process_state/app_state/app_state.h"
#include "process_state/worker_state/worker_state.h"

// ----------------------------------------------------------------------------------------------------

static ConnectionRSSIServiceState* prv_get_state(PebbleTask task) {
  if (task == PebbleTask_Unknown) {
    task = pebble_task_get_current();
  }

  if (task == PebbleTask_App) {
    return app_state_get_connection_rssi_service_state();
  } else if (task == PebbleTask_Worker) {
    return worker_state_get_connection_rssi_service_state();
  } else {
    WTF;
  }
}

static void prv_do_handle(PebbleEvent *e, void *context) {
  ConnectionRSSIServiceState *state = prv_get_state(PebbleTask_Unknown);
  PBL_ASSERTN(state->handler != NULL);

  ConnectionRSSIState rssi_state = {
    .rssi = e->connection_rssi.rssi,
    .is_connected = e->connection_rssi.is_connected,
  };

  state->handler(rssi_state);
}

void connection_rssi_service_subscribe(ConnectionRSSIHandler handler) {
  ConnectionRSSIServiceState *state = prv_get_state(PebbleTask_Unknown);
  state->handler = handler;
  event_service_client_subscribe(&state->event_info);
}

void connection_rssi_service_unsubscribe(void) {
  ConnectionRSSIServiceState *state = prv_get_state(PebbleTask_Unknown);
  event_service_client_unsubscribe(&state->event_info);
  state->handler = NULL;
}

ConnectionRSSIState connection_rssi_service_peek(void) {
  return sys_connection_rssi_peek();
}

void connection_rssi_service_state_init(ConnectionRSSIServiceState *state) {
  *state = (ConnectionRSSIServiceState) {
    .event_info = {
      .type = PEBBLE_CONNECTION_RSSI_EVENT,
      .handler = &prv_do_handle,
    },
  };
}
