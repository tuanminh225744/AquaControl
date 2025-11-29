#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"

#define PORT 8080

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct Message msg, res;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connect failed");
        return 1;
    }

    send_line(sockfd, "HELLO");
    printf("[CONTROLLER] Sent Handshake: HELLO\n");

    // Ví dụ gửi SCAN
    msg.type = MSG_SCAN_REQUEST;
    msg.app_id = 1;
    strcpy(msg.payload, "SCAN");

    send(sockfd, &msg, sizeof(msg), 0);
    recv(sockfd, &res, sizeof(res), 0);
    if(res.type==MSG_CONNECT_ACCEPT){
        printf("[CONTROLLER] Token received: %s\n", res.payload);
    }else{
        printf("[CONTROLLER] Access Denied. \n");
    }

    close(sockfd);
    return 0;
}