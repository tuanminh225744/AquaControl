#include "network_utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int recv_all(int sock, void *buffer, int size)
{
    int total = 0;
    int bytes_left = size;
    char *ptr = (char *)buffer;

    while (total < size)
    {
        int n = recv(sock, ptr + total, bytes_left, 0);
        if (n <= 0)
        {
            return -1;
        }
        total += n;
        bytes_left -= n;
    }
    return total;
}

int send_all(int sock, void *buffer, int size)
{
    int total = 0;
    int bytes_left = size;
    char *ptr = (char *)buffer;

    while (total < size)
    {
        int n = send(sock, ptr + total, bytes_left, 0);
        if (n == -1)
        {
            return -1;
        }
        total += n;
        bytes_left -= n;
    }
    return total;
}
