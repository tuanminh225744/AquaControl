#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "device_server.h"
#include "../../common/messages.h"
#include "../../common/network_utils.h"

// ==================== THREAD: XỬ LÝ CLIENT ====================
void *client_thread(void *arg)
{
    struct client_info *info = (struct client_info *)arg;
    int sock = info->sock;
    handle_msg_fn handler = info->handler;
    free(info);

    char buff[100];

    // --- Handshake: nhận "HELLO" ---
    if (recv_line(sock, buff, sizeof(buff)) <= 0)
    {
        printf("[DEVICE] Client disconnected during handshake.\n");
        close(sock);
        return NULL;
    }

    printf("[DEVICE] Handshake received: %s\n", buff);

    if (strcmp(buff, "HELLO") != 0)
    {
        printf("[DEVICE] Wrong handshake. Closing.\n");
        close(sock);
        return NULL;
    }

    // --- Vòng lặp nhận message ---
    struct Message msg;

    while (1)
    {
        int n = recv_all(sock, &msg, sizeof(msg));

        if (n <= 0)
        {
            printf("[DEVICE] Client disconnected.\n");
            break;
        }

        // Gọi callback của thiết bị
        handler(sock, &msg);
    }

    close(sock);
    return NULL;
}

// ==================== START SERVER ====================
int start_device_server(int port, handle_msg_fn handler)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("[DEVICE] Starting server on port %d...\n", port);

    // Tạo socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket failed");
        return 1;
    }

    // Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    listen(server_fd, 5);
    printf("[DEVICE] Server is running.\n");

    // Accept loop
    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("[DEVICE] New connection accepted.\n");

        // Gói dữ liệu cho thread
        struct client_info *info = malloc(sizeof(struct client_info));
        info->sock = client_fd;
        info->handler = handler;

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, info);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
