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
    LOG_LEVEL_DEBUGGING = LOG_LEVEL_DEBUG,
    LOG_LEVEL_D = LOG_LEVEL_DEBUG,
    LOG_LEVEL_debug = LOG_LEVEL_DEBUG,
    LOG_LEVEL_debugging = LOG_LEVEL_DEBUG,
    LOG_LEVEL_d = LOG_LEVEL_DEBUG,

    LOG_LEVEL_INFO,
    LOG_LEVEL_I = LOG_LEVEL_INFO,
    LOG_LEVEL_info = LOG_LEVEL_INFO,
    LOG_LEVEL_i = LOG_LEVEL_INFO,

    LOG_LEVEL_WARNING,
    LOG_LEVEL_WARN = LOG_LEVEL_WARNING,
    LOG_LEVEL_W = LOG_LEVEL_WARNING,
    LOG_LEVEL_warning = LOG_LEVEL_WARNING,
    LOG_LEVEL_warn = LOG_LEVEL_WARNING,
    LOG_LEVEL_w = LOG_LEVEL_WARNING,

    LOG_LEVEL_ERROR,
    LOG_LEVEL_E = LOG_LEVEL_ERROR,
    LOG_LEVEL_error = LOG_LEVEL_ERROR,
    LOG_LEVEL_e = LOG_LEVEL_ERROR,

    LOG_LEVEL_CRITICAL,
    LOG_LEVEL_C = LOG_LEVEL_CRITICAL,
    LOG_LEVEL_critical = LOG_LEVEL_CRITICAL,
    LOG_LEVEL_c = LOG_LEVEL_CRITICAL,

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
     * and their aliases(e.g.: d() is the alias of debug(), Java developers may like it).
     */

    int debug(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int d(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int DEBUGGING(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3); // DEBUG may be used as a macro somewhere.
    int D(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

    int info(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int i(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int INFO(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int I(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

    int warn(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int warning(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int w(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int WARN(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int WARNING(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int W(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

    int error(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int e(const char *fmt, ...) CA_NOTNULL(2);
    int ERROR(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int E(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

    int critical(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int c(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int CRITICAL(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);
    int C(const char *fmt, ...) CA_NOTNULL(2) CA_PRINTF_CHECK(2, 3);

/* ===================================
 * attributes:
 * =================================== */
public:

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
    //    process ID, etc. However, not all kinds of logger need a name, just assign nullptr
    //    to @base_name or ignore this argument if the log name is not needed.
    virtual int set_log_name(const char *base_name = nullptr) = 0;

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

    // Same as the above, except that this one is mutable.
    inline FILE* output_holder(void)
    {
        return m_output_holder;
    }

    inline logger& get_stream(enum_log_level log_level, bool starts_a_new_line = true)
    {
        m_instant_level = (enum_log_level)(log_level % LOG_LEVEL_COUNT);

        if (!is_open() || !level_enough(m_instant_level))
            return *this;

        if (starts_a_new_line && current_log_lines() > 0)
            fputs("\n", m_output_holder);

        output(HAS_LOG_PREFIX, m_instant_level, "%s", "");

        return *this;
    }

/* ===================================
 * status:
 * =================================== */
public:
    inline bool is_open(void) const
    {
        return m_is_open && (nullptr != m_output_holder);
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
    inline logger& operator<<(const enum_log_level input)
    {
        //return operator << <enum_log_level>(input);
        return stream_out<enum_log_level>(input, "%u");
    }

    inline logger& operator<<(const enum_log_line_limit input)
    {
        //return operator << <enum_log_line_limit>(input);
        return stream_out<enum_log_line_limit>(input, "%u");
    }

    inline logger& operator<<(const enum_log_cache_size input)
    {
        //return operator << <enum_log_cache_size>(input);
        return stream_out<enum_log_cache_size>(input, "%u");
    }

    inline logger& operator<<(const enum_log_path_len_limit input)
    {
        //return operator << <enum_log_path_len_limit>(input);
        return stream_out<enum_log_path_len_limit>(input, "%u");
    }

    inline logger& operator<<(const bool input)
    {
        if (is_open() && level_enough(m_instant_level))
        {
            color_control_start();

            if (input)
                fputs("true", m_output_holder); // fwrite("true", 4, 1, m_output_holder);
            else
                fputs("false", m_output_holder); // fwrite("false", 5, 1, m_output_holder);

            color_control_end();
        }

        return *this;
    }

    inline logger& operator<<(const float input)
    {
        //return operator << <float>(input);
        return stream_out<float>(input, "%f");
    }

    inline logger& operator<<(const double input)
    {
        //return operator << <double>(input);
        return stream_out<double>(input, "%lf");
    }

    inline logger& operator<<(const long double input)
    {
        //return operator << <long double>(input);
        return stream_out<long double>(input, "%Lf");
    }

    inline logger& operator<<(const char input)
    {
        //return operator << <char>(input);
        return stream_out<char>(input, "%hhd");
    }

    inline logger& operator<<(const unsigned char input)
    {
        //return operator << <unsigned char>(input);
        return stream_out<unsigned char>(input, "%hhu");
    }

    inline logger& operator<<(const short int input)
    {
        //return operator << <short int>(input);
        return stream_out<short int>(input, "%hd");
    }

    inline logger& operator<<(const unsigned short int input)
    {
        //return operator << <unsigned short int>(input);
        return stream_out<unsigned short int>(input, "%hu");
    }

    inline logger& operator<<(const int input)
    {
        //return operator << <int>(input);
        return stream_out<int>(input, "%d");
    }

    inline logger& operator<<(const unsigned int input)
    {
        //return operator << <unsigned int>(input);
        return stream_out<unsigned int>(input, "%u");
    }

    inline logger& operator<<(const long int input)
    {
        //return operator << <long int>(input);
        return stream_out<long int>(input, "%ld");
    }

    inline logger& operator<<(const unsigned long int input)
    {
        //return operator << <unsigned long int>(input);
        return stream_out<unsigned long int>(input, "%lu");
    }

    inline logger& operator<<(const long long int input)
    {
        //return operator << <long long int>(input);
        return stream_out<long long int>(input, "%lld");
    }

    inline logger& operator<<(const unsigned long long int input)
    {
        //return operator << <unsigned long long int>(input);
        return stream_out<unsigned long long int>(input, "%llu");
    }

    inline logger& operator<<(const char *input)
    {
        if (is_open() && level_enough(m_instant_level) && nullptr != input)
        {
            color_control_start();
            fputs(input, m_output_holder);
            color_control_end();
        }

        return *this;
    }

    inline logger& operator<<(const std::string &input)
    {
        if (is_open() && level_enough(m_instant_level) && input.length() > 0)
        {
            color_control_start();
            fputs(input.c_str(), m_output_holder);
            color_control_end();
        }

        return *this;
    }

/* ===================================
 * private methods:
 * =================================== */
protected:
    // Default implementation of the function switching logger status.
    virtual int __switch_logger_status(void)
    {
        return CA_RET_OK;
    }

    template<typename T>
    inline logger& operator<<(const T input) // TODO: does not work
    {
        if (is_open() && level_enough(m_instant_level))
        {
            color_control_start();
            fwrite((T*)&input, sizeof(T), 1, m_output_holder);
            color_control_end();
        }

        return *this;
    }

#if 0
    template<typename T, const char *fmt>
    inline logger& operator<<(const T input)
    {
        if (is_open() && level_enough(m_instant_level))
        {
            color_control_start();
            fprintf(m_output_holder, fmt, input);
            color_control_end();
        }

        return *this;
    }
#else
    template<typename T>
    inline logger& stream_out(const T input, const char *fmt)
    {
        if (is_open() && level_enough(m_instant_level))
        {
            color_control_start();
            fprintf(m_output_holder, fmt, input);
            color_control_end();
        }

        return *this;
    }
#endif

    void color_control_start(void)
    {
        if (!m_to_screen)
            return;

        if (LOG_LEVEL_WARNING == m_instant_level)
            fprintf(m_output_holder, "\033[1;33m");
        else if (LOG_LEVEL_ERROR == m_instant_level || LOG_LEVEL_CRITICAL == m_instant_level)
            fprintf(m_output_holder, "\033[1;31m");
        else;
    }

    void color_control_end(void)
    {
        if (!m_to_screen)
            return;

        if (m_instant_level >= LOG_LEVEL_WARNING)
            fprintf(m_output_holder, "\033[0m");
    }

    inline bool level_enough(enum_log_level level)
    {
        return level >= m_log_level;
    }

/* ===================================
 * data:
 * =================================== */
protected:
    enum_log_level m_log_level;
    enum_log_level m_instant_level;
    bool m_is_open;
    FILE *m_output_holder;
    int m_cur_line;
    int m_log_num;
    struct tm m_date;
    bool m_to_screen;
};

CA_LIB_NAMESPACE_END

#ifndef NO_CA_LOG

/*
 * NOTE: The logging macros below are convenient to use when writing simple programs,
 * but you probably need to redefine them in your own file when writing complex programs and needing high performance.
 */

#ifdef GET_LOG_INSTANCE
#undef GET_LOG_INSTANCE
#endif
#define GET_LOG_INSTANCE()                              CA_LIB_NAMESPACE::singleton<CA_LIB_NAMESPACE::screen_logger>::get_instance()

#ifdef GET_LOG_HOLDER
#undef GET_LOG_HOLDER
#endif
#define GET_LOG_HOLDER()                                GET_LOG_INSTANCE()->output_holder()

/*
 * [F]ormatted logging macros. x could be:
 *     d, debug, D, DEBUGGING (DEBUG should not be used because it's generally used as a debug controlling macro somewhere).
 *     i, info, I, INFO,
 *     w, warn, W, WARN,
 *     e, error, E, ERROR,
 *     c, critical, C, CRITICAL.
 */

// R is short for raw, which means outputting log contents as little as possible.
#ifdef RLOGF
#undef RLOGF
#endif
#define RLOGF(x, fmt, ...)                              GET_LOG_INSTANCE()->x(fmt, ##__VA_ARGS__)

#ifdef MULTI_THREADING

#define LOGF_BASE(x, class_str, ns_delim, fmt, ...)     do {\
    CA_LIB_NAMESPACE::lock_guard<CA_LIB_NAMESPACE::mutex> lock(*CA_LIB_NAMESPACE::singleton<CA_LIB_NAMESPACE::mutex>::get_instance()); \
    RLOGF(x, "[%d#%d] %s:%d, %s%s%s(): " fmt, getpid(), CA_LIB_NAMESPACE::gettid(), __FILE__, __LINE__, class_str, ns_delim, __FUNCTION__, ##__VA_ARGS__); \
} while(0)

#else

#define LOGF_BASE(x, class_str, ns_delim, fmt, ...)     RLOGF(x, "%s:%d, %s%s%s(): " fmt, __FILE__, __LINE__, class_str, ns_delim, __FUNCTION__, ##__VA_ARGS__)

#endif

#ifdef LOGF
#undef LOGF
#endif
#define LOGF(x, fmt, ...)                               LOGF_BASE(x, "", "", fmt, ##__VA_ARGS__)

// C is short for class, which means outputting name of the class calling this logging function.
#ifdef LOGF_C
#undef LOGF_C
#endif
#define LOGF_C(x, fmt, ...)                             LOGF_BASE(x, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)

// NS is short for namespace, which means outputting the current namespace.
#ifdef LOGF_NS
#undef LOGF_NS
#endif
#define LOGF_NS(x, _namespace_, fmt, ...)               LOGF_BASE(x, #_namespace_, "::", fmt, ##__VA_ARGS__)

/*
 * [S]tream-style logging macros. Value of x is the same as formatted logging.
 */

// R means the the same as the one in the formatted logging above.
#ifdef RLOGS
#undef RLOGS
#endif
#ifdef MULTI_THREADING // TODO: has not supported multi-threading so far, because locking is not easy.
#define RLOGS(x)
#else
#define RLOGS(x)                                        GET_LOG_INSTANCE()->get_stream(CA_LIB_NAMESPACE::LOG_LEVEL_##x)
#endif

#ifdef LOGS
#undef LOGS
#endif
#define LOGS(x)                                         RLOGS(x) << __FILE__ << ":" << __LINE__ << ", " << __FUNCTION__ << "(): "

// C means the the same as the one in the formatted logging above.
#ifdef LOGS_C
#undef LOGS_C
#endif
#define LOGS_C(x)                                       RLOGS(x) << __FILE__ << ":" << __LINE__ << ", " << typeid(*this).name() << "::" << __FUNCTION__ << "(): "

// NS means the the same as the one in the formatted logging above.
#ifdef LOGS_NS
#undef LOGS_NS
#endif
#define LOGS_NS(x, _namespace_)                         RLOGS(x) << __FILE__ << ":" << __LINE__ << ", " << #_namespace_ << "::" << __FUNCTION__ << "(): "

#endif // NO_CA_LOG

#endif // __CPP_ASSISTANT_LOG_BASE_H__

