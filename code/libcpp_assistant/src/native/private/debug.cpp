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
 * debug.cpp
 *
 *  Created on: 2017-09-10
 *      Author: wenxiongchang
 * Description: See debug.h.
 */

#include "debug.h"

#include <stdarg.h>
#include <sys/time.h>

#include "base/platforms/os_specific.h"
#include "base/platforms/threading.h"
#include "base/debug.h"
#include "logger.h"

CA_LIB_NAMESPACE_BEGIN

static bool s_has_output_prefix = true;

void __enable_output_prefix(void)
{
    s_has_output_prefix = true;
}

void __disable_output_prefix(void)
{
    s_has_output_prefix = false;
}

static void __formatted_output(int level, FILE *where, mutex *lock, const char *format, va_list args)
{
    if (nullptr == where)
        return;

    bool output_to_file = (stdout != where && stderr != where);
    bool needs_lock = (nullptr != lock && output_to_file);
    bool needs_color = (!output_to_file && level >= LOG_LEVEL_WARNING);

    if (needs_lock)
        lock->lock();

    if (s_has_output_prefix)
    {
        extern const char *G_LOG_LEVEL_STRINGS[];
        struct timeval tv;
        struct tm now;

        gettimeofday(&tv, nullptr);
        localtime_r((time_t *)&(tv.tv_sec), &now);

        fprintf(where, "[%04d-%02d-%02d %02d:%02d:%02d.%06ld]%s [PID:%d, TID:0x%x] [" CPP_ASSISTANT_NAME "]: ",
            now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
            now.tm_hour, now.tm_min, now.tm_sec, tv.tv_usec, G_LOG_LEVEL_STRINGS[level],
            getpid(), gettid());
    }

    if (needs_color)
        fprintf(where, ((LOG_LEVEL_WARNING == level) ? WARNING_COLOR : ERROR_COLOR));

    vfprintf(where, format, args);

    if (needs_color)
        fprintf(where, NO_PRINT_ATTRIBUTES);

    if (needs_lock)
        lock->unlock();
}

#define CALL_FORMATED_OUTPUT(level, to_where, lock, contents)   \
    va_list args; \
\
    va_start(args, contents); \
    __formatted_output((int)level, to_where, lock, contents, args); \
    va_end(args)

static FILE *s_debug_output = stdout;

void __set_debug_output(const FILE *where)
{
    s_debug_output = const_cast<FILE *>(where);

    if (nullptr == s_debug_output)
        debug::m_debug_enabled = false;
    else
        debug::m_debug_enabled = true;
}

const FILE* __get_debug_output_holder(void)
{
    return s_debug_output;
}

/*bool __debug_is_enabled(void)
{
    return debug::g_debug_enabled;
}*/

bool __debug_macro_is_defined(void)
{
#ifdef DEBUG
    return true;
#else
    return false;
#endif
}

static mutex *s_debug_lock = nullptr;

void __set_debug_lock(const mutex *lock)
{
    s_debug_lock = const_cast<mutex *>(lock);
}

void __debug(const char *format, ...)/* CA_NOTNULL(1) CA_PRINTF_CHECK(1, 2) */
{
    CALL_FORMATED_OUTPUT(LOG_LEVEL_DEBUG, s_debug_output, s_debug_lock, format);
}

static FILE *s_error_output = stderr;

void __set_error_output(const FILE *where)
{
    s_error_output = const_cast<FILE *>(where);
}

const FILE* __get_error_output_holder(void)
{
    return s_error_output;
}

bool __error_report_is_enabled(void)
{
    return (nullptr != s_error_output);
}

static mutex *s_error_lock = nullptr;

void __set_error_lock(const mutex *lock)
{
    s_error_lock = const_cast<mutex *>(lock);
}

void __warn(const char *format, ...)/* CA_NOTNULL(1) CA_PRINTF_CHECK(1, 2) */
{
    CALL_FORMATED_OUTPUT(LOG_LEVEL_WARNING, s_error_output, s_error_lock, format);
}

void __error(const char *format, ...)/* CA_NOTNULL(1) CA_PRINTF_CHECK(1, 2) */
{
    CALL_FORMATED_OUTPUT(LOG_LEVEL_ERROR, s_error_output, s_error_lock, format);
}

CA_LIB_NAMESPACE_END
