#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"
#include "device_server.h"

const char *msg_type_to_string(int type)
{
    switch (type)
    {
    case TYPE_SCAN:
        return "SCAN";
    case TYPE_LOGIN:
        return "LOGIN";
    case TYPE_CHPASS:
        return "CHPASS";
    case TYPE_GETINFO:
        return "GETINFO";
    case TYPE_TURN_ON:
        return "TURN_ON";
    case TYPE_TURN_OFF:
        return "TURN_OFF";
    case TYPE_SET_PUMP_DEVICE:
        return "SET_PUMP_DEVICE";
    case TYPE_SET_AERATOR_DEVICE:
        return "SET_AERATOR_DEVICE";
    case TYPE_SET_FEEDER_DEVICE:
        return "SET_FEEDER_DEVICE";
    case TYPE_SET_PH_REGULATOR_DEVICE:
        return "SET_PH_REGULATOR_DEVICE";
    case TYPE_GET_PUMP_DEVICE_INFO:
        return "GET_PUMP_DEVICE_INFO";
    case TYPE_GET_AERATOR_DEVICE_INFO:
        return "GET_AERATOR_DEVICE_INFO";
    case TYPE_GET_FEEDER_DEVICE_INFO:
        return "GET_FEEDER_DEVICE_INFO";
    case TYPE_GET_PH_REGULATOR_DEVICE_INFO:
        return "GET_PH_REGULATOR_DEVICE_INFO";
    case TYPE_GET_POND_INFO:
        return "GET_POND_INFO";
    case TYPE_MANUAL_AERATE:
        return "MANUAL_AERATE";
    case TYPE_MANUAL_FEED:
        return "MANUAL_FEED";
    case TYPE_MANUAL_PUMP:
        return "MANUAL_PUMP";
    default:
        return "UNKNOWN_TYPE";
    }
}

void handle_write_device_log(int sockfd, char *file_name, int req_type, const char *req_payload, int res_code, const char *res_payload)
{
    FILE *log_file = fopen(file_name, "a");
    if (log_file == NULL)
    {
        perror("open log file");
        return;
    }

    /* ===== Time ===== */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "[%d/%m/%Y %H:%M:%S]", tm_info);

    /* ===== Client IP : Port ===== */
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);
    char client_addr[64] = "UNKNOWN";

    if (getpeername(sockfd, (struct sockaddr *)&peer, &len) == 0)
    {
        snprintf(client_addr, sizeof(client_addr), "%s:%d", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
    }

    /* ===== Write log ===== */
    fprintf(log_file,
            "%s$%s$%s$%s$%d$%s\n",
            time_buf,
            client_addr,
            msg_type_to_string(req_type),
            req_payload,
            res_code,
            res_payload);

    fclose(log_file);
}

int handle_check_token(int sockfd, int req_token, TokenSession *tokenPtr, int number_of_tokens)
{
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);

    // Lấy IP + port thực tế của client đang gọi
    if (getpeername(sockfd, (struct sockaddr *)&peer, &len) < 0)
    {
        return 0;
    }

    for (int i = 0; i < number_of_tokens; i++)
    {
        if (tokenPtr[i].token == req_token && tokenPtr[i].addr.sin_addr.s_addr == peer.sin_addr.s_addr && tokenPtr[i].addr.sin_port == peer.sin_port)
        {
            return 1;
        }
    }

    return 0;
}

void invalid_message_response(int sockfd, struct Message *req, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.code = CODE_INVALID_MSG;
    strcpy(res.payload, "Invalid Format");

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[INVALID MESSAGE] Responded Code %d %s\n", res.code, res.payload);
}

void invalid_token_response(int sockfd, struct Message *req, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.code = CODE_TOKEN_INVALID;
    strcpy(res.payload, "Invalid Token");

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[INVALID TOKEN] Responded Code %d %s\n", res.code, res.payload);
}

void device_not_active_response(int sockfd, struct Message *req, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.code = CODE_DEVICE_OFF;
    strcpy(res.payload, "Device is OFF");

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[DEVICE OFF] Responded Code %d %s\n", res.code, res.payload);
}

void handle_scan_request(int sockfd, struct Message *req, int device_id, char *device_type, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.type = TYPE_SCAN;
    res.code = CODE_SCAN_OK;
    sprintf(res.payload, "%s;%d", device_type, device_id);

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[SCAN] Responded Code %d\n", res.code);
}

void handle_login_request(int sockfd, struct Message *req, int device_id, char *device_type, char *password, TokenSession *tokenPtr, int *number_of_tokensPtr, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.type = TYPE_LOGIN;

    int req_id;
    char req_pass[64];

    if (sscanf(req->payload, "%d %s", &req_id, req_pass) != 2)
    {
        invalid_message_response(sockfd, req, file_name);
        return;
    }
    else if (req_id != device_id)
    {
        res.code = CODE_LOGIN_NOID;
        strcpy(res.payload, "Device ID Not Found");
    }
    else if (strcmp(req_pass, password) != 0)
    {
        res.code = CODE_LOGIN_FAIL;
        strcpy(res.payload, "Wrong Password");
    }
    else
    {
        // ===== LOGIN OK =====
        res.code = CODE_LOGIN_OK;

        int token = rand() % 900000 + 100000;

        // Lấy IP + port của client
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        getpeername(sockfd, (struct sockaddr *)&peer, &len);

        // Lưu token + địa chỉ client
        tokenPtr[*number_of_tokensPtr].token = token;
        tokenPtr[*number_of_tokensPtr].addr = peer;
        (*number_of_tokensPtr)++;

        snprintf(res.payload, sizeof(res.payload), "%d %s %d", device_id, device_type, token);
    }

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[LOGIN] Responded Code %d %s\n", res.code, res.payload);
}

void handle_change_password(int sockfd, struct Message *req, char *password, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;
    char req_old_password[MAX_PASS_LENGTH];
    char req_new_password[MAX_PASS_LENGTH];
    int k = sscanf(req->payload, "%d %s %s", &req_token, req_old_password, req_new_password);
    if (k != 3)
    {
        invalid_message_response(sockfd, req, file_name);
        return;
    }
    else if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd, req, file_name);
        return;
    }
    else if (strcmp(req_old_password, password) != 0)
    {
        res.code = CODE_OLD_PASS_INCORRECT;
        strcpy(res.payload, "Old Password Incorrect");
    }
    else
    {
        strcpy(password, req_new_password);
        res.code = CODE_CHPASS_OK;
        strcpy(res.payload, "Change Password Success");
    }

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[CHANGE PASS] Responded Code %d %s\n", res.code, res.payload);
}

void handle_turn_on_request(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;

    int k = sscanf(req->payload, "%d", &req_token);

    if (k != 1)
    {
        invalid_message_response(sockfd, req, file_name);
        return;
    }
    else if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd, req, file_name);
        return;
    }
    else
    {
        res.code = CODE_TURN_ON_OK;
        *activePtr = 1;
        strcpy(res.payload, "Turn On Success");
    }

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[TURN ON] Responded Code %d %s\n", res.code, res.payload);
}

void handle_turn_off_request(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr, char *file_name)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;

    int k = sscanf(req->payload, "%d", &req_token);

    if (k != 1)
    {
        invalid_message_response(sockfd, req, file_name);
        return;
    }
    else if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd, req, file_name);
        return;
    }
    else
    {
        res.code = CODE_TURN_OFF_OK;
        *activePtr = 0;
        strcpy(res.payload, "Turn Off Success");
    }

    send_all(sockfd, &res, sizeof(res));
    handle_write_device_log(sockfd, file_name, req->type, req->payload, res.code, res.payload);
    printf("[TURN OFF] Responded Code %d %s\n", res.code, res.payload);
}
