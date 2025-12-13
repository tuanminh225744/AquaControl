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
    double flow_rate;
    double duration;
    char device_type[20];
    char password[20];
    int token[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} WaterPumpDevice;

WaterPumpDevice WPD;
int *tokenPtr = WPD.token;
int *number_of_tokensPtr = &WPD.number_of_tokens;
int *activePtr = &WPD.active;

void create_device()
{
    printf("Enter Device ID: ");
    scanf("%d", &WPD.device_id);
    printf("Enter Password: ");
    scanf("%s", WPD.password);
    printf("Enter fish pond ID: ");
    scanf("%d", &WPD.fish_pond_id);
    printf("Enter Flow Rate (m3/h): ");
    scanf("%lf", &WPD.flow_rate);
    printf("Enter Duration (h): ");
    scanf("%lf", &WPD.duration);
    WPD.active = 1;
    WPD.number_of_tokens = 0;
    strcpy(WPD.device_type, "WATERPUMP");
    printf("[DEVICE] Create device successful.\n");
}

void water_pump_handler(int sock, struct Message *msg)
{

    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, WPD.device_id, WPD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, WPD.device_id, WPD.device_type, WPD.password, tokenPtr, number_of_tokensPtr);
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
    return start_device_server(5400, water_pump_handler);
}