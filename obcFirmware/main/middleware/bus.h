#ifndef BUS_H
#define BUS_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "packet.h"   // for subsystem_id_t, bus_msg_t
#include "esp_err.h"

esp_err_t bus_init(void);

void bus_register(subsystem_id_t id, QueueHandle_t queue);

BaseType_t bus_send(subsystem_id_t dst, const bus_msg_t *msg);

#endif