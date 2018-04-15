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

/*
 * connection_cache.h
 *
 *  Created on: 2015-06-01
 *      Author: wenxiongchang
 * Description: for caching network connection data
 */

#ifndef __CASDK_FRAMEWORK_CONNECTION_CACHE_H__
#define __CASDK_FRAMEWORK_CONNECTION_CACHE_H__

#include <string.h>

#include <map>

#include <cpp_assistant/ca_full.h>
#include "char_dictionary.h"

namespace cafw
{

enum
{
    DEFAULT_CONN_BUF_SIZE = 1024 * 16
};

typedef struct net_conn_attr
{
    bool is_master; // true: as a master node; false: as a slave node
    // TODO: more ...
}net_conn_attr;

typedef struct net_conn_index
{
    char server_type[64];
    bool is_server;
    char conn_alias[cal::MAX_CONNECTION_NAME_LEN + 1];
    char peer_ip[cal::IPV4_LEN];
    uint16_t peer_port;
    cal::net_connection *conn_detail;
    int fd;
    net_conn_attr attribute;
}net_conn_index;

typedef std::multimap<std::string, std::string> server_group; // <server type, server name>
typedef server_group::iterator server_group_iter;

class connection_cache
{
/* ===================================
 * constructors:
 * =================================== */
public:
    connection_cache();
    connection_cache(int dict_size);

/* ===================================
 * copy control:
 * =================================== */
private:
    connection_cache(const connection_cache& src);
    connection_cache& operator=(const connection_cache& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~connection_cache();

/* ===================================
 * types:
 * =================================== */
public:
    enum enum_dict_size
    {
        DEFAULT_DICT_SIZE = 1024
    };

    enum enum_send_policy
    {
        SEND_POLICY_RANDOMLY = 0,
        SEND_POLICY_BY_ID,
        SEND_POLICY_TO_LEAST_LOAD
    };

/* ===================================
 * abilities:
 * =================================== */
public:
    int create(int dict_size = DEFAULT_DICT_SIZE);
    void destroy(void);
    int add(const char *name, const int name_len, const net_conn_index *conn, const int conn_len);
    int del(const char *name, const int name_len);
    net_conn_index* return_as_index(const char *name, const int name_len);
    dict_entry_ptr return_as_entry(const char *name, const int name_len);
    net_conn_index* pick_one_connection(const char *type, int policy, int route_id, bool pick_alive_only = true);
    void do_batch_operation(action_to_char_dict_element op);
    void profile(void);
    // TODO: more operations via server type may be added in future depending on actual needs.
    int send_to_connections_by_type(const char *type,
        const int max_conn_count,
        const void *msg,
        const int msg_len,
        const bool to_all, /* to_all: 0: to one single only; 1: to all or not more than @max_conn_count nodes*/
        const int policy,
        const int id);
    int send_to_single_connection(const char *name, const void *msg, const int msg_len);
    static bool is_accessible(const net_conn_attr &target, const net_conn_attr &visitor);
    static int attribute_to_string(const net_conn_attr &attr, std::string &str);
    static int string_to_attribute(const std::string &str, net_conn_attr &attr);

/* ===================================
 * attributes:
 * =================================== */
public:
    DEFINE_CLASS_NAME_FUNC()

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
    DECLARE_CLASS_NAME_VAR();
    dict *m_connection_dictionary;
    server_group *m_type_group;
};

typedef connection_cache conn_cache;

}

#endif /* __CASDK_FRAMEWORK_CONNECTION_CACHE_H__ */
