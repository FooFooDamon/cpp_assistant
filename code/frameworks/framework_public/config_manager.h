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

/*
 * config_manager.h
 *
 *  Created on: 2015-05-22(Rewritten: 2016-12-01)
 *      Author: wenxiongchang
 * Description: configuration manager
 */

#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include <stdint.h>

#include <string>
#include <map>
#include <set>
#include <vector>

#include "common_configurations.h"

struct extra_config_t;

namespace cafw
{

typedef struct private_config
{
    bool file_log_enabled;
    std::string basic_log_name;
    std::string log_directory;
    std::string file_log_level;
    std::string terminal_log_level;
    net_node_config self;
    std::vector<net_node_config> upstream_servers;
    std::set<uint32_t> time_consuming_cmd;
    struct extra_config_t *extra_items;
}private_config;

typedef struct config_content_t
{
    std::string config_file_path;
    void *config_file_ptr;

    // Real common configurations and can not be changed.
    fixed_common_config fixed_common_configs;

    // Mutable common configurations:
    // They are not real common configurations,
    // their values come from the configuration file for this program if there are such items in it,
    // or from the common configuration file otherwise.
    mutable_common_config mutable_common_configs;

    // private configurations only for this program
    private_config private_configs;
}config_content_t;

extern int load_extra_server_types(const config_file_t *config_file, std::map<std::string, int> &result);

class config_manager
{
/* ===================================
 * constructors:
 * =================================== */
public:
    config_manager();
    config_manager(const char *config_file);

/* ===================================
 * copy control:
 * =================================== */
private:
    config_manager(const config_manager& src);
    config_manager& operator=(const config_manager& src);

/* ===================================
 * destructors:
 * =================================== */
public:
    ~config_manager();

/* ===================================
 * types:
 * =================================== */
public:

/* ===================================
 * abilities:
 * =================================== */
public:
    int open_file(void);
    int open_file(const char *config_file);
    void close_file(void);
    int load(void);
    int reload_partial(void);

/* ===================================
 * attributes:
 * =================================== */
public:
    inline const config_content_t *config_content(void) const
    {
        return m_config_content;
    }

    int64_t get_timeout_raw_value(const char *name) const;

    inline int64_t get_timeout_in_microseconds(const char *name) const
    {
        int64_t src = get_timeout_raw_value(name);
        if (src < 0)
            return src;

        int time_unit = m_config_content->mutable_common_configs.time_unit_of_timed_task;
        int time_multiple = (TIME_UNIT_MILLISECOND == time_unit) ? 1000 : 1000000; // for millisecond or second

        return src * time_multiple;
    }

    int64_t get_time_interval_raw_value(const char *name) const;

    inline int64_t get_time_interval_in_microseconds(const char *name) const
    {
        int64_t src = get_time_interval_raw_value(name);
        if (src < 0)
            return src;

        int time_unit = m_config_content->mutable_common_configs.time_unit_of_timed_task;
        int time_multiple = (TIME_UNIT_MILLISECOND == time_unit) ? 1000 : 1000000; // for millisecond or second

        return src * time_multiple;
    }

    int64_t get_buffer_size(const char *name) const;
    int64_t get_counter(const char *name) const;
    int get_dispatch_policy(void) const;

/* ===================================
 * status:
 * =================================== */
public:
    const bool file_is_open(void) const
    {
        return m_file_is_open;
    }

    const bool is_available(void) const;


/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    int __inner_init(void);
    void __clear(void);
    int load_private_config(void);
    int __load_log_config(void);
    int __load_identity_config(void);
    int __load_upstream_server_config(void);
    int __load_network_nodes(const char *type);
    int64_t get_long_int_value_by_name(const char *name, const std::map<std::string, int64_t> &holder) const;

/* ===================================
 * data:
 * =================================== */
private:
    config_content_t *m_config_content;
    bool m_file_is_open;
};

}

#define CFG_GET_TIMEOUT_USEC(name)          calns::singleton<cafw::config_manager>::get_instance()->get_timeout_in_microseconds(name)
#define CFG_GET_TIME_INTERVAL_USEC(name)    calns::singleton<cafw::config_manager>::get_instance()->get_time_interval_in_microseconds(name)
#define CFG_GET_BUF_SIZE(name)              calns::singleton<cafw::config_manager>::get_instance()->get_buffer_size(name)
#define CFG_GET_COUNTER(name)               calns::singleton<cafw::config_manager>::get_instance()->get_counter(name)
#define CFG_GET_DISPATCH_POLICY()           calns::singleton<cafw::config_manager>::get_instance()->get_dispatch_policy()

#endif /* __CONFIG_MANAGER_H__ */
