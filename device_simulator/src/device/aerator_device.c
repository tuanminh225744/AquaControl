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
    int num_intervals;
    struct
    {
        int start_hour;
        int start_minute;
        int end_hour;
        int end_minute;
    } intervals[MAX_SCHEDULE_INTERVALS];
    char device_type[20];
    char password[20];
    int token[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} AeratorDevice;

AeratorDevice AD;
int *tokenPtr = AD.token;
int *number_of_tokensPtr = &AD.number_of_tokens;
int *activePtr = &AD.active;

void create_device()
{
    printf("Enter Device ID: ");
    scanf("%d", &AD.device_id);
    printf("Enter Password: ");
    scanf("%s", AD.password);
    printf("Enter fish pond ID: ");
    scanf("%d", &AD.fish_pond_id);
    printf("Enter RPM: ");
    scanf("%lf", &AD.rpm);
    int num_intervals;
    do
    {
        printf("Enter Number of Active Intervals per day (max %d): ", MAX_SCHEDULE_INTERVALS);
        scanf("%d", &num_intervals);
        if (num_intervals < 0 || num_intervals > MAX_SCHEDULE_INTERVALS)
        {
            printf("Invalid number. Please enter a value between 0 and %d.\n", MAX_SCHEDULE_INTERVALS);
        }
    } while (num_intervals < 0 || num_intervals > MAX_SCHEDULE_INTERVALS);

    AD.num_intervals = num_intervals;

    printf("Enter Specific Operating Intervals (Start Hour:Minute to End Hour:Minute):\n");
    for (int i = 0; i < AD.num_intervals; i++)
    {
        int start_hour, start_minute, end_hour, end_minute;

        // Nhập Giờ Bắt đầu
        do
        {
            printf("Interval %d - Start Hour (0-23): ", i + 1);
            scanf("%d", &start_hour);
        } while (start_hour < 0 || start_hour > 23);
        AD.intervals[i].start_hour = start_hour;

        // Nhập Phút Bắt đầu
        do
        {
            printf("Interval %d - Start Minute (0-59): ", i + 1);
            scanf("%d", &start_minute);
        } while (start_minute < 0 || start_minute > 59);
        AD.intervals[i].start_minute = start_minute;

        // Nhập Giờ Kết thúc
        do
        {
            printf("Interval %d - End Hour (0-23): ", i + 1);
            scanf("%d", &end_hour);
        } while (end_hour < 0 || end_hour > 23);
        AD.intervals[i].end_hour = end_hour;

        // Nhập Phút Kết thúc
        do
        {
            printf("Interval %d - End Minute (0-59): ", i + 1);
            scanf("%d", &end_minute);
        } while (end_minute < 0 || end_minute > 59);
        AD.intervals[i].end_minute = end_minute;
    }
    AD.active = 1;
    AD.number_of_tokens = 0;
    strcpy(AD.device_type, "AERATOR");
    printf("[DEVICE] Create device successful.\n");
}

void aerator_handler(int sock, struct Message *msg)
{
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, AD.device_id, AD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, AD.device_id, AD.device_type, AD.password, tokenPtr, number_of_tokensPtr);
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
    create_device();
    return start_device_server(5000, aerator_handler);
}