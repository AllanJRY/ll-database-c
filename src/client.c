#ifdef __unix__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#endif // __unix__

#include <stdio.h>

#include "common.h"
#include "socket.h"

void print_usage(char* bin_name) {
    printf("Usage: %s -h <server ip> -p <server port>\n", bin_name);
    printf("\t -h -- (required) ip to the database server\n");
    printf("\t -p -- (required) port to the database server\n");
}

bool send_hello(int fd) {
    char buf[BUFF_SIZE] = {0};

    Proto_Header* header = (Proto_Header*) buf;
    header->type         = htonl(MSG_HELLO_REQ);
    header->len          = htons(1);

    Proto_Hello_Req* hello_msg_req = (Proto_Hello_Req*) &header[1];
    hello_msg_req->proto_version   = htons(PROTO_VERSION);

    // write the hello req msg
    write(fd, buf, sizeof(Proto_Header) + sizeof(Proto_Hello_Req));

    // read the hello resp msg
    read(fd, buf, sizeof(buf));

    header->type = ntohl(header->type);
    header->len  = ntohs(header->len);

    if (header->type == MSG_ERROR) {
        printf("protocol mismatch\n");
        return false;
    }

    printf("connected to database (proto_version=%d)\n", PROTO_VERSION);
    return true;
}

int main(int argc, char* argv[]) {
    char* server_ip = NULL;
    u16 server_port = 0;


    for (int arg_idx = 1; arg_idx < argc; arg_idx += 1) {
        char* flag = argv[arg_idx];

        if (flag[0] != '-') {
            printf("Invalid option '%s'\n", flag);
            return -1;
        }

        switch(flag[1]) {
            case 'h': {
                arg_idx += 1;
                server_ip = argv[arg_idx];
                break;
            }
            case 'p': {
                arg_idx += 1;
                server_port = atoi(argv[arg_idx]);
                break;
            }
            default: {
                printf("Unknown option '%s'\n", flag);
            }
        }
    }

    if (server_ip == NULL) {
        printf("a server ip must be given\n");
        print_usage(argv[0]);
        return -1;
    }

    if (server_port == 0) {
        printf("a server port must be given\n");
        print_usage(argv[0]);
        return -1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_info = {0};
    server_info.sin_family         = AF_INET;
    server_info.sin_addr.s_addr    = inet_addr(server_ip);
    server_info.sin_port           = htons(server_port);

    if (connect(sock_fd, (struct sockaddr*) &server_info, sizeof(server_info)) == -1) {
        perror("connect");
        close(sock_fd);
        return -1;
    }

    if(!send_hello(sock_fd)) {
        close(sock_fd);
        return -1;
    }

    close(sock_fd);

    return 0;
}
