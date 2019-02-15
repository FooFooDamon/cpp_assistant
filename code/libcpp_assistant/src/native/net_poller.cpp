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

#include "net_poller.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "base/ca_return_code.h"
#include "private/debug.h"

#include "net_common.h"

CA_LIB_NAMESPACE_BEGIN


net_poller::net_poller()
{
    init();
}

net_poller::net_poller(int max_conn_count, int timeout)
{
    init();
    create(max_conn_count, timeout);
}

net_poller::~net_poller()
{
    clear();
}

int net_poller::create(int max_conn_count, int timeout)
{
    if (nullptr != m_epoll_events_holder)
        return CA_RET(OBJECT_ALREADY_EXISTS);

    int ret = CA_RET_OK;

    set_max_connection_count(max_conn_count);

    set_timeout(timeout);

    if ((m_fd = epoll_create(m_max_connection_count)) < 0)
    {
        ret = -errno;
        cerror("epoll_create() failed\n");
        goto CREATE_FAILED;
    }

    if (nullptr == (m_epoll_events_holder = (poll_event_t *)malloc(sizeof(poll_event_t) * m_max_connection_count)))
    {
        ret = CA_RET(MEMORY_ALLOC_FAILED);
        cerror("malloc() for m_epoll_events_holder failed\n");
        goto CREATE_FAILED;
    }
    m_active_connections.elements = m_epoll_events_holder;
    m_active_connections.count = 0;

    return CA_RET_OK;

CREATE_FAILED:

    clear();

    return ret;
}

void net_poller::destroy(void)
{
    clear();
}

int net_poller::poll(void)
{
    // The result containing active connections will be put into m_epoll_events_holder,
    // and m_epoll_events_holder is a member of m_active_connections.
    int ret = epoll_wait(m_fd, m_epoll_events_holder, m_current_connection_count, m_timeout);

    if (ret < 0)
    {
        ret = -errno;
        return ret;
    }

#ifdef SAVE_ALL_POLLER_CONNECTIONS
    /*
     * Synchronizes the changes.
     */
    for (int i = 0; i < ret; ++i)
    {
        int fd = ((net_connection*)(m_epoll_events_holder[i].data.ptr))->fd;

        m_total_connections[fd] = m_epoll_events_holder[i];
    }
#endif

    m_active_connections.count = ret;

    return m_active_connections.count;
}

bool net_poller::is_readable(int fd)
{
    return test_single_event(fd, EVENT_READ);
}

bool net_poller::is_writable(int fd)
{
    return test_single_event(fd, EVENT_WRITE);
}

void net_poller::init(void)
{
    m_fd = -1;
    m_max_connection_count = 0;
    m_current_connection_count = 0;
#ifdef SAVE_ALL_POLLER_CONNECTIONS
    m_total_connections.clear();
#endif
    memset(&m_active_connections, 0, sizeof(conn_info_array));
    m_epoll_events_holder = nullptr;
    m_timeout = 0;
}

void net_poller::clear(void)
{
    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
    }
    m_max_connection_count = 0;
    m_current_connection_count = 0;
#ifdef SAVE_ALL_POLLER_CONNECTIONS
    if (!(m_total_connections.empty()))
    {
        m_total_connections.clear();
        //DEBUG_PRINT_C("m_total_connections released\n");
    }
#endif
    memset(&m_active_connections, 0, sizeof(conn_info_array));
    if (nullptr != m_epoll_events_holder)
    {
        free(m_epoll_events_holder);
        m_epoll_events_holder = nullptr;
        //DEBUG_PRINT_C("m_epoll_events_holder released\n");
    }
    m_timeout = 0;
}

bool net_poller::test_single_event(int fd, int event)
{
    if ((int)EVENT_READ                  != event &&
        (int)EVENT_WRITE                 != event &&
        (int)EVENT_PEER_CLOSE            != event &&
        (int)EVENT_URGENT_READ           != event &&
        (int)EVENT_ERROR                 != event &&
        (int)EVENT_HANG_UP               != event &&
        (int)EVENT_EDGE_TRIGGERED        != event &&
        (int)EVENT_ONE_SHOT_MONITORING   != event)
        return false;

#ifdef SAVE_ALL_POLLER_CONNECTIONS
    ConnInfoMap::iterator it = m_total_connections.find(fd);

    if (m_total_connections.end() == it)
        return false;

    if (m_total_connections[fd].events & event)
        return true;
#else
    for (int i = 0; i < m_active_connections.count; ++i)
    {
        poll_event_t &conn_event = m_active_connections.elements[i];

        if (fd == ((net_connection*)(conn_event.data.ptr))->fd
            && (conn_event.events & event))
            return true;
    }
#endif

    return false;
}

int net_poller::handle_monitored_connection(const struct net_connection *conn, int events, int op_type)
{
    if (nullptr == conn)
        return CA_RET(NULL_PARAM);

    if (EVENT_HANDLE_TYPE_ADD == op_type
        && m_current_connection_count >= m_max_connection_count)
    {
        cerror("event count must not be greater than %d\n", m_max_connection_count);
        return CA_RET(EXCESS_OBJECT_COUNT);
    }

    int operation = 0;
    poll_event_t pe = {0};

    if (EVENT_HANDLE_TYPE_ADD == op_type)
        operation = EPOLL_CTL_ADD;
    else if (EVENT_HANDLE_TYPE_DEL == op_type)
        operation = EPOLL_CTL_DEL;
    else
        operation = EPOLL_CTL_MOD;

    pe.events = events;
    pe.data.ptr = (void*)conn;
    if (epoll_ctl(m_fd, operation, conn->fd, &pe) < 0)
    {
        int ret = -errno;

        cerror("epoll_ctl(%d, %d, %d, %p) failed, ret = %d\n",
            m_fd, operation, conn->fd, &pe, ret);

        return ret;
    }

    if (EVENT_HANDLE_TYPE_DEL == op_type)
    {
#ifdef SAVE_ALL_POLLER_CONNECTIONS
        if (m_total_connections.erase(conn->fd) > 0)
#endif
            --m_current_connection_count;
    }
    else
    {
#ifdef SAVE_ALL_POLLER_CONNECTIONS
        m_total_connections[conn->fd] = pe;
#endif
        if (EVENT_HANDLE_TYPE_ADD == op_type)
            ++m_current_connection_count;
    }

    return CA_RET_OK;
}

CA_LIB_NAMESPACE_END
