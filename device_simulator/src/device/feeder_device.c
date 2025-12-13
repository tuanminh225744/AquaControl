#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../device_server.h"
#include "../../../common/messages.h"

#define MAX_FEEDING_TIMES 10

typedef struct
{
    int device_id;
    int active;
    int num_feedings;
    struct
    {
        int hour;
        int minute;
    } feeding_times[MAX_FEEDING_TIMES];
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

void create_device()
{
    printf("Enter Device ID: ");
    scanf("%d", &FD.device_id);
    printf("Enter Password: ");
    scanf("%s", FD.password);
    printf("Enter fish pond ID: ");
    scanf("%d", &FD.fish_pond_id);

    int num_feed;
    do
    {
        printf("Enter Number of Feedings per day (max %d): ", MAX_FEEDING_TIMES);
        scanf("%d", &num_feed);
        if (num_feed < 0 || num_feed > MAX_FEEDING_TIMES)
        {
            printf("Invalid number. Please enter a value between 0 and %d.\n", MAX_FEEDING_TIMES);
        }
    } while (num_feed < 0 || num_feed > MAX_FEEDING_TIMES);

    FD.num_feedings = num_feed;

    printf("Enter Specific Feeding Times (Hour:Minute):\n");
    for (int i = 0; i < FD.num_feedings; i++)
    {
        int hour, minute;

        // Nhập Giờ
        do
        {
            printf("Feeding %d - Hour (0-23): ", i + 1);
            scanf("%d", &hour);
            if (hour < 0 || hour > 23)
            {
                printf("Invalid hour. Must be between 0 and 23.\n");
            }
        } while (hour < 0 || hour > 23);
        FD.feeding_times[i].hour = hour;

        // Nhập Phút
        do
        {
            printf("Feeding %d - Minute (0-59): ", i + 1);
            scanf("%d", &minute);
            if (minute < 0 || minute > 59)
            {
                printf("Invalid minute. Must be between 0 and 59.\n");
            }
        } while (minute < 0 || minute > 59);
        FD.feeding_times[i].minute = minute;
    }
    FD.active = 1;
    FD.number_of_tokens = 0;
    strcpy(FD.device_type, "FEEDER");
    printf("[DEVICE] Create device successful.\n");
}

void feeder_handler(int sock, struct Message *msg)
{
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
    create_device();
    return start_device_server(5100, feeder_handler);
}