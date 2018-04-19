/*
 * Copyright (c) 2018, Wen Xiongchang <udc577 at 126 dot com>
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 * not claim that you wrote the original software. If you use this
 * software in a product, an acknowledgment in the product documentation
 * would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

// NOTE: The original author also uses (short/code) names listed below,
//       for convenience or for a certain purpose, at different places:
//       wenxiongchang, wxc, Damon Wen, udc577

/*
 * net_common.h
 *
 *  Created on: 2016-10-28
 *      Author: wenxiongchang
 * Description: Public stuff for network operations.
 */

#ifndef __CPP_ASSISTANT_NET_COMMON_H__
#define __CPP_ASSISTANT_NET_COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>

#include <map>

#include "base/ca_inner_necessities.h"

CA_LIB_NAMESPACE_BEGIN

enum enum_connection_status
{
    CONN_STATUS_CONNECTING    = 0x00000001,
    CONN_STATUS_CONNECTED     = 0x00000002,
    CONN_STATUS_DISCONNECTING = 0x00000004,
    CONN_STATUS_DISCONNECTED  = 0x00000008,
    CONN_STATUS_BROKEN        = 0x00000010,
    CONN_STATUS_LISTENING     = 0x00000020,
    CONN_STATUS_LISTENED      = 0x00000040,
    CONN_STATUS_BLOCKED       = 0x00000080
};

CA_REENTRANT const char *desc_of_connection_status(int src_value);

enum enum_connection_type
{
    CONN_TYPE_MIN           = 1,

    CONN_TYPE_NONE          = 1,
    CONN_TYPE_SERVER        = 2,
    CONN_TYPE_CLIENT        = 3,
    CONN_TYPE_SAME_PEER     = 4,

    CONN_TYPE_MAX           = 4
};

CA_REENTRANT const char *desc_of_connection_type(int src_value);

#define INVALID_IP      "0.0.0.0"
#define NOT_A_SERVER     "NOT_A_SERVER"

enum
{
    INVALID_PORT        = 0,
    INVALID_SOCK_FD     = -1
};

enum
{
    MAX_CONNECTION_NAME_LEN = 127,
    IPV4_LEN = 16, // Maximum 15 bytes + '\0'
    IPV6_LEN = 129 // Maximum 128 bytes + '\0'
};

class sequential_buffer;
typedef sequential_buffer buffer;

typedef struct net_connection
{
    int fd;
    char self_ip[IPV4_LEN];
    uint16_t self_port;
    char self_name[MAX_CONNECTION_NAME_LEN + 1];
    char peer_ip[IPV4_LEN];
    uint16_t peer_port;
    char peer_name[MAX_CONNECTION_NAME_LEN + 1];
    int conn_status; // connection status defined by enum ConnectionStatus
    bool is_blocking;
    bool is_validated;
    buffer *send_buf;
    buffer *recv_buf;
    // last operation time, including but not limited to:
    // connected time, heart-beat time, send time, receive time
    int64_t last_op_time;
    void *owner;
}net_connection;
typedef net_connection net_conn;

CA_REENTRANT net_conn *create_net_connection(int send_buf_size, int recv_buf_size);
CA_REENTRANT void destroy_net_connection(net_conn **conn);

CA_REENTRANT bool is_valid_ipv4(const char *ip);

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_NET_COMMON_H__

