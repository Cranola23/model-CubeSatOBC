#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "packet.h"
#include "bus.h"
#include "cobs.h"
#include "uart_gs.h"
#include "esp_log.h"
#include <string.h>

#define TTC_QUEUE_LEN 10
static const char *TAG = "TTC";
static QueueHandle_t ttc_queue;
//Buffers
static uint8_t tx_raw[128];
static uint8_t tx_encoded[160];
static uint8_t rx_buf[128];
static uint8_t rx_frame[128];
static uint8_t rx_decoded[128];
static uint8_t seq = 0;

//forward declarations (yeah there were errors)
static void ttc_task(void *pvParameters);
static void ttc_handle_cmd(pkt_cmd_t *cmd);
static void ttc_send_telem(pkt_telem_t *telem);
static void ttc_process_frame(uint8_t *frame, size_t len);

//Main RTOS ttc_task loop
static void ttc_task(void *pvParameters){
    bus_msg_t msg;
    size_t rx_index = 0;
    while (1){
        //TX: CDH -> UART
        if(xQueueReceive(ttc_queue, &msg, 0) == pdTRUE){ 
            if(msg.type == MSG_TELEMETRY){
                ttc_send_telem((pkt_telem_t *)msg.data);
            }
        }
        //RX: UART -> CMD -> CDH
        int len = uart_gs_read(rx_buf, sizeof(rx_buf)); 
        for(int i=0;i<len;i++){
            uint8_t byte = rx_buf[i];
            if(byte == 0x00){ //0x00 is packet divider
                if(rx_index>0){
                    ttc_process_frame(rx_frame, rx_index);
                    rx_index = 0;
                }
            }
            else{
                if(rx_index < sizeof(rx_frame)){
                    rx_frame[rx_index++] = byte;
                }
                else{ //overflow protection
                    rx_index = 0;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


//Initialize Task
void ttc_task_init(void){
    uart_gs_init();

    ttc_queue = xQueueCreate(TTC_QUEUE_LEN, sizeof(bus_msg_t));
    bus_register(SUBSYS_TTC, ttc_queue);

    xTaskCreate(ttc_task, "ttc_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "TTC initialized");
}

//TTC send telemetry via UART
static void ttc_send_telem(pkt_telem_t *telem){
    pkt_header_t hdr;
    hdr.type = PKT_TYPE_TELEMETRY;
    hdr.seq  = seq++;
    hdr.len  = sizeof(pkt_telem_t);

    size_t offset = 0;

    //Copy Header to tx buffer
    memcpy(&tx_raw[offset], &hdr, sizeof(hdr));
    offset += sizeof(hdr);

    //Copy telemetry to tx buffer
    memcpy(&tx_raw[offset], telem, sizeof(pkt_telem_t));
    offset += sizeof(pkt_telem_t);

    //COBS encoding
    size_t enc_len = cobs_encode(tx_raw, offset, tx_encoded);

    //Adding protocol level delimiter (differenciate bw packets)
    tx_encoded[enc_len++] = 0x00;

    uart_gs_send(tx_encoded, enc_len); //Send via UART
    ESP_LOGI(TAG, "Sent telemetry (%d bytes)", enc_len);
}

//TTC recieve and decode frames (header/payload -> CMD)
static void ttc_process_frame(uint8_t *frame, size_t len){
    size_t dec_len = cobs_decode(frame, len, rx_decoded);
    if(dec_len == 0){
        ESP_LOGW(TAG, "COBS decode failed");
        return;
    }

    if(dec_len < sizeof(pkt_header_t)){
        ESP_LOGW(TAG, "Packet too small");
        return;
    }

    pkt_header_t *hdr = (pkt_header_t *)rx_decoded; //hdr pointer. first few bytes of rx_decoded is header struct (typecast)
    uint8_t *payload = rx_decoded + sizeof(pkt_header_t); //payload pointer. points to first byte afte header.

    switch(hdr->type){
        case PKT_TYPE_CMD:
            ttc_handle_cmd((pkt_cmd_t *)payload);
            break;

        default:
            ESP_LOGW(TAG, "Unknown packet type: %d", hdr->type);
            break;
    }
}

//Send CMD as bus msg to CDH
static void ttc_handle_cmd(pkt_cmd_t *cmd){
    bus_msg_t msg;
    msg.type = MSG_COMMAND;
    msg.src  = SUBSYS_TTC;
    msg.dst  = SUBSYS_CDH;

    memcpy(msg.data, cmd, sizeof(pkt_cmd_t));
    msg.len = sizeof(pkt_cmd_t);

    bus_send(SUBSYS_CDH, &msg);
    ESP_LOGI(TAG, "Forwarded CMD %d to CDH", cmd->cmd_id);
}

