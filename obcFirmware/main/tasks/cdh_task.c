#include "bus.h"
#include "packet.h"
#include "state_machine.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

static pkt_eps_telem_t eps_data;
static pkt_adcs_telem_t adcs_data;

//CDH send ACK 
static void cdh_send_ack(uint8_t seq){
    pkt_header_t hdr;
    hdr.type = PKT_TYPE_ACK; //acknowledge
    hdr.seq = seq;
    hdr.len = 1;
    uint8_t payload = 0xAA; //'OK'
    uint8_t buffer[16];
    size_t offset = 0;
    
    memcpy(&buffer[offset], &hdr, sizeof(hdr)); //copying header into buffer
    offset += sizeof(hdr);
    buffer[offset++] = payload; //copying payload into buffer after header
    
    //sending to ttc via bus
    bus_msg_t msg;
    msg.type = MSG_TELEMETRY; //SUPER HACK. PLEZ FIX TO SUPPORT MSG_COMMAND
    msg.src = SUBSYS_CDH;
    msg.dst = SUBSYS_TTC;

    memcpy(msg.data,buffer,offset);
    msg.len = offset;
    bus_send(SUBSYS_TTC,&msg);
}

//CDH handle Commands
static void cdh_handle_cmd(pkt_cmd_t *cmd){
    switch(cmd->cmd_id){
    case CMD_PING: {
        ESP_LOGI("CDH", "PING received");
        cdh_send_ack(0x01);
        break;
    }
    
    case CMD_GET_TELEM: {
        ESP_LOGI("CDH", "GET TELEMETRY");
        pkt_telem_t telem;
        telem.timestamp_ms = esp_timer_get_time()/1000;
        telem.state = getState();
        telem.adcs = adcs_data;
        telem.eps = eps_data;

        bus_msg_t msg;
        msg.type = MSG_TELEMETRY; //SUPER HACK. PLEX FIX TO SUPPORT MSG_COMMAND
        msg.src = SUBSYS_CDH;
        msg.dst = SUBSYS_TTC;
        memcpy(msg.data, &telem, sizeof(telem));
        msg.len = sizeof(telem);
        bus_send(SUBSYS_TTC, &msg);
        break;
    }
    
    case CMD_SET_STATE:{
        ESP_LOGI("CDH","SET_STATE to %d", cmd->arg);
        stateSet((obc_state_t)cmd->arg);
        break;
    }
        
    default:
        ESP_LOGW("CDH", "Unknown CMD");
        break;
    }
}

void cdh_task(void *pvParameters){
    QueueHandle_t cdh_queue = xQueueCreate(10, sizeof(bus_msg_t));
    bus_register(SUBSYS_CDH, cdh_queue);

    while(1){
        bus_msg_t msg;
        if(xQueueReceive(cdh_queue, &msg, portMAX_DELAY)){
            if(msg.type == MSG_COMMAND){ 
                cdh_handle_cmd((pkt_cmd_t *)msg.data);
            }
            else if(msg.type == MSG_TELEMETRY){
                if(msg.src == SUBSYS_EPS){
                    memcpy(&eps_data, msg.data, msg.len);
                }
                else if(msg.src == SUBSYS_ADCS){
                    memcpy(&adcs_data, msg.data, msg.len);
                }
            }

            //build complete telemetry 
            if(getState() == STATE_NOMINAL){
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
}

