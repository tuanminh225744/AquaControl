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
    double salinity;
    double dissolved_oxygen;
    double pH;
    char device_type[20];
    char password[20];
    int token[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} SensorDevice;

SensorDevice SD;
int *activePtr = &SD.active;
int *tokenPtr = SD.token;
int *number_of_tokensPtr = &SD.number_of_tokens;

void create_device()
{
    printf("Enter Device ID: ");
    scanf("%d", &SD.device_id);
    printf("Enter Password: ");
    scanf("%s", SD.password);
    printf("Enter fish pond ID: ");
    scanf("%d", &SD.fish_pond_id);
    SD.active = 1;
    SD.number_of_tokens = 0;
    strcpy(SD.device_type, "SENSOR");
    printf("[DEVICE] Create device successful.\n");
}

void sensor_handler(int sock, struct Message *msg)
{
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, SD.device_id, SD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, SD.device_id, SD.device_type, SD.password, tokenPtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_ON:
        handle_turn_on_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_OFF:
        handle_turn_off_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    }
}

int main()
{
    create_device();
    return start_device_server(5300, sensor_handler);
}