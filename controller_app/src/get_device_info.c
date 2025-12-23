#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"
#include "controller.h"

void get_pump_device_info(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));

    msg.type = TYPE_GET_PUMP_DEVICE_INFO;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_GET_PUMP_DEVICE_INFO_OK)
        {
            printf("[SUCCESS] Get OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Get failed. %d %s\n", res.code, res.payload);
        }
    }
}

void get_aerator_device_info(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));

    msg.type = TYPE_GET_AERATOR_DEVICE_INFO;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_GET_AERATOR_DEVICE_INFO_OK)
        {
            printf("[SUCCESS] Get OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Get failed. %d %s\n", res.code, res.payload);
        }
    }
}

void get_feeder_device_info(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));

    msg.type = TYPE_GET_FEEDER_DEVICE_INFO;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_GET_FEEDER_DEVICE_INFO_OK)
        {
            printf("[SUCCESS] Get OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Get failed. %d %s\n", res.code, res.payload);
        }
    }
}

void get_ph_regulator_device_info(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));

    msg.type = TYPE_GET_PH_REGULATOR_DEVICE_INFO;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_GET_PH_REGULATOR_DEVICE_INFO_OK)
        {
            printf("[SUCCESS] Get OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Get failed. %d %s\n", res.code, res.payload);
        }
    }
}

void get_sensor_device_info(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));

    msg.type = TYPE_GET_SENSOR_DEVICE_INFO;
    snprintf(msg.payload, sizeof(msg.payload), "%d", token);

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_GET_SENSOR_DEVICE_INFO_OK)
        {
            printf("[SUCCESS] Get OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Get failed. %d %s\n", res.code, res.payload);
        }
    }
}
