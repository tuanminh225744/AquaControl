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

void connect_new_device(char *ip, int port)
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
        printf("[FAILED] List device is full \n");
        return;
    }

    // char ip[20];
    // int port;
    // printf("Enter ip (Ex 127.0.0.1): ");
    // scanf("%s", ip);
    // printf("Enter Port (Ex 5500): ");
    // scanf("%d", &port);
    // clear_stdin();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    printf("[INIT] Connecting to %s:%d ...\n", ip, port);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connect failed");
        return;
    }

    struct Message req;
    memset(&req, 0, sizeof(req));
    req.type = TYPE_HANDSHAKE;
    strcpy(req.payload, "HELLO");
    send_all(sockfd, &req, sizeof(req));
    // send_line(sockfd, "HELLO");

    struct Message res;
    memset(&res, 0, sizeof(res));
    if (recv_all(sockfd, &res, sizeof(res)) <= 0)
    {
        printf("[FAILED] Failed to receive response\n");
        close(sockfd);
        return;
    }

    if (res.code == CODE_HANDSHAKE_OK)
    {
        printf("[SUCCESS] Device connected.\n");
    }
    else
    {
        printf("[FAILED] Device connection failed. %d %s\n", res.code, res.payload);
        close(sockfd);
        return;
    }

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

void connect_devices()
{
    char ip[20];
    printf("Enter ip (Ex 127.0.0.1): ");
    scanf("%s", ip);
    clear_stdin();
    for (int i = 0; i < 5; i++)
    {

        connect_new_device(ip, 5000 + 100 * i);
    }
}

void list_device()
{
    printf("=========== List devices ============\n");
    int count = 0;
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (devices[i].active == 1)
        {
            printf("%d: %s:%d\n", i, devices[i].ip, devices[i].port);
            count++;
        }
    }

    if (count == 0)
    {
        printf("Not have device connected! ");
        return;
    }

    printf("Enter ID of slot to tranfer device: ");
    int id;
    scanf("%d", &id);
    clear_stdin();

    if (id >= 0 && id < MAX_DEVICES && devices[id].active == 1)
    {
        currentId = id;
        printf("Moved the device to the slot %d\n", id);
    }
    else
    {
        printf("Invalid slot ID!\n");
    }
}

void scan_device()
{
    int sock = devices[currentId].sockfd;
    struct Message msg, res;
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
}

void login_device()
{
    struct Message msg, res;
    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_LOGIN;
    msg.code = 0;

    int target_id;
    char password[32];

    printf("Please enter id want to login: ");
    scanf("%d", &target_id);
    clear_stdin();
    printf("Please enter password: ");
    scanf("%s", password);

    snprintf(msg.payload, sizeof(msg.payload), "%d %s", target_id, password);

    send_all(devices[currentId].sockfd, &msg, sizeof(msg));

    if (recv_all(devices[currentId].sockfd, &res, sizeof(res)) > 0)
    {
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

                printf("[SUCCESS] Login OK! Type: %s\n", recv_type);
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
            printf("[FAILED] Login failed \n");
        }
    }
}

void change_password()
{
    struct Message msg, res;
    memset(&msg, 0, sizeof(msg));
    msg.type = TYPE_CHPASS;
    msg.code = 0;

    char old_password[MAX_PASS_LENGTH];
    char new_password[MAX_PASS_LENGTH];

    printf("Please enter old password : ");
    scanf("%s", old_password);
    clear_stdin();
    printf("Please enter new password : ");
    scanf("%s", new_password);
    clear_stdin();

    snprintf(msg.payload, sizeof(msg.payload), "%d %s %s", devices[currentId].token, old_password, new_password);

    send_all(devices[currentId].sockfd, &msg, sizeof(msg));

    if (recv_all(devices[currentId].sockfd, &res, sizeof(res)) > 0)
    {
        if (res.code == CODE_CHPASS_OK)
        {
            printf("[SUCCESS] Change password OK! %d %s\n", res.code, res.payload);
        }
        else
        {
            printf("[FAILED] Change password failed. %d %s\n", res.code, res.payload);
        }
    }
}

void logout_device()
{
    if (devices[currentId].is_logged_in)
    {
        devices[currentId].is_logged_in = 0;
        devices[currentId].token = 0;
        printf("[SUCCESS] Logout successfully!");
    }
    else
    {
        printf("[FAILED] You are not loggin!");
    }
}

void exit_device()
{
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (devices[i].active)
        {
            close(devices[i].sockfd);
        }
    }
    printf("Exiting...\n");
}

void show_menu()
{
    printf("\n========= MULTI-CONTROLLER =========\n");

    if (currentId != -1 && devices[currentId].active)
    {
        printf("STATUS: Connected | Slot [%d] | IP: %s\n",
               currentId, devices[currentId].ip);

        if (devices[currentId].is_logged_in)
        {
            printf("Device: %s | ID: %d | Token: %d\n",
                   devices[currentId].device_type,
                   devices[currentId].device_id,
                   devices[currentId].token);
        }
        else
        {
            printf("Device: Not logged in\n");
        }
    }
    else
    {
        printf("STATUS: No device selected\n");
    }

    printf("===================================\n");
    printf("1. Connect new device\n");
    printf("2. List devices\n");
    printf("3. Scan device\n");

    if (currentId != -1 && devices[currentId].active)
    {
        printf("-----------------------------------\n");
        printf("4. Login\n");
        printf("5. Logout\n");
        printf("6. Change password\n");

        if (devices[currentId].is_logged_in)
        {
            printf("-----------------------------------\n");
            printf("7. Turn ON device\n");
            printf("8. Turn OFF device\n");

            if (strstr(devices[currentId].device_type, "WATERPUMP"))
            {
                printf("9.  Set pump\n");
                printf("10. Get pump info\n");
                printf("11. Manual pump\n");
            }
            else if (strstr(devices[currentId].device_type, "AERATOR"))
            {
                printf("9.  Set aerator\n");
                printf("10. Get aerator info\n");
                printf("11. Manual aerate\n");
            }
            else if (strstr(devices[currentId].device_type, "FEEDER"))
            {
                printf("9.  Set feeder\n");
                printf("10. Get feeder info\n");
                printf("11. Manual feed\n");
            }
            else if (strstr(devices[currentId].device_type, "PHREGULATOR"))
            {
                printf("9.  Set PH regulator\n");
                printf("10. Get PH info\n");
            }
            else if (strstr(devices[currentId].device_type, "SENSOR"))
            {
                printf("9. Get sensor info\n");
            }
        }
    }

    printf("-----------------------------------\n");
    printf("0. Exit\n");
    printf("===================================\n");
    printf("Select: ");
}

int main()
{
    init_device_list();
    int command;

    while (1)
    {
        show_menu();
        scanf("%d", &command);
        clear_stdin();
        switch (command)
        {
        case 1: // CONNECT NEW DEVICE
        {
            connect_devices();
            break;
        }

        case 2: // LIST DEVICES
        {
            list_device();
            break;
        }

        case 3: // SCAN DEVICE
        {
            scan_device();
            break;
        }

        case 4: // LOGIN
        {
            login_device();
            break;
        }

        case 5: // LOGOUT
        {
            logout_device();
            break;
        }

        case 6: // CHANGE PASSWORD
        {
            change_password();
            break;
        }

        case 7: // TURN ON
        {
            turn_on_device(
                devices[currentId].sockfd,
                devices[currentId].token);
            break;
        }

        case 8: // TURN OFF
        {
            turn_off_device(
                devices[currentId].sockfd,
                devices[currentId].token);
            break;
        }

        case 9:
        {
            if (strstr(devices[currentId].device_type, "WATERPUMP"))
                set_pump_device(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "AERATOR"))
                set_aerator_device(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "FEEDER"))
                set_feeder_device(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "PHREGULATOR"))
                set_ph_regulator_device(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "SENSOR"))
                get_pond_info(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 10:
        {
            if (strstr(devices[currentId].device_type, "WATERPUMP"))
                get_pump_device_info(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "AERATOR"))
                get_aerator_device_info(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "FEEDER"))
                get_feeder_device_info(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "PHREGULATOR"))
                get_ph_regulator_device_info(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 11:
        {
            if (strstr(devices[currentId].device_type, "WATERPUMP"))
                manual_pump(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "AERATOR"))
                manual_aerate(devices[currentId].sockfd, devices[currentId].token);
            else if (strstr(devices[currentId].device_type, "FEEDER"))
                manual_feed(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 0:
        {
            exit_device();
            return 0;
        }

        default:
        {
            printf("[WARNING] Invalid option!\n");
            break;
        }
        }
    }
    return 0;
}