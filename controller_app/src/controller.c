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

    send_line(sockfd, "HELLO");

    struct Message res;
    memset(&res, 0, sizeof(res));
    if (recv_all(sockfd, &res, sizeof(res)) <= 0)
    {
        printf("[FAILED] Failed to receive response\n");
        close(sockfd);
        return;
    }

    if (res.code == CODE_CONNECT_OK)
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
    for (int i = 0; i < 5; i++)
    {

        connect_new_device("127.0.0.1", 5000 + 100 * i);
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
            printf("%d %s:%d\n", i, devices[i].ip, devices[i].port);
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

int main()
{
    init_device_list();
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
                printf("Device: Not loggin !\n");
            }
        }
        else
        {
            printf("STATUS: No Device selected \n");
        }
        printf("=============================\n");
        printf("1. Connect new device\n");
        printf("2. List of devices\n");
        printf("=============================\n");
        if (currentId != -1 && devices[currentId].active)
        {
            printf("3. Scan \n");
            printf("4. Login \n");

            if (devices[currentId].is_logged_in == 1)
            {
                printf("=============================\n");
                printf("5. Turn on \n");
                printf("6. Turn off \n");
                if (strstr(devices[currentId].device_type, "WATERPUMP") != NULL)
                {
                    printf("7. Set water pump device \n");
                    printf("8. Get water pump device info \n");
                    printf("17. Manual pump \n");
                }
                else if (strstr(devices[currentId].device_type, "AERATOR") != NULL)
                {
                    printf("9. Set aerator device \n");
                    printf("10. Get aerator device info \n");
                    printf("18. Manual aerate \n");
                }
                else if (strstr(devices[currentId].device_type, "FEEDER") != NULL)
                {
                    printf("11. Set feeder device \n");
                    printf("12. Get feeder device info\n");
                    printf("19. Manual feed \n");
                }
                else if (strstr(devices[currentId].device_type, "PHREGULATOR") != NULL)
                {
                    printf("13. Set PH regulator device \n");
                    printf("14. Get PH regulator device info\n");
                }
                else if (strstr(devices[currentId].device_type, "SENSOR") != NULL)
                {
                    printf("15. Get sensor device info\n");
                }
                printf("16. Log out\n");
                printf("20. Change password\n");
            }
        }
        printf("0. Exit\n");
        printf("======================================\n");
        printf("Select: ");

        if (scanf("%d", &comman) != 1)
        {
            clear_stdin();
            continue;
        }
        clear_stdin();

        switch (comman)
        {
        case 1: // CONNECT NEW DEVICE
        {
            // connect_new_device();
            connect_devices();
            break;
        }
        case 2: // LIST DEVICE
        {
            list_device();
            break;
        }
        case 3: // SCAN
        {
            scan_device();
            break;
        }
        case 4: // LOGIN
        {
            login_device();
            break;
        }
        case 5: // TURN ON
        {
            turn_on_device(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 6: // TURN OFF
        {
            turn_off_device(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 7: // SET PUMP DEVICE
        {

            set_pump_device(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 8: // GET PUMP DEVICE INFO
        {
            get_pump_device_info(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 17: // MANUAL PUMP
        {
            manual_pump(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 9: // SET AERATOR DEVICE
        {

            set_aerator_device(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 10: // GET AERATOR DEVICE INFO
        {
            get_aerator_device_info(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 18: // MANUAL AERATE
        {
            manual_aerate(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 11: // SET FEEDER DEVICE
        {

            set_feeder_device(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 12: // GET FEEDER DEVICE INFO
        {
            get_feeder_device_info(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 19: // MANUAL FEED
        {
            manual_feed(devices[currentId].sockfd, devices[currentId].token);
            break;
        }

        case 13: // SET PH REGULATOR DEVICE
        {

            set_ph_regulator_device(devices[currentId].sockfd, devices[currentId].token);
            break;
        }
        case 14: // GET PH REGULATOR DEVICE INFO
        {
            get_ph_regulator_device_info(devices[currentId].sockfd, devices[currentId].token);
            break;
        }
        case 15: // GET SENSOR DEVICE INFO
        {
            get_sensor_device_info(devices[currentId].sockfd, devices[currentId].token);
            break;
        }
        case 16: // LOGOUT
        {
            logout_device();
            break;
        }
        case 20: // CHANGE PASSWORD
        {
            change_password();
            break;
        }
        case 0: // EXIT
        {
            exit_device();
            return 0;
        }
        default:
        {
            printf("[WARNING] Invalid Option!\n");
            break;
        }
        }
    }
    return 0;
}