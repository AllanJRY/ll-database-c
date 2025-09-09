#ifndef SOCKET_H

#include "common.h"

#define BUFF_SIZE     4096
#define PROTO_VERSION 1

typedef enum Proto_Msg_Type {
    MSG_HELLO_REQ,
    MSG_HELLO_RESP,
    MSG_EMPLOYEE_LIST_REQ,
    MSG_EMPLOYEE_LIST_RESP,
    MSG_EMPLOYEE_ADD_REQ,
    MSG_EMPLOYEE_ADD_RESP,
    MSG_EMPLOYEE_DEL_REQ,
    MSG_EMPLOYEE_DEL_RESP,
    MSG_EMPLOYEE_UPT_REQ,
    MSG_EMPLOYEE_UPT_RESP,
} Proto_Msg_Type;

typedef struct Proto_Header {
    Proto_Msg_Type type;
    u16            len; // element count, maybe change the name
} Proto_Header;

typedef struct Proto_Hello_Req {
    u16 proto_version;
} Proto_Hello_Req;

typedef struct Proto_Hello_Resp {
    u16 proto_version;
} Proto_Hello_Resp;

#define SOCKET_H
#endif
