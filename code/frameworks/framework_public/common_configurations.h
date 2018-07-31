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
 * common_configurations.h
 *
 *  Created on: 2016-11-28
 *      Author: wenxiongchang
 * Description: Some common configurations needed by many other programs.
 */

#ifndef __COMMON_CONFIGURATIONS_H__
#define __COMMON_CONFIGURATIONS_H__

#include <stdint.h>
#include <string.h>

#include <string>
#include <map>
#include <vector>

#include "base/all.h"

class TiXmlDocument;

typedef TiXmlDocument config_file_t;

namespace cafw
{

enum
{
    LOG_LEVEL_COUNT = 6
};

enum
{
    SERVER_TYPE_SELF = 0,
    SERVER_TYPE_CLIENT,
    SERVER_TYPE_CASDK_TEST_SERVER,
    SERVER_TYPE_CASDK_TEST_CLIENT
};

enum
{
    TIME_UNIT_NANOSECOND = 0,
    TIME_UNIT_MICROSECOND,
    TIME_UNIT_MILLISECOND,
    TIME_UNIT_SECOND
};

enum
{
    BUF_UNIT_BYTE = 0,
    BUF_UNIT_KB,
    BUF_UNIT_MB,
    BUF_UNIT_GB,
    BUF_UNIT_TB
};

enum
{
    DISPATCHED_RANDOMLY = 0,
    DISPATCHED_BY_ID,
    DISPATCHED_TO_LEAST_LOAD,
};

typedef struct net_node_config
{
    std::string type_name;
    int type_value;
    std::string node_name;
    std::string node_alias;
    int node_num;
    std::string node_ip; // IPv4 or IPv6
    uint16_t node_port;
    std::string attributes;
}net_node_config;

typedef struct fixed_common_config
{
    std::map<std::string, int> log_levels; // <level name, level value>
    std::map<std::string, int> node_types; // <type name, type value>
    std::map<std::string, int> dispatch_policies; // <policy name, policy value>
    std::map<std::string, int> server_types; // <type name, type value>
}fixed_common_config;

typedef struct mutable_common_config
{
    int timezone;
    int time_unit_of_timed_task;
    std::map<std::string, int64_t> timed_task_intervals;
    std::map<std::string, int64_t> timed_task_timeouts;
    int unit_of_buffer;
    std::map<std::string, int64_t> buffer_sizes;
    std::map<std::string, int64_t> counters;
    std::map<std::string, int64_t> dispatch_settings;
}mutable_common_config;

#define XPATH_COMMON_CONSTANTS_ROOT                 "/root/constants"
#define XPATH_COMMON_VARIABLES_ROOT                 "/root/variables"

#define XPATH_PRIVATE_CONFIG_ROOT                   "/root/private"
#define XPATH_LOG_CONFIG_ROOT                       XPATH_PRIVATE_CONFIG_ROOT"/log-configs"
#define XPATH_DB_CONFIG_ROOT                        XPATH_PRIVATE_CONFIG_ROOT"/db-configs"

#define XPATH_DB_CONNECTION_ITEM                    XPATH_DB_CONFIG_ROOT"/connection"

#define ABSOLUTE_XPATH_LOG_LEVEL_ITEM               XPATH_COMMON_CONSTANTS_ROOT"/log-configs/levels/item"
#define ABSOLUTE_XPATH_SERVER_TYPE_ITEM             XPATH_COMMON_CONSTANTS_ROOT"/server-types/item"

#define XNODE_TIMEZONE                              "timezone"
#define XNODE_TIMED_TASK_SETTING                    "timed-task-settings"
#define XNODE_BUF_SETTING                           "buffer-settings"
#define XNODE_COUNTER                               "counters"
#define XNODE_DISPATCH_SETTING                      "dispatch-settings"

/*
 * timed task intervals
 */

#define RELATIVE_XPATH_TIMED_TASK_INTERVAL_ROOT     XNODE_TIMED_TASK_SETTING"/intervals"

#define XNODE_MSG_CLEAN                             "message-clean"
#define XNODE_SESSION_CLEAN                         "session-clean"
#define XNODE_HEARTBEAT                             "heartbeat"
#define XNODE_LOG_FLUSHING                          "log-flushing"

#define RELATIVE_XPATH_TIMED_TASK_INTERVAL_MSG_CLEAN        RELATIVE_XPATH_TIMED_TASK_INTERVAL_ROOT "/" XNODE_MSG_CLEAN
#define RELATIVE_XPATH_TIMED_TASK_INTERVAL_SESSION_CLEAN    RELATIVE_XPATH_TIMED_TASK_INTERVAL_ROOT "/" XNODE_SESSION_CLEAN
#define RELATIVE_XPATH_TIMED_TASK_INTERVAL_HEARTBEAT        RELATIVE_XPATH_TIMED_TASK_INTERVAL_ROOT "/" XNODE_HEARTBEAT
#define RELATIVE_XPATH_TIMED_TASK_INTERVAL_LOG_FLUSHING     RELATIVE_XPATH_TIMED_TASK_INTERVAL_ROOT "/" XNODE_LOG_FLUSHING

/*
 * timed task timeouts
 */

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT      XNODE_TIMED_TASK_SETTING"/timeouts"

#define XNODE_DEFAULT_MSG_PROCESS                   "default-message-processing"
#define XNODE_MAX_MSG_PROCESS                       "max-message-processing"
#define XNODE_SESSION_KEEPING                       "session-keeping"
#define XNODE_DEFAULT_WAITING_FOR_PEER_REPLY        "default-waiting-for-peer-reply"
#define XNODE_LONGEST_WAITING_FOR_PEER_REPLY        "longest-waiting-for-peer-reply"
#define XNODE_CONNECT_TRYING                        "connect-trying"
#define XNODE_POLL_WAITING                          "poll-waiting"

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_DEFAULT_MSG_PROCESS               \
    RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT "/" XNODE_DEFAULT_MSG_PROCESS

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_MAX_MSG_PROCESS                   \
    RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT "/" XNODE_MAX_MSG_PROCESS

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_SESSION_KEEPING                   \
    RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT "/" XNODE_SESSION_KEEPING

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_DEFAULT_WAITING_FOR_PEER_REPLY    \
    RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT "/" XNODE_DEFAULT_WAITING_FOR_PEER_REPLY

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_LONGEST_WAITING_FOR_PEER_REPLY    \
    RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT "/" XNODE_LONGEST_WAITING_FOR_PEER_REPLY

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_CONNECT_TRYING                    \
    RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT "/" XNODE_CONNECT_TRYING

#define RELATIVE_XPATH_TIMED_TASK_TIMEOUT_EPOLL_WAITING                     \
    RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT "/" XNODE_EPOLL_WAITING

/*
 * buffer settings
 */

#define XNODE_NET_NODE                              "net-node"
#define XNODE_TCP_SEND_BUF                          "tcp-send"
#define XNODE_TCP_RECV_BUF                          "tcp-receive"

/*
 * counters
 */

#define XNODE_MSG_PROCESS_COUNT_PER_ROUND           "message-processing-per-round"
#define XNODE_FORWARD_RETRIES_ON_FAILURE            "forward-retries-on-failure"
#define XNODE_WORKER_THREAD                         "worker-thread"

/*
 * dispatch relative items
 */

#define XNODE_DISPATCH_POLICY                       "policy"

template<typename T>
int convert(const std::string &original, T &result)
{
    return calns::str::from_string(original, result);
}

template<>
inline int convert(const std::string &original, std::string &result)
{
    result = original;

    return RET_OK;
}

/*
 * A template function that reads the value of the node with specific XPath,
 * and this node should exist and be unique.
 * Note: Parameter type Key should be string, int or int64,
 *     and Value should be string, int, int64, float or double.
 */
template<typename Value>
int load_unique_config_node_value(
    const config_file_t *config_file,
    const char *xpath,
    const bool from_common,
    Value &result)
{
    std::vector<calns::xml::node_t> nodes;
    int read_ret = calns::xml::find_and_parse_nodes(*config_file, xpath, 1, nodes);

    if (read_ret <= 0)
    {
        if (!from_common)
            QLOGF_NS(W, "cafw", "Failed to read nodes with XPath[%s]: %s\n", xpath, calns::what(read_ret).c_str());
        return RET_FAILED;
    }

    std::string &value = nodes[0].node_value;

    QLOGF_NS(I, "cafw", "%s: %s\n", xpath, value.c_str());

    convert(value, result);

    return RET_OK;
}

int get_common_config_filename(const config_file_t *config_file, std::string &result);

int load_fixed_common_configs(
    const config_file_t *config_file,
    fixed_common_config *result);

int load_mutable_common_configs(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    mutable_common_config *result);

int load_common_configurations(
    const char *config_file,
    fixed_common_config *fixed_part,
    mutable_common_config *mutable_part);

}

#endif /* __COMMON_CONFIGURATIONS_H__ */
