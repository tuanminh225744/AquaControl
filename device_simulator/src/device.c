#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"
#include "device_server.h"

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

void invalid_message_response(int sockfd)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.code = CODE_INVALID_MSG;
    strcpy(res.payload, "Invalid Format");

    send_all(sockfd, &res, sizeof(res));
}

void invalid_token_response(int sockfd)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.code = CODE_TOKEN_INVALID;
    strcpy(res.payload, "Invalid Token");

    send_all(sockfd, &res, sizeof(res));
}

void device_not_active_response(int sockfd)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.code = CODE_DEVICE_OFF;
    strcpy(res.payload, "Device is OFF");

    send_all(sockfd, &res, sizeof(res));
}

void handle_scan_request(int sockfd, struct Message *req, int device_id, char *device_type)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.type = TYPE_SCAN;
    res.code = CODE_SCAN_OK;
    sprintf(res.payload, "%s;%d", device_type, device_id);

    send_all(sockfd, &res, sizeof(res));
    printf("[SCAN] Responded Code %d\n", res.code);
}

void handle_connect_request(int sockfd, struct Message *req, int device_id, char *device_type, char *password, TokenSession *tokenPtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.type = TYPE_CONNECT;

    int req_id;
    char req_pass[64];

    if (sscanf(req->payload, "%d %s", &req_id, req_pass) != 2)
    {
        invalid_message_response(sockfd);
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

        printf("[LOGIN] Success. Token=%d From %s:%d\n", token, inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
    }

    send_all(sockfd, &res, sizeof(res));
}

void handle_turn_on_request(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;

    int k = sscanf(req->payload, "%d", &req_token);

    if (k != 1)
    {
        invalid_message_response(sockfd);
        return;
    }
    else if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }
    else
    {
        res.code = CODE_TURN_ON_OK;
        *activePtr = 1;
        strcpy(res.payload, "Turn On Success");
    }

    send_all(sockfd, &res, sizeof(res));
    printf("[TURN ON] Responded Code %d %s\n", res.code, res.payload);
}

void handle_turn_off_request(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr)
{
    struct Message res;
    memset(&res, 0, sizeof(res));
    int req_token;

    int k = sscanf(req->payload, "%d", &req_token);

    if (k != 1)
    {
        invalid_message_response(sockfd);
        return;
    }
    else if (!handle_check_token(sockfd, req_token, tokenPtr, *number_of_tokensPtr))
    {
        invalid_token_response(sockfd);
        return;
    }
    else
    {
        res.code = CODE_TURN_OFF_OK;
        *activePtr = 0;
        strcpy(res.payload, "Turn Off Success");
    }

    send_all(sockfd, &res, sizeof(res));
    printf("[TURN OFF] Responded Code %d %s\n", res.code, res.payload);
}
