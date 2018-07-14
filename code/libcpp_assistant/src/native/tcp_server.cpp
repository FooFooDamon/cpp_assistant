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

#include "tcp_server.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "base/ca_return_code.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

DEFINE_CLASS_NAME(tcp_server);

tcp_server::tcp_server()
    : tcp_base(),
      m_listening_conn(nullptr)
{
    ;
}

tcp_server::tcp_server(const char *self_name,
    int max_peer_count/* = net_poller::DEFAULT_CONNECTION_COUNT*/,
    int timeout/* = net_poller::DEFAULT_POLL_TIMEOUT*/)
    : tcp_base(self_name, max_peer_count, timeout),
      m_listening_conn(nullptr)
{
    ;
}

tcp_server::~tcp_server()
{
    destroy_listening_connection();
    //clear(); // Executed by base class later.
}

CA_REENTRANT bool tcp_server::can_be_listened(const char *ip, const uint16_t port)
{
    int fd = -1;
    int reuse_addr_flag = 1;
    struct sockaddr_in addr = {0};
    bool ip_ok = is_valid_ipv4(ip);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cserror(tcp_server, "socket() failed\n");
        goto CHECK_FAILED;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse_addr_flag, sizeof(int)) < 0)
    {
        cserror(tcp_server, "setsockopt() failed\n");
        goto CHECK_FAILED;
    }

    if (set_nonblocking(fd) < 0)
    {
        cserror(tcp_server, "SetNonblocking() failed\n");
        goto CHECK_FAILED;
    }

    addr.sin_family = AF_INET;
    if (ip_ok)
        inet_pton(AF_INET, ip, &(addr.sin_addr.s_addr));
    else
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
        cswarn(tcp_server, "bind() failed\n");
        goto CHECK_FAILED;
    }

    if (listen(fd, 5) < 0)
    {
        cswarn(tcp_server, "listen() failed\n");
        goto CHECK_FAILED;
    }

    close(fd);

    return true;

CHECK_FAILED:

    if (fd >= 0)
        close(fd);

    return false;
}

int tcp_server::start(const char *ip, uint16_t port)
{
    int fd = -1;
    int reuse_addr_flag = 1;
    struct sockaddr_in addr = {0};
    int ret = CA_RET_GENERAL_FAILURE;
    bool ip_ok = is_valid_ipv4(ip);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ret = -errno;
        cerror("socket() failed\n");
        goto START_FAILED;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse_addr_flag, sizeof(int)) < 0)
    {
        ret = -errno;
        cerror("setsockopt() failed\n");
        goto START_FAILED;
    }

    if ((ret = set_nonblocking(fd)) < 0)
    {
        cerror("SetNonblocking() failed\n");
        goto START_FAILED;
    }

    addr.sin_family = AF_INET;
    if (ip_ok)
        inet_pton(AF_INET, ip, &(addr.sin_addr.s_addr));
    else
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
        ret = -errno;
        cerror("bind() failed\n");
        goto START_FAILED;
    }

    if (listen(fd, 5) < 0)
    {
        ret = -errno;
        cerror("listen() failed\n");
        goto START_FAILED;
    }

    m_connection_type = CONN_TYPE_SERVER;

    m_listening_conn = create_net_connection(0, 0);
    if (nullptr == m_listening_conn)
    {
        cerror("create_net_connection() failed\n");
        ret = CA_RET(MEMORY_ALLOC_FAILED);
        goto START_FAILED;
    }

    m_listening_conn->fd = fd;
    memcpy(m_listening_conn->self_ip, ip, sizeof(m_listening_conn->self_ip));
    m_listening_conn->self_ip[sizeof(m_listening_conn->self_ip) - 1] = '\0';
    m_listening_conn->self_port = port;
    m_listening_conn->conn_status = CONN_STATUS_LISTENING;
    m_listening_conn->is_blocking = false;

    if ((ret = m_poller->add_monitored_connection(m_listening_conn, net_poller::EVENT_READ)) < 0)
    {
        cerror("AddMonitoredConnection() failed\n");
        goto START_FAILED;
    }

    return fd;

START_FAILED:

    m_connection_type = CONN_TYPE_NONE;
    if (fd >= 0)
        destroy_listening_connection();

    return ret;
}

int tcp_server::end(void)
{
    clear();

    return CA_RET_OK;
}

int tcp_server::accept_new_connection(int send_buf_size, int recv_buf_size, bool is_nonblocking/* = true*/)
{
    if (nullptr == m_listening_conn)
        return CA_RET(RESOURCE_NOT_AVAILABLE);

    struct sockaddr_in client = {0};
    uint32_t len = sizeof(client);
    struct sockaddr_in self = {0};
    socklen_t self_len = sizeof(self);
    int accfd = -1;
    net_connection *conn = nullptr;
    int ret = CA_RET_GENERAL_FAILURE;

    if ((accfd = accept(m_listening_conn->fd, (struct sockaddr *)&client, &len)) < 0)
    {
        ret = -errno;
        goto ACCEPT_FAILED;
    }
    cdebug("accept() ok, fd = %d\n", accfd);

    conn = create_net_connection(send_buf_size, recv_buf_size);
    if (nullptr == conn)
    {
        cerror("create_net_connection() failed\n");
        ret = CA_RET(MEMORY_ALLOC_FAILED);
        goto ACCEPT_FAILED;
    }

    conn->fd = accfd;
    getsockname(accfd, (struct sockaddr *)&self, &self_len);
    inet_ntop(AF_INET, &(self.sin_addr.s_addr), conn->self_ip, sizeof(conn->self_ip));
    conn->self_port = ntohs(self.sin_port);
    inet_ntop(AF_INET, &(client.sin_addr.s_addr), conn->peer_ip, sizeof(conn->peer_ip));
    conn->peer_port = ntohs(client.sin_port);
    conn->conn_status = CONN_STATUS_CONNECTED;
    conn->is_blocking = !(is_nonblocking);

    if ((ret = m_poller->add_monitored_connection(conn, net_poller::EVENT_READ)) < 0)
    {
        cerror("AddMonitoredConnection() failed\n");
        goto ACCEPT_FAILED;
    }

    if ((ret = add_connection(conn)) < 0)
    {
        cerror("AddConnection() failed\n");
        goto ACCEPT_FAILED;
    }

    if (is_nonblocking && (ret = set_nonblocking(accfd)) < 0)
    {
        cerror("SetNonblocking(%d) failed: %s\n", accfd, strerror(-ret));
        goto ACCEPT_FAILED;
    }

    return accfd;

ACCEPT_FAILED:

    delete_connection(conn);

    if (accfd >= 0)
        close(accfd);

    return ret;
}

void tcp_server::destroy_listening_connection(void)
{
    if (nullptr == m_listening_conn)
        return;

    if (nullptr != m_poller)
        m_poller->delete_monitored_connection(m_listening_conn);

    cdebug("listening connection with fd = %d will be released\n", m_listening_conn->fd);
    destroy_net_connection(&m_listening_conn);
}

CA_LIB_NAMESPACE_END
