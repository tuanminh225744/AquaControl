
#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

// --- MESSAGE (TYPE) ---
//
#define TYPE_SCAN 1
#define TYPE_CONNECT 2
#define TYPE_CHPASS 3
#define TYPE_GETINFO 4
#define TYPE_TURN_ON 5
#define TYPE_TURN_OFF 6
#define TYPE_SET_PUMP_DEVICE 7
#define TYPE_SET_AERATOR_DEVICE 8
#define TYPE_SET_FEEDER_DEVICE 9
#define TYPE_SET_PH_REGULATOR_DEVICE 10

// --- (RESPONSE CODES) ---
// success
#define CODE_SCAN_OK 100
#define CODE_LOGIN_OK 110
#define CODE_CHPASS_OK 120
#define CODE_GETINFO_OK 130
#define CODE_CONNECT_OK 150
#define CODE_TURN_ON_OK 140
#define CODE_TURN_OFF_OK 141
#define CODE_SET_PUMP_DEVICE_OK 150
#define CODE_SET_AERATOR_DEVICE_OK 151
#define CODE_SET_FEEDER_DEVICE_OK 152
#define CODE_SET_PH_REGULATOR_DEVICE_OK 153

// fail
#define CODE_SCAN_FAIL 210
#define CODE_LOGIN_NOID 211
#define CODE_LOGIN_FAIL 212
#define CODE_LOGIN_CONNECTED 213
#define CODE_TIMEOUT 220
#define CODE_OLD_PASS_FAIL 221
#define CODE_TOKEN_INVALID 222
#define CODE_DEVICE_OFF 243
#define CODE_TURN_ON_FAIL 244
#define CODE_TURN_OFF_FAIL 245

#define CODE_INVALID_MSG 300

// --- SIZE ---
#define PAYLOAD_SIZE 256
#define MAX_CLIENTS 10
#define MAX_FEEDING_TIMES 5
#define MAX_SCHEDULE_INTERVALS 5

struct Message
{
    int32_t type;
    int32_t code;
    char payload[PAYLOAD_SIZE];
};

#endif