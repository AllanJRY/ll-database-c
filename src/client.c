#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>

#include "socket.h"

void send_hello(int fd) {
    char buf[BUFF_SIZE] = {0};

    Proto_Header* header = (Proto_Header*) buf;
    header->type         = ntohl(MSG_HELLO_REQ);
    header->len          = ntohs(1);

    // int* data = (int*) &header[1];
    // *data     = ntohs(PROTO_VERSION);

    write(fd, buf, sizeof(Proto_Header));

    printf("connected to database (proto_version=%d)\n", PROTO_VERSION);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <ip of the host>\n", argv[0]);
        return 0;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_info = {0};
    server_info.sin_family         = AF_INET;
    server_info.sin_addr.s_addr    = inet_addr(argv[1]);
    server_info.sin_port           = htons(8080);

    if (connect(sock_fd, (struct sockaddr*) &server_info, sizeof(server_info)) == -1) {
        perror("connect");
        close(sock_fd);
        return -1;
    }

    send_hello(sock_fd);

    close(sock_fd);

    return 0;
}
