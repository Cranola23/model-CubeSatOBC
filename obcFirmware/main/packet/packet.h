#ifndef packet_H
#define packet_H

#include <stdint.h>
#include <stdbool.h>

//packet type ID
typedef enum {
    PKT_TYPE_TELEMETRY_FULL  = 0x10,
    PKT_TYPE_TELEMETRY_EPS   = 0x11,
    PKT_TYPE_TELEMETRY_ADCS  = 0x12,
    PKT_TYPE_TELEMETRY_TCS   = 0x13,
    PKT_TYPE_ACK             = 0x20,
    PKT_TYPE_NACK            = 0x21,
    PKT_TYPE_CMD             = 0x30,
    PKT_TYPE_CMD_RESPONSE    = 0x31,
    PKT_TYPE_LOG_DUMP        = 0x40,
    PKT_TYPE_PING            = 0x50,
    PKT_TYPE_PONG            = 0x51,
} pkt_type_t;


//subsystem ID
typedef enum {
    SUBSYS_OBC     = 0x01,
    SUBSYS_EPS     = 0x02,
    SUBSYS_ADCS    = 0x03,
    SUBSYS_TCS     = 0x04,
    SUBSYS_CDH     = 0x05,
    SUBSYS_TTC     = 0x06,
    SUBSYS_PAYLOAD = 0x07,
} subsystem_id_t;


//OBC States
typedef enum {
    STATE_BOOT    = 0,
    STATE_SAFE    = 1,
    STATE_NOMINAL = 2,
    STATE_PAYLOAD = 3,
    STATE_FAULT   = 4,
} obc_state_t;


//Command IDs
typedef enum {
    CMD_PING           = 0x01,
    CMD_REBOOT         = 0x02,
    CMD_SAFE_MODE      = 0x03,
    CMD_GO_NOMINAL     = 0x04,
    CMD_PAYLOAD_ON     = 0x05,
    CMD_PAYLOAD_OFF    = 0x06,
    CMD_RESET_FAULT    = 0x07,
    CMD_REQUEST_TELEM  = 0x08,
    CMD_DUMP_LOG       = 0x09,
    CMD_SET_ADCS_MODE  = 0x0A,
} cmd_id_t;

//Fault Codes
typedef enum {
    FAULT_NONE               = 0x00,
    FAULT_EPS_UNDERVOLTAGE   = 0x01,
    FAULT_EPS_OVERCURRENT    = 0x02,
    FAULT_ADCS_NO_DATA       = 0x03,
    FAULT_TCS_OVERTEMP       = 0x04,
    FAULT_TCS_UNDERTEMP      = 0x05,
    FAULT_CDH_TIMEOUT        = 0x06,
    FAULT_WDT_TIMEOUT        = 0x07,
    FAULT_SD_WRITE_FAIL      = 0x08,
} fault_code_t;

//TELEMETRY PAYLOADS

//eps telemetry
typedef struct __attribute__((packed)) {
    uint32_t timestamp_ms;
    float    bus_voltage_v;
    float    battery_voltage_v;
    float    solar_current_ma;
    float    load_current_ma;
    float    battery_soc_pct;      /* 0.0 – 100.0 */
    int8_t   eps_temp_c;
    uint8_t  charge_state;         /* 0=discharging 1=charging 2=full */
} pkt_eps_telem_t;

//adcs telemetry (gotta remove some)
typedef struct __attribute__((packed)) {
    uint32_t timestamp_ms;
    float    gyro_x_dps;           /* real MPU-6050 */
    float    gyro_y_dps;
    float    gyro_z_dps;
    float    accel_x_g;
    float    accel_y_g;
    float    accel_z_g;
    float    roll_deg;             /* complementary filter output */
    float    pitch_deg;
    float    yaw_deg;              /* integrated, will drift */
    float    mag_x_ut;             /* simulated */
    float    mag_y_ut;
    float    mag_z_ut;
    uint8_t  adcs_mode;            /* 0=rate_ctrl 1=detumble 2=pointing */
} pkt_adcs_telem_t;

//tcs telemetry (might remove??)
typedef struct __attribute__((packed)) {
    uint32_t timestamp_ms;
    int8_t   obc_temp_c;
    int8_t   battery_temp_c;
    int8_t   payload_temp_c;
    int8_t   panel_a_temp_c;
    int8_t   panel_b_temp_c;
    uint8_t  heater_state;         /* bit0=battery_heater bit1=payload_heater */
} pkt_tcs_telem_t;


//COMLETE TELEMETRY PACKET
typedef struct __attribute__((packed)) {
    uint32_t         timestamp_ms;
    uint8_t          obc_state;
    uint8_t          fault_flags;  /* bitmask of active fault_code_t */
    uint16_t         uptime_s;
    pkt_eps_telem_t  eps;
    pkt_adcs_telem_t adcs;
    pkt_tcs_telem_t  tcs;
} pkt_full_telem_t;

//command payload
typedef struct __attribute__((packed)) {
    uint8_t cmd_id;
    uint8_t cmd_args[8];
} pkt_cmd_t;

//acknowloadge/nack payload
typedef struct __attribute__((packed)) {
    uint16_t ack_seq_num;
    uint8_t  result;
    uint8_t  reason;
} pkt_ack_t;

//common 8byte header
typedef struct __attribute__((packed)) {
    uint8_t  magic;
    uint8_t  version;
    uint8_t  pkt_type;
    uint8_t  src_subsys;
    uint16_t seq_num;
    uint16_t payload_len;
} pkt_header_t;

//INTERNAL SOFTWARE BUS MESSAGE 

typedef enum {
    MSG_TELEMETRY  = 0x01,
    MSG_COMMAND    = 0x02,
    MSG_FAULT      = 0x03,
    MSG_HEARTBEAT  = 0x04,
} bus_msg_type_t;

typedef struct {
    bus_msg_type_t type;
    subsystem_id_t src;
    subsystem_id_t dst;
    uint32_t       timestamp_ms;
    uint8_t        data[64];
    uint8_t        data_len;
} bus_msg_t;


#endif