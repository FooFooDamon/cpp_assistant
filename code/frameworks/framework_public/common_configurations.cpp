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

#include "common_configurations.h"

namespace cafw
{

/*
 * A template function that reads the key-value-relation attributes of nodes with specific XPath.
 * Note: Parameter type Key should be string, int or int64,
 *     and Value should be string, int, int64, float or double.
 */
template<typename Key, typename Value>
int __load_key_value_attributes(
    const config_file_t *config_file,
    const char *xpath,
    const char *key_attr,
    const char *value_attr,
    int node_count_hint,
    std::map<Key, Value> &result)
{
    result.clear();

    std::vector<cal::xml::config_node> nodes;
    int read_ret = cal::xml::read_config_nodes(*config_file, xpath, node_count_hint,
        nodes, false, key_attr, value_attr, NULL);

    if (read_ret <= 0)
    {
        GLOG_WARN_NS("cafw", "Failed to read nodes with XPath[%s], or failed to read"
            " their attributes with name [%s] or [%s], ret = %d\n",
            xpath, key_attr, value_attr, read_ret);
        return RET_FAILED;
    }

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        cal::xml::config_node &node = nodes[i];
        const std::string &key = node.attributes[key_attr];
        const std::string value = node.attributes[value_attr];

        Key key_converted;
        Value value_converted;

        convert(key, key_converted);
        convert(value, value_converted);

        result.insert(std::make_pair(key_converted, value_converted));

        GQ_LOG_INFO_NS("cafw", "%s: %s[%s], %s[%s]\n",
            xpath, key_attr, key.c_str(), value_attr, value.c_str());
    }

    return RET_OK;
}

int get_common_config_filename(const config_file_t *config_file, std::string &result)
{
    result.clear();

    const char *xpath_shared_file = "/root/shared";
    const char *file_link_attr = "ref";
    std::vector<cal::xml::config_node> nodes;
    int read_ret = cal::xml::read_config_nodes(*config_file, xpath_shared_file, 1,
        nodes, false, file_link_attr, NULL);

    if (read_ret <= 0)
    {
        GLOG_ERROR_NS("cafw", "Failed to read node[%s] or its attribute[%s], ret = %d\n",
            xpath_shared_file, file_link_attr, read_ret);
        return RET_FAILED;
    }

    result = nodes[0].attributes[file_link_attr];

    return RET_OK;
}

static int __load_log_levels(
    const config_file_t *config_file,
    std::map<std::string, int> &result)
{
    return __load_key_value_attributes(
        config_file,
        XPATH_COMMON_CONSTANTS_ROOT"/log-configs/levels/item",
        "name",
        "value",
        LOG_LEVEL_COUNT,
        result);
}

static int __load_node_types(
    const config_file_t *config_file,
    std::map<std::string, int> &result)
{
    return __load_key_value_attributes(
        config_file,
        XPATH_COMMON_CONSTANTS_ROOT"/master-types/item",
        "name",
        "value",
        100,
        result);
}

static int __load_dispatch_policies(
    const config_file_t *config_file,
    std::map<std::string, int> &result)
{
    return __load_key_value_attributes(
        config_file,
        XPATH_COMMON_CONSTANTS_ROOT"/dispatch-policies/item",
        "name",
        "value",
        100,
        result);
}

static int __load_server_types(
    const config_file_t *config_file,
    std::map<std::string, int> &result)
{
    return __load_key_value_attributes(
        config_file,
        XPATH_COMMON_CONSTANTS_ROOT"/server-types/item",
        "name",
        "value",
        100,
        result);
}

int load_fixed_common_configs(
    const config_file_t *config_file,
    fixed_common_config *result)
{
    if (NULL == config_file
        || NULL == result)
    {
        GLOG_ERROR_NS("cafw", "null params\n");
        return RET_FAILED;
    }

    struct config_item
    {
        const char *name;
        std::map<std::string, int> &holder;
        typedef int (*load_func)(const config_file_t *, std::map<std::string, int> &);
        load_func load_function;
    } config_items[] = {
        { "log level predefinitions", result->log_levels, __load_log_levels },
        { "node type predefinitions", result->node_types, __load_node_types },
        { "dispatch policy predefinitions", result->dispatch_policies, __load_dispatch_policies },
        { "server type predefinitions", result->server_types, __load_server_types }
    };

    for (size_t i = 0; i < sizeof(config_items) / sizeof(struct config_item); ++i)
    {
        if (config_items[i].load_function(config_file, config_items[i].holder) < 0)
        {
            GLOG_ERROR_NS("cafw", "failed to load %s\n", config_items[i].name);
            return RET_FAILED;
        }
    }

    return RET_OK;
}

static int __load_timezone(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    int &timezone)
{
    std::string xpath(xpath_prefix);
    std::vector<TiXmlElement> timezones;

    xpath.append("/"XNODE_TIMEZONE);

    int load_ret = load_unique_config_node_value(config_file, xpath.c_str(), from_common, timezone);

    if (load_ret < 0)
    {
        if (from_common)
        {
            GLOG_ERROR_NS("cafw", "can not find time zone setting in common configuration.\n");
            return RET_FAILED;
        }

        GQ_LOG_INFO_NS("cafw", "can not find time zone setting in private configuration,"
            " take the value from common configuration.\n");
    }

    return RET_OK;
}

static int __load_timed_task_unit(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    int &unit)
{
    std::string xpath(xpath_prefix);
    const char *unit_attr = "unit";

    xpath.append("/"XNODE_TIMED_TASK_SETTING);

    std::vector<cal::xml::config_node> nodes;
    int read_ret = cal::xml::read_config_nodes(*config_file, xpath.c_str(), 1,
        nodes, false, unit_attr, NULL);

    if (read_ret <= 0)
    {
        if (from_common)
        {
            GLOG_ERROR_NS("cafw", "can not find unit of timed task in common configuration file.\n");
            return RET_FAILED;
        }

        GQ_LOG_INFO_NS("cafw", "can not find unit of timed task in private configuration,"
            " take the value from common configuration.\n");

        return RET_OK;
    }


    const char *unit_text = nodes[0].attributes[unit_attr].c_str();

    if (0 == strcasecmp(unit_text, "millisecond"))
    {
        GQ_LOG_INFO_NS("cafw", "time unit to be used: %s\n", unit_text);
        unit = TIME_UNIT_MILLISECOND;
    }
    else
    {
        GQ_LOG_INFO_NS("cafw", "time unit to be used: %s\n", "second");
        unit = TIME_UNIT_SECOND;
    }

    return RET_OK;
}

static int __load_integer_items(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    const char *item_parent,
    const char **item_names,
    std::map<std::string, int64_t> &result)
{
    std::string full_path;
    std::vector<TiXmlElement> items;

    if (from_common)
        result.clear();

    //GLOG_INFO_NS("cafw", "items in %s/%s:\n", xpath_prefix, item_parent);
    for (int i = 0; NULL != item_names[i]; ++i)
    {
        full_path.clear();
        full_path.append(xpath_prefix)
            .append("/")
            .append(item_parent)
            .append("/")
            .append(item_names[i]);

        int64_t value = 0;
        int read_ret = load_unique_config_node_value(config_file, full_path.c_str(), from_common, value);

        if (read_ret < 0)
        {
            if (from_common)
            {
                GLOG_ERROR_NS("cafw", "can not find %s item in common configuration.\n", full_path.c_str());
                return RET_FAILED;
            }

            GQ_LOG_INFO_NS("cafw", "can not find %s item in private configuration,"
                " take the value from common configuration.\n", full_path.c_str());

            continue;
        }

        result[item_names[i]] = value;

        //GLOG_INFO_NS("cafw", "%s = %ld\n", item_names[i], result[item_names[i]]);
    }

    return RET_OK;
}

static int __load_timed_task_timeouts(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    std::map<std::string, int64_t> &result)
{
    const char *timeout_nodes[] = {
        XNODE_DEFAULT_MSG_PROCESS,
        XNODE_MAX_MSG_PROCESS,
        XNODE_SESSION_KEEPING,
        XNODE_DEFAULT_WAITING_FOR_PEER_REPLY,
        XNODE_LONGEST_WAITING_FOR_PEER_REPLY,
        XNODE_CONNECT_TRYING,
        XNODE_POLL_WAITING,
        NULL
    };

    return __load_integer_items(
        config_file,
        xpath_prefix,
        from_common,
        RELATIVE_XPATH_TIMED_TASK_TIMEOUT_ROOT,
        &(timeout_nodes[0]),
        result
    );
}

static int __load_timed_task_intervals(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    std::map<std::string, int64_t> &result)
{
    const char *interval_nodes[] = {
        XNODE_MSG_CLEAN,
        XNODE_SESSION_CLEAN,
        XNODE_HEARTBEAT,
        XNODE_LOG_FLUSHING,
        NULL
    };

    return __load_integer_items(
        config_file,
        xpath_prefix,
        from_common,
        RELATIVE_XPATH_TIMED_TASK_INTERVAL_ROOT,
        &(interval_nodes[0]),
        result
    );
}

static int __load_buffer_settings(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    std::map<std::string, int64_t> &result)
{
    const char *buf_setting_nodes[] = {
        XNODE_TCP_SEND_BUF,
        XNODE_TCP_RECV_BUF,
        NULL
    };

    return __load_integer_items(
        config_file,
        xpath_prefix,
        from_common,
        XNODE_BUF_SETTING,
        &(buf_setting_nodes[0]),
        result
    );
}

static int __load_counters(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    std::map<std::string, int64_t> &result)
{
    const char *counter_nodes[] = {
        XNODE_MSG_PROCESS_COUNT_PER_ROUND,
        XNODE_FORWARD_RETRIES_ON_FAILURE,
        XNODE_WORKER_THREAD,
        NULL
    };

    return __load_integer_items(
        config_file,
        xpath_prefix,
        from_common,
        XNODE_COUNTER,
        &(counter_nodes[0]),
        result
    );
}

static int __load_dispatch_settings(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    std::map<std::string, int64_t> &result)
{
    std::string xpath(xpath_prefix);

    xpath.append("/")
        .append(XNODE_DISPATCH_SETTING)
        .append("/")
        .append(XNODE_DISPATCH_POLICY);

    std::vector<cal::xml::config_node> nodes;
    int read_ret = cal::xml::read_config_nodes(*config_file, xpath.c_str(), 1, nodes);

    if (read_ret <= 0)
    {
        if (from_common)
        {
            GLOG_ERROR_NS("cafw", "can not find %s item in common configuration.\n", xpath.c_str());
            return RET_FAILED;
        }

        GQ_LOG_INFO_NS("cafw", "can not find %s item in private configuration,"
            " take the value from common configuration.\n", xpath.c_str());

        return RET_OK;
    }

    const char *value_str = nodes[0].node_value.c_str();

    if (0 == strcmp(value_str, "randomly"))
        result[XNODE_DISPATCH_POLICY] = DISPATCHED_RANDOMLY;
    else if (0 == strcmp(value_str, "by-id"))
        result[XNODE_DISPATCH_POLICY] = DISPATCHED_BY_ID;
    else
        result[XNODE_DISPATCH_POLICY] = DISPATCHED_TO_LEAST_LOAD;

    GQ_LOG_INFO_NS("cafw", "%s: original text [%s], converted value [%ld]\n",
        xpath.c_str(), value_str, result[XNODE_DISPATCH_POLICY]);

    return RET_OK;
}

int load_mutable_common_configs(
    const config_file_t *config_file,
    const char *xpath_prefix,
    const bool from_common,
    mutable_common_config *result)
{
    if (NULL == config_file
        || NULL == xpath_prefix
        || NULL == result)
    {
        GLOG_ERROR_NS("cafw", "null params\n");
        return RET_FAILED;
    }

    if (__load_timezone(config_file, xpath_prefix, from_common, result->timezone) < 0)
    {
        GLOG_ERROR_NS("cafw", "failed to load time zone\n");
        return RET_FAILED;
    }

    if (__load_timed_task_unit(config_file, xpath_prefix, from_common, result->time_unit_of_timed_task) < 0)
    {
        GLOG_ERROR_NS("cafw", "failed to load timed task unit\n");
        return RET_FAILED;
    }

    typedef int (*IntItemLoader)(const config_file_t*, const char*, const bool, std::map<std::string, int64_t>&);
    struct loader_info
    {
        IntItemLoader func;
        std::map<std::string, int64_t> &holder;
        const char *name;
    } loader_items[] = {
        { __load_timed_task_timeouts, result->timed_task_timeouts, "timed task timeouts" },
        { __load_timed_task_intervals, result->timed_task_intervals, "timed task intervals" },
        { __load_buffer_settings, result->buffer_sizes, "buffer settings" },
        { __load_counters, result->counters, "counters" },
        { __load_dispatch_settings, result->dispatch_settings, "dispatch settings" }
    };

    for (size_t i = 0; i < sizeof(loader_items) / sizeof(struct loader_info); ++i)
    {
        if (loader_items[i].func(config_file, xpath_prefix, from_common, loader_items[i].holder) < 0)
        {
            GLOG_ERROR_NS("cafw", "failed to load %s\n", loader_items[i].name);
            return RET_FAILED;
        }
    }

    return RET_OK;
}

int load_common_configurations(
    const char *config_file,
    fixed_common_config *fixed_part,
    mutable_common_config *mutable_part)
{
    if (NULL == config_file
        || NULL == fixed_part
        || NULL == mutable_part)
    {
        GLOG_ERROR_NS("cafw", "null params\n");
        return RET_FAILED;
    }

    TiXmlDocument doc(config_file);
    bool load_ok = doc.LoadFile();

    if (!load_ok)
    {
        GLOG_ERROR_NS("cafw", "failed to load XML: %s\n", config_file);
        return RET_FAILED;
    }

    if (RET_OK != load_fixed_common_configs(&doc, fixed_part))
    {
        GLOG_ERROR_NS("cafw", "failed to load fixed part\n");
        return RET_FAILED;
    }

    if (RET_OK != load_mutable_common_configs(&doc, XPATH_COMMON_VARIABLES_ROOT, true, mutable_part))
    {
        GLOG_ERROR_NS("cafw", "failed to load mutable part from common file\n");
        return RET_FAILED;
    }

    // NOTE: How to close the file ?? XML file will be closed automatically by TiXmlDocument
    //     instance when exiting this function.

    return RET_OK;
}

}
