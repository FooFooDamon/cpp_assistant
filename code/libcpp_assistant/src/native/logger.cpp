/*
 * Copyright (c) 2017, Wen Xiongchang <udc577 at 126 dot com>
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

#include "logger.h"

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include <typeinfo>
#include <cctype>
#include <string>
#include <algorithm>

#include "base/ca_return_code.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

const bool HAS_LOG_PREFIX = true;
const bool NO_LOG_PREFIX = false;
const bool RELEASES_LOG_BUF_ON_CLOSE = true;
const bool NOT_RELEASE_LOG_BUF_ON_CLOSE = false;

DEFINE_CLASS_NAME(logger);

logger::logger()
    : m_log_level(LOG_LEVEL_ALL)
    , m_is_open(false)
    , m_output_holder(NULL)
    , m_cur_line(0)
    , m_log_num(0)
{
    memset(&m_date, 0, sizeof(struct tm));
#if 0 // It is meaningless in initialization of base class.
    std::string who;

    who.append(typeid(*this).name());
    std::transform(who.begin(), who.end(), who.begin(), tolower);
    if (NULL != strstr(who.c_str(), "screen") || NULL != strstr(who.c_str(), "terminal"))
        m_to_screen = true;
    else
        m_to_screen = false;
    cdebug("type: %s, m_to_screen: %d\n", who.c_str(), m_to_screen);
#else
    m_to_screen = false; // defaults to false.
#endif
}

logger::~logger()
{
    ;
}

/*virtual */int logger::prepare(const char *log_base_name,
    enum_log_level log_level/* = LOG_LEVEL_INFO*/,
    const char *log_dir/* = "."*/,
    int dir_len_limit/* = LOG_DIR_LEN_LIMIT_MAX*/,
    int name_len_limit/* = LOG_NAME_LEN_LIMIT_MAX*/,
    int line_limit/* = LOG_LINE_LIMIT_DEFAULT*/) /*  CA_NOTNULL(2,4) */
{
    /*typedef int (*set_limit_func)(int);
    typedef int (*set_path_func)(const char *);
    struct setting_config
    {
        const char *name;
        bool is_path;
        union
        {
            set_limit_func limit_func;
            set_path_func path_func;
        };
        union
        {
            int limit;
            const char *path;
        };
    } settings[] = {
        { "log directory length limit", false,  set_directory_length_limit, dir_len_limit   },
        { "log name length limit",      false,  set_name_length_limit,      name_len_limit  },
        { "log line limit",             false,  set_log_line_limit,         line_limit      },
        { "log directory",              true,   set_log_directory,          log_dir         },
        { "log name",                   true,   set_log_name,               log_base_name   }
    };*/
    int ret = CA_RET(GENERAL_FAILURE);

    set_log_level(log_level);

    /*for (unsigned int i = 0; i < sizeof(settings) / sizeof(setting_config); ++i)
    {
        struct setting_config &item = settings[i];

        ret = item.is_path ? item.path_func(item.path) : item.limit_func(item.limit);
        if (CA_RET_OK != ret)
        {
            cerror("failed to set %s\n", item.name);
            return ret;
        }
    }*/

    if (CA_RET_OK != (ret = set_directory_length_limit(dir_len_limit)))
    {
        cerror("failed to set log directory length limit\n");
        return ret;
    }

    if (CA_RET_OK != (ret = set_name_length_limit(name_len_limit)))
    {
        cerror("failed to set log name length limit\n");
        return ret;
    }

    if (CA_RET_OK != (ret = set_log_line_limit(line_limit)))
    {
        cerror("failed to set log line limit\n");
        return ret;
    }

    if (CA_RET_OK != (ret = set_log_directory(log_dir)))
    {
        cerror("failed to set log directory\n");
        return ret;
    }

    if (CA_RET_OK != (ret = set_log_name(log_base_name)))
    {
        cerror("failed to set log name\n");
        return ret;
    }

    return CA_RET_OK;
}

void logger::flush(void)
{
    if (!is_open())
        return;

    fflush(m_output_holder);
}

const char *G_LOG_LEVEL_STRINGS[] = { "[D]", "[I]", "[W]", "[E]", "[C]" };

int logger::output(bool has_prefix,
    enum_log_level log_level,
    const char *fmt,
    va_list args) /* CA_NOTNULL(4) */
{
    if (NULL == fmt)
        return CA_RET(NULL_PARAM);

    if (!is_open())
        return CA_RET(FILE_OR_STREAM_NOT_OPEN);

    if (log_level < m_log_level)
        return 0;

    bool needs_color = (m_to_screen && log_level >= LOG_LEVEL_WARNING);
    FILE *destination = needs_color ? stderr : m_output_holder;
    struct timeval tv;
    struct tm now;

    gettimeofday(&tv, NULL);
    localtime_r((time_t *)&(tv.tv_sec), &now);

    /*
     * If the logger needs to switch, save previous contents first and do switching.
     */
    if (m_cur_line >= log_line_limit() || now.tm_mday != m_date.tm_mday)
    {
        /*cdebug("m_to_screen: %d, needs_color: %d, converted destination fd: %d\n",
            m_to_screen, needs_color, fileno(destination));*/

        fflush(m_output_holder);
        if (needs_color)
            fflush(stderr);

        int switch_ret = __switch_logger_status();

        if (CA_RET_OK != switch_ret)
            return switch_ret;

        destination = needs_color ? stderr : m_output_holder; // NOTE: MUST refresh the file handle after switching.
    }

    /*
     * Then, continue to write contents of this time.
     */

    int write_ret = 0;

    if (has_prefix)
    {
        const char *LEVEL_STR = G_LOG_LEVEL_STRINGS[log_level % LOG_LEVEL_COUNT];

#if 0 // Enable it when you want to test the "wired problem" in unittest/test_file_logger.cpp.
        if (debug_is_enabled())
            fprintf(destination, "%08d: ", m_cur_line + 1);
#endif

        if (!m_to_screen)
            /*write_ret += */fprintf(destination, "[%02d:%02d:%02d.%06ld]%s ",
                now.tm_hour, now.tm_min, now.tm_sec, tv.tv_usec, LEVEL_STR);
        else
            /*write_ret += */fprintf(destination, "[%04d-%02d-%02d %02d:%02d:%02d.%06ld]%s ",
                now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, \
                now.tm_hour, now.tm_min, now.tm_sec, tv.tv_usec, LEVEL_STR);
    }

    if (needs_color)
    {
        // More readable but will cause a compile warning.
        //const char *kColorStr = (LOG_LEVEL_WARNING == log_level) ? WARNING_COLOR : ERROR_COLOR;

        fprintf(destination, ((LOG_LEVEL_WARNING == log_level) ? WARNING_COLOR : ERROR_COLOR));
    }

    write_ret += vfprintf(destination, fmt, args);

    if (needs_color)
        fprintf(destination, NO_PRINT_ATTRIBUTES);

    ++m_cur_line;

    return write_ret;
}

#define CALL_FORMATED_OUTPUT(has_prefix, level, contents)   \
    va_list args; \
    int ret; \
\
    va_start(args, contents); \
    ret = output(has_prefix, level, contents, args); \
    va_end(args); \
\
    return ret

int logger::output(bool has_prefix,
    enum_log_level log_level,
    const char *fmt,
    ...) /* CA_NOTNULL(4) CA_PRINTF_CHECK(4, 5) */
{
    CALL_FORMATED_OUTPUT(has_prefix, log_level, fmt);
}

int logger::debug(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_DEBUG, fmt);
}

int logger::d(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_DEBUG, fmt);
}

int logger::info(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_INFO, fmt);
}

int logger::i(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_INFO, fmt);
}

int logger::warn(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_WARNING, fmt);
}

int logger::w(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_WARNING, fmt);
}

int logger::error(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_ERROR, fmt);
}

int logger::e(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_ERROR, fmt);
}

int logger::critical(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_CRITICAL, fmt);
}

int logger::c(const char *fmt, ...) /* CA_NOTNULL(2)  CA_PRINTF_CHECK(2, 3) */
{
    CALL_FORMATED_OUTPUT(true, LOG_LEVEL_CRITICAL, fmt);
}

CA_LIB_NAMESPACE_END
