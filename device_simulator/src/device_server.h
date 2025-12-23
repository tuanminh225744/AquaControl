#ifndef DEVICE_SERVER_H
#define DEVICE_SERVER_H

#include "../../common/messages.h"

// Callback xử lý message cho từng thiết bị
typedef void (*handle_msg_fn)(int sock, struct Message *msg);

// Thông tin truyền vào mỗi thread client
struct client_info
{
    int sock;
    handle_msg_fn handler;
};

typedef struct
{
    int token;
    struct sockaddr_in addr;
} TokenSession;

int start_device_server(int port, handle_msg_fn handler);
void handle_write_device_log(int sockfd, char *file_name, int req_type, const char *req_payload, int res_code, const char *res_payload);
int handle_check_token(int sockfd, int req_token, TokenSession *tokenPtr, int number_of_tokens);
void handle_scan_request(int sockfd, struct Message *req, int device_id, char *device_type, char *file_name);
void handle_login_request(int sockfd, struct Message *req, int device_id, char *device_type, char *password, TokenSession *tokenPtr, int *number_of_tokensPtr, char *file_name);
void handle_turn_on_request(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr, char *file_name);
void handle_turn_off_request(int sockfd, struct Message *req, TokenSession *tokenPtr, int *activePtr, int *number_of_tokensPtr, char *file_name);
void invalid_message_response(int sockfd, struct Message *req, char *file_name);
void invalid_token_response(int sockfd, struct Message *req, char *file_name);
void device_not_active_response(int sockfd, struct Message *req, char *file_name);
#endif
