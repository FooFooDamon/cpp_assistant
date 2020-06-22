/*
 * Copyright (c) 2018-2020, Wen Xiongchang <udc577 at 126 dot com>
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

#include "net_common.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "private/debug.h"
#include "sequential_buffer.h"

CA_LIB_NAMESPACE_BEGIN

CA_REENTRANT const char *desc_of_connection_status(int src_value)
{
    switch(src_value)
    {
    case CONN_STATUS_CONNECTING:
        return "connection status: connecting";

    case CONN_STATUS_CONNECTED:
        return "connection status: connected";

    case CONN_STATUS_DISCONNECTING:
        return "connection status: disconnecting";

    case CONN_STATUS_DISCONNECTED:
        return "connection status: disconnected";

    case CONN_STATUS_BROKEN:
        return "connection status: broken";

    case CONN_STATUS_LISTENING:
        return "connection status: listening";

    case CONN_STATUS_LISTENED:
        return "connection status: listened";

    case CONN_STATUS_BLOCKED:
        return "connection status: blocked";

    default:
        return "unknown connection status";
    }
}

CA_REENTRANT const char *desc_of_connection_type(int src_value)
{
    if (src_value < CONN_TYPE_MIN || src_value > CONN_TYPE_MAX)
        return nullptr;

    const char *desc[] = {
        "connection type: none",
        "connection type: server",
        "connection type: client",
        "connection type: peer"
    };

    return desc[src_value - 1];
}

static void init_net_connection(net_conn &conn)
{
    conn.fd = INVALID_SOCK_FD;
    memset(conn.self_ip, 0, sizeof(conn.self_ip));
    conn.self_port = INVALID_PORT;
    memset(conn.self_name, 0, sizeof(conn.self_name));
    memset(conn.peer_ip, 0, sizeof(conn.peer_ip));
    conn.peer_port = INVALID_PORT;
    memset(conn.peer_name, 0, sizeof(conn.peer_name));
    conn.conn_status = CONN_STATUS_DISCONNECTED;
    conn.is_blocking = true;
    conn.is_validated = false;
    conn.send_buf = nullptr;
    conn.recv_buf = nullptr;
    conn.last_op_time = 0;
    conn.owner = nullptr;
}

CA_REENTRANT net_conn *create_net_connection(int send_buf_size, int recv_buf_size)
{
    net_conn *conn = (net_conn *)malloc(sizeof(net_conn));
    bool needs_send_buf = (send_buf_size > 0), needs_recv_buf = (recv_buf_size > 0);
    bool send_buf_ok = false, recv_buf_ok = false;

    if (nullptr == conn)
    {
        nserror(::, "malloc() for new connection node failed\n");
        goto CREATE_FAILED;
    }
    init_net_connection(*conn);

    try
    {
        conn->send_buf = needs_send_buf ? (new buffer(send_buf_size)) : nullptr;
        conn->recv_buf = needs_recv_buf ? (new buffer(recv_buf_size)) : nullptr;
    }
    catch (std::bad_alloc& e)
    {
        nserror(::, "exception caught during new() for connection buffer: %s\n", e.what());
        goto CREATE_FAILED;
    }

    send_buf_ok = (!needs_send_buf) ? true
        : (nullptr != conn->send_buf && nullptr != conn->send_buf->data());
    recv_buf_ok = (!needs_recv_buf) ? true
        : (nullptr != conn->recv_buf && nullptr != conn->recv_buf->data());
    if (!send_buf_ok || !recv_buf_ok)
    {
        nserror(::, "new() for connection buffer failed\n");
        goto CREATE_FAILED;
    }

    return conn;

CREATE_FAILED:

    if (nullptr != conn)
    {
        if (nullptr != conn->send_buf)
        {
            delete(conn->send_buf);
            conn->send_buf = nullptr;
        }
        if (nullptr != conn->recv_buf)
        {
            delete(conn->recv_buf);
            conn->recv_buf = nullptr;
        }
        free(conn);
        conn = nullptr;
    }

    return nullptr;
}

CA_REENTRANT void destroy_net_connection(net_conn **conn)
{
    if ((net_conn **)nullptr == conn)
        return;

    if (nullptr != *conn)
    {
        if ((*conn)->fd >= 0)
            close((*conn)->fd);

        if (nullptr != (*conn)->send_buf)
        {
            delete((*conn)->send_buf);
            (*conn)->send_buf = nullptr;
        }

        if (nullptr != (*conn)->recv_buf)
        {
            delete((*conn)->recv_buf);
            (*conn)->recv_buf = nullptr;
        }

        free(*conn);
        *conn = nullptr;
    }
}

CA_REENTRANT bool is_valid_ipv4(const char *ip)
{
    if (nullptr == ip)
        return false;

    uint32_t addr_num_value = 0;

    if (inet_pton(AF_INET, ip, &addr_num_value) <= 0)
        return false;

    return true;
}

CA_LIB_NAMESPACE_END
