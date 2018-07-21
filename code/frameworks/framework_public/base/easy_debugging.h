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
 * easy_debugging.h
 *
 *  Created on: 2016-11-11
 *      Author: wenxiongchang
 * Description: Some operations convenient for debugging.
 */

#ifndef __CASDK_FRAMEWORK_EASY_DEBUG_H__
#define __CASDK_FRAMEWORK_EASY_DEBUG_H__

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <typeinfo>

#include "cpp_assistant/ca_full.h"

#define __FUNC__                                    __FUNCTION__

#define GLOG_BASE(log_level, fmt, ...)              do{\
    g_screen_logger->output(cal::HAS_LOG_PREFIX, log_level, fmt, ##__VA_ARGS__); \
    if (NULL != g_file_logger) \
        g_file_logger->output(cal::HAS_LOG_PREFIX, log_level, fmt, ##__VA_ARGS__); \
}while(0)

#define GQ_LOG_BASE(log_level, fmt, ...)            do{\
    if (cafw::is_quiet_mode()) \
        break; \
    GLOG_BASE(log_level, fmt, ##__VA_ARGS__); \
}while(0)

#ifdef LOG_HAS_PID
#define GLOG_BASE_V(log_level, class_str, ns_delim, fmt, ...)   GLOG_BASE(log_level, "[PID:%d, TID:0x%lx] %s, %s%s%s(): Line %d: " fmt, \
    getpid(), pthread_self(), __FILE__, class_str, ns_delim, __FUNC__, __LINE__, ##__VA_ARGS__)
#else
#define GLOG_BASE_V(log_level, class_str, ns_delim, fmt, ...)   GLOG_BASE(log_level, "%s, %s%s%s(): Line %d: " fmt, \
    __FILE__, class_str, ns_delim, __FUNC__, __LINE__, ##__VA_ARGS__)
#endif

#define GQ_LOG_BASE_V(log_level, class_str, ns_delim, fmt, ...) do{\
    if (cafw::is_quiet_mode()) \
        break; \
    GLOG_BASE_V(log_level, class_str, ns_delim, fmt, ##__VA_ARGS__); \
}while(0)

#define GLOG_DEBUG(fmt, ...)                        GLOG_BASE(cal::LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define GQ_LOG_DEBUG(fmt, ...)                      GQ_LOG_BASE(cal::LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define GLOG_DEBUG_C(fmt, ...)                      GLOG_BASE_V(cal::LOG_LEVEL_DEBUG, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_DEBUG_C(fmt, ...)                    GQ_LOG_BASE_V(cal::LOG_LEVEL_DEBUG, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_DEBUG_CS(_class_, fmt, ...)            GLOG_BASE_V(cal::LOG_LEVEL_DEBUG, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_DEBUG_CS(_class_, fmt, ...)          GQ_LOG_BASE_V(cal::LOG_LEVEL_DEBUG, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_DEBUG_NS(ns_str, fmt, ...)             GLOG_BASE_V(cal::LOG_LEVEL_DEBUG, ns_str, "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_DEBUG_NS(ns_str, fmt, ...)           GQ_LOG_BASE_V(cal::LOG_LEVEL_DEBUG, ns_str, "::", fmt, ##__VA_ARGS__)

#define GLOG_INFO(fmt, ...)                         GLOG_BASE(cal::LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define GQ_LOG_INFO(fmt, ...)                       GQ_LOG_BASE(cal::LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define GLOG_INFO_C(fmt, ...)                       GLOG_BASE_V(cal::LOG_LEVEL_INFO, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_INFO_C(fmt, ...)                     GQ_LOG_BASE_V(cal::LOG_LEVEL_INFO, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_INFO_CS(_class_, fmt, ...)             GLOG_BASE_V(cal::LOG_LEVEL_INFO, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_INFO_CS(_class_, fmt, ...)           GQ_LOG_BASE_V(cal::LOG_LEVEL_INFO, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_INFO_NS(ns_str, fmt, ...)              GLOG_BASE_V(cal::LOG_LEVEL_INFO, ns_str, "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_INFO_NS(ns_str, fmt, ...)            GQ_LOG_BASE_V(cal::LOG_LEVEL_INFO, ns_str, "::", fmt, ##__VA_ARGS__)

#define GLOG_WARN(fmt, ...)                         GLOG_BASE(cal::LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define GQ_LOG_WARN(fmt, ...)                       GQ_LOG_BASE(cal::LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define GLOG_WARN_C(fmt, ...)                       GLOG_BASE_V(cal::LOG_LEVEL_WARNING, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_WARN_C(fmt, ...)                     GQ_LOG_BASE_V(cal::LOG_LEVEL_WARNING, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_WARN_CS(_class_, fmt, ...)             GLOG_BASE_V(cal::LOG_LEVEL_WARNING, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_WARN_CS(_class_, fmt, ...)           GQ_LOG_BASE_V(cal::LOG_LEVEL_WARNING, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_WARN_NS(ns_str, fmt, ...)              GLOG_BASE_V(cal::LOG_LEVEL_WARNING, ns_str, "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_WARN_NS(ns_str, fmt, ...)            GQ_LOG_BASE_V(cal::LOG_LEVEL_WARNING, ns_str, "::", fmt, ##__VA_ARGS__)

#define GLOG_ERROR(fmt, ...)                        GLOG_BASE(cal::LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define GQ_LOG_ERROR(fmt, ...)                      GQ_LOG_BASE(cal::LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define GLOG_ERROR_C(fmt, ...)                      GLOG_BASE_V(cal::LOG_LEVEL_ERROR, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_ERROR_C(fmt, ...)                    GQ_LOG_BASE_V(cal::LOG_LEVEL_ERROR, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_ERROR_CS(_class_, fmt, ...)            GLOG_BASE_V(cal::LOG_LEVEL_ERROR, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_ERROR_CS(_class_, fmt, ...)          GQ_LOG_BASE_V(cal::LOG_LEVEL_ERROR, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_ERROR_NS(ns_str, fmt, ...)             GLOG_BASE_V(cal::LOG_LEVEL_ERROR, ns_str, "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_ERROR_NS(ns_str, fmt, ...)           GQ_LOG_BASE_V(cal::LOG_LEVEL_ERROR, ns_str, "::", fmt, ##__VA_ARGS__)

#define GLOG_CRITICAL(fmt, ...)                     GLOG_BASE(cal::LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)
#define GQ_LOG_CRITICAL(fmt, ...)                   GQ_LOG_BASE(cal::LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)
#define GLOG_CRITICAL_C(fmt, ...)                   GLOG_BASE_V(cal::LOG_LEVEL_CRITICAL, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_CRITICAL_C(fmt, ...)                 GQ_LOG_BASE_V(cal::LOG_LEVEL_CRITICAL, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_CRITICAL_CS(_class_, fmt, ...)         GLOG_BASE_V(cal::LOG_LEVEL_CRITICAL, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_CRITICAL_CS(_class_, fmt, ...)       GQ_LOG_BASE_V(cal::LOG_LEVEL_CRITICAL, _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define GLOG_CRITICAL_NS(ns_str, fmt, ...)          GLOG_BASE_V(cal::LOG_LEVEL_CRITICAL, ns_str, "::", fmt, ##__VA_ARGS__)
#define GQ_LOG_CRITICAL_NS(ns_str, fmt, ...)        GQ_LOG_BASE_V(cal::LOG_LEVEL_CRITICAL, ns_str, "::", fmt, ##__VA_ARGS__)

#define GLOG_FLUSH()                                do{\
    if (NULL != g_file_logger) \
        g_file_logger->flush(); \
}while(0)

extern bool g_is_quiet_mode;
extern cal::screen_logger *g_screen_logger;
extern cal::file_logger *g_file_logger;

namespace cafw
{

struct thread_context;

inline void enable_quiet_mode(void)
{
    g_is_quiet_mode = true;
}

inline void disable_quiet_mode(void)
{
    g_is_quiet_mode = false;
}

inline bool is_quiet_mode(void)
{
    return g_is_quiet_mode;
}

int init_global_logger(
    const int terminal_level,
    const bool enables_file_logger = false,
    const int file_level = -1,
    const char *log_dir = ".",
    const char *log_name = "unknown_program");

void clear_global_logger(void);

}

extern cafw::thread_context *g_thread_contexts;

#endif /* __CASDK_FRAMEWORK_EASY_DEBUG_H__ */
