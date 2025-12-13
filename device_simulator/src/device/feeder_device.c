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
        printf("Feeding %d - Enter Time (HH MM): ", i + 1);
        scanf("%d %d", &hour, &minute);

        FD.feeding_times[i].hour = hour;
        FD.feeding_times[i].minute = minute;
    }
    FD.active = 1;
    FD.number_of_tokens = 0;
    strcpy(FD.device_type, "FEEDER");
    printf("[DEVICE] Create device successful.\n");
}

void handle_setup_device(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    int req_num_feedings;

    int temp_feeding_times[MAX_FEEDING_TIMES][2];

    char *ptr = req->payload;
    int offset = 0;

    if (sscanf(ptr, "%d%n", &req_token, &offset) != 1)
    {
        invalid_message_response(sockfd);
        return;
    }
    ptr += offset;

    if (sscanf(ptr, " N=%d%n", &req_num_feedings, &offset) != 1 ||
        req_num_feedings < 0 || req_num_feedings > MAX_FEEDING_TIMES)
    {
        invalid_message_response(sockfd);
        return;
    }
    ptr += offset;

    for (int i = 0; i < req_num_feedings; i++)
    {
        char format_str[16];
        snprintf(format_str, sizeof(format_str), " T%d=%%d:%%d%%n", i + 1);

        if (sscanf(ptr, format_str,
                   &temp_feeding_times[i][0],
                   &temp_feeding_times[i][1],
                   &offset) != 2)
        {
            invalid_message_response(sockfd);
            return;
        }
        if (temp_feeding_times[i][0] < 0 || temp_feeding_times[i][0] > 23 ||
            temp_feeding_times[i][1] < 0 || temp_feeding_times[i][1] > 59)
        {
            invalid_message_response(sockfd);
            return;
        }
        ptr += offset;
    }

    if (!handle_check_token(req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }

    FD.num_feedings = req_num_feedings;
    for (int i = 0; i < req_num_feedings; i++)
    {
        FD.feeding_times[i].hour = temp_feeding_times[i][0];
        FD.feeding_times[i].minute = temp_feeding_times[i][1];
    }

    res.code = CODE_SET_FEEDER_DEVICE_OK;
    strcpy(res.payload, "Feeder Setup Success");

    send(sockfd, &res, sizeof(res), 0);
    printf("[SETUP DEVICE] Responded Code %d %s\n", res.code, res.payload);
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
    case TYPE_SET_FEEDER_DEVICE:
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
    return start_device_server(5100, feeder_handler);
}