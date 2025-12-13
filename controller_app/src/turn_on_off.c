#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"

void turn_on_device(int sock, int token)
{
    struct Message msg;
    struct Message res;

    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_TURN_ON;
    msg.code = 0;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);

    send(sock, &msg, sizeof(msg), 0);

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_TURN_ON_OK)
        {
            printf("[SUCCESS] Turn on OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[ERROR] Turn on failed %d %s\n", res.code, res.payload);
        }
    }
}

void turn_off_device(int sock, int token)
{
    struct Message msg;
    struct Message res;

    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_TURN_OFF;
    msg.code = 0;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);

    send(sock, &msg, sizeof(msg), 0);

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_TURN_OFF_OK)
        {
            printf("[SUCCESS] Turn off OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[ERROR] Turn off failed. %d %s\n", res.code, res.payload);
        }
    }
}