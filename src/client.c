#ifdef __unix__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#endif // __unix__

#include <stdio.h>
#include <string.h>

#include "common.h"

#include "employee.c" // TODO: make a header for employee.c
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

    printf("connected to database (proto_version=%d)\n\n", PROTO_VERSION);
    return true;
}

bool send_employee_creation(int fd, char* add_str) {
    if (add_str == NULL) {
        printf("send_employee_creation: add string is null\n");
        return false;
    }

    char buf[BUFF_SIZE] = {0};

    Proto_Header* header = (Proto_Header*) buf;
    header->type         = htonl(MSG_EMPLOYEE_ADD_REQ);
    header->len          = htons(1);

    Proto_Employee_Add_Req* employee_add_req = (Proto_Employee_Add_Req*) &header[1];
    memcpy(employee_add_req->add_str, add_str, sizeof(employee_add_req->add_str));

    write(fd, buf, sizeof(Proto_Header) + sizeof(employee_add_req->add_str));

    read(fd, buf, sizeof(buf));

    header->type = ntohl(header->type);
    header->len  = ntohs(header->len);

    if (header->type == MSG_ERROR) {
        printf("send_employee_creation: server error while trying to add employee\n");
        return false;
    }

    Proto_Employee_Add_Resp* employee_add_resp = (Proto_Employee_Add_Resp*) &header[1];
    employee_add_resp->new_employee_idx        = ntohl(employee_add_resp->new_employee_idx);

    printf("employee added (idx=%d)\n", employee_add_resp->new_employee_idx);
    return true;
}

bool send_employee_list(int fd) {
    char buf[BUFF_SIZE] = {0};

    Proto_Header* header = (Proto_Header*) buf;
    header->type         = htonl(MSG_EMPLOYEE_LIST_REQ);
    header->len          = htons(0);

    write(fd, buf, sizeof(Proto_Header));

    // read the header, to get the number of employees that has to be handled
    read(fd, buf, sizeof(buf));

    header->type = ntohl(header->type);
    header->len  = ntohs(header->len);

    if (header->type == MSG_ERROR) {
        printf("send_employee_list: server error while trying to list employees\n");
        return false;
    }

    // TODO: check if header->type == MSG_EMPLOYEE_LIST
    
    Employee* employee = (Employee*) &header[1];

    // read the employees, they are received one at a time in the socket buffer
    for (int i = 0; i < header->len; i += 1) {
        read(fd, employee, sizeof(Employee));
        employee->hours = ntohl(employee->hours);
        printf("#%d\t name=%s\n\t address=%s\n\t hours=%d\n\n", i, employee->name, employee->address, employee->hours);
    }

    return true;
}

int main(int argc, char* argv[]) {
    char* server_ip = NULL;
    u16 server_port = 0;

    char* add_str = NULL;
    bool list     = false;

    for (int arg_idx = 1; arg_idx < argc; arg_idx += 1) {
        char* flag = argv[arg_idx];

        if (flag[0] != '-') {
            printf("Invalid option '%s'\n", flag);
            return -1;
        }

        switch(flag[1]) {
            case 'a': {
                arg_idx += 1;
                add_str  = argv[arg_idx];
                break;
            }
            case 'h': {
                arg_idx += 1;
                server_ip = argv[arg_idx];
                break;
            }
            case 'l': {
                list = true;
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

    if (add_str != NULL) {
        if (!send_employee_creation(sock_fd, add_str)) {
            printf("error occured while trying to create employee\n");
        }
    }

    if (list) {
        if (!send_employee_list(sock_fd)) {
            printf("error occured while trying to list employees\n");
        }
    }

    close(sock_fd);

    return 0;
}
