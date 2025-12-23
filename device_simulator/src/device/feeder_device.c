#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../device_server.h"
#include "../../../common/messages.h"
#include "../../../common/network_utils.h"

#define FILE_LOG "feeder_device.log"

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
    TokenSession token_sessions[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} FeederDevice;

FeederDevice FD;
int *activePtr = &FD.active;
TokenSession *tokenPtr = FD.token_sessions;
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
        do
        {
            printf("Feeding %d - Enter Time (HH MM): ", i + 1);
            scanf("%d %d", &hour, &minute);
            if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
            {
                printf("Invalid time. Please enter hour (0-23) and minute (0-59).\n");
            }
        } while (hour < 0 || hour > 23 || minute < 0 || minute > 59);
        FD.feeding_times[i].hour = hour;
        FD.feeding_times[i].minute = minute;
    }
    FD.active = 1;
    FD.number_of_tokens = 0;
    strcpy(FD.device_type, "FEEDER");
    printf("[DEVICE] Create device successful.\n");
}

void handle_setup_device(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr)
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
        invalid_message_response(sockfd, req, FILE_LOG);
        return;
    }
    ptr += offset;

    if (sscanf(ptr, " N=%d%n", &req_num_feedings, &offset) != 1 ||
        req_num_feedings < 0 || req_num_feedings > MAX_FEEDING_TIMES)
    {
        invalid_message_response(sockfd, req, FILE_LOG);
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
            invalid_message_response(sockfd, req, FILE_LOG);
            return;
        }
        ptr += offset;
    }

    if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd, req, FILE_LOG);
        return;
    }

    if (!(*activePtr))
    {
        device_not_active_response(sockfd, req, FILE_LOG);
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

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, FILE_LOG, req->type, req->payload, res.code, res.payload);
    printf("[SETUP DEVICE] Responded Code %d %s\n", res.code, res.payload);
}

void handle_get_feeder_device_info(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    char *ptr = req->payload;

    // --- 1. Đọc Token ---
    if (sscanf(ptr, "%d", &req_token) != 1)
    {
        invalid_message_response(sockfd, req, FILE_LOG);
        return;
    }

    // --- 2. Validate Token ---
    if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd, req, FILE_LOG);
        return;
    }

    // --- 3. Check Active Status ---
    if (!(*activePtr))
    {
        device_not_active_response(sockfd, req, FILE_LOG);
        return;
    }

    // --- 4. Build Response Payload ---
    char payload_buffer[PAYLOAD_SIZE];
    int current_len = 0;

    // A. Thêm Device ID (I=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "I=%d ", FD.device_id);

    // B. Thêm Trạng thái hoạt động (S=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "S=%d ", FD.active);

    // C. Thêm Số lượng lần cho ăn (N=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "N=%d", FD.num_feedings);

    // D. Thêm các Thời gian cho ăn (T1=HH:MM, T2=HH:MM ...)
    for (int i = 0; i < FD.num_feedings; i++)
    {
        current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                                " T%d=%02d:%02d",
                                i + 1,
                                FD.feeding_times[i].hour,
                                FD.feeding_times[i].minute);
    }

    // --- 5. Send Info Response ---
    res.code = CODE_GET_FEEDER_DEVICE_INFO_OK;
    strcpy(res.payload, payload_buffer);

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, FILE_LOG, req->type, req->payload, res.code, res.payload);
    printf("[GET INFO DEVICE] Responded Code %d Payload: %s\n", res.code, res.payload);
}

void handle_manual_feed(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    if (sscanf(req->payload, "%d", &req_token) != 1)
    {
        invalid_message_response(sockfd, req, FILE_LOG);
        return;
    }
    else if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd, req, FILE_LOG);
        return;
    }
    else if (!(*activePtr))
    {
        device_not_active_response(sockfd, req, FILE_LOG);
        return;
    }
    else
    {
        res.code = CODE_MANUAL_FEED_OK;
        strcpy(res.payload, "Manual Feed Success");
    }
    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, FILE_LOG, req->type, req->payload, res.code, res.payload);
    printf("[MANUAL FEED] Responded Code %d %s\n", res.code, res.payload);
}

void feeder_handler(int sock, struct Message *msg)
{
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, FD.device_id, FD.device_type, FILE_LOG);
        break;
    case TYPE_LOGIN:
        handle_login_request(sock, msg, FD.device_id, FD.device_type, FD.password, tokenPtr, number_of_tokensPtr, FILE_LOG);
        break;
    case TYPE_TURN_ON:
        handle_turn_on_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr, FILE_LOG);
        break;
    case TYPE_TURN_OFF:
        handle_turn_off_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr, FILE_LOG);
        break;
    case TYPE_SET_FEEDER_DEVICE:
        handle_setup_device(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_GET_FEEDER_DEVICE_INFO:
        handle_get_feeder_device_info(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_MANUAL_FEED:
        handle_manual_feed(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    default:
        invalid_message_response(sock, msg, FILE_LOG);
        break;
    }
}

int main()
{
    create_device();
    return start_device_server(5100, feeder_handler);
}