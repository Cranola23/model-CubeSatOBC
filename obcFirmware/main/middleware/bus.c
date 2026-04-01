#include "bus.h"
#include "packet.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include <string.h>

static const char *TAG = "BUS";

#define MAX_SUBSYS 6
static QueueHandle_t s_queues[MAX_SUBSYS] = {0};

//TODO: MUTEX IMPLEMENTATION

//Initialize (return esp_err_t)
esp_err_t bus_init(void){
    memset(s_queues, 0, sizeof(s_queues));
    ESP_LOGI(TAG, "Bus initialized");
    return ESP_OK;
}

//Register and log to queue
void bus_register(subsystem_id_t id, QueueHandle_t queue){
    if(id >= MAX_SUBSYS){
        ESP_LOGE(TAG, "Invalid subsystem %d", id);
        return;
    }
    s_queues[id] = queue;
}

//Send
BaseType_t bus_send(subsystem_id_t dst, const bus_msg_t *msg){
    if (dst >= MAX_SUBSYS) return pdFALSE;
    QueueHandle_t q = s_queues[dst];
    if (!q) return pdFALSE;
    return xQueueSend(q, msg, 0); //non-blocking
}