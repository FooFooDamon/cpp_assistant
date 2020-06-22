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

/*
 * tcp_server.h
 *
 *  Created on: 2016-11-01
 *      Author: wenxiongchang
 * Description: TCP server that can accept and manage multiple clients.
 */

#ifndef __CPP_ASSISTANT_TCP_SERVER_H__
#define __CPP_ASSISTANT_TCP_SERVER_H__

#include <stdint.h>

#include "tcp_base.h"

CA_LIB_NAMESPACE_BEGIN

class tcp_server : public tcp_base
{
/* ===================================
 * constructors:
 * =================================== */
public:
    tcp_server();
    tcp_server(const char *self_name,
        int max_peer_count = net_poller::DEFAULT_CONNECTION_COUNT,
        int timeout = net_poller::DEFAULT_POLL_TIMEOUT);

/* ===================================
 * copy control:
 * =================================== */
private:

/* ===================================
 * destructor:
 * =================================== */
public:
    ~tcp_server();

/* ===================================
 * abilities:
 * =================================== */
public:
    static CA_REENTRANT bool can_be_listened(const char *ip, const uint16_t port);

    // Starts listening on the address of @ip:@port.
    // Returns a socket fd for listening on success, or a negative number on failure.
    int start(const char *ip, uint16_t port);

    int end(void);

    // Accepts a new client connection and allocates @send_buf_size bytes for this connection's
    // send buffer and @recv_buf_size for its receive buffer if the accept operation is OK.
    // Communication with this connection will be nonblocking if @is_nonblocking is true.
    // Returns a socket fd for later communication with this new connection on success,
    // or a negative number on failure.
    int accept_new_connection(int send_buf_size, int recv_buf_size, bool is_nonblocking = true);

    inline int shutdown_client(net_connection *conn)
    {
        return delete_connection(conn);
    }

    inline int shutdown_client(int fd)
    {
        return shutdown_connection(fd);
    }

/* ===================================
 * attributes:
 * =================================== */
public:

    inline int listening_fd(void) const override
    {
        if (nullptr == m_listening_conn)
            return INVALID_SOCK_FD;

        return m_listening_conn->fd;
    }

    inline const char* listening_ip(void) const override
    {
        if (nullptr == m_listening_conn)
            return INVALID_IP;

        return m_listening_conn->self_ip;
    }

    inline uint16_t listening_port(void) const override
    {
        if (nullptr == m_listening_conn)
            return INVALID_PORT;

        return m_listening_conn->self_port;
    }

/* ===================================
 * status:
 * =================================== */
public:

/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    void destroy_listening_connection(void);

/* ===================================
 * data:
 * =================================== */
protected:
    net_connection *m_listening_conn;
};

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_TCP_SERVER_H__

