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
 * logger.h
 *
 *  Created on: 2017/09/17
 *      Author: wenxiongchang
 * Description: Base class of practical logger classes.
 */

#ifndef __CPP_ASSISTANT_LOG_BASE_H__
#define __CPP_ASSISTANT_LOG_BASE_H__

#include <stdio.h>
#include <time.h>

#include "base/ca_inner_necessities.h"
#include "base/ca_return_code.h"

CA_LIB_NAMESPACE_BEGIN

enum enum_log_level
{
    LOG_LEVEL_MIN = 0,
    LOG_LEVEL_ALL = LOG_LEVEL_MIN,

    LOG_LEVEL_DEBUG = LOG_LEVEL_MIN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL,

    LOG_LEVEL_MAX = LOG_LEVEL_CRITICAL,
    LOG_LEVEL_COUNT,
    LOG_LEVEL_NONE
};

enum enum_log_line_limit
{
    LOG_LINE_LIMIT_MIN = 1000,
    LOG_LINE_LIMIT_MAX = 1000000,
    LOG_LINE_LIMIT_DEFAULT = 10000
};

enum enum_log_cache_size
{
    MIN_LOG_CACHE_SIZE = 64 * 1024,
    DEFAULT_LOG_CACHE_SIZE = 2 * 1024 * 1024,
    MAX_LOG_CACHE_SIZE = 16 * 1024 * 1024
};

enum enum_log_path_len_limit
{
    LOG_DIR_LEN_LIMIT_MIN = 1,
    LOG_DIR_LEN_LIMIT_DEFAULT = 127,
    LOG_DIR_LEN_LIMIT_MAX = 2 * 1024 * 1024 - 1,

    LOG_NAME_LEN_LIMIT_MIN = LOG_DIR_LEN_LIMIT_MIN,
    LOG_NAME_LEN_LIMIT_DEFAULT = LOG_DIR_LEN_LIMIT_DEFAULT,
    LOG_NAME_LEN_LIMIT_MAX = LOG_DIR_LEN_LIMIT_MAX
};

extern const bool HAS_LOG_PREFIX/* = true*/;
extern const bool NO_LOG_PREFIX/* = false*/;
extern const bool RELEASES_LOG_BUF_ON_CLOSE/* = true*/;
extern const bool NOT_RELEASE_LOG_BUF_ON_CLOSE/* = false*/;

class logger
{
/* ===================================
 * constructors:
 * =================================== */
public:
    logger();

/* ===================================
 * copy control:
 * =================================== */
private:
    logger(const logger& src);
    const logger& operator=(const logger& src);

/* ===================================
 * types:
 * =================================== */
public:
    typedef int (*FormattedOutput)(const char *fmt, ...);

/* ===================================
 * destructor:
 * =================================== */
public:
    virtual ~logger();

/* ===================================
 * abilities:
 * =================================== */
public:
    // Prepares resources and operations before opening the logger.
    // Actually it is just a combination of set_log_level(), set_log_directory(), etc.
    virtual int prepare(const char *log_base_name,
        enum_log_level log_level = LOG_LEVEL_INFO,
        const char *log_dir = ".",
        int dir_len_limit = LOG_DIR_LEN_LIMIT_MAX,
        int name_len_limit = LOG_NAME_LEN_LIMIT_MAX,
        int line_limit = LOG_LINE_LIMIT_DEFAULT) CA_NOTNULL(2,4);

    // Opens the logger with its cache buffer size being set to @cache_buf_size,
    // note that we do not support dynamic modification to this parameter in run-time.
    virtual int open(int cache_buf_size = DEFAULT_LOG_CACHE_SIZE) = 0;

    // Closes the logger with or without the cache buffer being released.
    // It is called in such cases:
    //    1) When the logger switches status(@release_buffer == false).
    //    2) When the logger is about to terminate (@release_buffer == true).
    virtual int close(bool release_buffer = true) = 0;

    // Flushes log contents to destination (that is: a file or the terminal).
    // This function can be called by user or by other member functions.
    void flush(void);

    /*
     * Outputs log contents to cache or to a certain destination (that is: a file or the terminal).
     * NOTE: The logger probably caches log contents in memory and flushes them to destination
     *     when it's proper. This is helpful to performance improvement.
     * Different derived classes can have different implementations.
     */

    int output(bool has_prefix,
        enum_log_level log_level,
        const char *fmt,
        va_list args) CA_NOTNULL(4);

    int output(bool has_prefix,
        enum_log_level log_level,
        const char *fmt,
        ...) CA_NOTNULL(4) CA_PRINTF_CHECK(4, 5);

    /*
     * Functions below are specific/concrete versions of output(),
     * and their alias(e.g.: d() is the alias of debug(), Java developers may like it).
     */

    int debug(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int d(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

    int info(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int i(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

    int warn(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int w(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

    int error(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int e(const char *fmt, ...) CA_NOTNULL(2);

    int critical(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int c(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

/* ===================================
 * attributes:
 * =================================== */
public:
    DEFINE_CLASS_NAME_FUNC()

    inline enum_log_level log_level(void) const
    {
        return m_log_level;
    }

    inline void set_log_level(enum_log_level log_level)
    {
        m_log_level = (enum_log_level)(log_level % LOG_LEVEL_COUNT);
    }

    // A default implementation of getting directory in order to support polymorphism.
    virtual const char *log_directory(void) const
    {
        return ".";
    }

    // A default implementation of setting directory in order to support polymorphism.
    virtual int set_log_directory(const char *dir)
    {
        return CA_RET_OK;
    }

    // Directory length limit restricts how long a directory string can be.
    virtual int directory_length_limit(void) = 0;

    virtual int set_directory_length_limit(int limit) = 0;

    virtual const char *log_name(void) const = 0;

    // Sets the name of a logger.
    // NOTE: The name of a logger consists of multiple parts such as base name, date info,
    //    process ID, etc. However, not all kinds of logger need a name, just assign NULL
    //    to @base_name or ignore this argument if the log name is not needed.
    virtual int set_log_name(const char *base_name = NULL) = 0;

    // Name length limit restricts how long a log name string can be.
    virtual int name_length_limit(void) = 0;

    virtual int set_name_length_limit(int limit) = 0;

    // Log line limit restricts how many lines a logger fragment can hold at most.
    virtual int log_line_limit(void) = 0;

    virtual int set_log_line_limit(int limit) = 0;

    // Gets the output holder of a logger.
    // Output holder is the destination where log contents are output.
    // It can be a file handle, stdout, stderr, etc.
    inline const FILE* output_holder(void) const
    {
        return m_output_holder;
    }

/* ===================================
 * status:
 * =================================== */
public:
    inline bool is_open(void) const
    {
        return m_is_open && (NULL != m_output_holder);
    }

    // Shows how many lines have been output by the logger in current fragment.
    inline int current_log_lines(void) const
    {
        return m_cur_line;
    }

    // To prevent the initial generated stuff, generally a log file, becoming too big,
    // we split it into multiple fragments if needed, and give each of them a number.
    // This function shows how many fragments are generated currently.
    inline int current_log_fragments(void) const
    {
        return m_log_num;
    }

/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    // Default implementation of the function switching logger status.
    virtual int __switch_logger_status(void)
    {
        return CA_RET_OK;
    }

/* ===================================
 * data:
 * =================================== */
protected:
    DECLARE_CLASS_NAME_VAR();
    enum_log_level m_log_level;
    bool m_is_open;
    FILE *m_output_holder;
    int m_cur_line;
    int m_log_num;
    struct tm m_date;
    bool m_to_screen;
};

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_LOG_BASE_H__

