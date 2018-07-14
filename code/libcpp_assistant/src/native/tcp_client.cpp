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

#include "tcp_client.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "base/ca_return_code.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

DEFINE_CLASS_NAME(tcp_client);

tcp_client::tcp_client()
    : tcp_base()
{
    ;
}

tcp_client::tcp_client(const char *self_name,
    int max_peer_count/* = net_poller::DEFAULT_CONNECTION_COUNT*/,
    int timeout/* = net_poller::DEFAULT_POLL_TIMEOUT*/)
    : tcp_base(self_name, max_peer_count, timeout)
{
    ;
}

tcp_client::~tcp_client()
{
    ;
}

int tcp_client::connect_server(const char *ip,
    uint16_t port,
    int send_buf_size,
    int recv_buf_size,
    bool is_nonblocking/* = true*/)
{
    int fd = -1;
    struct sockaddr_in server = {0};
    struct sockaddr_in self = {0};
    socklen_t self_len = sizeof(self);
    net_connection *conn = nullptr;
    int ret = CA_RET_GENERAL_FAILURE;
    bool ip_ok = is_valid_ipv4(ip);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ret = -errno;
        cerror("socket() failed\n");
        goto CONNECT_FAILED;
    }

    if (is_nonblocking && (ret = set_nonblocking(fd)) < 0)
    {
        cerror("SetNonblocking() failed\n");
        goto CONNECT_FAILED;
    }

    server.sin_family = AF_INET;
    if (ip_ok)
        inet_pton(AF_INET, ip, &(server.sin_addr.s_addr));
    else
        server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0)
    {
        ret = -errno;
        cdebug("connect() failed temporarily, errno = %d, reason: %s\n", -ret, what(ret).c_str());
        if (EINPROGRESS != -ret)
        {
            cerror("connect() failed\n");
            goto CONNECT_FAILED;
        }

        /*
         * Checks the condition of fd to determine if connect() succeeds.
         * Possibilities are:
         * (1) fd is writable when connection is done and data do not arrive.
         * (2) fd is writable and readable when connection is done and data arrive.
         * (3) fd is writable and readable when an error occurs on it.
         */
        if (!is_writable(fd, m_timeout * 1000))
        {
            cerror("IsWritable() failed\n");
            ret = CA_RET(CONNECTION_NOT_READY);
            goto CONNECT_FAILED;
        }
    }

    conn = create_net_connection(send_buf_size, recv_buf_size);
    if (nullptr == conn)
    {
        cerror("create_net_connection() failed\n");
        ret = CA_RET(MEMORY_ALLOC_FAILED);
        goto CONNECT_FAILED;
    }

    strncpy(conn->self_name, self_name(), sizeof(conn->self_name) - 1);
    getsockname(fd, (struct sockaddr *)&self, &self_len);
    inet_ntop(AF_INET, &(self.sin_addr.s_addr), conn->self_ip, sizeof(conn->self_ip));
    conn->self_port = ntohs(self.sin_port);
    if ((port == conn->self_port)
        && (0 == strncmp(ip, conn->self_ip, IPV4_LEN)))
    {
        cerror("TCP self-connection\n");
        ret = CA_RET(TCP_SELF_CONNECT);
        goto CONNECT_FAILED;
    }

    memcpy(conn->peer_ip, ip, sizeof(conn->peer_ip));
    conn->peer_port = port;

    conn->fd = fd;
    conn->conn_status = CONN_STATUS_CONNECTED;
    conn->is_blocking = !(is_nonblocking);
    conn->is_validated = false;

    if ((ret = m_poller->add_monitored_connection(conn, net_poller::EVENT_READ)) < 0)
    {
        cerror("AddMonitoredConnection() failed\n");
        goto CONNECT_FAILED;
    }

    if ((ret = add_connection(conn)) < 0)
    {
        cerror("AddConnection() failed\n");
        goto CONNECT_FAILED;
    }

    m_connection_type = CONN_TYPE_CLIENT;

    return fd;

CONNECT_FAILED:

    delete_connection(conn);

    if (fd >= 0)
        close(fd);

    return ret;
}

int tcp_client::reconnect_server(const char *ip,
    uint16_t port,
    int send_buf_size,
    int recv_buf_size,
    bool is_nonblocking/* = true*/)
{
    if (!is_valid_ipv4(ip))
        return CA_RET(INVALID_PARAM_VALUE);

    for (connection_map::iterator it = m_peers->begin(); it != m_peers->end(); ++it)
    {
        net_connection *conn = it->second;

        if ((0 == memcmp(ip, conn->peer_ip, sizeof(conn->peer_ip))) &&
            (port == conn->peer_port))
        {
            delete_connection(conn);
            break;
        }
    }

    return connect_server(ip, port, send_buf_size, recv_buf_size, is_nonblocking);
}

int tcp_client::disconnect_all_servers(void)
{
    clear();

    return CA_RET_OK;
}

CA_LIB_NAMESPACE_END
