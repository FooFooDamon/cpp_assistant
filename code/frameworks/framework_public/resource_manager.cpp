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

#include "resource_manager.h"

#include <string.h>

#include "connection_cache.h"
#include "config_manager.h"

extern int prepare_extra_resource(const void *condition, struct extra_resource_t *target);
extern void release_extra_resource(struct extra_resource_t **target);

namespace cafw
{

resource_manager::resource_manager()
    : m_resource(NULL),
    m_is_ready(false)
{
    __inner_init();
}

resource_manager::~resource_manager()
{
    __clear();
}

#define PREPARE_RESOURCE_ITEM(func, name)   if (func(condition) < 0){\
        LOGF_C(E, "failed to prepare %s resources\n", name);\
        goto PREPARATION_FAILED;\
    }\
    QLOGF_C(I, "%s resources prepared successfully\n", name)

int resource_manager::prepare(const void *condition)
{
#ifdef HAS_CONFIG_FILES // Complex resources need info from configuration files.
    PREPARE_RESOURCE_ITEM(__prepare_loggers, "logger");
    PREPARE_RESOURCE_ITEM(__prepare_network, "network");
#endif
    PREPARE_RESOURCE_ITEM(__prepare_extra_resource, "extra resource");

    m_is_ready = true;

    return RET_OK;

PREPARATION_FAILED:

    __clear();

    return RET_FAILED;
}

void resource_manager::clean(void)
{
    __clear();
}

const bool resource_manager::is_available(void) const
{
    return (NULL != m_resource);
}

const bool resource_manager::is_ready(void) const
{
    return m_is_ready;
}

int resource_manager::__inner_init(void)
{
    if ((NULL == m_resource) &&
        (NULL == (m_resource = new resource_t)))
    {
        LOGF_C(E, "Resource structure initialization failed\n");
        return RET_FAILED;
    }
    // Do not do this if it contains complex non-pointer objects.
    memset(m_resource, 0, sizeof(resource_t));

    m_is_ready = false;

    return RET_OK;
}

void resource_manager::__clear(void)
{
    if (NULL != m_resource)
    {
#ifdef HAS_CONFIG_FILES
        __release_network();
#endif
        if (NULL != m_resource->extra_resource)
            __release_extra_resource();

        delete m_resource;
        m_resource = NULL;
    }

    m_is_ready = false;
}

int resource_manager::__prepare_loggers(const void *condition)
{
    config_content_t *config = (config_content_t*)condition;
    std::map<std::string, int> &level_definitions = config->fixed_common_configs.log_levels;
    private_config &private_configs = config->private_configs;

    std::string &term_level_str = private_configs.terminal_log_level;

    if (level_definitions.end() == level_definitions.find(term_level_str))
    {
        LOGF_C(E, "unknown terminal logger level: %s\n", term_level_str.c_str());
        return RET_FAILED;
    }

    int term_level = level_definitions[term_level_str];
    int file_level = term_level;

    bool enables_file_logger = private_configs.file_log_enabled;
    const char *log_dir = private_configs.log_directory.c_str();
    const char *log_name = private_configs.basic_log_name.c_str();

    if (calns::daemon::is_daemonized() || enables_file_logger)
    {
        std::string &file_level_str = private_configs.file_log_level;

        if (level_definitions.end() == level_definitions.find(file_level_str))
        {
            LOGF_C(E, "unknown file logger level: %s\n", file_level_str.c_str());
            return RET_FAILED;
        }
        file_level = level_definitions[file_level_str];
    }
    LOGF_C(D, "enables_file_logger: %d, log_dir: %s, log_name: %s\n",
        enables_file_logger, log_dir, log_name);

    return init_logger(term_level, enables_file_logger, file_level, log_dir, log_name);
}

int resource_manager::__prepare_network(const void *condition)
{
#if defined(ACCEPTS_CLIENTS)
    int ret = -1;
#endif
    config_content_t *config = (config_content_t*)condition;
    const net_node_config &self_info = config->private_configs.self;
    const char *self_node_name = self_info.node_name.c_str();

    calns::tcp_client *client_requester = NULL;
    calns::tcp_server *server_listener = NULL;

    struct TcpManagerInfo
    {
        calns::tcp_base **pptr;
        bool is_server;
        const char *name;
    } tcp_mgr_item[] = {
        { (calns::tcp_base **)&(m_resource->server_listener), true, "server listener" },
        { (calns::tcp_base **)&(m_resource->client_requester), false, "client requester" }
    };

    struct ConnCacheInfo
    {
        connection_cache **pptr;
        const char *name;
    } conn_cache_items[] = {
        { &(m_resource->master_connection_cache), "master connection cache"},
        { &(m_resource->slave_connection_cache), "slave connection cache"}
    };

    size_t upstream_server_count;

    for (size_t i = 0; i < sizeof(tcp_mgr_item) / sizeof(struct TcpManagerInfo); ++i)
    {
        struct TcpManagerInfo &mgr_item = tcp_mgr_item[i];

        if (NULL != *(mgr_item.pptr))
            continue;

        calns::tcp_base *mgr = mgr_item.is_server
            ? ((calns::tcp_base *)(new calns::tcp_server))
            : ((calns::tcp_base *)(new calns::tcp_client));

        if (NULL == (*(mgr_item.pptr) = mgr))
        {
            LOGF_C(E, "%s initialization failed\n", mgr_item.name);
            goto PREPARATION_FAILED;
        }

        QLOGF_C(I, "%s initialization successful\n", mgr_item.name);
    }

    client_requester = m_resource->client_requester;
    client_requester->set_self_name(self_node_name);
    if (!is_quiet_mode())
        LOGF_C(I, "client requester name set as %s\n", client_requester->self_name());

#if defined(ACCEPTS_CLIENTS)
    server_listener = m_resource->server_listener;
    server_listener->set_self_name(self_node_name);
    if (!is_quiet_mode())
        LOGF_C(I, "server listener name set as %s\n", server_listener->self_name());

    if ((ret = server_listener->start(self_info.node_ip.c_str(), self_info.node_port)) < 0)
    {
        LOGF_C(E, "failed to start server listener, ret = %d\n", ret);
        goto PREPARATION_FAILED;
    }
    QLOGF_C(I, "server listener has been started successfully, address = %s:%u\n",
        server_listener->listening_ip(), server_listener->listening_port());
#endif

    for (size_t i = 0; i < sizeof(conn_cache_items) / sizeof(struct ConnCacheInfo); ++i)
    {
        struct ConnCacheInfo &cache_item = conn_cache_items[i];

        if ((NULL == *(cache_item.pptr)) &&
            (NULL == (*(cache_item.pptr) = new connection_cache)))
        {
            LOGF_C(E, "%s initialization failed\n", cache_item.name);
            goto PREPARATION_FAILED;
        }

        QLOGF_C(I, "%s initialization successful\n", cache_item.name);

        if ((*(cache_item.pptr))->create())
        {
            LOGF_C(E, "%s creation failed\n", cache_item.name);
            goto PREPARATION_FAILED;
        }

        QLOGF_C(I, "%s creation successful\n", cache_item.name);
    }

    if (NULL == condition)
    {
        LOGF_C(I, "no more network resources needed to be initialized\n");
        return RET_OK;
    }

    upstream_server_count = config->private_configs.upstream_servers.size();

    if (upstream_server_count > 0 && !is_quiet_mode())
        LOGF_C(I, "filling connection cache ...\n");

    for (size_t i = 0; i < upstream_server_count; ++i)
    {
        const net_node_config &server = config->private_configs.upstream_servers[i];
        const char *name = server.node_name.c_str();

        if ((0 == server.node_ip.compare(server_listener->listening_ip()))
            && server.node_port == server_listener->listening_port())
        {
            LOGF_C(W, "node address collides with server listening address, skip it:"
                " node name = %s, address = %s:%u\n", name, server.node_ip.c_str(),
                server.node_port);
            continue;
        }

        const char *type = server.type_name.c_str();
        const char *alias = server.node_alias.c_str();

        net_conn_index *conn_index = char_dict_alloc_val< net_conn_index >();
        if (NULL == conn_index)
        {
            LOGF_C(E, "char_dict_alloc_val() for %s failed\n", name);
            return RET_FAILED;
        }

        net_conn_attr node_attributes;

        if (connection_cache::string_to_attribute(server.attributes, node_attributes) < 0)
        {
            LOGF_C(E, "invalid attribution[%s] of server[%s]\n", server.attributes.c_str(), name);
            char_dict_release_val(&conn_index);
            return RET_FAILED;
        }

        net_conn_attr *attribute = &(conn_index->attribute);
        connection_cache *conn_cache = node_attributes.is_master ? *(conn_cache_items[0].pptr) : *(conn_cache_items[1].pptr);

        conn_index->is_server = true;
        conn_index->fd = calns::INVALID_SOCK_FD;
        conn_index->conn_detail = NULL;
        memset(conn_index->server_type, 0, sizeof(conn_index->server_type));
        strncpy(conn_index->server_type, type, sizeof(conn_index->server_type) - 1);
        memset(conn_index->conn_alias, 0, sizeof(conn_index->conn_alias));
        strncpy(conn_index->conn_alias, alias, sizeof(conn_index->conn_alias) - 1);
        memset(attribute, 0, sizeof(conn_index->attribute));
        attribute->is_master = node_attributes.is_master;
        memset(conn_index->peer_ip, 0, sizeof(conn_index->peer_ip));
        server.node_ip.copy(conn_index->peer_ip, sizeof(conn_index->peer_ip));
        conn_index->peer_port = server.node_port;

        //LOGF_C(I, "node name before ConnectionCache::Add(): %s, name length: %d\n", name, strlen(name));
        if (RET_OK != conn_cache->add(name, strlen(name), conn_index, sizeof(net_conn_index)))
        {
            LOGF_C(E, "ConnectionCache::Add() for %s failed\n", name);
            char_dict_release_val(&conn_index);
            return RET_FAILED;
        }
        //LOGF_C(I, "%s added into connection cache\n", serv_name);
    }

    if (!is_quiet_mode())
    {
        for (size_t i = 0; i < sizeof(conn_cache_items) / sizeof(struct ConnCacheInfo); ++i)
        {
            struct ConnCacheInfo &cache_item = conn_cache_items[i];

            LOGF_C(I, "---- %s:\n", cache_item.name);
            (*(cache_item.pptr))->profile();
        }
    }

    return RET_OK;

PREPARATION_FAILED:

    __clear();

    return RET_FAILED;
}

void resource_manager::__release_network(void)
{
    calns::tcp_base **tcp_managers[] = {
        (calns::tcp_base **)&(m_resource->server_listener),
        (calns::tcp_base **)&(m_resource->client_requester)
    };

    if (NULL != m_resource->server_listener)
        m_resource->server_listener->end();

    for (size_t i = 0; i < sizeof(tcp_managers) / sizeof(calns::tcp_base**); ++i)
    {
        if (NULL != *(tcp_managers[i]))
        {
            delete *(tcp_managers[i]);
            *(tcp_managers[i]) = NULL;
        }
    }

    connection_cache **conn_caches[] = {
        &(m_resource->master_connection_cache),
        &(m_resource->slave_connection_cache)
    };

    for (size_t i = 0; i < sizeof(conn_caches) / sizeof(connection_cache**); ++i)
    {
        if (NULL != *(conn_caches[i]))
        {
            delete *(conn_caches[i]);
            *(conn_caches[i]) = NULL;
        }
    }
}

int resource_manager::__prepare_extra_resource(const void *condition)
{
    return prepare_extra_resource(condition, m_resource->extra_resource);
}

void resource_manager::__release_extra_resource(void)
{
    release_extra_resource(&(m_resource->extra_resource));
}

} // namespace cafw
