#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../device_server.h"
#include "../../../common/messages.h"
#include "../../../common/network_utils.h"

typedef struct
{
    int device_id;
    int active;
    double flow_rate;
    double duration;
    char device_type[20];
    char password[20];
    int token[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} WaterPumpDevice;

WaterPumpDevice WPD;
int *tokenPtr = WPD.token;
int *number_of_tokensPtr = &WPD.number_of_tokens;
int *activePtr = &WPD.active;

void create_device()
{
    printf("Enter Device ID: ");
    scanf("%d", &WPD.device_id);
    printf("Enter Password: ");
    scanf("%s", WPD.password);
    printf("Enter fish pond ID: ");
    scanf("%d", &WPD.fish_pond_id);
    printf("Enter Flow Rate (m3/h): ");
    scanf("%lf", &WPD.flow_rate);
    printf("Enter Duration (h): ");
    scanf("%lf", &WPD.duration);
    WPD.active = 1;
    WPD.number_of_tokens = 0;
    strcpy(WPD.device_type, "WATERPUMP");
    printf("[DEVICE] Create device successful.\n");
}

void handle_setup_device(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    double req_flow_rate, req_duration;

    int k = sscanf(req->payload, "%d V=%lf T=%lf", &req_token, &req_flow_rate, &req_duration);

    if (k != 3)
    {
        invalid_message_response(sockfd);
        return;
    }
    else if (!handle_check_token(req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }
    else
    {
        res.code = CODE_SET_PUMP_DEVICE_OK;
        WPD.flow_rate = req_flow_rate;
        WPD.duration = req_duration;
        strcpy(res.payload, "Settings Updated");
    }

    send_all(sockfd, &res, sizeof(res));
    printf("[SET DEVICE] Responded Code %d %s\n", res.code, res.payload);
}

void handle_get_water_pump_device_info(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    char *ptr = req->payload;
    int offset = 0;

    // --- 1. Đọc Token ---
    if (sscanf(ptr, "%d%n", &req_token, &offset) != 1)
    {
        invalid_message_response(sockfd);
        return;
    }

    // --- 2. Validate Token ---
    if (!handle_check_token(req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }

    // --- 3. Build Response Payload ---
    char payload_buffer[PAYLOAD_SIZE];
    int current_len = 0;

    // A. Thêm Device ID (I=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "I=%d ", WPD.device_id);

    // B. Thêm Trạng thái hoạt động (S=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "S=%d ", WPD.active);

    // C. Thêm Tốc độ dòng chảy (V=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "V=%.2lf ", WPD.flow_rate);

    // D. Thêm Thời gian chạy (T=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "T=%.2lf", WPD.duration);

    // --- 4. Send Info Response ---
    res.code = CODE_GET_PUMP_DEVICE_INFO_OK;
    strcpy(res.payload, payload_buffer);

    send_all(sockfd, &res, sizeof(res));
    printf("[GET INFO DEVICE] Responded Code %d Payload: %s\n", res.code, res.payload);
}

void water_pump_handler(int sock, struct Message *msg)
{

    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, WPD.device_id, WPD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, WPD.device_id, WPD.device_type, WPD.password, tokenPtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_ON:
        handle_turn_on_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_OFF:
        handle_turn_off_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_SET_PUMP_DEVICE:
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
    return start_device_server(5400, water_pump_handler);
}