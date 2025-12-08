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
    int token;
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

    struct Message res;
    memset(&res, 0, sizeof(res));
    if (recv_all(sockfd, &res, sizeof(res)) <= 0)
    {
        printf("[ERROR] Failed to receive response\n");
        close(sockfd);
        return;
    }

    if (res.code != CODE_CONNECT_OK)
    {
        printf("[ERROR] Device connection failed. Code: %d\n", res.code);
        close(sockfd);
        return;
    }

    printf("[SUCCESS] Device connected.\n");

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
            printf("%d %s:%d | ID: %d | %d\n", i, devices[i].ip, devices[i].port, devices[i].device_id, devices[i].is_logged_in);
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

    while (1)
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
        printf("5. TURN ON \n");
        printf("6. TURN OFF \n");
        printf("0. Exit\n");
        printf("======================================\n");
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
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = TYPE_SCAN;
            msg.code = 0;
            strcpy(msg.payload, "SCAN");

            send(sock, &msg, sizeof(msg), 0);

            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                if (res.code == CODE_SCAN_OK)
                { // 100
                    printf(">> [FOUND] Info: %s\n", res.payload);

                    char *type_token = strtok(res.payload, ";");

                    char *id_token = strtok(NULL, ";");

                    if (type_token != NULL && id_token != NULL)
                    {
                        int id_val = atoi(id_token);

                        printf("   + Parsed Type: %s\n", type_token);
                        printf("   + Parsed ID:   %d\n", id_val);

                        devices[currentId].device_id = id_val;
                        printf("   -> [UPDATED] Device ID saved.\n");
                    }
                    else
                    {
                        printf("   -> [WARNING] Payload format error!\n");
                    }
                }
                else if (res.code == CODE_SCAN_FAIL)
                {
                    printf(">> [INFO] No device found (210)\n");
                }
                else
                {
                    printf(">> [ERROR] Scan failed. Code: %d\n", res.code);
                }
            }
            break;
        }

        case 4: // LOGIN
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = TYPE_CONNECT;
            msg.code = 0;

            int target_id;
            char password[32];

            printf("Enter Device ID to login: ");
            scanf("%d", &target_id);
            clear_stdin();

            printf("Enter Password: ");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = 0;

            snprintf(msg.payload, sizeof(msg.payload), "%d %s", target_id, password);

            send(sock, &msg, sizeof(msg), 0);

            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                if (res.code == CODE_LOGIN_OK)
                {
                    ///// code......
                }
                else if (res.code == CODE_LOGIN_FAIL)
                {
                    printf(">> [FAILED] Wrong Password (212)\n");
                }
                else if (res.code == CODE_LOGIN_NOID)
                {
                    printf(">> [FAILED] Device ID not found (211)\n");
                }
                else
                {
                    printf(">> [ERROR] Login failed. Code: %d\n", res.code);
                }
            }
            break;
        }

        case 5: // TURN ON
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = TYPE_TURN_ON;
            msg.code = 0;
            snprintf(msg.payload, sizeof(msg.payload), devices[currentId].token ? "%d" : "", devices[currentId].token);

            send(sock, &msg, sizeof(msg), 0);

            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                if (res.code == CODE_TURN_ON_OK)
                { // 110
                    printf(">> [SUCCESS] Turn on OK! %d %s\n", res.code, res.payload);
                }
                else
                {
                    printf(">> [ERROR] Turn on failed %d %s\n", res.code, res.payload);
                }
            }
            break;
        }

        case 6: // TURN OFF
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = TYPE_TURN_OFF;
            msg.code = 0;
            snprintf(msg.payload, sizeof(msg.payload), devices[currentId].token ? "%d" : "", devices[currentId].token);

            send(sock, &msg, sizeof(msg), 0);

            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                if (res.code == CODE_TURN_OFF_OK)
                { // 110
                    printf(">> [SUCCESS] Turn off OK! %d %s\n", res.code, res.payload);
                }
                else
                {
                    printf(">> [ERROR] Turn off failed. %d %s\n", res.code, res.payload);
                }
            }
            break;
        }

        case 0:
        {
            for (int i = 0; i < MAX_DEVICES; i++)
            {
                if (devices[i].active)
                {
                    close(devices[i].sockfd);
                }
            }
            printf("Exiting...\n");
            return 0;
        }
        }
    }
}
