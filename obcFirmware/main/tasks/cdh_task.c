#include "bus.h"
#include "packet.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

void cdh_task(void *pvParameters){

    QueueHandle_t cdh_queue = xQueueCreate(10, sizeof(bus_msg_t));
    bus_register(SUBSYS_CDH, cdh_queue);

    pkt_eps_telem_t eps_data;
    pkt_adcs_telem_t adcs_data;

    while(1){
        bus_msg_t msg;
        if(xQueueReceive(cdh_queue, &msg, portMAX_DELAY)){
            if(msg.type == MSG_TELEMETRY){
                if(msg.src == SUBSYS_EPS){
                    memcpy(&eps_data, msg.data, msg.len);
                }
                else if(msg.src == SUBSYS_ADCS){
                    memcpy(&adcs_data, msg.data, msg.len);
                }
            }

            //build complete telemetry 
            pkt_telem_t fullTelem;
            fullTelem.timestamp_ms = esp_timer_get_time() / 1000; 
            fullTelem.state = STATE_NOMINAL;
            fullTelem.adcs = adcs_data;
            fullTelem.eps = eps_data;

            //send to TTC
            bus_msg_t outTTC;
            outTTC.type = MSG_TELEMETRY;
            outTTC.src = SUBSYS_CDH;
            outTTC.dst = SUBSYS_TTC;
            memcpy(outTTC.data, &fullTelem, sizeof(fullTelem));
            outTTC.len = sizeof(fullTelem);

            bus_send(SUBSYS_TTC, &outTTC);
        } 
    }
}

