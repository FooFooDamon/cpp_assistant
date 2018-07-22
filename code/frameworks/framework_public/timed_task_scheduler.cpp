/*
 * Copyright (c) 2016-2018, Wen Xiongchang <udc577 at 126 dot com>
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

#include "timed_task_scheduler.h"

#include "base/all.h"
#include "config_manager.h"
#include "resource_manager.h"
#include "connection_cache.h"
#include "protocol_common.h"

namespace cafw
{

timed_task_scheduler::timed_task_scheduler()
    : m_tasks(NULL)
{
    __inner_init();
}

timed_task_scheduler::~timed_task_scheduler()
{
    __clear();
}

int timed_task_scheduler::register_one(const std::string &name, const timed_task_config &info)
{
    if (!is_ready())
    {
        GLOG_ERROR_C("resource not ready\n");
        return RET_FAILED;
    }

    if (exists(name))
    {
        GLOG_INFO_C("task[%s] has already been registered, nothing needs to be done\n", name.c_str());
        return RET_OK;
    }

    if (!__task_info_is_ok(info))
    {
        GLOG_ERROR_C("invalid task info\n");
        return RET_FAILED;
    }

    timed_task_config actual_info = info;
    int trigger_type = info.trigger_type;
    const int64_t kMinInterval = static_cast<int64_t>(timed_task_config::MIN_TRIGGER_INTERVAL);
    const int64_t kMaxInterval = static_cast<int64_t>(timed_task_config::MAX_TRIGGER_INTERVAL);
    //GLOG_DEBUG_C("MinInterval: %ld, MaxInterval: %ld\n", kMinInterval, kMaxInterval);

    if (timed_task_config::TRIGGERED_PERIODICALLY == trigger_type)
    {
        int64_t interval = labs(info.time_interval);
        //GLOG_DEBUG_C("target interval: %ld\n", interval);

        if (interval < kMinInterval)
            actual_info.time_interval = kMinInterval;
        else if (interval > kMaxInterval)
            actual_info.time_interval = kMaxInterval;
        else
            actual_info.time_interval = interval;
    }
    else
    {
        actual_info.event_time = labs(info.event_time);
        actual_info.time_offset = info.time_offset;
    }

    if (!(m_tasks->insert(std::make_pair(name, actual_info)).second))
    {
        GLOG_ERROR_C("std::map::insert() failed\n");
        return RET_FAILED;
    }

    return RET_OK;
}

int timed_task_scheduler::unregister_one(const std::string &name)
{
    if (!is_ready())
    {
        GLOG_ERROR_C("resource not ready\n");
        return RET_FAILED;
    }

    int erase_count = m_tasks->erase(name);
    if (0 == erase_count)
    {
        GLOG_ERROR_C("task[%s] not found or deletion failed\n", name.c_str());
        return RET_FAILED;
    }

    return RET_OK;
}

bool timed_task_scheduler::exists(const std::string &name)
{
    if (!is_ready())
        return false;

    timed_task_map::iterator it = m_tasks->find(name);
    if (m_tasks->end() == it)
        return false;

    return true;
}

int timed_task_scheduler::set_event_trigger_time(const std::string &name, const int64_t event_time, const int time_offset)
{
    if (!exists(name))
    {
        GLOG_ERROR_C("task[%s] not found\n", name.c_str());
        return RET_FAILED;
    }

    timed_task_config &info = m_tasks->find(name)->second;

    if (timed_task_config::TRIGGERED_PERIODICALLY == info.trigger_type)
    {
        GLOG_ERROR_C("[%s] is not an event-triggered task\n", name.c_str());
        return RET_FAILED;
    }

    info.event_time = labs(event_time);
    info.time_offset = labs(time_offset);

    return RET_OK;
}

void timed_task_scheduler::check_and_execute(void)
{
    if (!is_ready())
    {
        GLOG_ERROR_C("resource not ready\n");
        return;
    }

    timed_task_map::iterator begin = m_tasks->begin();
    timed_task_map::iterator end = m_tasks->end();
    int64_t cur_time = calns::time_util::get_utc_microseconds();

    for (timed_task_map::iterator it = begin; end != it; ++it)
    {
        timed_task_config &task_info = const_cast<timed_task_config&>(it->second);
        int64_t trigger_time = 0;

        if (NULL == task_info.operation)
            continue;

        if (timed_task_config::TRIGGERED_PERIODICALLY == task_info.trigger_type)
            trigger_time = task_info.last_op_time + task_info.time_interval * 1000;
        else
            trigger_time = task_info.event_time + task_info.trigger_type * task_info.time_offset * 1000;

        if (cur_time >= trigger_time)
        {
            int64_t start_time = calns::time_util::get_utc_microseconds();

            if (timed_task_config::TRIGGERED_PERIODICALLY == task_info.trigger_type)
            {
                task_info.operation();
                task_info.last_op_time = cur_time;
            }
            else
            {
                if (task_info.has_triggered)
                    continue;

                task_info.operation(); // event_time and has_triggered may be updated within this function or somewhere
                task_info.has_triggered = true;
            }

            GLOG_INFO_C("one round of [%s] done, time spent: %ld us\n",
                it->first.c_str(), calns::time_util::get_utc_microseconds() - start_time);
        }
        cur_time = calns::time_util::get_utc_microseconds();
    }
}

timed_task_map* timed_task_scheduler::tasks(void)
{
    return m_tasks;
}

const char* timed_task_scheduler::get_trigger_type_description(int type_num)
{
    static const char *descptions[] = {
        "TRIGGERED_BEFORE_SOME_EVENT",
        "TRIGGERED_ON_SOME_EVENT",
        "TRIGGERED_AFTER_SOME_EVENT",
        "TRIGGERED_PERIODICALLY"
    };

    return descptions[(type_num + 1) % (sizeof(descptions) / sizeof(char*))];
}

bool timed_task_scheduler::is_available(void)
{
    return (NULL != m_tasks);
}

bool timed_task_scheduler::is_ready(void)
{
    return (NULL != m_tasks);
}

int timed_task_scheduler::__inner_init(void)
{
    if ((NULL == m_tasks) &&
        (NULL == (m_tasks = new timed_task_map)))
    {
        GLOG_ERROR_C("TimedTask structure initialization failed\n");
        return RET_FAILED;
    }

    return RET_OK;
}

void timed_task_scheduler::__clear(void)
{
    if (NULL != m_tasks)
    {
        delete m_tasks;
        m_tasks = NULL;
    }
}

bool timed_task_scheduler::__task_info_is_ok(const timed_task_config &info)
{
    if (NULL == info.operation)
        return false;

    int trigger_type = info.trigger_type;
    if (timed_task_config::TRIGGERED_BEFORE_SOME_EVENT != trigger_type &&
        timed_task_config::TRIGGERED_ON_SOME_EVENT != trigger_type &&
        timed_task_config::TRIGGERED_AFTER_SOME_EVENT != trigger_type &&
        timed_task_config::TRIGGERED_PERIODICALLY != trigger_type)
        return false;

    return true;
}

void default_message_clean_timed_task(void)
{
    /*MessageCache *msg_cache = calns::SingletonT<PacketProcessor>::instance()->message_cache();

    msg_cache->CleanExpiredMessages(calns::TimeHelper::GetUtcMicroseconds());*/
}

void default_session_clean_timed_task(void)
{
    //clean_expired_sessions();
}

#if defined(HAS_TCP)
// this function may do a lot of searches, execute it when it has to.
static bool has_time_consuming_messages(const char *conn_name)
{
    /*MessageCache *msg_cache = calns::SingletonT<PacketProcessor>::instance()->message_cache();

    return (msg_cache->TimeConsumingMessageCount(conn_name) > 0);*/
    return false;
}


static void update_connection_status(char *name, void *conn)
{
    if (NULL == conn)
        return;

    net_conn_index *peer_index = (net_conn_index *)conn;

    if (!peer_index->is_server)
    {
        if (NULL == peer_index->conn_detail)
            return;

        if (!peer_index->conn_detail->is_validated)
        {
            // TODO: more operations like shutting down this connection
            GLOG_WARN("this client is not validated!!!\n");
        }

        return;
    }

    calns::tcp_client *client = calns::singleton<resource_manager>::get_instance()->resource()->client_requester;
    calns::net_connection *conn_found = client->find_peer(peer_index->fd);
    int64_t cur_time = calns::time_util::get_utc_microseconds();

    bool has_no_detail = (NULL == peer_index->conn_detail);
    bool conn_not_in_client_requester = (NULL == conn_found);
    bool conn_info_inconsistent = (
            (NULL != conn_found)
            && (
                (0 != strcmp(conn_found->peer_ip, peer_index->peer_ip))
                || (conn_found->peer_port != peer_index->peer_port)
            )
        );

    bool server_not_connected = has_no_detail
        || conn_not_in_client_requester
        || conn_info_inconsistent;

    if (!server_not_connected)
    {
        int64_t last_heartbeat_time = conn_found->last_op_time;
        int64_t actual_timeout = cur_time - last_heartbeat_time;
        int64_t default_timeout = CFG_GET_TIMEOUT_USEC(XNODE_DEFAULT_WAITING_FOR_PEER_REPLY);
        int64_t longest_timeout = CFG_GET_TIMEOUT_USEC(XNODE_LONGEST_WAITING_FOR_PEER_REPLY);

        GLOG_DEBUG("cur_time: %ld, last_heartbeat_time: %ld,"
            " actual_timeout = cur_time - last_heartbeat_time = %ld, default_timeout = %ld,"
            " longest_timeout = %ld\n", cur_time, last_heartbeat_time, actual_timeout,
            default_timeout, longest_timeout);

        if (actual_timeout <= default_timeout)
            goto SEND_HEARTBEAT;

        if (actual_timeout > longest_timeout)
            goto DISCONNECT;

        if (!has_time_consuming_messages(conn_found->peer_name))
            goto DISCONNECT;

SEND_HEARTBEAT:

        GLOG_DEBUG("^~^~^~^~ request to: [%s][%s:%u]\n",
            peer_index->conn_alias, peer_index->peer_ip, peer_index->peer_port);
        send_heartbeat_request(peer_index->fd);

        return;

DISCONNECT:

        client->disconnect_server(conn_found);
        peer_index->fd = calns::INVALID_SOCK_FD;
        peer_index->conn_detail = NULL;
        GLOG_ERROR("no heart beat response for too long, detached from server %s\n",
            conn_found->peer_name);

        return;
    }

    const int kSendBufSize = CFG_GET_BUF_SIZE(XNODE_TCP_SEND_BUF);
    const int kRecvBufSize = CFG_GET_BUF_SIZE(XNODE_TCP_RECV_BUF);
    int ret = client->connect_server(peer_index->peer_ip, peer_index->peer_port, kSendBufSize, kRecvBufSize, true);

    if (ret < 0)
    {
        GLOG_ERROR("! ! ! ! ! connection to [%s][%s:%u] failed, ret = %d, msg = %s\n",
            peer_index->conn_alias, peer_index->peer_ip, peer_index->peer_port, ret, calns::what(ret).c_str());
        peer_index->fd = calns::INVALID_SOCK_FD;
        peer_index->conn_detail = NULL;
        return;
    }

    int fd = ret;

    calns::net_connection *detail = (*(client->peers()))[fd];

    GLOG_INFO("~ ~ ~ ~ ~ connection to [%s][%s:%u] successful, fd = %d, local address = [%s:%u]\n",
        peer_index->conn_alias, peer_index->peer_ip, peer_index->peer_port, fd, detail->self_ip, detail->self_port);

    GLOG_INFO("identity report request to: [%s][%s:%u]\n",
        peer_index->conn_alias, peer_index->peer_ip, peer_index->peer_port);
    send_identity_report_request(fd);

    GLOG_DEBUG("^~^~^~^~ request to: [%s][%s:%u]\n",
        peer_index->conn_alias, peer_index->peer_ip, peer_index->peer_port);
    send_heartbeat_request(fd);

    peer_index->fd = fd;
    peer_index->conn_detail = detail;
    detail->last_op_time = cur_time;
    if (NULL != name)
    {
        connection_cache *conn_cache = (peer_index->attribute.is_master)
            ? calns::singleton<resource_manager>::get_instance()->resource()->master_connection_cache
            : calns::singleton<resource_manager>::get_instance()->resource()->slave_connection_cache;
        dict_entry_ptr owner_ptr = conn_cache->return_as_entry(name, strlen(name));

        if (NULL != owner_ptr)
            detail->owner = owner_ptr;
        else
            GLOG_WARN_NS("", "can not find connection cache item with name[%s]|is_master[%d],"
                " owner info not set\n", name, peer_index->attribute.is_master);

        strncpy(detail->peer_name, name, calns::MAX_CONNECTION_NAME_LEN);
    }
}
#endif

void default_heartbeat_timed_task(void)
{
#if defined(HAS_TCP)
    connection_cache *master_conn_cache = calns::singleton<resource_manager>::get_instance()->resource()->master_connection_cache;
    connection_cache *slave_conn_cache = calns::singleton<resource_manager>::get_instance()->resource()->slave_connection_cache;

    master_conn_cache->do_batch_operation(update_connection_status);
    slave_conn_cache->do_batch_operation(update_connection_status);
#endif

    /*db_heartbeat();
#ifdef HAS_DATABASE
    if (db_needs_reconnect())
    {
        kpi_send(KPI_DEF_CONNECT_DB_ERR);
        db_reconnect();
    }
#ifdef MULTI_THREADING
    const int kThreadCount = CFG_GET_COUNTER(XNODE_WORKER_THREAD);
    int64_t cur_time = calns::Time::GetUtcMicroseconds();

    for (int i = 0; i < kThreadCount; ++i)
    {
        ThreadContext &ctx = g_thread_contexts[i];

        if (ThreadContext::THREAD_STATUS_EXITED_NORMALLY == ctx.status)
            continue;

        ctx.timed_task_refresh_times[ThreadContext::WORKER_TASK_DB_HEARTBEAT] = cur_time;
    }
#endif
#endif

    kpi_send(KPI_DEF_HEARTBEAT);*/
}

void default_log_flushing_timed_task(void)
{
    GLOG_FLUSH();
#ifdef MULTI_THREADING
    const int kThreadCount = CFG_GET_COUNTER(XNODE_WORKER_THREAD);
    int64_t cur_time = calns::time_util::GetUtcMicroseconds();

    for (int i = 0; i < kThreadCount; ++i)
    {
        ThreadContext &ctx = g_thread_contexts[i];

        if (ThreadContext::THREAD_STATUS_EXITED_NORMALLY == ctx.status)
            continue;

        ctx.timed_task_refresh_times[ThreadContext::WORKER_TASK_LOG_FLUSHING] = cur_time;
    }
#endif
}

} // namespace cafw
