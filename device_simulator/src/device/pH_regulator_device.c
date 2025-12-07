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
    double pH_min;
    double w_ca;
    char device_type[20];
    char password[20];
    int token;
} PHRegulatorDevice;

PHRegulatorDevice PRD;
int *activePtr = &PRD.active;
int *tokenPtr = &PRD.token;

void pH_regulator_handler(int sock, struct Message *msg, int device_id, char *password)
{
    PRD.device_id = device_id;
    strcpy(PRD.password, password);
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, PRD.device_id, PRD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, PRD.device_id, PRD.device_type, PRD.password, tokenPtr);
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
    strcpy(PRD.device_type, "PHREGULATOR");
    return start_device_server(5000, pH_regulator_handler);
}