/*
 * Copyright (c) 2017-2018, Wen Xiongchang <udc577 at 126 dot com>
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
 * debug.h
 *
 *  Created on: 2017-09-17
 *      Author: wenxiongchang
 * Description: Providing easy ways of outputting readable and well-formatted info
 *              during debugging.
 */

#ifndef __CPP_ASSISTANT_DEBUG_H__
#define __CPP_ASSISTANT_DEBUG_H__

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "ca_inner_necessities.h"
#include "platforms/threading.h"

#define CA_GET_CONST_DEBUG_OUTPUT_HOLDER()      CA_LIB_NAMESPACE::debug::get_debug_output_holder()
#define CA_GET_MUTABLE_DEBUG_OUTPUT_HOLDER()    const_cast<FILE*>(CA_GET_CONST_DEBUG_OUTPUT_HOLDER())

#define CA_GET_CONST_ERROR_OUTPUT_HOLDER()      CA_LIB_NAMESPACE::debug::get_error_output_holder()
#define CA_GET_MUTABLE_ERROR_OUTPUT_HOLDER()    const_cast<FILE*>(CA_GET_CONST_ERROR_OUTPUT_HOLDER())

#define CA_OUTPUT_RAW(dev, fmt, ...)            fprintf(dev, fmt, ##__VA_ARGS__)

#define CA_OUTPUT_BASE(dev, preamble, type_str, class_str, ns_delim, fmt, ...) do{ \
    struct timeval __tv; \
    gettimeofday(&__tv, NULL); \
    ::fprintf(dev, "[PID:%d, TID:0x%lx][Time:%ld.%ld] "preamble" "type_str" %s, %s%s%s(): Line %d: "fmt, \
        getpid(), pthread_self(), __tv.tv_sec, __tv.tv_usec, __FILE__, class_str, ns_delim, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}while(0)

#define CA_DEBUG_BASE(preamble, class_str, ns_delim, fmt, ...)      CA_OUTPUT_BASE(CA_GET_MUTABLE_DEBUG_OUTPUT_HOLDER(), preamble, "DEBUG:", class_str, ns_delim, fmt, ##__VA_ARGS__)
#define CA_INFO_BASE(preamble, class_str, ns_delim, fmt, ...)       CA_OUTPUT_BASE(CA_GET_MUTABLE_DEBUG_OUTPUT_HOLDER(), preamble, "", class_str, ns_delim, fmt, ##__VA_ARGS__)
#define CA_WARN_BASE(preamble, class_str, ns_delim, fmt, ...)       CA_OUTPUT_BASE(CA_GET_MUTABLE_ERROR_OUTPUT_HOLDER(), preamble, "** WARNING:", class_str, ns_delim, fmt, ##__VA_ARGS__)
#define CA_ERROR_BASE(preamble, class_str, ns_delim, fmt, ...)      CA_OUTPUT_BASE(CA_GET_MUTABLE_ERROR_OUTPUT_HOLDER(), preamble, "**** ERROR:", class_str, ns_delim, fmt, ##__VA_ARGS__)
#define CA_CRITICAL_BASE(preamble, class_str, ns_delim, fmt, ...)   CA_OUTPUT_BASE(CA_GET_MUTABLE_ERROR_OUTPUT_HOLDER(), preamble, "**** CRITICAL:", class_str, ns_delim, fmt, ##__VA_ARGS__)

#if defined(DEBUG)
#define CA_RAW_DEBUG(fmt, ...)                  CA_OUTPUT_RAW(CA_GET_MUTABLE_DEBUG_OUTPUT_HOLDER(), fmt, ##__VA_ARGS__)
#define CA_DEBUG(fmt, ...)                      CA_DEBUG_BASE("", "", "", fmt, ##__VA_ARGS__)
#define CA_DEBUG_C(fmt, ...)                    CA_DEBUG_BASE("", typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define CA_DEBUG_CS(_class_, fmt, ...)          CA_DEBUG_BASE("", _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define CA_DEBUG_NS(ns_str, fmt, ...)           CA_DEBUG_BASE("", ns_str, "::", fmt, ##__VA_ARGS__)
#else
#define CA_RAW_DEBUG(fmt, ...)                  do{;}while(0)
#define CA_DEBUG(fmt, ...)                      do{;}while(0)
#define CA_DEBUG_C(fmt, ...)                    do{;}while(0)
#define CA_DEBUG_CS(fmt, ...)                   do{;}while(0)
#define CA_DEBUG_NS(ns_str, fmt, ...)           do{;}while(0)
#endif

#define CA_RAW_INFO(fmt, ...)                   CA_OUTPUT_RAW(CA_GET_MUTABLE_DEBUG_OUTPUT_HOLDER(), fmt, ##__VA_ARGS__)
#define CA_INFO(fmt, ...)                       CA_INFO_BASE("", "", "", fmt, ##__VA_ARGS__)
#define CA_INFO_C(fmt, ...)                     CA_INFO_BASE("", typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define CA_INFO_CS(_class_, fmt, ...)           CA_INFO_BASE("", _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define CA_INFO_NS(ns_str, fmt, ...)            CA_INFO_BASE("", ns_str, "::", fmt, ##__VA_ARGS__)

#define CA_RAW_WARN(fmt, ...)                   CA_OUTPUT_RAW(CA_GET_MUTABLE_ERROR_OUTPUT_HOLDER(), fmt, ##__VA_ARGS__)
#define CA_WARN(fmt, ...)                       CA_WARN_BASE("", "", "", fmt, ##__VA_ARGS__)
#define CA_WARN_C(fmt, ...)                     CA_WARN_BASE("", typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define CA_WARN_CS(_class_, fmt, ...)           CA_WARN_BASE("", _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define CA_WARN_NS(ns_str, fmt, ...)            CA_WARN_BASE("", ns_str, "::", fmt, ##__VA_ARGS__)

#define CA_RAW_ERROR(fmt, ...)                  CA_OUTPUT_RAW(CA_GET_MUTABLE_ERROR_OUTPUT_HOLDER(), fmt, ##__VA_ARGS__)
#define CA_ERROR(fmt, ...)                      CA_ERROR_BASE("", "", "", fmt, ##__VA_ARGS__)
#define CA_ERROR_C(fmt, ...)                    CA_ERROR_BASE("", typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define CA_ERROR_CS(_class_, fmt, ...)          CA_ERROR_BASE("", _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define CA_ERROR_NS(ns_str, fmt, ...)           CA_ERROR_BASE("", ns_str, "::", fmt, ##__VA_ARGS__)

#define CA_RAW_CRITICAL(fmt, ...)               CA_OUTPUT_RAW(CA_GET_MUTABLE_ERROR_OUTPUT_HOLDER(), fmt, ##__VA_ARGS__)
#define CA_CRITICAL(fmt, ...)                   CA_CRITICAL_BASE("", "", "", fmt, ##__VA_ARGS__)
#define CA_CRITICAL_C(fmt, ...)                 CA_CRITICAL_BASE("", typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define CA_CRITICAL_CS(_class_, fmt, ...)       CA_CRITICAL_BASE("", _class_::class_name(), "::", fmt, ##__VA_ARGS__)
#define CA_CRITICAL_NS(ns_str, fmt, ...)        CA_CRITICAL_BASE("", ns_str, "::", fmt, ##__VA_ARGS__)

CA_LIB_NAMESPACE_BEGIN

typedef int (*format_output_func)(const char *fmt, ...);

namespace debug
{

/*
 * It's easy to understand that cpp-assistant library may output debug, warning or error info
 * when exceptions occur or for some other purposes.
 *
 * APIs below can be used to do such settings as:
 *     enabling or disabling the output prefix displaying,
 *     enabling or disabling the output,
 *     setting the output destinations (e.g. to stdout, stderr or to files)
 * and so on. See comments of each API for more details.
 */

extern bool g_debug_enabled; // can not be set or unset directly

// Makes the debug or warning/error output display with prefix, like:
// [2018-01-01 00:00:00.000000][PID:1234] I'm the output with prefix...
// or something else.
void enable_output_prefix(void);

// Makes the debug or warning/error output display without prefix.
void disable_output_prefix(void);

/*
 * xx_is_enabled(): Checks whether the specified output is enabled.
 *
 * redirect_xx_output(): Redirects the specified output to stdout, stderr or a file.
 *      If @where is NULL, then the specified output will be disabled.
 *
 * get_xx_output_holder(): Gets the output file pointer.
 *
 * set_xx_lock(): Sets a lock for the specified output, only required in multi-threading environment.
 *
 */

inline bool debug_is_enabled(void)
{
    return g_debug_enabled;
}

void redirect_debug_output(const FILE *where);

const FILE* get_debug_output_holder(void);

void set_debug_lock(const mutex_t *lock);

bool error_report_is_enabled(void);

void redirect_error_output(const FILE *where);

const FILE* get_error_output_holder(void);

void set_error_lock(const mutex_t *lock);

} // namespace debug

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_DEBUG_H__ */
