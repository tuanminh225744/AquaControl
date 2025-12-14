#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../common/messages.h"
#include "../../common/network_utils.h"
#include "controller.h"

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
    char device_type[20];
} DeviceConnection;

DeviceConnection devices[MAX_DEVICES];
int currentId = -1;

// Hàm dùng để xóa bộ đệm bàn phím tránh cho bị trôi lệnh khi dùng scanf
void clear_stdin()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        continue;
    }
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
    printf("[SUCCESS] Connected at slot [%d]\n", slot);
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
        if (currentId != -1 && devices[currentId].active)
        {
            printf("STATUS: Connected to Slot [%d] | IP: %s\n",
                   currentId, devices[currentId].ip);

            if (devices[currentId].is_logged_in)
            {
                printf("Device: [%s] | ID: %d | Token: %d\n", devices[currentId].device_type, devices[currentId].device_id, devices[currentId].token);
            }
            else
            {
                printf("Device: Not loggin !");
            }
        }
        else
        {
            printf("STATUS: No Device selected \n");
        }
        printf("------------------------------------\n");
        printf("1. Connect new device\n");
        printf("2. List of devices\n");
        printf("------------------------------------\n");
        if (currentId != -1 && devices[currentId].active)
        {
            printf("3. Scan \n");
            printf("4. Login \n");

            if (devices[currentId].is_logged_in == 1)
            {
                printf("=============================\n");
                printf("5. Turn on \n");
                printf("6. Turn off \n");
                if (strstr(devices[currentId].device_type, "PUMP") != NULL)
                {
                    printf("7. Set pump device \n");
                }
                else if (strstr(devices[currentId].device_type, "AERATOR") != NULL)
                {
                    printf("8. Set aerator device \n");
                }
                else if (strstr(devices[currentId].device_type, "FEEDER") != NULL)
                {
                    printf("9. Set feed device \n");
                }
                else if (strstr(devices[currentId].device_type, "PH") != NULL)
                {
                    printf("10. Set PH regulator device \n");
                }
                printf("11. Log out\n");
            }
        }
        printf("0. Exit\n");
        printf("======================================\n");
        printf("Select: \n");

        if (scanf("%d", &comman) != 1)
        {
            clear_stdin();
            continue;
        }
        clear_stdin();

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
            printf("[WARNING] Please select a device first (Option 1 or 2)!\n");
            continue;
        }

        int sock = devices[currentId].sockfd;

        if (comman == 3)
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = TYPE_SCAN;
            msg.code = 0;
            strcpy(msg.payload, "SCAN");

            send_all(sock, &msg, sizeof(msg));

            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                if (res.code == CODE_SCAN_OK)
                {
                    printf("Found Infor: %s\n", res.payload);

                    char *type_device = strtok(res.payload, ";");
                    char *id_device = strtok(NULL, ";");

                    if (type_device != NULL && id_device != NULL)
                    {
                        int id_value = atoi(id_device);
                        devices[currentId].device_id = id_value;
                        strcpy(devices[currentId].device_type, type_device);
                        printf("Infor: Type:%s  ID: %d\n", type_device, id_value);
                    }
                }
                else if (res.code == CODE_SCAN_FAIL)
                {
                    printf("Resquest Sacn fail!\n");
                }
            }
            continue;
        }
        if (comman == 4)
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = TYPE_CONNECT;
            msg.code = 0;

            int target_id;
            char password[32];

            printf("Please enter id want to loggin");
            scanf("%d", &target_id);
            clear_stdin();
            printf("Please enter password");
            fgets(password, sizeof(password), stdin);
            password[sizeof(password) - 1] = '\0';

            snprintf(msg.payload, sizeof(msg.payload), "%d %s", target_id, password);

            send_all(sock, &msg, sizeof(msg));

            if (recv_all(sock, &res, sizeof(res)) > 0)
            {
                printf("%d", res.code);
                if (res.code == CODE_LOGIN_OK)
                {
                    int recv_id;
                    char recv_type[20];
                    int recv_token;

                    if (sscanf(res.payload, "%d %s %d", &recv_id, recv_type, &recv_token) == 3)
                    {
                        devices[currentId].is_logged_in = 1;
                        devices[currentId].token = recv_token;
                        devices[currentId].device_id = recv_id;
                        strcpy(devices[currentId].device_type, recv_type);

                        printf("Success Login OK! Type: %s\n", recv_type);
                    }
                }
                else if (res.code == CODE_LOGIN_FAIL)
                {
                    printf("[FAILED] Wrong Password\n");
                }

                else if (res.code == CODE_LOGIN_NOID)
                {
                    printf("[FAILED] Device ID not found\n");
                }
                else
                {
                    printf("[ERROR] Login failed \n");
                }
            }
            continue;
        }
        if (devices[currentId].is_logged_in == 0)
        {
            printf("[ACCESS DENIED] You must LOGIN first (Option 4) to use this function!\n");
            continue;
        }

        switch (comman)
        {
        case 5: // TURN ON
        {
            turn_on_device(sock, devices[currentId].token);
            break;
        }

        case 6: // TURN OFF
        {
            turn_off_device(sock, devices[currentId].token);
            break;
        }

        case 7: // SET PUMP DEVICE
        {

            set_pump_device(sock, devices[currentId].token);

            break;
        }

        case 8: // SET AERATOR DEVICE
        {

            set_aerator_device(sock, devices[currentId].token);

            break;
        }

        case 9: // SET FEEDER DEVICE
        {

            set_feeder_device(sock, devices[currentId].token);

            break;
        }

        case 10: // SET PH REGULATOR DEVICE
        {

            set_ph_regulator_device(sock, devices[currentId].token);

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
        default:
            printf("[WARNING] Invalid Option!\n");
            break;
        }

        if (comman == 11)
        {
            if (devices[currentId].is_logged_in)
            {
                devices[currentId].is_logged_in = 0;
                devices[currentId].token = 0;
                printf("SUCCESS Loged out successfully!");
            }
            else
            {
                printf("You are not loggin!");
            }
            continue;
        }
    }
    return 0;
}