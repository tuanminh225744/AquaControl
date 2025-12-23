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
    double salinity;
    double dissolved_oxygen;
    double pH;
    char device_type[20];
    char password[20];
    TokenSession token_sessions[MAX_CLIENTS];
    int number_of_tokens;
    int fish_pond_id;
} SensorDevice;

SensorDevice SD;
int *activePtr = &SD.active;
TokenSession *tokenPtr = SD.token_sessions;
int *number_of_tokensPtr = &SD.number_of_tokens;

void create_device()
{
    printf("Enter Device ID: ");
    scanf("%d", &SD.device_id);
    printf("Enter Password: ");
    scanf("%s", SD.password);
    printf("Enter fish pond ID: ");
    scanf("%d", &SD.fish_pond_id);
    SD.active = 1;
    SD.number_of_tokens = 0;
    strcpy(SD.device_type, "SENSOR");
    printf("[DEVICE] Create device successful.\n");
}

void handle_get_sensor_device_info(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    char *ptr = req->payload;

    // --- 1. Đọc Token ---
    if (sscanf(ptr, "%d", &req_token) != 1)
    {
        invalid_message_response(sockfd);
        return;
    }

    // --- 2. Validate Token ---
    if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }

    // --- 3. Check Active Status ---
    if (!(*activePtr))
    {
        device_not_active_response(sockfd);
        return;
    }

    // --- 4. Build Response Payload ---
    char payload_buffer[PAYLOAD_SIZE];
    int current_len = 0;

    // A. Thêm Device ID (I=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "I=%d ", SD.device_id);

    // B. Thêm Trạng thái hoạt động (S=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "S=%d ", SD.active);

    // C. Thêm Độ mặn (SAL=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "SAL=%.2lf ", SD.salinity);

    // D. Thêm Oxy hòa tan (DO=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "DO=%.2lf ", SD.dissolved_oxygen);

    // E. Thêm pH (PH=...)
    current_len += snprintf(payload_buffer + current_len, PAYLOAD_SIZE - current_len,
                            "PH=%.2lf", SD.pH);

    // --- 5. Send Info Response ---
    res.code = CODE_GET_SENSOR_DEVICE_INFO_OK;
    strcpy(res.payload, payload_buffer);

    send_all(sockfd, &res, sizeof(res));
    printf("[GET INFO DEVICE] Responded Code %d Payload: %s\n", res.code, res.payload);
}

void sensor_handler(int sock, struct Message *msg)
{
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, SD.device_id, SD.device_type);
        break;
    case TYPE_CONNECT:
        handle_connect_request(sock, msg, SD.device_id, SD.device_type, SD.password, tokenPtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_ON:
        handle_turn_on_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_TURN_OFF:
        handle_turn_off_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    case TYPE_GET_SENSOR_DEVICE_INFO:
        handle_get_sensor_device_info(sock, msg, tokenPtr, activePtr, number_of_tokensPtr);
        break;
    default:
        invalid_message_response(sock);
        break;
    }
}

int main()
{
    create_device();
    return start_device_server(5300, sensor_handler);
}