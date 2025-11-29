#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <sys/socket.h>
#include <unistd.h>


// -- Xu ly truyen dong ---
int recv_line(int sock, char *buffer, int size)
{
    int i = 0;
    char c = '\0';
    while (i < size - 1)
    {
        int n = recv(sock, &c, 1, 0);
        if (n <= 0)
        {
            return -1;
        }
        if (c == '\r')
        {
            char next;
            if (recv(sock, &next, 1, MSG_PEEK) > 0 && next == '\n')
            {
                recv(sock, &next, 1, 0); // Đọc bỏ \n
                break;
            }
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i;
}
// 2. HÀM ĐỌC ĐỦ BYTES
// Đảm bảo nhận đủ 100% kích thước Struct
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

// Tự động chèn \r\n vào cuối chuỗi message
void send_line(int sock, char *message)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s\r\n", message);
    send(sock, buffer, strlen(buffer), 0);
}
#endif