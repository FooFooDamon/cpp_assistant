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
 * tcp_base.h
 *
 *  Created on: 2016-10-31
 *      Author: wenxiongchang
 * Description: Base class for TCP utility classes.
 */

#ifndef __CPP_ASSISTANT_TCP_BASE_H__
#define __CPP_ASSISTANT_TCP_BASE_H__

#include <stdint.h>
#include <string.h>

#include <map>
#include <vector>
#include <string>

#include "net_common.h"
#include "net_poller.h"

CA_LIB_NAMESPACE_BEGIN

class tcp_base
{
/* ===================================
 * constructors:
 * =================================== */
public:
    tcp_base();
    tcp_base(const char *self_name,
        int max_peer_count = net_poller::DEFAULT_CONNECTION_COUNT,
        int timeout = net_poller::DEFAULT_POLL_TIMEOUT);

/* ===================================
 * copy control:
 * =================================== */
private:
    tcp_base(const tcp_base& src);
    tcp_base& operator=(const tcp_base& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    virtual ~tcp_base();

/* ===================================
 * types:
 * =================================== */
public:
    typedef std::map<int, net_conn*> connection_map;
    typedef connection_map conn_map;

    typedef net_poller::conn_info_array conn_info_array;

    typedef int (*format_output_func)(const char *fmt, ...);

protected:

    enum enum_check_operation
    {
        CHK_OP_READABLE = 1,
        CHK_OP_WRITEABLE = 2,
        CHK_OP_ABNORMAL = 3
    };

/* ===================================
 * abilities:
 * =================================== */
public:
    static CA_REENTRANT int set_nonblocking(int fd);

    /*
     * Sends/Receives at most @len bytes of data fragment, data destination/origin is @buf,
     * and the target connection is specified by file descriptor @fd.
     * Returns bytes sent/received on success, or a negative number on failure.
     */
    static CA_REENTRANT int send_fragment(int fd, const void *buf, int len);
    static CA_REENTRANT int recv_fragment(int fd, void *buf, int len);

    /*
     * Like xx_fragment() above, except that the target is the connection
     * specified by @conn or @fd.
     */
    static CA_REENTRANT int send_from_connection(net_connection *conn);
    static CA_REENTRANT int recv_to_connection(net_connection *conn);
    int send_from_connection(int fd);
    int recv_to_connection(int fd);

    // Shows contents of a TCP instance, such as IP, port, its peers, etc.
    // By default, these contents will be copied into @result_holder if it's not null, otherwise,
    // they'll be printed or logged by logger or by system print function, depending on how
    // @logger is like.
    virtual void has_what(std::string *result_holder = nullptr, format_output_func logger = nullptr);

    net_connection *find_peer(int fd);

    conn_info_array& get_active_peers(void) const;

    virtual int shutdown_connection(net_connection *conn);
    virtual int shutdown_connection(int fd);

    int poll(void);

/* ===================================
 * attributes:
 * =================================== */
public:

    inline const char *self_name(void) const
    {
        return m_self_name;
    }

    inline void set_self_name(const char *name)
    {
        if (nullptr == name)
            return;

        strncpy(m_self_name, name, sizeof(m_self_name));
        m_self_name[sizeof(m_self_name) - 1] = '\0';
    }

    inline const int connection_type(void) const
    {
        return m_connection_type;
    }

    inline connection_map *peers(void) const
    {
        return m_peers;
    }

    inline int timeout(void) const
    {
        return m_timeout;
    }

    inline void set_timeout(int timeout)
    {
        if (m_timeout < 0 && net_poller::INIFITE_POLL_TIMEOUT != m_timeout)
            m_timeout = net_poller::DEFAULT_POLL_TIMEOUT;
        else
            m_timeout = timeout;

        if (nullptr != m_poller)
            m_poller->set_timeout(timeout);
    }

    // listening_xxx() returns the socket file descriptor, IP or port number for listening.
    // It's meaningful only if the instance is a server.
    // Below are default implementations, re-write them in a server class afterwards!

    virtual inline int listening_fd(void) const
    {
        return INVALID_SOCK_FD;
    }

    virtual inline const char* listening_ip(void) const
    {
        return INVALID_IP;
    }

    virtual inline uint16_t listening_port(void) const
    {
        return INVALID_PORT;
    }

/* ===================================
 * status:
 * =================================== */
public:
    /*
     * Tests if a file descriptor is writable/readable.
     * The testing may be blocked temporarily, but will not be longer than
     * @timeout_usec microseconds.
     */

    static inline CA_REENTRANT bool is_writable(int fd, int timeout_usec = net_poller::DEFAULT_POLL_TIMEOUT)
    {
        return is_ready(fd, CHK_OP_WRITEABLE, timeout_usec);
    }

    static inline CA_REENTRANT bool is_readable(int fd, int timeout_usec = net_poller::DEFAULT_POLL_TIMEOUT)
    {
        return is_ready(fd, CHK_OP_READABLE, timeout_usec);
    }

/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    virtual int init(const char *self_name = nullptr,
        int max_peer_count = net_poller::DEFAULT_CONNECTION_COUNT,
        int timeout = net_poller::DEFAULT_POLL_TIMEOUT);

    virtual void clear(void);

    int add_connection(net_connection *conn);

    int delete_connection(net_connection *conn, bool delete_peer_node_now = false);

    static CA_REENTRANT bool is_ready(int fd, enum_check_operation check_type, int timeout_usec);

/* ===================================
 * data:
 * =================================== */
protected:
    char m_self_name[MAX_CONNECTION_NAME_LEN + 1];
    int m_connection_type;
    connection_map *m_peers;
    int m_max_peer_count;
    net_poller *m_poller;
    int m_timeout; // in milliseconds
};

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_TCP_BASE_H__

