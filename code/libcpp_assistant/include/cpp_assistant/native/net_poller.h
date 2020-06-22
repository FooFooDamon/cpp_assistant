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
 * net_poller.h
 *
 *  Created on: 2016-10-28
 *      Author: wenxiongchang
 * Description: Poller for network connection.
 */

#ifndef __CPP_ASSISTANT_NET_POLLER_H__
#define __CPP_ASSISTANT_NET_POLLER_H__

#include <sys/epoll.h>

#include <map>

#include "base/ca_inner_necessities.h"

CA_LIB_NAMESPACE_BEGIN

typedef struct epoll_event poll_event_t;
struct net_connection;

//#define SAVE_ALL_POLLER_CONNECTIONS

class net_poller
{
/* ===================================
 * constructors:
 * =================================== */
public:
    net_poller();
    net_poller(int max_conn_count, int timeout);

/* ===================================
 * copy control:
 * =================================== */
private:
    net_poller(const net_poller& src);
    const net_poller& operator=(const net_poller& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~net_poller();

/* ===================================
 * types:
 * =================================== */
public:
    typedef std::map<int, poll_event_t> conn_info_map;

    typedef struct conn_info_array
    {
        poll_event_t *elements; // (net_connection*)(elements[i].data.ptr) is what we need.
        int count;
    }conn_info_array;

    enum enum_poll_timeout // unit: millisecond
    {
        DEFAULT_POLL_TIMEOUT = 10,
        INIFITE_POLL_TIMEOUT = -1
    };

    enum enum_conn_count
    {
        DEFAULT_CONNECTION_COUNT = 1024,
        MAX_CONNECTION_COUNT = 10000
    };

    enum enum_event
    {
        EVENT_READ                  = EPOLLIN,
        EVENT_WRITE                 = EPOLLOUT,
        EVENT_PEER_CLOSE            = EPOLLRDHUP, // for Edge Triggered Monitoring only
        EVENT_URGENT_READ           = EPOLLPRI,
        EVENT_ERROR                 = EPOLLERR, // no need to set it manually
        EVENT_HANG_UP               = EPOLLHUP, // no need to set it manually
        EVENT_EDGE_TRIGGERED        = EPOLLET,
        EVENT_ONE_SHOT_MONITORING   = EPOLLONESHOT
    };

protected:
    enum enum_event_handle_type
    {
        EVENT_HANDLE_TYPE_ADD = 1,
        EVENT_HANDLE_TYPE_MOD = 2,
        EVENT_HANDLE_TYPE_DEL = 3
    };

/* ===================================
 * abilities:
 * =================================== */
public:
    // Creates a poller instance which can monitor at most @max_conn_count
    // or MAX_CONNECTION_COUNT connections and limits poll timeout to @timeout milliseconds.
    int create(int max_conn_count, int timeout);

    // Destroys the current poller instance.
    void destroy(void);

    /*
     * xx_monitored_connection() adds a new connection into the monitor queue,
     * or modifies/deletes an existent connection in/from the monitor queue.
     * This connection has a file descriptor @fd and cares about events
     * specified by @events whose value is the bitwise-or result of one or more than one
     * value(s) of enum Event.
     */

    int add_monitored_connection(const struct net_connection *conn, int events)
    {
        return handle_monitored_connection(conn, events, EVENT_HANDLE_TYPE_ADD);
    }

    int modify_monitored_connection(const struct net_connection *conn, int events)
    {
        return handle_monitored_connection(conn, events, EVENT_HANDLE_TYPE_MOD);
    }

    int delete_monitored_connection(const struct net_connection *conn)
    {
        return handle_monitored_connection(conn, 0, EVENT_HANDLE_TYPE_DEL);
    }

    // Polls and finds out active connections, and then they can be accessed by active_connections().
    // Returns the count of active connections on success, or a negative number on failure.
    int poll(void);

    // Tests if an connection with file descriptor @fd is readable/writable.
    bool is_readable(int fd);
    bool is_writable(int fd);

/* ===================================
 * attributes:
 * =================================== */
public:

    inline void set_timeout(int milliseconds)
    {
        if (m_timeout < 0 && INIFITE_POLL_TIMEOUT != m_timeout)
            m_timeout = DEFAULT_POLL_TIMEOUT;
        else
            m_timeout = milliseconds;
    }

    inline int timeout(void) const
    {
        return m_timeout;
    }

    inline int max_connection_count(void) const
    {
        return m_max_connection_count;
    }

    inline int current_connection_count(void) const
    {
        return m_current_connection_count;
    }

    inline conn_info_array& active_connections(void)
    {
        return m_active_connections;
    }

#ifdef SAVE_ALL_POLLER_CONNECTIONS
    inline const conn_info_map& total_connections(void) const
    {
        return m_total_connections;
    }
#endif

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
    void init(void);
    void clear(void);
    inline void set_max_connection_count(int count)
    {
        if (count <= 0)
            m_max_connection_count = DEFAULT_CONNECTION_COUNT;
        else if (count > MAX_CONNECTION_COUNT)
            m_max_connection_count = MAX_CONNECTION_COUNT;
        else
            m_max_connection_count = count;
    }
    bool test_single_event(int fd, int event);
    int handle_monitored_connection(const struct net_connection *conn, int events, int op_type);

/* ===================================
 * data:
 * =================================== */
private:
    int m_fd;
    int m_max_connection_count;
    int m_current_connection_count;
#ifdef SAVE_ALL_POLLER_CONNECTIONS
    conn_info_map m_total_connections;
#endif
    conn_info_array m_active_connections;
    poll_event_t *m_epoll_events_holder;
    int m_timeout; // in milliseconds
};

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_NET_POLLER_H__

