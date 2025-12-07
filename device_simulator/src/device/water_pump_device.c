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
    int active;       // 1 = đang bơm, 0 = tắt
    double flow_rate; // Lưu lượng V (m3/giờ)
    double duration;  // Thời gian bơm T (giờ)
} WaterPumpDevice;

WaterPumpDevice WPD;

void water_pump_handler(int sock, struct Message *msg)
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
        WPD.device_id = 1;
        printf("%d", WPD.device_id);
        break;
    }
}

int main()
{
    return start_device_server(5400, water_pump_handler);
}