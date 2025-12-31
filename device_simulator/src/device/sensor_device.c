#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../device_server.h"
#include "../../../common/messages.h"
#include "../../../common/network_utils.h"

#define FILE_LOG "sensor_device.log"

typedef struct
{
    int device_id;
    int active;
    double salinity;
    double dissolved_oxygen;
    double pH;
    char device_type[20];
    char password[MAX_PASS_LENGTH];
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

void handle_get_pond_info(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokens, int pond_id, char *file_read_name, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;

    if (sscanf(req->payload, "%d", &req_token) != 1)
    {
        invalid_message_response(sockfd, req, file_name);
        return;
    }

    if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokens))
    {
        invalid_token_response(sockfd, req, file_name);
        return;
    }

    if (!(*activePtr))
    {
        device_not_active_response(sockfd, req, FILE_LOG);
        return;
    }

    FILE *fp = fopen(file_read_name, "r");
    if (!fp)
    {
        res.code = CODE_FILE_OPEN_ERROR;
        strcpy(res.payload, "Cannot open pond file");
        send_all(sockfd, &res, sizeof(res));
        return;
    }

    int id;
    double salinity, dissolved_oxygen, pH;
    int found = 0;

    while (fscanf(fp, "%d$%lf$%lf$%lf\n", &id, &salinity, &dissolved_oxygen, &pH) == 4)
    {
        if (id == pond_id)
        {
            snprintf(res.payload, PAYLOAD_SIZE, "ID:%d SAL:%.2lf DO:%.2lf PH:%.2lf", id, salinity, dissolved_oxygen, pH);
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (found)
    {
        res.code = CODE_POND_INFO_OK;
    }
    else
    {
        res.code = CODE_POND_INFO_FAIL;
        strcpy(res.payload, "Pond not found");
    }

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);

    printf("[GET POND INFO] Responded Code %d %s\n",
           res.code, res.payload);
}

void sensor_handler(int sock, struct Message *msg)
{
    switch (msg->type)
    {
    case TYPE_SCAN:
        handle_scan_request(sock, msg, SD.device_id, SD.device_type, FILE_LOG);
        break;
    case TYPE_LOGIN:
        handle_login_request(sock, msg, SD.device_id, SD.device_type, SD.password, tokenPtr, number_of_tokensPtr, FILE_LOG);
        break;
    case TYPE_TURN_ON:
        handle_turn_on_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr, FILE_LOG);
        break;
    case TYPE_TURN_OFF:
        handle_turn_off_request(sock, msg, tokenPtr, activePtr, number_of_tokensPtr, FILE_LOG);
        break;
    case TYPE_GET_POND_INFO:
        handle_get_pond_info(sock, msg, tokenPtr, activePtr, number_of_tokensPtr, SD.fish_pond_id, "database/pond_data.txt", FILE_LOG);
        break;
    case TYPE_CHPASS:
        handle_change_password(sock, msg, SD.password, tokenPtr, activePtr, number_of_tokensPtr, FILE_LOG);
        break;
    default:
        invalid_message_response(sock, msg, FILE_LOG);
        break;
    }
}

int main()
{
    create_device();
    return start_device_server(5300, sensor_handler);
}