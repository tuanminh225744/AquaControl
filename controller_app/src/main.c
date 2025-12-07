#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"

#define MAX_DEVICES 4

typedef struct
{
    int sockfd;
    char ip[20];
    int port;
    int device_id;
    int is_logged_in;
    int active;
} DeviceConnection;

DeviceConnection devices[MAX_DEVICES];
int currentId = -1;

// Hàm dùng để xóa bộ đệm bàn phím tránh cho bị trôi lệnh khi dùng scanf
void clear_stdin()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void init_device_list()
{
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        devices[i].active = 0;
        devices[i].sockfd = -1;
        devices[i].is_logged_in = 0;
    }
}

void connect_new_device()
{
    int slot = -1;
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (devices[i].active == 0)
        {
            slot = i;
            break;
        }
    }

    if (slot == -1)
    {
        printf("[ERROR] List device is full \n");
        return;
    }

    char ip[20];
    int port;
    printf("Enter ip (Ex 127.0.0.1): ");
    scanf("%s", ip);
    printf("Enter Port (Ex 5500): ");
    scanf("%d", &port);
    clear_stdin();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    printf("[INIT] Connecting to %s:%d ...\n", ip, port);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connect failed");
        return;
    }

    send_line(sockfd, "HELLO");

    // Lưu vào mảng
    devices[slot].active = 1;
    devices[slot].sockfd = sockfd;
    strcpy(devices[slot].ip, ip);
    devices[slot].port = port;
    devices[slot].is_logged_in = 0;

    // Tự động chọn thiết bị này
    currentId = slot;
    printf(">> [SUCCESS] Connected at slot [%d]\n", slot);
}

void list_device()
{
    printf("=========== List devices ============\n");
    int count = 0;
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (devices[i].active == 1)
        {
            printf("%d %s:%d | ID: %d | %s %s\n", i, devices[i].ip, devices[i].port, devices[i].device_id, devices[i].is_logged_in ? "[LOGGED IN]" : "[NOT LOGIN]");
            count++;
        }
    }

    if (count == 0)
    {
        printf("Not have device connected! ");
        return;
    }

    printf("Enter ID of slot to tranfer device");
    int id;
    scanf("%d", &id);
    clear_stdin();

    if (id >= 0 && id < MAX_DEVICES && devices[id].active == 1)
    {
        currentId = id;
        printf("Moved the device to the slot %d\n", id);
    }
}

int main()
{
    init_device_list();
    struct Message msg, res;
    int comman;
    int app_running = 1;

    while (app_running)
    {
        printf("\n========= MULTI-CONTROLLER =========\n");
        if (currentId != -1)
        {
            printf("TARGET: [%d] %s:%d\n", currentId,
                   devices[currentId].ip, devices[currentId].port);
        }
        else
        {
            printf("TARGET: None \n");
        }
        printf("------------------------------------\n");
        printf("1. Connect new device\n");
        printf("2. List of devices\n");
        printf("------------------------------------\n");
        printf("3. SCAN \n");
        printf("4. LOGIN \n");
        printf("5. CONTROL \n");
        printf("0. Exit\n");
        printf("=======================================");
        printf("Select: ");

        if (scanf("%d", &comman) != 1)
        {
            clear_stdin();
            continue;
        }
        clear_stdin();

        if (comman == 0)
        {
            break;
        }
        if (comman == 1)
        {
            connect_new_device();
            continue;
        }
        if (comman == 2)
        {
            list_device();
            continue;
        }

        if (currentId == -1)
        {
            printf(">> [ERROR] Chua chon thiet bi!\n");
            continue;
        }

        int sock = devices[currentId].sockfd;
        switch (comman)
        {
        case 3: // SCAN
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_SCAN_REQUEST;
            strcpy(msg.payload, "SCAN");
            send(sock, &msg, sizeof(msg), 0);

            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                if (res.type == MSG_SCAN_RESPONSE)
                {
                    printf(">> [FOUND] ID: %d | %s\n", res.device_id, res.payload);
                    devices[currentId].device_id = res.device_id;
                }
            }
            break;

        case 4: // LOGIN
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_CONNECT_REQUEST;
            printf("Password: ");
            fgets(msg.payload, sizeof(msg.payload), stdin);
            msg.payload[strcspn(msg.payload, "\n")] = 0;

            send(sock, &msg, sizeof(msg), 0);
            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                if (res.type == MSG_CONNECT_ACCEPT)
                {
                    printf(">> Login OK! Token: %s\n", res.payload);
                    devices[currentId].is_logged_in = 1;
                }
                else
                {
                    printf(">> Login Failed: %s\n", res.payload);
                }
            }
            break;

        case 5: // CONTROL
            if (!devices[currentId].is_logged_in)
            {
                printf(">> [ERROR] Login required!\n");
            }
            else
            {
                memset(&msg, 0, sizeof(msg));
                msg.type = MSG_PUMP_CONTROL; // Đảm bảo đã define trong messages.h
                strcpy(msg.payload, "TOGGLE");
                send(sock, &msg, sizeof(msg), 0);
                printf(">> Sent Control Command.\n");
            }
            break;
        }
    }

    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (devices[i].active)
        {
            close(devices[i].sockfd);
        }
    }

    return 0;
}