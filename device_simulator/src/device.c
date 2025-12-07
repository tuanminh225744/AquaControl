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

    res.type = TYPE_SCAN;
    res.code = CODE_SCAN_OK
        sprintf(res.payload, "%s;%d", DEVICE_TYPE, DEVICE_ID);

    // Gửi struct qua TCP
    send(sockfd, &res, sizeof(res), 0);
    printf(" -> [SCAN] Responded Code %d\n", res.code);
}

void handle_connect_request(int sockfd, struct Message *req)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.type = TYPE_CONNECT;

    int req_id;
    char req_pass[64];

    if (sscanf(req->payload, "%d %s", &req_id, req_pass) != 2)
    {
        res.code = CODE_INVALID_MSG; // 300: Lỗi cú pháp
        strcpy(res.payload, "Invalid Format");
    }
    else if (req_id != G_DEVICE_ID)
    {
        res.code = CODE_LOGIN_NOID; // 211: Sai ID
        strcpy(res.payload, "Device ID Not Found");
    }
    else if (strcmp(req_pass, G_PASSWORD) != 0)
    {
        res.code = CODE_LOGIN_FAIL; // 212: Sai mật khẩu
        strcpy(res.payload, "Wrong Password");
    }
    else
    {
        res.code = CODE_LOGIN_OK;
        int token = rand() % 9999 + 1000;

        snprintf(res.payload, sizeof(res.payload), "%d %s %d", G_DEVICE_ID, G_DEVICE_TYPE, token);
        printf(" -> [LOGIN] Success. Token: %d\n", token);
    }

    send(sockfd, &res, sizeof(res), 0);
}