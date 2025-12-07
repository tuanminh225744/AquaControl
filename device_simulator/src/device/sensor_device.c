#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../device_server.h"
#include "../../../common/messages.h"

typedef struct
{
    int device_id;
    int active;
    double value;
    char device_type[20];
    char password[20];
    int token;
} SensorDevice;

SensorDevice SD;
int *activePtr = &SD.active;
int *tokenPtr = &SD.token;

void sensor_handler(int sock, struct Message *msg, int device_id, char *password)
{
    SD.device_id = device_id;
    strcpy(SD.password, password);
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, SD.device_id, SD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, SD.device_id, SD.device_type, SD.password, tokenPtr);
        break;
    case TYPE_TURN_ON:
        handle_turn_on_request(sock, msg, tokenPtr, activePtr);
        break;
    case TYPE_TURN_OFF:
        handle_turn_off_request(sock, msg, tokenPtr, activePtr);
        break;
    }
}

int main()
{
    return start_device_server(5000, sensor_handler);
}