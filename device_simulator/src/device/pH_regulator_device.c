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
    int token[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} PHRegulatorDevice;

PHRegulatorDevice PRD;
int *activePtr = &PRD.active;
int *tokenPtr = PRD.token;
int *number_of_tokensPtr = &PRD.number_of_tokens;

void create_device()
{
    printf("Enter Device ID: ");
    scanf("%d", &PRD.device_id);
    printf("Enter Password: ");
    scanf("%s", PRD.password);
    printf("Enter fish pond ID: ");
    scanf("%d", &PRD.fish_pond_id);
    printf("Enter pH Minimum: ");
    scanf("%lf", &PRD.pH_min);
    printf("Enter Lime weight (kg): ");
    scanf("%lf", &PRD.w_ca);
    PRD.active = 1;
    PRD.number_of_tokens = 0;
    strcpy(PRD.device_type, "PHREGULATOR");
    printf("[DEVICE] Create device successful.\n");
}

void handle_setup_device(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    double req_pH_min, req_w_ca;

    int k = sscanf(req->payload, "%d PH_MIN=%lf W_CA=%lf", &req_token, &req_pH_min, &req_w_ca);

    if (k != 3)
    {
        invalid_message_response(sockfd);
        return;
    }
    else if (!handle_check_token(req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }
    else
    {
        res.code = CODE_SET_PH_REGULATOR_DEVICE_OK;
        PRD.pH_min = req_pH_min;
        PRD.w_ca = req_w_ca;
        strcpy(res.payload, "pH Regulator Setup Success");
    }

    send(sockfd, &res, sizeof(res), 0);
    printf("[SETUP DEVICE] Responded Code %d %s\n", res.code, res.payload);
}

void pH_regulator_handler(int sock, struct Message *msg)
{
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, PRD.device_id, PRD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, PRD.device_id, PRD.device_type, PRD.password, tokenPtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_ON:
        handle_turn_on_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_OFF:
        handle_turn_off_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_SET_PH_REGULATOR_DEVICE:
        handle_setup_device(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    default:
        invalid_message_response(sock);
        break;
    }
}

int main()
{
    create_device();
    return start_device_server(5200, pH_regulator_handler);
}