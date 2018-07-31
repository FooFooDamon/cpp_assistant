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
 * resource_manager.h
 *
 *  Created on: 2015-05-21
 *      Author: wenxiongchang
 * Description: resource manager
 */

#ifndef __CASDK_FRAMEWORK_RESOURCE_MANAGER_H__
#define __CASDK_FRAMEWORK_RESOURCE_MANAGER_H__

#include <stddef.h>

#include "base/all.h"

struct extra_resource_t;

namespace cafw
{

class connection_cache;

typedef struct resource_t
{
    calns::tcp_server *server_listener;
    calns::tcp_client *client_requester;
    connection_cache *master_connection_cache;
    connection_cache *slave_connection_cache;
    struct extra_resource_t *extra_resource;
}resource_t;

class resource_manager
{
/* ===================================
 * constructors:
 * =================================== */
public:
    resource_manager();

/* ===================================
 * copy control:
 * =================================== */
private:
    resource_manager(const resource_manager& src);
    resource_manager& operator=(const resource_manager& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~resource_manager();

/* ===================================
 * types:
 * =================================== */
public:

/* ===================================
 * abilities:
 * =================================== */
public:
    int prepare(const void *condition = NULL);
    void clean(void);

/* ===================================
 * attributes:
 * =================================== */
public:
    inline const resource_t *resource(void) const
    {
        return m_resource;
    }

/* ===================================
 * status:
 * =================================== */
public:
    const bool is_available(void) const;
    const bool is_ready(void) const;


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
    int __prepare_loggers(const void *condition = NULL);
    int __prepare_network(const void *condition = NULL);
    void __release_network(void);
    int __prepare_extra_resource(const void *condition = NULL);
    void __release_extra_resource(void);

/* ===================================
 * data:
 * =================================== */
protected:
    resource_t *m_resource;
    bool m_is_ready;
};
#if 0 // used in future
typedef struct thread_context
{
    enum
    {
        TYPE_SUPERVISOR = 0,
        TYPE_WORKER
    };

    enum
    {
        STATUS_UNINITIALIZED = 0,
        STATUS_BASIC_PART_INITIALIZED,
        //STATUS_SELF_PART_INITIALIZED,
        STATUS_FULLY_INITIALIZED,
        STATUS_IDLE,
        //STATUS_SLEEPING,
        //STATUS_ACTIVE,
        STATUS_OCCUPIED_BY_SUPERVISOR,
        STATUS_OCCUPIED_BY_WORKER,
        STATUS_ZOMBIE,
        STATUS_EXITED_NORMALLY,
        STATUS_KILLED,
    };

    enum
    {
        WORKER_TASK_DB_HEARTBEAT = 0,
        WORKER_TASK_LOG_FLUSHING,

        WORKER_TASK_COUNT,
    };

    int num;
    pthread_t tid;
    int type;
    std::string name;
    pthread_mutex_t lock;
    calns::screen_logger screen_logger;
    calns::file_logger file_logger;
    int status;
    bool should_exit;
    // int load_count; // for load balancing
    int64_t timed_task_refresh_times[WORKER_TASK_COUNT];
    calns::buffer *input_packet_cache;
    calns::buffer *output_packet_cache;
    resource_t resource;
}thread_context;

typedef thread_context thrdctx;

int prepare_thread_resource(void);
void release_thread_resource(void);
#endif
}

#endif /* __CASDK_FRAMEWORK_RESOURCE_MANAGER_H__ */
