#include "driver/uart.h"
#include "esp_log.h"

#define UART_PORT UART_NUM_0 //default usb uart (logging)
#define BUF_SIZE 1024

static const char *TAG = "UART_GS";

//Initialize UART
void uart_gs_init(void){
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_PORT, BUF_SIZE, BUF_SIZE, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "UART initialized");
}

//Send via UART
int uart_gs_send(const uint8_t *data, size_t len){
    return uart_write_bytes(UART_PORT, data, len);
}

//Read via UART
int uart_gs_read(uint8_t *data, size_t max_len){
    return uart_read_bytes(UART_PORT, data, max_len, 10 / portTICK_PERIOD_MS);
}