#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../device_server.h"
#include "../../../common/messages.h"

typedef struct
{
    int device_id;
    int active;       // 1 = đang chạy, 0 = tắt
    double rpm;       // Tốc độ quay C (vòng/phút)
    int schedule[24]; // Lịch chạy theo giờ trong ngày: 1 = bật, 0 = tắt
} EaratorDevice;

void aerator_handler(int sock, struct Message *msg)
{
    switch (msg->type)
    {
    case MSG_SCAN_REQUEST:
        handle_scan_request(sock, msg);
        break;
    case MSG_CONNECT_REQUEST:
        handle_connect_request(sock, msg);
        break;
    case MSG_PUMP_CONTROL:
        printf("control");
        break;
    }
}

int main()
{
    return start_device_server(5000, aerator_handler);
}