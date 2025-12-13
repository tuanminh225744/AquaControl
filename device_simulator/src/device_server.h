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

// Hàm khởi động server cho thiết bị
int start_device_server(int port, handle_msg_fn handler);
int handle_check_token(int req_token, int *tokenPtr, int number_of_tokens);
void handle_scan_request(int sockfd, struct Message *req, int device_id, char *device_type);
void handle_connect_request(int sockfd, struct Message *req, int device_id, char *device_type, char *password, int *tokenPtr, int *number_of_tokensPtr);
void handle_turn_on_request(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr);
void handle_turn_off_request(int sockfd, struct Message *req, int *tokenPtr, int *activePtr, int *number_of_tokensPtr);
void invalid_message_response(int sockfd);
void invalid_token_response(int sockfd);
#endif
