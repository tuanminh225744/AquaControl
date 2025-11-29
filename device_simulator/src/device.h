#ifndef DEVICE_H
#define DEVICE_H

#include "../../common/messages.h"


void handle_scan_request(int sockfd, struct Message *req);
void handle_connect_request(int sockfd, struct Message *req);

#endif