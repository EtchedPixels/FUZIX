/*
   Echoping - A UDP test client
   Sends some text to a UDP echo server, waits for reply, prints it.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd;
    char buf[1024];
    int x;
    int port = 7;
    const char *message = "This is a test!";
    socklen_t addr_len = sizeof(addr);

    if (argc < 2 || argc > 4)
    {
        fprintf(stderr, "Usage: %s <ip address> [port] [message]\n", argv[0]);
        exit(1);
    }

    if (argc >= 3)
    {
        port = atoi(argv[2]);
    }

    if (argc == 4)
    {
        message = argv[3];
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket()");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        exit(1);
    }

    printf("Sending to %s:%d\n", argv[1], port);
    x = sendto(fd, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (x < 0)
    {
        perror("sendto");
        close(fd);
        exit(1);
    }

    printf("Sent message: %s\n", message);

    /* Need a timeout here */
    x = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&addr, &addr_len);
    if (x >= 0)
    {
        buf[x] = '\0';
        printf("Received %d bytes: >%s<\n", x, buf);
    }
    else
    {
        perror("recvfrom");
        printf("No response received.\n");
    }

    close(fd);
    return 0;
}
