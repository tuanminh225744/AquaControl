#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>

#include "device_server.h"
#include "../../common/messages.h"
#include "../../common/network_utils.h"

int do_handshake(int fd)
{
    char buff[100];
    struct Message res = {0};

    if (recv_line(fd, buff, sizeof(buff)) <= 0)
        return -1;

    if (strcmp(buff, "HELLO") != 0)
    {
        res.code = CODE_INVALID_MSG;
        sprintf(res.payload, "Invalid handshake");
        send_all(fd, &res, sizeof(res));
        return -1;
    }

    res.code = CODE_CONNECT_OK;
    sprintf(res.payload, "Device connected");
    send_all(fd, &res, sizeof(res));
    return 0;
}

// ==================== START SERVER ====================
int start_device_server(int port, handle_msg_fn handler)
{
    int listen_sock, connfd, sockfd;
    int maxfd, maxi, i, nready;
    int client[FD_SETSIZE];

    struct sockaddr_in server_addr, client_addr;
    socklen_t clilen;

    fd_set allset, readfds;

    printf("[DEVICE] Starting server on port %d...\n", port);

    // ===== Create socket =====
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        close(listen_sock);
        return 1;
    }

    if (listen(listen_sock, 5) < 0)
    {
        perror("listen error");
        close(listen_sock);
        return 1;
    }

    // ===== Init client array =====
    maxfd = listen_sock;
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    FD_ZERO(&allset);
    FD_SET(listen_sock, &allset);

    printf("[DEVICE] Server is running.\n");

    // ==================== MAIN LOOP ====================
    while (1)
    {
        readfds = allset;
        nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (nready < 0)
        {
            perror("select error");
            continue;
        }

        // ===== New connection =====
        if (FD_ISSET(listen_sock, &readfds))
        {
            clilen = sizeof(client_addr);
            connfd = accept(listen_sock, (struct sockaddr *)&client_addr, &clilen);
            if (connfd < 0)
            {
                perror("accept error");
            }
            else
            {
                printf("[DEVICE] New connection from %s\n", inet_ntoa(client_addr.sin_addr));

                if (do_handshake(connfd) < 0)
                {
                    close(connfd);
                }
                else
                {
                    // ===== Save client =====
                    for (i = 0; i < FD_SETSIZE; i++)
                    {
                        if (client[i] < 0)
                        {
                            client[i] = connfd;
                            break;
                        }
                    }

                    if (i == FD_SETSIZE)
                    {
                        printf("[DEVICE] Too many clients.\n");
                        close(connfd);
                    }
                    else
                    {
                        FD_SET(connfd, &allset);
                        if (connfd > maxfd)
                            maxfd = connfd;
                        if (i > maxi)
                            maxi = i;
                        printf("[DEVICE] Connected successfully.\n");
                    }
                }
            }

            if (--nready <= 0)
                continue;
        }

        // ===== Handle client data =====
        for (i = 0; i <= maxi; i++)
        {
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &readfds))
            {
                struct Message msg;
                memset(&msg, 0, sizeof(msg));
                int n = recv_all(sockfd, &msg, sizeof(msg));
                msg.payload[sizeof(msg.payload) - 1] = '\0';

                if (n <= 0)
                {
                    printf("[DEVICE] Client disconnected (fd=%d)\n", sockfd);
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }
                else
                {
                    handler(sockfd, &msg);
                }

                if (--nready <= 0)
                    break;
            }
        }
    }

    close(listen_sock);
    return 0;
}
