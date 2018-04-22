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
//#include "program_template/app_tools/db_auxiliary.h"
//#include "program_template/business_tools/message_cache.h"
#include "config_manager.h"

cafw::thread_context *g_thread_contexts = NULL;

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
        GLOG_ERROR_C("failed to prepare %s resources\n", name);\
        goto PREPARATION_FAILED;\
    }\
    GQ_LOG_INFO_C("%s resources prepared successfully\n", name)

int resource_manager::prepare(const void *condition)
{
#ifdef HAS_CONFIG_FILES // Complex resources need info from configuration files.
    PREPARE_RESOURCE_ITEM(__prepare_loggers, "logger");
    PREPARE_RESOURCE_ITEM(__prepare_network, "network");
    PREPARE_RESOURCE_ITEM(__prepare_database, "database");
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
        GLOG_ERROR_C("Resource structure initialization failed\n");
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
        __detach_from_database();
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
        GLOG_ERROR_C("unknown terminal logger level: %s\n", term_level_str.c_str());
        return RET_FAILED;
    }

    int term_level = level_definitions[term_level_str];
    int file_level = term_level;

    bool enables_file_logger = private_configs.file_log_enabled;
    const char *log_dir = private_configs.log_directory.c_str();
    const char *log_name = private_configs.basic_log_name.c_str();

    if (enables_file_logger)
    {
        std::string &file_level_str = private_configs.file_log_level;

        if (level_definitions.end() == level_definitions.find(file_level_str))
        {
            GLOG_ERROR_C("unknown file logger level: %s\n", file_level_str.c_str());
            return RET_FAILED;
        }
        file_level = level_definitions[file_level_str];
    }
    GLOG_DEBUG_C("enables_file_logger: %d, log_dir: %s, log_name: %s\n",
        enables_file_logger, log_dir, log_name);

    return init_global_logger(term_level, enables_file_logger, file_level, log_dir, log_name);
}

int resource_manager::__prepare_network(const void *condition)
{
#if defined(ACCEPTS_CLIENTS)
    int ret = -1;
#endif
    config_content_t *config = (config_content_t*)condition;
    const net_node_config &self_info = config->private_configs.self;
    const char *self_node_name = self_info.node_name.c_str();

    cal::tcp_client *client_requester = NULL;
    cal::tcp_server *server_listener = NULL;

    struct TcpManagerInfo
    {
        cal::tcp_base **pptr;
        bool is_server;
        const char *name;
    } tcp_mgr_item[] = {
        { (cal::tcp_base **)&(m_resource->server_listener), true, "server listener" },
        { (cal::tcp_base **)&(m_resource->client_requester), false, "client requester" }
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

        cal::tcp_base *mgr = mgr_item.is_server
            ? ((cal::tcp_base *)(new cal::tcp_server))
            : ((cal::tcp_base *)(new cal::tcp_client));

        if (NULL == (*(mgr_item.pptr) = mgr))
        {
            GLOG_ERROR_C("%s initialization failed\n", mgr_item.name);
            goto PREPARATION_FAILED;
        }

        GQ_LOG_INFO_C("%s initialization successful\n", mgr_item.name);
    }

    client_requester = m_resource->client_requester;
    client_requester->set_self_name(self_node_name);
    if (!is_quiet_mode())
        GLOG_INFO_C("client requester name set as %s\n", client_requester->self_name());

#if defined(ACCEPTS_CLIENTS)
    server_listener = m_resource->server_listener;
    server_listener->set_self_name(self_node_name);
    if (!is_quiet_mode())
        GLOG_INFO_C("server listener name set as %s\n", server_listener->self_name());

    if ((ret = server_listener->start(self_info.node_ip.c_str(), self_info.node_port)) < 0)
    {
        GLOG_ERROR_C("failed to start server listener, ret = %d\n", ret);
        goto PREPARATION_FAILED;
    }
    GQ_LOG_INFO_C("server listener has been started successfully, address = %s:%u\n",
        server_listener->listening_ip(), server_listener->listening_port());
#endif

    for (size_t i = 0; i < sizeof(conn_cache_items) / sizeof(struct ConnCacheInfo); ++i)
    {
        struct ConnCacheInfo &cache_item = conn_cache_items[i];

        if ((NULL == *(cache_item.pptr)) &&
            (NULL == (*(cache_item.pptr) = new connection_cache)))
        {
            GLOG_ERROR_C("%s initialization failed\n", cache_item.name);
            goto PREPARATION_FAILED;
        }

        GQ_LOG_INFO_C("%s initialization successful\n", cache_item.name);

        if ((*(cache_item.pptr))->create())
        {
            GLOG_ERROR_C("%s creation failed\n", cache_item.name);
            goto PREPARATION_FAILED;
        }

        GQ_LOG_INFO_C("%s creation successful\n", cache_item.name);
    }

    if (NULL == condition)
    {
        GLOG_INFO_C("no more network resources needed to be initialized\n");
        return RET_OK;
    }

    upstream_server_count = config->private_configs.upstream_servers.size();

    if (upstream_server_count > 0 && !is_quiet_mode())
        GLOG_INFO_C("filling connection cache ...\n");

    for (size_t i = 0; i < upstream_server_count; ++i)
    {
        const net_node_config &server = config->private_configs.upstream_servers[i];
        const char *name = server.node_name.c_str();

        if ((0 == server.node_ip.compare(server_listener->listening_ip()))
            && server.node_port == server_listener->listening_port())
        {
            GLOG_WARN_C("node address collides with server listening address, skip it:"
                " node name = %s, address = %s:%u\n", name, server.node_ip.c_str(),
                server.node_port);
            continue;
        }

        const char *type = server.type_name.c_str();
        const char *alias = server.node_alias.c_str();

        net_conn_index *conn_index = char_dict_alloc_val< net_conn_index >();
        if (NULL == conn_index)
        {
            GLOG_ERROR_C("char_dict_alloc_val() for %s failed\n", name);
            return RET_FAILED;
        }

        net_conn_attr node_attributes;

        if (connection_cache::string_to_attribute(server.attributes, node_attributes) < 0)
        {
            GLOG_ERROR_C("invalid attribution[%s] of server[%s]\n", server.attributes.c_str(), name);
            char_dict_release_val(&conn_index);
            return RET_FAILED;
        }

        net_conn_attr *attribute = &(conn_index->attribute);
        connection_cache *conn_cache = node_attributes.is_master ? *(conn_cache_items[0].pptr) : *(conn_cache_items[1].pptr);

        conn_index->is_server = true;
        conn_index->fd = cal::INVALID_SOCK_FD;
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

        //GLOG_INFO_C("node name before ConnectionCache::Add(): %s, name length: %d\n", name, strlen(name));
        if (RET_OK != conn_cache->add(name, strlen(name), conn_index, sizeof(net_conn_index)))
        {
            GLOG_ERROR_C("ConnectionCache::Add() for %s failed\n", name);
            char_dict_release_val(&conn_index);
            return RET_FAILED;
        }
        //GLOG_INFO_C("%s added into connection cache\n", serv_name);
    }

    if (!is_quiet_mode())
    {
        for (size_t i = 0; i < sizeof(conn_cache_items) / sizeof(struct ConnCacheInfo); ++i)
        {
            struct ConnCacheInfo &cache_item = conn_cache_items[i];

            GLOG_INFO_C("---- %s:\n", cache_item.name);
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
    cal::tcp_base **tcp_managers[] = {
        (cal::tcp_base **)&(m_resource->server_listener),
        (cal::tcp_base **)&(m_resource->client_requester)
    };

    if (NULL != m_resource->server_listener)
        m_resource->server_listener->end();

    for (size_t i = 0; i < sizeof(tcp_managers) / sizeof(cal::tcp_base**); ++i)
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

int resource_manager::__prepare_database(const void *condition)
{
#ifdef HAS_DATABASE
    if (NULL == config)
    {
        LOG_INFO("no database resources needed to be initialized\n");
        return RET_OK;
    }

    std::string &db_dsn = const_cast<std::string &>(config->private_configs.db_dsn);
    const char *encrypted_dsn = db_dsn.c_str();
    int encrypted_len = db_dsn.length();
    char decrypted_dsn[128] = {0};
    int decrypted_len = sizeof(decrypted_dsn);
    int trade_mode = -1;

    //LOG_DEBUG_CV("src: dsn = %s, len = %d\n", encrypted_dsn, encrypted_len);

    if (db_initialize() < 0)
    {
        LOG_ERROR_CV("db_initialize() failed\n");
        goto PREPARATION_FAILED;
    }
    LOG_INFO_CV("database initialization successful\n");

    if (ts_db_dec(encrypted_dsn, encrypted_len, decrypted_dsn, &decrypted_len) < 0)
    {
        LOG_ERROR_CV("failed to decrypt database DSN\n");
        goto PREPARATION_FAILED;
    }
    //LOG_DEBUG_CV("dst: dsn = %s, len = %d\n", decrypted_dsn, decrypted_len);
    LOG_INFO_CV("DSN decryption successful\n");

    db_dsn = decrypted_dsn;

    if (db_login(db_dsn.c_str()) < 0)
    {
        LOG_ERROR_CV("failed to login to database\n");
        goto PREPARATION_FAILED;
    }
    LOG_INFO_CV("login successful\n");

    m_resource->db_connection = &g_db_conn;

    if ((trade_mode = db_query_trade_mode()) < 0)
    {
        LOG_ERROR_CV("failed to get trade mode\n");
        goto PREPARATION_FAILED;
    }
    db_set_trade_mode(trade_mode);
    LOG_INFO_CV("trade mode is %d and is loaded into memory\n", trade_mode);

    return RET_OK;

PREPARATION_FAILED:

    __clear();

    return RET_FAILED;
#else
    m_resource->db_connection = NULL;
    GQ_LOG_INFO_C("no database resources\n");
    return RET_OK;
#endif
}

void resource_manager::__detach_from_database(void)
{
    if (NULL == m_resource->db_connection)
        return;

    //db_logoff();

    m_resource->db_connection = NULL;
}

int resource_manager::__prepare_extra_resource(const void *condition)
{
    return prepare_extra_resource(condition, m_resource->extra_resource);
}

void resource_manager::__release_extra_resource(void)
{
    release_extra_resource(&(m_resource->extra_resource));
}

int prepare_thread_resource(void)
{
    // TODO: how to get these parameters when there is no configuration file?
    const int kThreadCount = CFG_GET_COUNTER(XNODE_WORKER_THREAD);
    const int kInputBufSize = CFG_GET_COUNTER(XNODE_TCP_RECV_BUF);
    const int kOutputBufSize = CFG_GET_COUNTER(XNODE_TCP_SEND_BUF);

    if (kThreadCount <= 0)
    {
        GLOG_ERROR_NS("cafw", "invalid worker-thread count: <= 0\n");
        return RET_FAILED;
    }

    g_thread_contexts = new thread_context[kThreadCount];

    for (int i = 0; i < kThreadCount; ++i)
    {
        thread_context &ctx = g_thread_contexts[i];
        char thread_num[8] = {0};

        ctx.num = i;
        ctx.type = thread_context::TYPE_WORKER;
        ctx.name = g_file_logger->log_name();
        snprintf(thread_num, sizeof(thread_num), "%04d", ctx.num);
        ctx.name.append((thread_context::TYPE_WORKER == ctx.type) ? "_worker_" : "_supervisor_")
            .append(thread_num);
        pthread_mutex_init(&(ctx.lock), NULL);
        ctx.screen_logger.set_log_level(g_screen_logger->log_level());
        ctx.file_logger.set_log_level(g_file_logger->log_level());
        ctx.file_logger.set_log_directory(g_file_logger->log_directory());
        ctx.file_logger.set_log_name(ctx.name.c_str());
        ctx.file_logger.open();
        ctx.status = thread_context::STATUS_BASIC_PART_INITIALIZED;
        ctx.should_exit = false;
        memset(&(ctx.timed_task_refresh_times), 0, sizeof(ctx.timed_task_refresh_times));
        ctx.input_packet_cache = new cal::buffer(kInputBufSize);
        ctx.output_packet_cache = new cal::buffer(kOutputBufSize);

#ifdef HAS_DATABASE
        // TODO: ctx.resource.db_connection;
#endif
    }

    return RET_OK;

/*PREPARATION_FAILED:

    release_thread_resource();

    return RET_FAILED;*/
}

void release_thread_resource(void)
{
    if (NULL == g_thread_contexts)
        return;

    const int kThreadCount = CFG_GET_COUNTER(XNODE_WORKER_THREAD);

    int wait_secs = 0;

    while (wait_secs < THREAD_TERMINATION_TIMEOUT_SECS)
    {
        bool all_exited_normally = true;

        for (int i = 0; i < kThreadCount; ++i)
        {
            if (thread_context::STATUS_EXITED_NORMALLY != g_thread_contexts[i].status)
            {
                all_exited_normally = false;
                break;
            }
        }

        if (all_exited_normally)
        {
            GLOG_INFO_NS("cafw", "all worker threads have exited normally, wait_secs = %d\n", wait_secs);
            break;
        }

        GLOG_INFO_NS("cafw", "waiting for worker threads exiting, wait_secs = %d\n", wait_secs);
        sleep(1);
        ++wait_secs;
    }

    for (int i = 0; i < kThreadCount; ++i)
    {
        thread_context &ctx = g_thread_contexts[i];

        if (NULL != ctx.input_packet_cache)
        {
            delete ctx.input_packet_cache;
            ctx.input_packet_cache = NULL;
        }

        if (NULL != ctx.output_packet_cache)
        {
            delete ctx.output_packet_cache;
            ctx.output_packet_cache = NULL;
        }

#ifdef HAS_DATABASE
        // TODO: ctx.resource.db_connection;
#endif
    }

    delete[] g_thread_contexts;
    g_thread_contexts = NULL;
}

} // namespace cafw
