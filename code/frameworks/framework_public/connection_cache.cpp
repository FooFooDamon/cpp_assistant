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

#include "connection_cache.h"

#include <sstream>

#include "base/all.h"

namespace cafw
{

//DEFINE_CLASS_NAME(connection_cache);

connection_cache::connection_cache()
    : m_connection_dictionary(NULL)
    , m_type_group(NULL)
{
    ;
}

connection_cache::connection_cache(int dict_size)
    : m_connection_dictionary(NULL)
    , m_type_group(NULL)
{
    create(dict_size);
}

connection_cache::~connection_cache()
{
    destroy();
}

int connection_cache::create(int dict_size)
{
    if ((NULL == m_connection_dictionary) &&
        (NULL == (m_connection_dictionary = char_dict_create(DEFAULT_DICT_SIZE))))
    {
        GLOG_ERROR_C("char_dict_create() failed\n");
        return RET_FAILED;
    }

    if ((NULL == m_type_group) &&
        (NULL == (m_type_group = new server_group)))
    {
        GLOG_ERROR_C("new(ServerGroup) failed\n");
        return RET_FAILED;
    }

    return RET_OK;
}

static void free_connection_dict_value(uint32_t unused_hint, void **val)
{
    char_dict_release_val((net_conn_index **)val);
}

void connection_cache::destroy(void)
{
    if (NULL != m_type_group)
    {
        delete m_type_group;
        m_type_group = NULL;
    }

    char_dict_destroy(&m_connection_dictionary, free_connection_dict_value, NO_FREE_HINT);
}

int connection_cache::add(
    const char *name,
    const int name_len,
    const net_conn_index *conn,
    const int conn_len)
{
    // note: a new connection must has a different name, otherwise it would not be added successfully
    if (char_dict_add_element(name, name_len, conn, conn_len, m_connection_dictionary) < 0)
    {
        GLOG_ERROR_C("char_dict_add_element() failed\n");
        return RET_FAILED;
    }

    m_type_group->insert(std::make_pair(conn->server_type, name));

    return RET_OK;
}

int connection_cache::del(const char *name, const int name_len)
{
    if (NULL == name || name_len < 0)
    {
        GLOG_ERROR_C("null name or negative length\n");
        return RET_FAILED;
    }

    net_conn_index *conn_index = return_as_index(name, name_len);

    if (NULL != conn_index)
    {
        std::pair<server_group_iter, server_group_iter> group = m_type_group->equal_range(conn_index->server_type);

        for (server_group_iter it = group.first; it != group.second; ++it)
        {
            if (name == it->second)
            {
                m_type_group->erase(it);
                break;
            }
        }
    }

    return char_dict_delete_element(name, name_len, m_connection_dictionary, free_connection_dict_value, NO_FREE_HINT);
}

net_conn_index* connection_cache::return_as_index(const char *name, const int name_len)
{
    return (net_conn_index *)char_dict_find_value(name, name_len, m_connection_dictionary);
}

dict_entry_ptr connection_cache::return_as_entry(const char *name, const int name_len)
{
    return (dict_entry_ptr)char_dict_find_iterator(name, name_len, m_connection_dictionary);
}

net_conn_index* connection_cache::pick_one_connection(const char *type, int policy, int id, bool pick_alive_only/* = true*/)
{
    if (NULL == type)
    {
        GLOG_ERROR_C("null type\n");
        return NULL;
    }

    std::string type_str(type);
    int node_count = m_type_group->count(type_str);

    if (0 == node_count)
    {
        GLOG_ERROR_C("0 nodes of type[%s]\n", type);
        return NULL;
    }

    std::pair<server_group_iter, server_group_iter> group = m_type_group->equal_range(type_str);
    static unsigned int s_rand_seed = time(NULL);
    int node_offset = (SEND_POLICY_BY_ID == policy)
        ? (abs(id) % node_count)
        : (rand_r(&s_rand_seed) % node_count);
    bool uses_offset = true; // that is: picking a connection randomly at first time.
    int offset_count = 0;
    net_conn_index *target = NULL;

SEARCH_CYCLE:

    for (server_group_iter it = group.first; it != group.second; ++it)
    {
        int current_offset = offset_count++;

        if (uses_offset && current_offset < node_offset)
            continue;

        const char *serv_name = it->second.c_str();

        net_conn_index *conn_index = return_as_index(serv_name, strlen(serv_name));

        if (NULL == conn_index)
        {
            GLOG_WARN_C("can not find connection index of server[%s]\n", serv_name);

            if (uses_offset)
            {
                uses_offset = false; // gives up random selection
                offset_count = 0;
                if (node_count > 1)
                    goto SEARCH_CYCLE; // and does searching from the beginning until finds an available one
                else
                    return NULL;
            }
            else
            {
                GLOG_INFO_C("continue to find next one of the same type ...\n");
                continue;
            }
        }

        if (!pick_alive_only)
        {
            target = conn_index;
            break;
        }

        cal::net_connection *conn_detail = conn_index->conn_detail;

        if (NULL == conn_detail)
        {
            GLOG_WARN_C("can not find connection detail of server[%s]\n", serv_name);

            if (uses_offset)
            {
                uses_offset = false;
                offset_count = 0;
                if (node_count > 1)
                    goto SEARCH_CYCLE;
                else
                    return NULL;
            }
            else
            {
                GLOG_INFO_C("continue to find next one of the same type ...\n");
                continue;
            }
        }

        target = conn_index;
        break;
    }

    return target;
}

void connection_cache::do_batch_operation(action_to_char_dict_element op)
{
    char_dict_batch_operation(m_connection_dictionary, op);
}

static void __profile_each_connection(char *name, void *conn)
{
    if (NULL == name || NULL == conn)
        return;

    net_conn_index *conn_index = (net_conn_index *)conn;
    net_conn_attr *attr = &(conn_index->attribute);

    GLOG_INFO("name[%s] | fd[%d] | peer_address[%s:%u]"
        " | alias[%s] | attributes{ server_type[%s] | is_server[%d] | is_master[%d] }\n",
        name, conn_index->fd, conn_index->peer_ip, conn_index->peer_port,
        conn_index->conn_alias, conn_index->server_type, conn_index->is_server, attr->is_master);
}

void connection_cache::profile(void)
{
    GLOG_INFO("connection cache profile begin:\n");
    char_dict_batch_operation(m_connection_dictionary, __profile_each_connection);
    GLOG_INFO("connection cache profile end\n");
}

int connection_cache::send_to_connections_by_type(const char *type,
    const int max_conn_count,
    const void *msg,
    const int msg_len,
    const bool to_all,
    const int policy,
    const int id)
{
    if (NULL == type ||
        max_conn_count < 0 ||
        NULL == msg ||
        msg_len < 0)
    {
        GLOG_ERROR_C("invalid params\n");
        return RET_FAILED;
    }

    if (!to_all)
    {
        net_conn_index *target_conn = pick_one_connection(type, policy, id);

        if (NULL == target_conn)
        {
            GLOG_ERROR_C("failed to pick a connection\n");
            return RET_FAILED;
        }

        const char *target_server = target_conn->conn_detail->peer_name;
        int target_fd = target_conn->conn_detail->fd;
        int ret = cal::tcp_base::send_fragment(target_fd, msg, msg_len);

        if (ret < 0)
        {
            GLOG_ERROR_C("failed to send message to node[%s], ret = %d\n", target_server, ret);
            return RET_FAILED;
        }

        GLOG_INFO_C("message sent to a node [name: %s, ip: %s, port: %u, fd: %d] successfully,"
            " expected bytes = %d, actual bytes = %d\n",
            target_server, target_conn->peer_ip, target_conn->peer_port, target_fd, msg_len, ret);

        return ret;
    }

    const char *serv_name = NULL;
    int ok_count = 0;
    int bytes_sent = 0;
    std::string type_str(type);
    std::pair<server_group_iter, server_group_iter> group = m_type_group->equal_range(type_str);

    GLOG_INFO_C("about to send message to servers of type[%s],"
        " max receiver count: %d, to all or not: %d\n", type, max_conn_count, to_all);

    for (server_group_iter it = group.first; it != group.second; ++it)
    {
        serv_name = it->second.c_str();

        net_conn_index *conn_index = return_as_index(serv_name, strlen(serv_name));

        if (NULL == conn_index)
            continue;

        cal::net_connection *conn_detail = conn_index->conn_detail;

        if (NULL == conn_detail)
        {
            GLOG_ERROR_C("server[%s] is in an invalid status\n", serv_name);
            continue;
        }

        int fd = conn_detail->fd;
        int ret = cal::tcp_base::send_fragment(fd, msg, msg_len);

        if (ret < 0)
        {
            GLOG_ERROR_C("failed to send message to node[%s], ret = %d\n", serv_name, ret);
            continue;
        }
        else
        {
            GLOG_INFO_C("message sent to node [name: %s, ip: %s, port: %u, fd: %d] successfully,"
                " expected bytes = %d, actual bytes = %d\n",
                serv_name, conn_index->peer_ip, conn_index->peer_port, fd, msg_len, ret);
            ++ok_count;
            bytes_sent += ret;
            if (ok_count >= max_conn_count)
                break;
        }
    }

    if (ok_count <= 0)
    {
        GLOG_ERROR_C("! ! ! ! ! all servers of type[%s] dead or not found\n", type);
        return RET_FAILED;
    }

    return bytes_sent;
}

int connection_cache::send_to_single_connection(const char *name, const void *msg, const int msg_len)
{
    if (NULL == name ||
        NULL == msg ||
        msg_len < 0)
    {
        GLOG_ERROR_C("invalid params\n");
        return RET_FAILED;
    }

    net_conn_index *conn_index = return_as_index(name, strlen(name));
    if (NULL == conn_index)
    {
        GLOG_ERROR_C("server[%s] not found\n", name);
        return RET_FAILED;
    }

    cal::net_connection *conn_detail = conn_index->conn_detail;
    if (NULL == conn_detail)
    {
        GLOG_ERROR_C("server[%s] is in an invalid status\n", name);
        return RET_FAILED;
    }

    int fd = conn_detail->fd;
    int ret = cal::tcp_base::send_fragment(fd, msg, msg_len);
    if (ret < 0)
    {
        GLOG_ERROR_C("failed to send message to node[%s], ret = %d\n", name, ret);
        return RET_FAILED;
    }

    GLOG_INFO_C("message sent to node [name: %s, ip: %s, port: %u, fd: %d] successfully,"
        " expected bytes = %d, actual bytes = %d\n",
        name, conn_index->peer_ip, conn_index->peer_port, fd, msg_len, ret);

    return ret;
}

bool connection_cache::is_accessible(const net_conn_attr &target, const net_conn_attr &visitor)
{
    /*bool group_checking_passed = (0 == strcasecmp(target.belonging_group, GROUP_TYPE_ALL)) ||
        (0 == strcasecmp(visitor.belonging_group, GROUP_TYPE_ADMIN)) ||
        (0 == strcasecmp(visitor.belonging_group, target.belonging_group));

    bool env_checking_passed = (0 == strcasecmp(target.environment, ENV_TYPE_ALL)) ||
        (0 == strcasecmp(visitor.environment, ENV_TYPE_ALL)) ||
        (0 == strcasecmp(visitor.environment, target.environment));

    bool market_checking_passed = (0 == strcasecmp(target.market, MARKET_TYPE_ALL)) ||
        (0 == strcasecmp(visitor.market, MARKET_TYPE_ALL)) ||
        (0 == strcasecmp(visitor.market, target.market));

    bool exchange_checking_passed = (0 == strcasecmp(target.exchange, EXCHANGE_TYPE_ALL)) ||
        (0 == strcasecmp(visitor.exchange, EXCHANGE_TYPE_ALL)) ||
        (0 == strcasecmp(visitor.exchange, target.exchange));

    GLOG_DEBUG_CS(ConnectionCache, "target attribute: %s:%s:%s:%s, visitor attribute: %s:%s:%s:%s, "
        "group_checking_passed = %d, env_checking_passed = %d, market_checking_passed = %d, "
        "exchange_checking_passed = %d\n", target.belonging_group, target.environment,
        target.market, target.exchange, visitor.belonging_group, visitor.environment,
        visitor.market, visitor.exchange, group_checking_passed, env_checking_passed,
        market_checking_passed, exchange_checking_passed);

    return (group_checking_passed &&
        env_checking_passed &&
        market_checking_passed &&
        exchange_checking_passed);*/
    return true;
}

int connection_cache::attribute_to_string(const net_conn_attr &attr, std::string &str)
{
    std::stringstream formated_str;
    const char *is_master_str = attr.is_master ? "master" : "slave";

    formated_str /*<< attr.belonging_group << ":"
        << attr.environment << ":"
        << attr.market << ":"
        << attr.exchange << ":"*/
        << is_master_str;
    str = formated_str.str();

    return RET_OK;
}

int connection_cache::string_to_attribute(const std::string &str, net_conn_attr &attr)
{
    /*if (str.empty())
    {
        GLOG_ERROR_CS(ConnectionCache, "empty source string\n");
        return RET_FAILED;
    }

    std::vector<std::string> fragments;
    int split_ret = cal::str::split(str.c_str(), str.length(), ":", fragments);

    if ((split_ret < 0) ||
        (fragments.size() < 4))
    {
        GLOG_ERROR_CS(ConnectionCache, "cal::StringHelper::Split() failed, or source string is not"
            " correctly formatted, split ret = %d, source string = %s\n",
            split_ret, str.c_str());
        return RET_FAILED;
    }

    if (fragments.size() > 4)
    {
        if (0 == strcasecmp(fragments[4].c_str(), MASTER_STRING))
            attr.is_master = true;
        else
            attr.is_master = false;
    }
    else*/
        attr.is_master = true;

    return RET_OK;
}

}
