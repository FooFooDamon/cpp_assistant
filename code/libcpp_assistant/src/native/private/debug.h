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
 *  Created on: 2017-09-10
 *      Author: wenxiongchang
 * Description: Providing easy ways of outputting readable and well-formatted info
 *              during debugging.
 */

#ifndef __CPP_ASSISTANT_PRIVATE_DEBUG_H__
#define __CPP_ASSISTANT_PRIVATE_DEBUG_H__

#include <stdio.h>

#include <typeinfo>

#include "base/ca_inner_necessities.h"
#include "base/platforms/threading.h"
#include "base/debug.h"

CA_LIB_NAMESPACE_BEGIN

#define HIGHLIGHT_COLOR_BLACK                       "\033[1;30m"
#define HIGHLIGHT_COLOR_RED                         "\033[1;31m"
#define HIGHLIGHT_COLOR_GREEN                       "\033[1;32m"
#define HIGHLIGHT_COLOR_YELLOW                      "\033[1;33m"
#define HIGHLIGHT_COLOR_BLUE                        "\033[1;34m"
#define HIGHLIGHT_COLOR_PURPLE                      "\033[1;35m"
#define HIGHLIGHT_COLOR_DARK_GREEN                  "\033[1;36m"
#define HIGHLIGHT_COLOR_WHITE                       "\033[1;37m"

#define NO_PRINT_ATTRIBUTES                         "\033[0m"

#define WARNING_COLOR                               HIGHLIGHT_COLOR_YELLOW
#define ERROR_COLOR                                 HIGHLIGHT_COLOR_RED
#define FATAL_COLOR                                 HIGHLIGHT_COLOR_RED

void __enable_output_prefix(void);

void __disable_output_prefix(void);

//void __formatted_output(int level, FILE *where, mutex_t *lock, const char *format, va_list args);

void __set_debug_output(const FILE *where);

const FILE* __get_debug_output_holder(void);

inline bool __debug_is_enabled(void)
{
    return debug::m_debug_enabled;
}

bool __debug_macro_is_defined(void);

void __set_debug_lock(const mutex *lock);

void __debug(const char *format, ...) CA_NOTNULL(1) CA_PRINTF_CHECK(1, 2);

void __set_error_output(const FILE *where);

const FILE* __get_error_output_holder(void);

bool __error_report_is_enabled(void);

void __set_error_lock(const mutex *lock);

void __warn(const char *format, ...) CA_NOTNULL(1) CA_PRINTF_CHECK(1, 2);

void __error(const char *format, ...) CA_NOTNULL(1) CA_PRINTF_CHECK(1, 2);

#define __output(func, class_str, ns_delim, fmt, ...)   \
    func("%s:%d, %s%s%s(): " fmt, __FILE__, __LINE__, class_str, ns_delim, __FUNCTION__, ##__VA_ARGS__)

#define debug_is_enabled                                __debug_is_enabled
#define cdebug(fmt, ...)                                __output(__debug, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define nsdebug(_namespace_, fmt, ...)                  __output(__debug, CA_LIB_NAMESPACE_STR "::" #_namespace_, "::", fmt, ##__VA_ARGS__)

#define __error_report_is_enabled                       __error_report_is_enabled

#define cwarn(fmt, ...)                                 __output(__warn, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define nswarn(_namespace_, fmt, ...)                   __output(__warn, CA_LIB_NAMESPACE_STR "::" #_namespace_, "::", fmt, ##__VA_ARGS__)

#define cerror(fmt, ...)                                __output(__error, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)
#define nserror(_namespace_, fmt, ...)                  __output(__error, CA_LIB_NAMESPACE_STR "::" #_namespace_, "::", fmt, ##__VA_ARGS__)

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_PRIVATE_DEBUG_H__ */
