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
 * main_app.h
 *
 *  Created on: 2016-11-11
 *      Author: wenxiongchang
 * Description: The executer of an application.
 */

#ifndef __CASDK_FRAMEWORK_MAIN_APP_H__
#define __CASDK_FRAMEWORK_MAIN_APP_H__

#include "base/all.h"

namespace cafw
{

class config_manager;
class resource_manager;
class timed_task_scheduler;
class packet_processor;

class main_app : public calns::singleton<main_app>
{
/* ===================================
 * constructors:
 * =================================== */
private:
    main_app();

/* ===================================
 * copy control:
 * =================================== */
private:
    main_app(const main_app& src);
    main_app& operator=(const main_app& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~main_app();

/* ===================================
 * types:
 * =================================== */
public:
    friend class calns::singleton<main_app>;

/* ===================================
 * abilities:
 * =================================== */
public:
    int prepare_prerequisites(void);
    int parse_command_line(int argc, char **argv);
#if defined(HAS_CONFIG_FILES)
    int load_configurations(void);
#endif
    int prepare_resources(void);
    int register_signals(void);
    int register_timed_tasks(void);
    int initialize_business(int argc, char **argv);
    void finalize_business(void);
    int run_business(void);
    void release_resources(void);

/* ===================================
 * accessors:
 * =================================== */
public:

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
#if defined(HAS_TCP)
    void poll_and_process(calns::tcp_base *tcp_manager, bool &should_exit);
    int accept_new_connection(calns::tcp_server *tcp_server, int send_buf_size, int recv_buf_size);
    void shut_bad_connection(calns::tcp_base *tcp_manager, calns::net_connection *bad_conn);
    void handle_received_packets(calns::net_connection *input_conn, int max_packet_count);
    void send_result_packets(calns::tcp_base *tcp_manager);
#endif

/* ===================================
 * data:
 * =================================== */
private:
    calns::command_line *m_cmdline;
    config_manager *m_config_manager;
    resource_manager *m_resource_manager;
    timed_task_scheduler *m_timed_task_scheduler;
    packet_processor *m_packet_processor;
};

}

#endif /* __CASDK_FRAMEWORK_MAIN_APP_H__ */
