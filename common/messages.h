
#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

// --- CÁC LOẠI MESSAGE (OPCODES) ---
#define MSG_SCAN_REQUEST 1
#define MSG_SCAN_RESPONSE 2
#define MSG_CONNECT_REQUEST 3
#define MSG_CONNECT_ACCEPT 4
#define MSG_CONNECT_DENY 5
#define MSG_SET_PASSWORD 6
#define MSG_CONTROL_TOGGLE 7
#define MSG_SET_PARAM 8
#define MSG_GET_INFO 9

#define PAYLOAD_SIZE 256

struct Message
{
    int32_t type;
    int32_t app_id;
    int32_t device_id;
    char payload[PAYLOAD_SIZE];
};

#endif