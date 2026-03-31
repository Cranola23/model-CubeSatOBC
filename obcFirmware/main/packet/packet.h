#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

typedef enum {
    PKT_TYPE_TELEMETRY = 0x10,
    PKT_TYPE_CMD       = 0x20,
    PKT_TYPE_ACK       = 0x30,
} pkt_type_t;

//Subsystem ID
typedef enum {
    SUBSYS_OBC  = 0x01,
    SUBSYS_EPS  = 0x02,
    SUBSYS_ADCS = 0x03,
    SUBSYS_CDH  = 0x04,
    SUBSYS_TTC  = 0x05,
} subsystem_id_t;

//OBC states
typedef enum {
    STATE_SAFE    = 0,
    STATE_NOMINAL = 1,
} obc_state_t;


//Commands
typedef enum {
    CMD_PING          = 0x01,
    CMD_GET_TELEM     = 0x02,
    CMD_SET_STATE     = 0x03,
} cmd_id_t;


//Telemetry

//EPS(simulated)
typedef struct __attribute__((packed)) {
    float bus_voltage_v;
    float battery_pct;
} pkt_eps_telem_t;


//ADCS(real gyro)
typedef struct __attribute__((packed)) {
    float gyro_x;
    float gyro_y;
    float gyro_z;
} pkt_adcs_telem_t;


//Complete Telemetry
typedef struct __attribute__((packed)) {
    uint32_t timestamp_ms;
    uint8_t state;

    pkt_eps_telem_t eps;
    pkt_adcs_telem_t adcs;

} pkt_telem_t;


//Command
typedef struct __attribute__((packed)) {
    uint8_t cmd_id;
    uint8_t arg;
} pkt_cmd_t;

//Header
typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint8_t  seq;
    uint16_t len;
} pkt_header_t;


//Bus message enum
typedef enum {
    MSG_TELEMETRY,
    MSG_COMMAND,
} bus_msg_type_t;

//Bus message 
typedef struct {
    bus_msg_type_t type;
    subsystem_id_t src;
    subsystem_id_t dst;

    uint8_t data[64];
    uint8_t len;
} bus_msg_t;

#endif