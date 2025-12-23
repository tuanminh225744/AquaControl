#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"
#include "controller.h"

void manual_feed(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));
    memset(&res, 0, sizeof(res));
    msg.type = TYPE_MANUAL_FEED;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);
    send_all(sock, &msg, sizeof(msg));
    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_MANUAL_FEED_OK)
        {
            printf("[SUCCESS] Manual Feed OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Manual Feed failed. %d %s\n", res.code, res.payload);
        }
    }
}

void manual_aerate(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));
    memset(&res, 0, sizeof(res));
    msg.type = TYPE_MANUAL_AERATE;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);
    send_all(sock, &msg, sizeof(msg));
    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_MANUAL_AERATE_OK)
        {
            printf("[SUCCESS] Manual Aerate OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Manual Aerate failed. %d %s\n", res.code, res.payload);
        }
    }
}

void manual_pump(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));
    memset(&res, 0, sizeof(res));
    msg.type = TYPE_MANUAL_PUMP;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);
    send_all(sock, &msg, sizeof(msg));
    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_MANUAL_PUMP_OK)
        {
            printf("[SUCCESS] Manual Pump OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Manual Pump failed. %d %s\n", res.code, res.payload);
        }
    }
}