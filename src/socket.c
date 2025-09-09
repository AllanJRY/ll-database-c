#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include <stdio.h>
#include <string.h>

#include <socket.h>

#define MAX_CLIENTS   256

typedef enum State {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED,
    STATE_HELLO,
    STATE_MSG,
    STATE_GOODBYE,
} State;

typedef struct Client_State {
    int   fd;
    State state;
    char  buffer[BUFF_SIZE];
} Client_State;

Client_State client_states[MAX_CLIENTS];

void clients_init(void) {
    for(int i = 0; i < MAX_CLIENTS; i += 1) {
        client_states[i].fd = -1; // -1 indicates a free slot
        client_states[i].state = STATE_NEW;
        memset(&client_states[i].buffer, '\0', BUFF_SIZE);
    }
}

int clients_find_free_slot_idx(void) {
    for(int i = 0; i < MAX_CLIENTS; i += 1) {
        if (client_states[i].fd == -1) {
            return i;
        }
    }

    return -1; // No free slot found
}

int clients_find_slot_by_fd(int fd) {
    for(int i = 0; i < MAX_CLIENTS; i += 1) {
        if (client_states[i].fd == fd) {
            return i;
        }
    }

    return -1; // not found
}

void client_send_error(Client_State* client, Proto_Header* header) {
    header->type = htonl(MSG_ERROR);
    header->len  = htons(0);

    write(client->fd, client->buffer, sizeof(Proto_Header));
}

// handle client exchanges with a finite state machine
void client_handle_fsm(Db_File_Header* db_file_header, Employee* employees, Client_State* client) {
    Proto_Header* header = (Proto_Header*) client->buffer;

    header->type = ntohl(header->type);
    header->len  = ntohs(header->len);

    if (client->state == STATE_HELLO) {
        if (header->type != MSG_HELLO_REQ || header->len != 1) {
            printf("didn't get MSG_HELLO in HELLO state\n");
            // send error msg
        }

        Proto_Hello_Req* hello_req = (Proto_Hello_Req*) &header[1];
        hello_req->proto_version = ntohs(hello_req->proto_version);
        if (hello_req->proto_version != PROTO_VERSION) {
            printf("protocol version mismatch\n");
            client_send_error(client, header);
            return;
        }


        header->type                 = htonl(MSG_HELLO_RESP);
        header->len                  = htons(1);
        Proto_Hello_Resp* hello_resp = (Proto_Hello_Resp*) hello_req;
        hello_resp->proto_version    = htons(hello_resp->proto_version);
        write(client->fd, client->buffer, sizeof(Proto_Header) + sizeof(Proto_Hello_Resp));

        client->state = STATE_MSG;
        printf("client upgraded to STATE_MSG\n");
    }

    if (client->state == STATE_MSG) {
    }

}

bool socket_run(u16 socket_port, Db_File_Header* db_file_header, Employee* employees) {
    struct sockaddr_in server_info = {0};
    server_info.sin_family         = AF_INET;
    server_info.sin_addr.s_addr    = INADDR_ANY;
    server_info.sin_port           = htons(socket_port);
    int opt                        = 1;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return false;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return false;
    }

    if (bind(server_fd, (struct sockaddr*) &server_info, sizeof(server_info)) == -1) {
        perror("bind");
        close(server_fd);
        return false;
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return false;
    }

    printf("server listening on port %d\n", socket_port);

    clients_init();

    struct sockaddr_in client_info = {0};
    socklen_t client_size          = sizeof(client_info);
    int tracked_fd_count           = 1;
    int next_free_slot             = 0;

    struct pollfd fds[MAX_CLIENTS + 1];
    memset(fds, 0, sizeof(fds));
    fds[0].fd        = server_fd;
    fds[0].events    = POLLIN;
    tracked_fd_count = 1;

    while(1) {

        // add active connections to poll events
        int ii = 1;
        for (int i = 0; i < MAX_CLIENTS; i += 1) {
            if (client_states[i].fd != -1) {
                fds[ii].fd      = client_states[i].fd; // offset by 1 for server_fd
                fds[ii].events  = POLLIN;
                ii             += 1;
            }
        }

        // wait for an event on one of the sockets
        int n_events = poll(fds, tracked_fd_count, -1); // -1 means no timeout
        if (n_events == -1) {
            perror("poll");
            close(server_fd);
            return false;
        }

        // check for new connections
        if(fds[0].revents & POLLIN) {
            int client_fd = accept(server_fd, (struct sockaddr*) &client_info, &client_size);
            if (client_fd == -1) {
                perror("accept");
                continue;
            }

            printf("new connection from %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

            // find free slot for a new connection
            next_free_slot = clients_find_free_slot_idx();
            if (next_free_slot == -1) {
                printf("Server full: closing new connection\n");
                close(client_fd);
            } else {
                client_states[next_free_slot].fd     = client_fd;
                client_states[next_free_slot].state  = STATE_HELLO;
                tracked_fd_count                    += 1;
                printf("slot %d has fd %d\n", next_free_slot, client_states[next_free_slot].fd);
            }

            n_events -= 1;
        }

        // Check each client for read/write activity
        for (int i = 1; i <= tracked_fd_count && n_events > 0; i += 1) { // start from 1 to skip the server_fd
            if (fds[i].revents & POLLIN) {
                n_events -= 1;

                int fd = fds[i].fd;
                int slot = clients_find_slot_by_fd(fd);
                ssize_t bytes_read = read(fd, &client_states[slot].buffer, sizeof(client_states[slot].buffer));
                if (bytes_read <= 0) {
                    // connection closed or error
                    close(fd);
                    if (slot == -1) {
                        printf("tried to close fd that doesnt exists?\n");
                    } else {
                        client_states[slot].fd    = -1; // free up the slot
                        client_states[slot].state = STATE_DISCONNECTED;
                        printf("client disconnected or error\n");
                        tracked_fd_count -= 1;
                    }
                } else {
                    client_handle_fsm(db_file_header, employees, &client_states[slot]);
                }
            }
        }
    }

    return true;
}
