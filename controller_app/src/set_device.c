#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"
#include "controller.h"

void set_pump_device(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_SET_PUMP_DEVICE;
    msg.code = 0;

    double flow_rate;
    double duration;

    printf("Enter Flow Rate (m3/h): ");
    scanf("%lf", &flow_rate);
    printf("Enter Duration (h): ");
    scanf("%lf", &duration);

    // Format: [token] V=[flow_rate] T=[duration]
    snprintf(msg.payload, sizeof(msg.payload), "%d V=%lf T=%lf", token, flow_rate, duration);

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_SET_PUMP_DEVICE_OK)
        {
            printf("[SUCCESS] Set OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[ERROR] Set failed. %d %s\n", res.code, res.payload);
        }
    }
}

void set_aerator_device(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_SET_AERATOR_DEVICE;
    msg.code = 0;

    double rpm;
    int num_intervals;

    printf("Enter RPM: ");
    scanf("%lf", &rpm);

    // Nhập số lượng khoảng thời gian
    do
    {
        printf("Enter Number of Active Intervals per day (max %d): ", MAX_SCHEDULE_INTERVALS);
        scanf("%d", &num_intervals);
    } while (num_intervals < 0 || num_intervals > MAX_SCHEDULE_INTERVALS);

    // Bắt đầu xây dựng payload với token và RPM
    // Format: [token] C=[rpm] N=[num_intervals] T1=HH:MM-HH:MM T2=HH:MM-HH:MM ...
    int offset = snprintf(msg.payload, sizeof(msg.payload), "%d C=%.2lf N=%d", token, rpm, num_intervals);

    // Nhập và thêm các khoảng thời gian vào payload
    printf("Enter Specific Operating Intervals (Start Hour:Minute to End Hour:Minute):\n");
    for (int i = 0; i < num_intervals; i++)
    {
        int start_h, start_m, end_h, end_m;

        printf("Interval %d - Start Hour:Minute (HH MM): ", i + 1);
        scanf("%d %d", &start_h, &start_m);
        printf("Interval %d - End Hour:Minute (HH MM): ", i + 1);
        scanf("%d %d", &end_h, &end_m);

        offset += snprintf(msg.payload + offset, sizeof(msg.payload) - offset, " T%d=%02d:%02d-%02d:%02d",
                           i + 1, start_h, start_m, end_h, end_m);
    }

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_SET_AERATOR_DEVICE_OK)
        {
            printf("[SUCCESS] Set OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[ERROR] Set failed. %d %s\n", res.code, res.payload);
        }
    }
}

void set_feeder_device(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_SET_FEEDER_DEVICE;
    msg.code = 0;

    int num_feedings;

    // Nhập số lần cho ăn
    do
    {
        printf("Enter Number of Feedings per day (max %d): ", MAX_FEEDING_TIMES);
        scanf("%d", &num_feedings);
    } while (num_feedings < 0 || num_feedings > MAX_FEEDING_TIMES);

    // Bắt đầu xây dựng payload với token và số lần cho ăn
    // Format: [token] N=[num_feedings] T1=HH:MM T2=HH:MM ...
    int offset = snprintf(msg.payload, sizeof(msg.payload), "%d N=%d", token, num_feedings);

    // Nhập và thêm các thời điểm cho ăn vào payload
    printf("Enter Specific Feeding Times (Hour:Minute):\n");
    for (int i = 0; i < num_feedings; i++)
    {
        int hour, minute;

        printf("Feeding %d - Hour:Minute (HH MM): ", i + 1);
        scanf("%d %d", &hour, &minute);

        // Thêm thời điểm cho ăn vào chuỗi payload
        offset += snprintf(msg.payload + offset, sizeof(msg.payload) - offset, " T%d=%02d:%02d",
                           i + 1, hour, minute);
    }

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_SET_FEEDER_DEVICE_OK)
        {
            printf("[SUCCESS] Set OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[ERROR] Set failed. %d %s\n", res.code, res.payload);
        }
    }
}

void set_ph_regulator_device(int sock, int token)
{
    struct Message msg;
    struct Message res;
    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_SET_PH_REGULATOR_DEVICE;
    msg.code = 0;

    double ph_min;
    double w_ca;

    printf("Enter pH Minimum: ");
    scanf("%lf", &ph_min);
    printf("Enter Lime weight (kg): ");
    scanf("%lf", &w_ca);

    // Format: [token] PH=[pH_min] W=[w_ca]
    snprintf(msg.payload, sizeof(msg.payload), "%d PH=%lf W=%lf", token, ph_min, w_ca);

    send_all(sock, &msg, sizeof(msg));

    if (recv_all(sock, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_SET_PH_REGULATOR_DEVICE_OK)
        {
            printf("[SUCCESS] Set OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[ERROR] Set failed. %d %s\n", res.code, res.payload);
        }
    }
}