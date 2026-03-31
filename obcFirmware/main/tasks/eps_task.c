#include "bus.h"
#include "packet.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

//Electrical and power systems task
void eps_task(void *pvParameters){
    //eps_task queue registration with bus
    QueueHandle_t eps_queue = xQueueCreate(10, sizeof(bus_msg_t));
    bus_register(SUBSYS_EPS, eps_queue);
    
    while(1){
        pkt_eps_telem_t eps_data;
        
        //simulating data
        eps_data.battery_pct = 75 + (rand()%5);
        eps_data.bus_voltage_v = 3.7 + (rand()%10) * 0.01;

        //filling bus message  
        bus_msg_t msg;
        msg.type = MSG_TELEMETRY;
        msg.src = SUBSYS_EPS;
        msg.dst = SUBSYS_CDH;
        
        memcpy(msg.data, &eps_data, sizeof(eps_data));
        msg.len = sizeof(eps_data);

        //send
        bus_send(SUBSYS_CDH, &msg);
        vTaskDelay(pdMS_TO_TICKS(1000)); //block for 1000ms

    }
}
