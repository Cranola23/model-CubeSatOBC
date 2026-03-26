#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"

static const char *TAG = "MAIN";

typedef struct {
    float temperature;
    float voltage;
} telemetry_t;

float simulate_temp() {
    return 25.0 + (esp_random() % 100) / 10.0;
}

float simulate_voltage() {
    return 7.0 + (esp_random() % 100) / 100.0;
}

void app_main(void)
{
    while (1) {
        telemetry_t t;
        t.temperature = simulate_temp();
        t.voltage = simulate_voltage();
        ESP_LOGI(TAG, "Satellite is alive");
        ESP_LOGI("TAG", "TEMP:%.2f,VOLT:%.2f", t.temperature, t.voltage);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
                
    }
}
