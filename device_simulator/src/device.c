#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "../../common/messages.h"

int DEVICE_ID = 1234;
char DEVICE_TYPE[20] = "SENSOR_PH";
char PASSWORD[20] = "admin";

void handle_scan_request(int sockfd, struct Message *req)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.type = MSG_SCAN_RESPONSE;
    res.device_id = DEVICE_ID;
    sprintf(res.payload, "%s;%d", DEVICE_TYPE, DEVICE_ID);

    // Gửi struct qua TCP
    send(sockfd, &res, sizeof(res), 0);
    printf(" -> Sent SCAN Response\n");
}

void handle_connect_request(int sockfd, struct Message *req)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    if (strcmp(req->payload, PASSWORD) == 0)
    {
        res.type = MSG_CONNECT_ACCEPT;
        int token = rand() % 9999;
        sprintf(res.payload, "%d", token);
        printf(" -> Login OK. Token: %d\n", token);
    }
    else
    {
        res.type = MSG_CONNECT_DENY;
        strcpy(res.payload, "Wrong Password");
        printf(" -> Login Failed.\n");
    }

    send(sockfd, &res, sizeof(res), 0);
}