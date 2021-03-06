/*
 * Copyright (c) 2016-2019, Wen Xiongchang <udc577 at 126 dot com>
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

#include "config_manager.h"

#include <string.h>

#include <vector>

#include "base/all.h"

#include "customization.h"

namespace cafw
{

#define LOAD_PRIVATE_CFG_ITEM(func, name)   if (func() < 0){\
        LOGF_C(E, "failed to load %s configurations\n", name);\
        return RET_FAILED;\
    }\
    QLOGF_C(I, "%s configurations was loaded successfully\n", name)

config_manager::config_manager()
    : m_config_content(NULL),
      m_file_is_open(false)
{
    __inner_init();
}

config_manager::config_manager(const char *config_file)
    : m_config_content(NULL),
      m_file_is_open(false)
{
    if (__inner_init() < 0)
        return;

    open_file(config_file);
}

config_manager::~config_manager()
{
    __clear();
}

int config_manager::open_file(void)
{
    return open_file(m_config_content->config_file_path.c_str());
}

int config_manager::open_file(const char *config_file)
{
    if (file_is_open())
        return RET_OK;

    if (NULL == config_file)
    {
        LOGF_C(E, "null config_file pointer\n");
        return RET_FAILED;
    }

    if ((!is_available()) &&
        (__inner_init() < 0))
    {
        LOGF_C(E, "inner initialization failed\n");
        return RET_FAILED;
    }

    m_config_content->config_file_ptr = new TiXmlDocument(config_file);
    if (NULL == m_config_content->config_file_ptr)
    {
        LOGF_C(E, "new TiXmlDocument(%s) failed\n", config_file);
        return RET_FAILED;
    }

    bool load_ok = ((TiXmlDocument *)m_config_content->config_file_ptr)->LoadFile();

    if (!load_ok)
    {
        LOGF_C(E, "failed to load xml file: %s\n", config_file);
        close_file();
        return RET_FAILED;
    }

    m_config_content->config_file_path.assign(config_file);
    m_file_is_open = true;
    QLOGF_C(I, "configuration file [%s] opened successfully\n", config_file);

    return RET_OK;
}

void config_manager::close_file(void)
{
    if (NULL != m_config_content->config_file_ptr)
    {
        delete (TiXmlDocument *)m_config_content->config_file_ptr;
        m_config_content->config_file_ptr = NULL;
        QLOGF_C(I, "%s closed\n", m_config_content->config_file_path.c_str());
    }
}

int config_manager::load(void)
{
    if (!file_is_open())
    {
        LOGF_C(E, "config file not open yet\n");
        return RET_FAILED;
    }

    const config_file_t *private_config = (config_file_t *)(m_config_content->config_file_ptr);
    std::string common_config;

    if (get_common_config_filename(private_config, common_config) < 0)
    {
        LOGF_C(E, "failed to get common configuration file name\n");
        return RET_FAILED;
    }

    QLOGF_C(I, "common configuration file: %s\n", common_config.c_str());

    std::string common_config_dir = calns::str::get_directory(common_config);

    if ('/' != common_config_dir[0])
    {
        std::string private_config_dir(calns::str::get_directory(m_config_content->config_file_path));

        common_config = private_config_dir + "/" + common_config;
        QLOGF_C(I, "file path has been fixed due to relative path usage: %s\n", common_config.c_str());
    }

    fixed_common_config *fixed_common_config = &(m_config_content->fixed_common_configs);
    mutable_common_config *mutable_common_config = &(m_config_content->mutable_common_configs);

    if (load_common_configurations(common_config.c_str(), fixed_common_config, mutable_common_config) < 0)
    {
        LOGF_C(E, "failed to load common configurations\n");
        return RET_FAILED;
    }

    QLOGF_C(I, "common configurations was loaded successfully\n");

    if (load_mutable_common_configs(private_config, XPATH_PRIVATE_CONFIG_ROOT, false, mutable_common_config) < 0)
    {
        LOGF_C(E, "failed to load private part of mutable common configurations\n");
        return RET_FAILED;
    }

    QLOGF_C(I, "private part of mutable common configurations was loaded successfully\n");

    if (load_private_config() < 0)
    {
        LOGF_C(E, "failed to load private configurations\n");
        return RET_FAILED;
    }

    QLOGF_C(I, "private configurations was loaded successfully\n");

    return RET_OK;
}

int config_manager::reload_partial(void)
{
    config_file_t *private_config = (config_file_t *)(m_config_content->config_file_ptr);
    bool load_ok = private_config->LoadFile();

    if (!load_ok)
    {
        LOGF_C(E, "failed to reload private configuration\n");
        return RET_FAILED;
    }

    LOAD_PRIVATE_CFG_ITEM(__load_log_config, "log");
    reload_partial_extra_config(m_config_content->private_configs.extra_items);

    return RET_OK;
}

int64_t config_manager::get_timeout_raw_value(const char *name) const
{
    return get_long_int_value_by_name(name, m_config_content->mutable_common_configs.timed_task_timeouts);
}

int64_t config_manager::get_time_interval_raw_value(const char *name) const
{
    return get_long_int_value_by_name(name, m_config_content->mutable_common_configs.timed_task_intervals);
}

int64_t config_manager::get_buffer_size(const char *name) const
{
    int64_t bytes = get_long_int_value_by_name(name, m_config_content->mutable_common_configs.buffer_sizes);

    return bytes * 1024; // from KB to B
}

int64_t config_manager::get_counter(const char *name) const
{
    return get_long_int_value_by_name(name, m_config_content->mutable_common_configs.counters);
}

int config_manager::get_dispatch_policy(void) const
{
    std::map<std::string, int64_t> &dispatch_map = m_config_content->mutable_common_configs.dispatch_settings;
    std::map<std::string, int64_t>::iterator it = dispatch_map.find(XNODE_DISPATCH_POLICY);

    if (dispatch_map.end() == it)
        return DISPATCHED_BY_ID; // returns a default value

    return static_cast<int>(it->second);
}

const bool config_manager::is_available(void) const
{
    return (NULL != m_config_content);
}

int config_manager::__inner_init(void)
{
    if ((NULL == m_config_content) &&
        (NULL == (m_config_content = new config_content_t)))
    {
        LOGF_C(E, "ConfigContent structure initialization failed\n");
        return RET_FAILED;
    }
    m_config_content->config_file_ptr = NULL;
    m_config_content->private_configs.extra_items = NULL;

    m_file_is_open = false;

    return RET_OK;
}

void config_manager::__clear(void)
{
    close_file();

    if (NULL != m_config_content)
    {
        if (NULL != m_config_content->private_configs.extra_items)
        {
            destroy_extra_config(&(m_config_content->private_configs.extra_items));
        }

        //m_config_content->config_file_path.clear();
        delete m_config_content;
        m_config_content = NULL;
    }

    m_file_is_open = false;
}

int config_manager::load_private_config(void)
{
    LOAD_PRIVATE_CFG_ITEM(__load_log_config, "log");
    load_extra_server_types((config_file_t *)(m_config_content->config_file_ptr),
        m_config_content->fixed_common_configs.server_types);
    LOAD_PRIVATE_CFG_ITEM(__load_identity_config, "identity");
    LOAD_PRIVATE_CFG_ITEM(__load_upstream_server_config, "upstream server");

    if (NULL == m_config_content->private_configs.extra_items
        && init_extra_config(&(m_config_content->private_configs.extra_items)) < 0)
    {
        LOGF_C(E, "failed to initialize extra configuration structure\n");
        return RET_FAILED;
    }

    if (load_extra_config(m_config_content->private_configs.extra_items) < 0)
    {
        LOGF_C(E, "failed to load extra configurations\n");
        return RET_FAILED;
    }

    return RET_OK;
}

int config_manager::__load_log_config(void)
{
    config_file_t *file = (config_file_t *)m_config_content->config_file_ptr;
    private_config &private_config = m_config_content->private_configs;

    if (load_unique_config_node_value(file, XPATH_LOG_CONFIG_ROOT"/terminal-logger/level",
        false, private_config.terminal_log_level) < 0)
    {
        LOGF_C(E, "failed to load terminal logger level setting\n");
        return RET_FAILED;
    }

    std::vector<calns::xml::node_t> file_log_node;
    int read_ret = calns::xml::find_and_parse_nodes(*file, XPATH_LOG_CONFIG_ROOT"/file-logger", 1,
        file_log_node, false, "enabled", NULL);

    if (read_ret <= 0)
    {
        LOGF_C(E, "failed to load file log enable attribute\n");
        return RET_FAILED;
    }

    if (0 != strncasecmp("yes", file_log_node[0].attributes["enabled"].c_str(), 3))
    {
        private_config.file_log_enabled = false;
        return RET_OK;
    }

    private_config.file_log_enabled = true;

    if (load_unique_config_node_value(file, XPATH_LOG_CONFIG_ROOT"/file-logger/level",
        false, private_config.file_log_level) < 0)
    {
        LOGF_C(E, "failed to load file logger level setting\n");
        return RET_FAILED;
    }

    if (load_unique_config_node_value(file, XPATH_LOG_CONFIG_ROOT"/file-logger/directory",
        false, private_config.log_directory) < 0)
    {
        LOGF_C(E, "failed to load file logger directory setting\n");
        return RET_FAILED;
    }
    //LOGF_C(D, "private_config.log_directory: %s\n", private_config.log_directory.c_str());

    if (load_unique_config_node_value(file, XPATH_LOG_CONFIG_ROOT"/file-logger/basename",
        false, private_config.basic_log_name) < 0)
    {
        LOGF_C(E, "failed to load file logger basename setting\n");
        return RET_FAILED;
    }

    return RET_OK;
}

int config_manager::__load_identity_config(void)
{
#ifdef ACCEPTS_CLIENTS
    return __load_network_nodes("identities");
#else
    LOGF_C(I, "This is a TCP client which needs no listening address, skip\n");
    return RET_OK;
#endif
}

int config_manager::__load_upstream_server_config(void)
{
#ifdef HAS_UPSTREAM_SERVERS
    return __load_network_nodes("upstream-servers");
#else
    LOGF_C(I, "no upstream servers needed, skip\n");
    return RET_OK;
#endif
}

int config_manager::__load_network_nodes(const char *type)
{
    config_file_t *file = (config_file_t *)m_config_content->config_file_ptr;

    std::string xpath(XPATH_PRIVATE_CONFIG_ROOT"/");

    xpath.append(type).append("/item");
    QLOGF_C(I, "Reading nodes with XPath[%s] ...\n", xpath.c_str());

    bool is_self_info = (0 == strcmp(type, "identities"));
    std::vector<calns::xml::node_t> net_nodes;
    int read_ret = calns::xml::find_and_parse_nodes(*file, xpath.c_str(), 100, net_nodes, true,
        "enabled", "type", "name", "alias", "address", "attributes", NULL);

    if (read_ret <= 0)
    {
#if defined(HAS_UPSTREAM_SERVERS)
        if (!is_self_info
            && (CA_RET(OBJECT_DOES_NOT_EXIST) == read_ret || 0 == read_ret)) // Upstream servers are optional.
            return RET_OK;
#endif

#if defined(ACCEPTS_CLIENTS)
        LOGF_C(E, "failed to load %s settings, ret = %d\n", type, read_ret);
        return RET_FAILED;
#else
        return RET_OK;
#endif
    }

    net_node_config tmp_node;
    net_node_config *node_ptr = (is_self_info) ? &(m_config_content->private_configs.self)
        : &tmp_node;
    std::map<std::string, std::string>::iterator attr_it;
    std::vector<std::string> str_fragments;
    std::vector<net_node_config> &upstream_servers = m_config_content->private_configs.upstream_servers;
    std::map<std::string, int> &server_type_map = m_config_content->fixed_common_configs.server_types;
    bool found_target = false;

    upstream_servers.clear();

    for (size_t i = 0; i < net_nodes.size(); ++i)
    {
        calns::xml::node_t &node = net_nodes[i];

        attr_it = node.attributes.find("enabled");

        if (node.attributes.end() == attr_it
            || 0 != strncasecmp("yes", attr_it->second.c_str(), 3))
            continue;

        node_ptr->type_name.clear();
        node_ptr->type_value = 0;
        node_ptr->node_name.clear();
        node_ptr->node_alias.clear();
        node_ptr->node_num = 0;
        node_ptr->node_ip.clear();
        node_ptr->node_port = 0;
        node_ptr->attributes.clear();

        if (node.attributes.end() == (attr_it = node.attributes.find("address")))
        {
            LOGF_C(E, "address missing in node[%lu]\n", i);
            return RET_FAILED;
        }

        std::string &address = attr_it->second;

        if (calns::str::split(address.c_str(), address.length(), ":", str_fragments) < 0
            || str_fragments.size() < 0)
        {
            LOGF_C(E, "invalid format or contents of address[%s], i = %lu\n", address.c_str(), i);
            return RET_FAILED;
        }

        node_ptr->node_ip = str_fragments[0];
        node_ptr->node_port = static_cast<uint16_t>(atoi(str_fragments[1].c_str()));
        if (is_self_info)
        {
            if (!calns::tcp_server::can_be_listened(node_ptr->node_ip.c_str(), node_ptr->node_port))
            {
                LOGF_C(W, "address of this node is already used, continue to try next one\n");
                continue;
            }

            QLOGF_C(I, "address available, take contents of this item as the server info\n");
        }
        found_target = true;

        if (node.attributes.end() == (attr_it = node.attributes.find("type")))
        {
            if (!is_self_info)
            {
                LOGF_C(E, "can not find server type attribute, i = %lu\n", i);
                return RET_FAILED;
            }

            node_ptr->type_name = "self";
        }
        else
            node_ptr->type_name = attr_it->second;

        if (server_type_map.end() == server_type_map.find(node_ptr->type_name))
        {
            LOGF_C(E, "unknown server type %s, i = %lu\n", node_ptr->type_name.c_str(), i);
            return RET_FAILED;
        }
        node_ptr->type_value = server_type_map[node_ptr->type_name];

        if (node.attributes.end() == (attr_it = node.attributes.find("name")))
        {
            LOGF_C(E, "node name missing, i = %lu\n", i);
            return RET_FAILED;
        }
        node_ptr->node_name = attr_it->second;

        if (node.attributes.end() != (attr_it = node.attributes.find("alias")))
            node_ptr->node_alias = attr_it->second;
        else
            node_ptr->node_alias = node_ptr->node_name;

        if (node.attributes.end() != (attr_it = node.attributes.find("attributes")))
            node_ptr->attributes = attr_it->second;

        QLOGF_C(I, "%s[%lu]: type = %s(value after conversion: %d),"
            " basic name = %s, alias = %s,"
            " address = %s:%u, attributes = %s\n",
            type, i, node_ptr->type_name.c_str(), node_ptr->type_value,
            node_ptr->node_name.c_str(), node_ptr->node_alias.c_str(),
            node_ptr->node_ip.c_str(), node_ptr->node_port, node_ptr->attributes.c_str());

        if (is_self_info)
            break;

        upstream_servers.push_back(*node_ptr);
    }

    if (!found_target)
    {
        LOGF_C(E, "no available connections found\n");
        return RET_FAILED;
    }

    return RET_OK;
}


int64_t config_manager::get_long_int_value_by_name(const char *name, const std::map<std::string, int64_t> &holder) const
{
    if (NULL == name ||
        NULL == m_config_content)
        return RET_FAILED;

    std::map<std::string, int64_t>::const_iterator it = holder.find(name);

    if (holder.end() == it)
        return RET_FAILED;

    return it->second;
}

}
