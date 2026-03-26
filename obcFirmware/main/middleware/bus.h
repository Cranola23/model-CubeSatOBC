#ifndef BUS_H
#define BUS_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "packet.h"   // for subsystem_id_t, bus_msg_t
#include "esp_err.h"

// Init bus system
esp_err_t bus_init(void);

// Register a subsystem queue
void bus_register(subsystem_id_t id, QueueHandle_t queue);

// Get queue for a subsystem
QueueHandle_t bus_get_queue(subsystem_id_t id);

// Send message
BaseType_t bus_send(subsystem_id_t dst, const bus_msg_t *msg);

// Helpers
void bus_send_heartbeat(subsystem_id_t src);
void bus_send_fault(subsystem_id_t src, uint8_t fault_code);


#endif