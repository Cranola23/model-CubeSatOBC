#include "esp_log.h"
#include "packet.h"
#include "state_machine.h"

static const char *TAG = "STATE";
static obc_state_t currentState = STATE_SAFE;

void stateInit(void){
    currentState = STATE_SAFE;
    ESP_LOGI(TAG, "Initialized SAFE MODE");
}

void stateSet(obc_state_t newState){
    if(newState == currentState) return;
    ESP_LOGI(TAG,"State Changed from %d to %d",currentState,newState);
    currentState = newState;
}

obc_state_t getState(void){
    return currentState;
}