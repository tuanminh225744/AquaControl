#include "network_utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// 1. ĐỊNH NGHĨA HÀM ĐỌC TỪNG DÒNG
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
            // Kiểm tra xem ký tự tiếp theo có phải là \n không (không đọc nó ra khỏi buffer)
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

// 2. ĐỊNH NGHĨA HÀM ĐỌC ĐỦ BYTES
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

// 3. ĐỊNH NGHĨA HÀM GỬI TỪNG DÒNG
void send_line(int sock, char *message)
{
    char buffer[1024];
    // Tự động chèn \r\n vào cuối chuỗi message
    snprintf(buffer, sizeof(buffer), "%s\r\n", message);
    send(sock, buffer, strlen(buffer), 0);
}