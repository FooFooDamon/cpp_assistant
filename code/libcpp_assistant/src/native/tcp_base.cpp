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

#include "tcp_base.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <typeinfo>

#include "base/ca_return_code.h"
#include "private/debug.h"
#include "time_util.h"
#include "sequential_buffer.h"

CA_LIB_NAMESPACE_BEGIN


tcp_base::tcp_base()
{
    init(nullptr, net_poller::DEFAULT_CONNECTION_COUNT, net_poller::DEFAULT_POLL_TIMEOUT);
}

tcp_base::tcp_base(const char *self_name,
    int max_peer_count/* = net_poller::DEFAULT_CONNECTION_COUNT*/,
    int timeout/* = net_poller::DEFAULT_POLL_TIMEOUT*/)
{
    init(self_name, max_peer_count, timeout);
}

/*virtual */tcp_base::~tcp_base()
{
    clear();
}

CA_REENTRANT int tcp_base::set_nonblocking(int fd)
{
    int err = 0;
    int opts = fcntl(fd, F_GETFL);

    if (opts < 0)
    {
        err = errno;
        return (0 != err) ? (-err) : CA_RET_GENERAL_FAILURE;
    }

    opts |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0)
    {
        err = errno;
        return (0 != err) ? (-err) : CA_RET_GENERAL_FAILURE;
    }

    return CA_RET_OK;
}

CA_REENTRANT int tcp_base::send_fragment(int fd, const void *buf, int len)
{
    if (fd < 0 ||
        len < 0 ||
        nullptr == buf)
        return CA_RET(INVALID_PARAM_VALUE);

    if (0 == len)
        return 0;

    int ret = 0;

    while (1)
    {
        int slen = send(fd, buf, len, 0);

        if (slen < 0)
        {
            int err = errno;

            if (EINTR == err)
                continue;
            else if (EAGAIN == err || EWOULDBLOCK == err)
                break;
            else
            {
                ret = -err;
                break;
            }
        }

        ret += slen;
        if (ret >= len)
            break;
    }

    return ret;
}

CA_REENTRANT int tcp_base::recv_fragment(int fd, void *buf, int len)
{
    if (fd < 0 ||
        len < 0 ||
        nullptr == buf)
        return CA_RET(INVALID_PARAM_VALUE);

    if (0 == len)
        return 0;

    int ret = 0;

    while (1)
    {
        int rlen = recv(fd, buf, len, 0);

        if (0 == rlen)
        {
            ret = CA_RET(CONNECTION_BROKEN);
            break;
        }
        else if (rlen < 0)
        {
            int err = errno;

            if (EINTR == err)
                continue;
            else if (EAGAIN == err || EWOULDBLOCK == err)
                break;
            else
            {
                ret = -err;
                break;
            }
        }

        ret += rlen;
        if (ret >= len)
            break;
    }

    return ret;
}

CA_REENTRANT int tcp_base::send_from_connection(net_connection *conn)
{
    if (nullptr == conn)
        return CA_RET(NULL_PARAM);

    buffer *buf = conn->send_buf;

    if (nullptr == buf)
        return CA_RET(RESOURCE_NOT_AVAILABLE);

    int status = conn->conn_status;

    if ((0 == (CONN_STATUS_CONNECTED & status)) ||
        (0 != (CONN_STATUS_DISCONNECTED & status)) ||
        (0 != (CONN_STATUS_DISCONNECTING & status)))
        return CA_RET(CONNECTION_BROKEN);
    else if (0 != (CONN_STATUS_CONNECTING & status))
        return CA_RET(CONNECTION_NOT_READY);

    void *data = buf->get_read_pointer();

    if (seqbuf::OVERFLOW_PTR == data)
        return CA_RET(POINTER_OUT_OF_BOUND);

    int data_len = buf->data_size();

    if (data_len <= 0)
        return 0;

    int ret = send_fragment(conn->fd, data, data_len);

    if (CA_RET(CONNECTION_BROKEN) == ret)
        conn->conn_status = CONN_STATUS_BROKEN;

    if (ret > 0)
    {
        buf->move_read_pointer(ret);
        conn->last_op_time = time_util::get_utc_microseconds();
    }

    return ret;
}

CA_REENTRANT int tcp_base::recv_to_connection(net_connection *conn)
{
    if (nullptr == conn)
        return CA_RET(NULL_PARAM);

    buffer *buf = conn->recv_buf;

    if (nullptr == buf)
        return CA_RET(RESOURCE_NOT_AVAILABLE);

    int status = conn->conn_status;

    if ((0 == (CONN_STATUS_CONNECTED & status)) ||
        (0 != (CONN_STATUS_DISCONNECTED & status)) ||
        (0 != (CONN_STATUS_DISCONNECTING & status)))
        return CA_RET(CONNECTION_BROKEN);
    else if (0 != (CONN_STATUS_CONNECTING & status))
        return CA_RET(CONNECTION_NOT_READY);

    void* write_ptr = buf->get_write_pointer();
    int available_len = buf->total_size() - buf->write_position();

    if (seqbuf::OVERFLOW_PTR == write_ptr || available_len > buf->total_size())
        return CA_RET(POINTER_OUT_OF_BOUND);

    if (available_len <= 0)
        return CA_RET(SPACE_NOT_ENOUGH);

    int ret = recv_fragment(conn->fd, write_ptr, available_len);

    if (CA_RET(CONNECTION_BROKEN) == ret)
        conn->conn_status = CONN_STATUS_BROKEN;

    if (ret > 0)
    {
        buf->move_write_pointer(ret);
        conn->last_op_time = time_util::get_utc_microseconds();
    }

    return ret;
}

int tcp_base::send_from_connection(int fd)
{
    net_connection *conn = find_peer(fd);

    if (nullptr == conn)
        return CA_RET(OBJECT_DOES_NOT_EXIST);

    return send_from_connection(conn);
}

int tcp_base::recv_to_connection(int fd)
{
    net_connection *conn = find_peer(fd);

    if (nullptr == conn)
        return CA_RET(OBJECT_DOES_NOT_EXIST);

    return recv_to_connection(conn);
}

/*virtual */void tcp_base::has_what(std::string *result_holder/* = nullptr */, format_output_func logger/* = nullptr */)
{
    net_connection *conn = nullptr;
    format_output_func ofunc = nullptr;

    if (nullptr == ofunc)
        ofunc = printf;
    else
        ofunc = logger;

    if (nullptr == result_holder)
    {
        ofunc("connection type: 0x%08X[%s]\n", m_connection_type, desc_of_connection_type(m_connection_type));
        for (connection_map::iterator it = m_peers->begin(); it != m_peers->end(); ++it)
        {
            conn = it->second;
            if (nullptr == conn)
            {
                ofunc("connection[%d] null\n", it->first);
                continue;
            }

            ofunc("connection[%d]: fd[%d] | self_ip[%s] | self_port[%u] | self_name[%s]"
                " | peer_ip[%s] | peer_port[%u] | peer_name[%s] | status[0x%08X-%s]"
                " | is_blocking[%d] | is_validated[%d] | send_buffer[%d bytes] | recv_buffer[%d bytes]\n",
                it->first, conn->fd, conn->self_ip, conn->self_port, conn->self_name,
                conn->peer_ip, conn->peer_port, conn->peer_name, conn->conn_status, desc_of_connection_status(conn->conn_status),
                conn->is_blocking, conn->is_validated, conn->send_buf->total_size(), conn->recv_buf->total_size());
        }
    }
    else
    {
        char buf[1024] = {0};

        result_holder->clear();
        snprintf(buf, sizeof(buf), "connection type: 0x%08X[%s]\n", m_connection_type, desc_of_connection_type(m_connection_type));
        result_holder->append(buf);
        for (connection_map::iterator it = m_peers->begin(); it != m_peers->end(); ++it)
        {
            conn = it->second;
            if (nullptr == conn)
            {
                snprintf(buf, sizeof(buf), "connection[%d] null\n", it->first);
            }
            else
            {
                snprintf(buf, sizeof(buf), "connection[%d]: fd[%d] | self_ip[%s] | self_port[%u] | self_name[%s]"
                    " | peer_ip[%s] | peer_port[%u] | peer_name[%s] | status[0x%08X-%s]"
                    " | is_blocking[%d] | is_validated[%d] | send_buffer[%d bytes] | recv_buffer[%d bytes]\n",
                    it->first, conn->fd, conn->self_ip, conn->self_port, conn->self_name,
                    conn->peer_ip, conn->peer_port, conn->peer_name, conn->conn_status, desc_of_connection_status(conn->conn_status),
                    conn->is_blocking, conn->is_validated, conn->send_buf->total_size(), conn->recv_buf->total_size());
            }
            result_holder->append(buf);
        }
    }
}

net_connection *tcp_base::find_peer(int fd)
{
    if (nullptr == m_peers)
        return nullptr;

    connection_map::iterator it = m_peers->find(fd);
    if (m_peers->end() == it)
        return nullptr;

    return it->second;
}

tcp_base::conn_info_array& tcp_base::get_active_peers(void) const
{
    return m_poller->active_connections();
}

/*virtual */int tcp_base::shutdown_connection(net_connection *conn)
{
    return delete_connection(conn);
}

/*virtual */int tcp_base::shutdown_connection(int fd)
{
    net_connection *conn = find_peer(fd);

    if (nullptr == conn)
        return CA_RET(OBJECT_DOES_NOT_EXIST);

    return shutdown_connection(conn);
}

int tcp_base::poll(void)
{
    return m_poller->poll();
}

/*virtual */int tcp_base::init(const char *self_name/* = nullptr*/,
    int max_peer_count/* = net_poller::DEFAULT_CONNECTION_COUNT*/,
    int timeout/* = net_poller::DEFAULT_POLL_TIMEOUT*/)
{
    int ret = CA_RET_GENERAL_FAILURE;

    set_self_name(self_name);
    m_connection_type = CONN_TYPE_NONE;
    m_peers = nullptr;
    m_max_peer_count = max_peer_count;
    m_poller = nullptr;
    m_timeout = timeout;

    try
    {
        m_peers = new connection_map;
        m_poller = new net_poller(m_max_peer_count, m_timeout);
    }
    catch (std::bad_alloc& e)
    {
        cerror("exception caught during new() for connection map or poller: %s\n", e.what());
        ret = CA_RET(MEMORY_ALLOC_FAILED);
        goto INIT_FAILED;
    }

    if (nullptr == m_peers ||
        nullptr == m_poller)
    {
        ret = CA_RET(MEMORY_ALLOC_FAILED);
        goto INIT_FAILED;
    }

    m_max_peer_count = m_poller->max_connection_count();
    m_timeout = m_poller->timeout();

    return CA_RET_OK;

INIT_FAILED:

    clear();

    return ret;
}

/*virtual */void tcp_base::clear(void)
{
    memset(m_self_name, 0, sizeof(m_self_name));
    m_connection_type = CONN_TYPE_NONE;
    if (nullptr != m_peers)
    {
        //for (connection_map::iterator it = m_peers->begin(); it != m_peers->end(); ++it)
        for (connection_map::iterator it = m_peers->begin(); it != m_peers->end();  )
        {
            delete_connection((it++)->second); // erase() within this function may affect increment of it !!
        }
        m_peers->clear();
        delete m_peers;
        m_peers = nullptr;
        //cdebug("%s m_peers released\n", typeid(*this).name());
    }
    m_max_peer_count = 0;
    if (nullptr != m_poller)
    {
        delete m_poller;
        m_poller = nullptr;
        //cdebug("%s m_poller released\n", typeid(*this).name());
    }
    m_timeout = 0;
}

int tcp_base::add_connection(net_connection *conn)
{
    if (nullptr == conn)
        return CA_RET(NULL_PARAM);

    connection_map::iterator it = m_peers->find(conn->fd);

    if (m_peers->end() == it)
    {
        if (m_peers->insert(std::make_pair(conn->fd, conn)).second) // (1) insertion for new connections
            return CA_RET_OK;
        else
            return CA_RET(INNER_DATA_STRUCT_ERROR);
    }

    net_connection *conn_found = it->second;

    if (nullptr == conn_found)
    {
        it->second = conn;
        return CA_RET_OK;
    }

    int status = conn_found->conn_status;

    if (CONN_STATUS_BROKEN != status && CONN_STATUS_DISCONNECTED != status)
        return CA_RET(OBJECT_ALREADY_EXISTS);

    destroy_net_connection(&conn_found);

    it->second = conn; // (2) replacement for re-connections

    return CA_RET_OK;
}

int tcp_base::delete_connection(net_connection *conn, bool delete_peer_node_now/* = false*/)
{
    if (nullptr == conn)
        return CA_RET(NULL_PARAM);

    int fd = conn->fd;
    connection_map::iterator it = m_peers->find(fd);

    if (m_peers->end() == it)
    {
        cdebug("connection with fd = %d not found\n", fd);
        return CA_RET_OK;
    }

    net_connection *cur_conn = it->second;

    if (nullptr == cur_conn)
    {
        m_peers->erase(it);
        cdebug("a null connection with fd = %d erased from map\n", fd);
        return CA_RET_OK;
    }

    if (nullptr != m_poller)
        m_poller->delete_monitored_connection(conn); // do not forget to delete monitored event

    destroy_net_connection(&cur_conn);
    cdebug("node of connection with fd = %d released\n", fd);

    m_peers->erase(it);
    cdebug("connection with fd = %d erased from map\n", fd);

    return CA_RET_OK;
}

CA_REENTRANT bool tcp_base::is_ready(int fd, enum_check_operation check_type, int timeout_usec)
{
    if (fd < 0 || timeout_usec < 0)
    {
        nserror(tcp_base, "invalid params\n");
        return false;
    }

    struct timeval tv;
    fd_set fdset;
    int ret = -1;

    tv.tv_sec = timeout_usec / 1000000;
    tv.tv_usec = timeout_usec % 1000000;

    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    if (CHK_OP_READABLE == check_type)
        ret = select(fd + 1, &fdset, nullptr, nullptr, &tv);
    else if (CHK_OP_WRITEABLE == check_type)
        ret = select(fd + 1, nullptr, &fdset, nullptr, &tv);
    else
        ret = select(fd + 1, nullptr, nullptr, &fdset, &tv);

    if (ret <= 0 ||
        !FD_ISSET(fd, &fdset))
    {
        nserror(tcp_base, "select() failed, errno = %d\n", errno);
        return false;
    }

    /*
     * fd is readable or writable or both in some conditions, say, during connect(),
     * therefore, getsockopt() is called to determine if there is an error occurring on
     * the specified fd.
     */
    int err = 0;
    socklen_t errlen = sizeof(err);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
    {
        nserror(tcp_base, "getsockopt() failed, errno = %d\n", errno);
        return false;
    }

    if (err)
    {
        nserror(tcp_base, "error occurred on sock fd, errno = %d\n", err);
        return false;
    }

    return true;
}

CA_LIB_NAMESPACE_END
