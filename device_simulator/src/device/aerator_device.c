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

        printf("Interval %d - Enter Start Time (HH MM): ", i + 1);
        scanf("%d %d", &start_hour, &start_minute);
        printf("Interval %d - Enter End Time (HH MM): ", i + 1);
        scanf("%d %d", &end_hour, &end_minute);
        AD.intervals[i].start_hour = start_hour;
        AD.intervals[i].start_minute = start_minute;
        AD.intervals[i].end_hour = end_hour;
        AD.intervals[i].end_minute = end_minute;
    }
    AD.active = 1;
    AD.number_of_tokens = 0;
    strcpy(AD.device_type, "AERATOR");
    printf("[DEVICE] Create device successful.\n");
}

void handle_setup_aerator_device(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    double req_rpm;
    int req_num_intervals;

    // Mảng tạm để parse 4 giá trị cho mỗi khoảng thời gian: StartH, StartM, EndH, EndM
    int temp_intervals[MAX_SCHEDULE_INTERVALS][4];

    char *ptr = req->payload;
    int offset = 0;

    // --- 1. Đọc Token ---
    if (sscanf(ptr, "%d%n", &req_token, &offset) != 1)
    {
        invalid_message_response(sockfd);
        return;
    }
    ptr += offset;

    // --- 2. Đọc RPM (C=...) ---
    if (sscanf(ptr, " C=%lf%n", &req_rpm, &offset) != 1 || req_rpm < 0)
    {
        invalid_message_response(sockfd);
        return;
    }
    ptr += offset;

    // --- 3. Đọc Số lượng Khoảng thời gian (N=...) ---
    if (sscanf(ptr, " N=%d%n", &req_num_intervals, &offset) != 1 ||
        req_num_intervals < 0 || req_num_intervals > MAX_SCHEDULE_INTERVALS)
    {
        invalid_message_response(sockfd);
        return;
    }
    ptr += offset;

    // --- 4. Đọc các Khoảng thời gian (T1=HH:MM-HH:MM ...) ---
    for (int i = 0; i < req_num_intervals; i++)
    {
        char format_str[32];
        snprintf(format_str, sizeof(format_str), " T%d=%%d:%%d-%%d:%%d%%n", i + 1);

        if (sscanf(ptr, format_str,
                   &temp_intervals[i][0], // Start Hour
                   &temp_intervals[i][1], // Start Minute
                   &temp_intervals[i][2], // End Hour
                   &temp_intervals[i][3], // End Minute
                   &offset) != 4)
        {
            invalid_message_response(sockfd);
            return;
        }
        if (temp_intervals[i][0] < 0 || temp_intervals[i][0] > 23 ||
            temp_intervals[i][1] < 0 || temp_intervals[i][1] > 59 ||
            temp_intervals[i][2] < 0 || temp_intervals[i][2] > 23 ||
            temp_intervals[i][3] < 0 || temp_intervals[i][3] > 59)
        {
            invalid_message_response(sockfd);
            return;
        }
        ptr += offset;
    }

    // --- 5. Validate Token ---
    if (!handle_check_token(req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }

    // --- 6. Update device settings ---
    AD.rpm = req_rpm;
    AD.num_intervals = req_num_intervals;
    for (int i = 0; i < req_num_intervals; i++)
    {
        AD.intervals[i].start_hour = temp_intervals[i][0];
        AD.intervals[i].start_minute = temp_intervals[i][1];
        AD.intervals[i].end_hour = temp_intervals[i][2];
        AD.intervals[i].end_minute = temp_intervals[i][3];
    }

    // --- 7. Send Success Response ---
    res.code = CODE_SET_AERATOR_DEVICE_OK;
    strcpy(res.payload, "Aerator Setup Success");

    send_all(sockfd, &res, sizeof(res));
    printf("[SETUP DEVICE] Responded Code %d %s\n", res.code, res.payload);
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
    case TYPE_SET_AERATOR_DEVICE:
        handle_setup_aerator_device(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    default:
        invalid_message_response(sock);
        break;
    }
}

int main()
{
    create_device();
    return start_device_server(5000, aerator_handler);
}