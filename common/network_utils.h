#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <sys/socket.h>
#include <unistd.h>

// -- Xu ly truyen dong ---
int recv_line(int sock, char *buffer, int size);
int recv_all(int sock, void *buffer, int size);
void send_line(int sock, char *message);
#endif