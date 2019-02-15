/*
 * Copyright (c) 2018-2019, Wen Xiongchang <udc577 at 126 dot com>
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
 * tcp_client.h
 *
 *  Created on: 2016-11-01
 *      Author: wenxiongchang
 * Description: TCP client that can connect multiple servers.
 */

#ifndef __CPP_ASSISTANT_TCP_CLIENT_H__
#define __CPP_ASSISTANT_TCP_CLIENT_H__

#include <stdint.h>

#include "tcp_base.h"

CA_LIB_NAMESPACE_BEGIN

class tcp_client : public tcp_base
{
/* ===================================
 * constructors:
 * =================================== */
public:
    tcp_client();
    tcp_client(const char *self_name,
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
    ~tcp_client();

/* ===================================
 * abilities:
 * =================================== */
public:
    /*
     * connect_server() and reconnect_server() both return a file descriptor on success,
     * or a negative number on failure.
     */

    int connect_server(const char *ip,
        uint16_t port,
        int send_buf_size,
        int recv_buf_size,
        bool is_nonblocking = true);

    int reconnect_server(const char *ip,
        uint16_t port,
        int send_buf_size,
        int recv_buf_size,
        bool is_nonblocking = true);

    inline int disconnect_server(net_connection *conn)
    {
        return delete_connection(conn);
    }

    inline int disconnect_server(int fd)
    {
        return shutdown_connection(fd);
    }

    int disconnect_all_servers(void);

/* ===================================
 * attributes:
 * =================================== */
public:

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

/* ===================================
 * data:
 * =================================== */
protected:
};

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_TCP_CLIENT_H__

