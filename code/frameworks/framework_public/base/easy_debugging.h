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

namespace calns = CA_LIB_NAMESPACE;

#define __FUNC__                                    __FUNCTION__

#undef LOGF_BASE
#define LOGF_BASE(log_level, fmt, ...)              do{\
    g_screen_logger->output(calns::HAS_LOG_PREFIX, log_level, fmt, ##__VA_ARGS__); \
    if (NULL != g_file_logger) \
        g_file_logger->output(calns::HAS_LOG_PREFIX, log_level, fmt, ##__VA_ARGS__); \
}while(0)

#define QLOGF_BASE(log_level, fmt, ...)             do{\
    if (cafw::is_quiet_mode()) \
        break; \
    LOGF_BASE(log_level, fmt, ##__VA_ARGS__); \
}while(0)

#ifdef MULTI_THREADING
#define LOGF_BASE_V(log_level, class_str, ns_delim, fmt, ...)   LOGF_BASE(log_level, "[%d#%d] %s:%d, %s%s%s(): " fmt, \
    getpid(), CA_LIB_NAMESPACE::gettid(), __FILE__, __LINE__, class_str, ns_delim, __FUNC__, ##__VA_ARGS__)
#else
#define LOGF_BASE_V(log_level, class_str, ns_delim, fmt, ...)   LOGF_BASE(log_level, "%s:%d, %s%s%s(): " fmt, \
    __FILE__, __LINE__, class_str, ns_delim, __FUNC__, ##__VA_ARGS__)
#endif

#define QLOGF_BASE_V(log_level, class_str, ns_delim, fmt, ...)  do{\
    if (cafw::is_quiet_mode()) \
        break; \
    LOGF_BASE_V(log_level, class_str, ns_delim, fmt, ##__VA_ARGS__); \
}while(0)

#define LOG_FLUSH()                                 do{\
    if (NULL != g_file_logger) \
        g_file_logger->flush(); \
}while(0)

// R is short for raw, which means outputting log contents as little as possible.
#ifdef RLOGF
#undef RLOGF
#endif
#define RLOGF(x, fmt, ...)                          LOGF_BASE(calns::LOG_LEVEL_##x, fmt, ##__VA_ARGS__)

// Q is short for quiet, which means quiet mode and outputting log contents as little as possible on startup.
#ifdef RQLOGF
#undef RQLOGF
#endif
#define RQLOGF(x, fmt, ...)                         QLOGF_BASE(calns::LOG_LEVEL_##x, fmt, ##__VA_ARGS__)

// F is short for format, which means formatted logging, just like printf().
#undef LOGF
#define LOGF(x, fmt, ...)                           LOGF_BASE_V(calns::LOG_LEVEL_##x, "", "", fmt, ##__VA_ARGS__)

// C is short for class, which means outputting name of the class calling this logging function.
#undef LOGF_C
#define LOGF_C(x, fmt, ...)                         LOGF_BASE_V(calns::LOG_LEVEL_##x, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)

// NS is short for namespace, which means outputting the current namespace.
#undef LOGF_NS
#define LOGF_NS(x, _namespace_, fmt, ...)           LOGF_BASE_V(calns::LOG_LEVEL_##x, #_namespace_, "::", fmt, ##__VA_ARGS__)

#ifdef QLOGF
#undef QLOGF
#endif
#define QLOGF(x, fmt, ...)                          QLOGF_BASE_V(calns::LOG_LEVEL_##x, "", "", fmt, ##__VA_ARGS__)

#ifdef QLOGF_C
#undef QLOGF_C
#endif
#define QLOGF_C(x, fmt, ...)                        QLOGF_BASE_V(calns::LOG_LEVEL_##x, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)

#ifdef QLOGF_NS
#undef QLOGF_NS
#endif
#define QLOGF_NS(x, _namespace_, fmt, ...)          QLOGF_BASE_V(calns::LOG_LEVEL_##x, #_namespace_, "::", fmt, ##__VA_ARGS__)

extern bool g_is_quiet_mode;
extern calns::screen_logger *g_screen_logger;
extern calns::file_logger *g_file_logger;

namespace cafw
{

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

int init_logger(
    const int terminal_level,
    const bool enables_file_logger = false,
    const int file_level = -1,
    const char *log_dir = ".",
    const char *log_name = "unknown_program");

void clear_logger(void);

}

#endif /* __CASDK_FRAMEWORK_EASY_DEBUG_H__ */
