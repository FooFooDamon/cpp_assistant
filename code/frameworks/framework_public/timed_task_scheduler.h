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
 * timed_task_scheduler.h
 *
 *  Created on: 2015-06-02
 *      Author: wenxiongchang
 * Description: for managing and doing all kinds of timed tasks
 */

#ifndef __CASDK_FRAMEWORK_TIMED_TASK_SCHEDULER_H__
#define __CASDK_FRAMEWORK_TIMED_TASK_SCHEDULER_H__

#include <stdint.h>

#include <string>
#include <map>

namespace cafw
{

typedef void (*timed_task_func)(void);

typedef struct timed_task_config
{
    enum enum_trigger_type
    {
        /*
         * event-triggered
         */
        TRIGGERED_BEFORE_SOME_EVENT = -1,
        TRIGGERED_ON_SOME_EVENT = 0,
        TRIGGERED_AFTER_SOME_EVENT = 1,
        /*
         * pure time-triggered
         */
        TRIGGERED_PERIODICALLY = 2
    };

    enum enum_trigger_interval
    {
        MIN_TRIGGER_INTERVAL = 1,
        DEFAULT_TRIGGER_INTERVAL = 1000,
        MAX_TRIGGER_INTERVAL = 24 * 3600 * 1000
    };

    int trigger_type;
    bool has_triggered;
    union
    {
        // both in microseconds
        int64_t event_time; // for event-triggered tasks
        int64_t last_op_time; // for pure time-triggered tasks
    };
    union
    {
        // both in milliseconds
        int64_t time_offset; // for event-triggered tasks
        int64_t time_interval; // for pure time-triggered tasks
    };
    timed_task_func operation;
}timed_task_config;

typedef std::map<std::string, timed_task_config> timed_task_map;

class timed_task_scheduler
{
/* ===================================
 * constructors:
 * =================================== */
public:
    timed_task_scheduler();

/* ===================================
 * copy control:
 * =================================== */
private:
    timed_task_scheduler(const timed_task_scheduler& src);
    timed_task_scheduler& operator=(const timed_task_scheduler& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~timed_task_scheduler();

/* ===================================
 * types:
 * =================================== */
public:

/* ===================================
 * abilities:
 * =================================== */
public:
    int register_one(const std::string &name, const timed_task_config &info);
    int unregister_one(const std::string &name);
    bool exists(const std::string &name);
    int set_event_trigger_time(const std::string &name, const int64_t event_time, const int time_offset);
    void check_and_execute(void);
    static const char *get_trigger_type_description(int type_num);

/* ===================================
 * attributes:
 * =================================== */
public:
    timed_task_map *tasks(void);

/* ===================================
 * status:
 * =================================== */
public:
    bool is_available(void);
    bool is_ready(void);


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
    bool __task_info_is_ok(const timed_task_config &info);

/* ===================================
 * data:
 * =================================== */
protected:
    timed_task_map *m_tasks;
};

void default_message_clean_timed_task(void);
void default_session_clean_timed_task(void);
void default_heartbeat_timed_task(void);
void default_log_flushing_timed_task(void);

}

#endif /* __CASDK_FRAMEWORK_TIMED_TASK_SCHEDULER_H__ */
