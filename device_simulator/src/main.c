#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "device.h"
#include "../../common/messages.h"
#include "../../common/network_utils.h"

int PORT;

void *client_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    free(socket_desc);

    // Yêu cầu Client phải gửi "HELLO" để xác nhận giao thức
    char buff[100];
    if (recv_line(sock, buff, sizeof(buff)) > 0)
    {
        printf("[DEVICE] Handshake received: %s\n", buff);
        if (strcmp(buff, "HELLO") != 0)
        {
            close(sock);
            return NULL;
        }
    }
    else
    {
        close(sock);
        return NULL;
    }

    // --- Xử lý yêu cầu ---
    struct Message msg;
    while (1)
    {
        int n = recv_all(sock, &msg, sizeof(msg));

        if (n <= 0)
        {
            printf("[DEVICE] Client disconnected.\n");
            break;
        }

        switch (msg.type)
        {
        case MSG_SCAN_REQUEST:
            handle_scan_request(sock, &msg);
            break;
        case MSG_CONNECT_REQUEST:
            handle_connect_request(sock, &msg);
            break;
        default:
            printf("[DEVICE] Unknown MSG Type: %d\n", msg.type);
        }
    }

    close(sock);
    return NULL;
}

int main()
{
    // --- 1. NHẬP PORT TỪ BÀN PHÍM ---
    printf("--- DEVICE CONFIGURATION ---\n");
    printf("Enter Port to listen: ");
    scanf("%d", &PORT);

    int server_fd, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t tid;

    // 1. Tạo Socket TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    listen(server_fd, 5);

    printf("[DEVICE] TCP Server running on port %d\n", PORT);

    // 3. Vòng lặp Accept
    while (1)
    {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }
        printf("[DEVICE] New connection.\n");

        new_sock = malloc(sizeof(int));
        *new_sock = client_fd;

        pthread_create(&tid, NULL, client_handler, (void *)new_sock);
        pthread_detach(tid);
    }

    return 0;
}