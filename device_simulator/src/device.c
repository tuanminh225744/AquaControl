#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "../../common/messages.h"

int handle_check_token(int req_token, int *tokenPtr, int number_of_tokens)
{
    for (int i = 0; i < number_of_tokens; i++)
    {
        if (req_token == tokenPtr[i])
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

    send(sockfd, &res, sizeof(res), 0);
}

void invalid_token_response(int sockfd)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.code = CODE_TOKEN_INVALID;
    strcpy(res.payload, "Invalid Token");

    send(sockfd, &res, sizeof(res), 0);
}

void handle_scan_request(int sockfd, struct Message *req, int device_id, char *device_type)
{
    struct Message res;
    memset(&res, 0, sizeof(res));

    res.type = TYPE_SCAN;
    res.code = CODE_SCAN_OK;
    sprintf(res.payload, "%s;%d", device_type, device_id);

    send(sockfd, &res, sizeof(res), 0);
    printf("[SCAN] Responded Code %d\n", res.code);
}

void handle_connect_request(int sockfd, struct Message *req, int device_id, char *device_type, char *password, int *tokenPtr, int *number_of_tokensPtr)
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
        res.code = CODE_LOGIN_OK;
        int token = rand() % 9999 + 1000;

        snprintf(res.payload, sizeof(res.payload), "%d %s %d", device_id, device_type, token);
        tokenPtr[*number_of_tokensPtr] = token;
        (*number_of_tokensPtr)++;
        printf("[LOGIN] Success. Token: %d\n", token);
    }

    send(sockfd, &res, sizeof(res), 0);
}

void handle_turn_on_request(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr)
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
    else if (!handle_check_token(req_token, tokenPtr, *number_of_tokensPtr))
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

    send(sockfd, &res, sizeof(res), 0);
    printf("[TURN ON] Responded Code %d %s\n", res.code, res.payload);
}

void handle_turn_off_request(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr)
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
    else if (!handle_check_token(req_token, tokenPtr, *number_of_tokensPtr))
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

    send(sockfd, &res, sizeof(res), 0);
    printf("[TURN OFF] Responded Code %d %s\n", res.code, res.payload);
}
