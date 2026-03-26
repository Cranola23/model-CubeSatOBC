#include "bus.h"
#include "packet.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
 
static const char *TAG = "BUS";
 
/* One queue handle per subsystem ID.
 * Index 0 unused (no SUBSYS_NONE); IDs are 1-based. */
#define MAX_SUBSYS 8
static QueueHandle_t s_queues[MAX_SUBSYS] = {NULL};
 
esp_err_t bus_init(void)
{
    memset(s_queues, 0, sizeof(s_queues));
    ESP_LOGI(TAG, "Software bus initialised (%d subsystem slots)", MAX_SUBSYS);
    return ESP_OK;
}
 
void bus_register(subsystem_id_t id, QueueHandle_t queue)
{
    if (id == 0 || id >= MAX_SUBSYS) {
        ESP_LOGE(TAG, "bus_register: invalid subsystem id %d", id);
        return;
    }
    s_queues[id] = queue;
    ESP_LOGI(TAG, "Registered subsystem 0x%02X", id);
}
 
QueueHandle_t bus_get_queue(subsystem_id_t id)
{
    if (id == 0 || id >= MAX_SUBSYS) return NULL;
    return s_queues[id];
}
 
BaseType_t bus_send(subsystem_id_t dst, const bus_msg_t *msg)
{
    if (dst == 0 || dst >= MAX_SUBSYS) return pdFALSE;
    QueueHandle_t q = s_queues[dst];
    if (!q) {
        ESP_LOGW(TAG, "bus_send: no queue for subsystem 0x%02X", dst);
        return pdFALSE;
    }
    BaseType_t ret = xQueueSend(q, msg, 0);   /* non-blocking: drop if full */
    if (ret != pdTRUE) {
        ESP_LOGW(TAG, "bus_send: queue full for subsystem 0x%02X — dropped", dst);
    }
    return ret;
}
 
void bus_send_heartbeat(subsystem_id_t src)
{
    bus_msg_t msg = {
        .type         = MSG_HEARTBEAT,
        .src          = src,
        .dst          = SUBSYS_CDH,
        .timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000),
        .data_len     = 0,
    };
    bus_send(SUBSYS_CDH, &msg);
}
 
void bus_send_fault(subsystem_id_t src, uint8_t fault_code)
{
    bus_msg_t msg = {
        .type         = MSG_FAULT,
        .src          = src,
        .dst          = SUBSYS_CDH,
        .timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000),
        .data_len     = 1,
    };
    msg.data[0] = fault_code;
    bus_send(SUBSYS_CDH, &msg);
}