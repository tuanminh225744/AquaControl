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
    int active;
    double w_food;    // Lượng thức ăn Wf (kg) mỗi lần
    int schedule[24]; // Lịch cho ăn: 1 = cho ăn, 0 = không
} FeederDevice;

void feeder_handler(int sock, struct Message *msg)
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
    return start_device_server(5100, feeder_handler);
}