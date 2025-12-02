#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"

// Hàm dùng để xóa bộ đệm bàn phím tránh cho bị trôi lệnh khi dùng scanf
void clear_stdin()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int main()
{
    char server_ip[50];
    int server_port;

    // --- 1. NHẬP CẤU HÌNH TỪ BÀN PHÍM ---
    printf("--- CONTROLLER CONFIGURATION ---\n");

    printf("Enter Device IP (e.g., 127.0.0.1): ");
    scanf("%s", server_ip);

    printf("Enter Device Port (e.g., 8080): ");
    scanf("%d", &server_port);
    clear_stdin();
    printf("[INIT] Connecting to %s:%d ...\n", server_ip, server_port);

    int sockfd;
    struct sockaddr_in serv_addr;
    struct Message msg, res;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connect failed");
        return 1;
    }

    send_line(sockfd, "HELLO");
    printf("[CONTROLLER] Sent Handshake: HELLO\n");

    int comman;
    int app_running = 1;

    while (app_running)
    {
        printf("\n========= AQUACULTURE CONTROL =========\n");
        printf("1. Scan Devices (Quét thiết bị)\n");
        printf("2. Login to Device (Kết nối)\n");
        printf("0. Exit\n");
        printf("=========================================\n");
        printf("Select option: ");

        if (scanf("%d", &comman) != 1)
        {
            clear_stdin();
            continue;
        }
        clear_stdin();

        switch (comman)
        {
        case 1: // SCAN
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_SCAN_REQUEST;
            msg.app_id = 1;
            strcpy(msg.payload, "SCAN");

            send(sockfd, &msg, sizeof(msg), 0);

            if (recv_all(sockfd, &res, sizeof(res)) > 0)
            {
                if (res.type == MSG_SCAN_RESPONSE)
                {
                    printf(">> [FOUND] Device ID: %d | Info: %s\n", res.device_id, res.payload);
                }
                else
                {
                    printf(">> [ERROR] Unexpected response type: %d\n", res.type);
                }
            }
            break;

        case 2: // LOGIN
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_CONNECT_REQUEST;
            msg.app_id = 1;

            printf("Enter Password: ");
            fgets(msg.payload, sizeof(msg.payload), stdin);
            msg.payload[strcspn(msg.payload, "\n")] = 0;

            send(sockfd, &msg, sizeof(msg), 0);

            if (recv_all(sockfd, &res, sizeof(res)) > 0)
            {
                if (res.type == MSG_CONNECT_ACCEPT)
                {
                    printf(">> [SUCCESS] Login OK! Token: %s\n", res.payload);
                }
                else if (res.type == MSG_CONNECT_DENY)
                {
                    printf(">> [FAILED] Wrong password: %s\n", res.payload);
                }
            }
            break;

        case 0: // EXIT
            app_running = 0;
            printf("Exiting...\n");
            printf("See you again..... Bye bye\n");
            break;

        default:
            printf("Invalid option!\n");
        }
    }

    close(sockfd);
    return 0;
}