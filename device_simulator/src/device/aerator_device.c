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
    double rpm;
    int schedule[24];
    char device_type[20];
    char password[20];
    int token;
} AeratorDevice;

AeratorDevice AD;

int *tokenPtr = &AD.token;
int *activePtr = &AD.active;

void aerator_handler(int sock, struct Message *msg, int device_id, char *password)
{
    AD.device_id = device_id;
    strcpy(AD.password, password);
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, AD.device_id, AD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, AD.device_id, AD.device_type, AD.password, tokenPtr);
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
    strcpy(AD.device_type, "AERATOR");
    return start_device_server(5000, aerator_handler);
}