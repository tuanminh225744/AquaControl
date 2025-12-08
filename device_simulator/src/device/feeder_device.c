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
    double w_food;
    int schedule[24];
    char device_type[20];
    char password[20];
    int token[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} FeederDevice;

FeederDevice FD;
int *activePtr = &FD.active;
int *tokenPtr = FD.token;
int *number_of_tokensPtr = &FD.number_of_tokens;

void feeder_handler(int sock, struct Message *msg, int device_id, char *password)
{
    FD.device_id = device_id;
    strcpy(FD.password, password);
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, FD.device_id, FD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, FD.device_id, FD.device_type, FD.password, tokenPtr, number_of_tokensPtr);
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
    strcpy(FD.device_type, "FEEDER");
    return start_device_server(5100, feeder_handler);
}