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

/*
 * file_logger.h
 *
 *  Created on: 2017/09/17
 *      Author: wenxiongchang
 * Description: The logger recording log contents to file.
 */

#ifndef __CPP_ASSISTANT_FILE_LOGGER_H__
#define __CPP_ASSISTANT_FILE_LOGGER_H__

#include "logger.h"

CA_LIB_NAMESPACE_BEGIN

class file_logger : public logger
{
/* ===================================
 * constructors:
 * =================================== */
public:
    file_logger();

/* ===================================
 * copy control:
 * =================================== */
private:
    file_logger(const file_logger& src);
    file_logger& operator=(const file_logger& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~file_logger();

/* ===================================
 * abilities:
 * =================================== */
public:
    virtual int open(int cache_buf_size = DEFAULT_LOG_CACHE_SIZE)/*  = 0 */;
    virtual int close(bool release_buffer = true)/*  = 0 */;

/* ===================================
 * attributes:
 * =================================== */
public:
    DEFINE_CLASS_NAME_FUNC()

    virtual const char *log_directory(void) const
    {
        return m_log_directory;
    }

    // Sets directory where log files locate in.
    // NOTE: It can not be called when the logger is open.
    virtual int set_log_directory(const char *dir) CA_NOTNULL(2);

    virtual int directory_length_limit(void)/*  = 0 */
    {
        return m_dir_len_limit;
    }

    virtual int set_directory_length_limit(int limit)/*  = 0 */
    {
        return __adjust_path_length_limit(LOG_DIR_LEN_LIMIT_MAX,
            LOG_DIR_LEN_LIMIT_MIN,
            limit,
            m_dir_len_limit,
            m_log_directory);
    }

    virtual const char *log_name(void) const/*  = 0 */
    {
        return m_log_name;
    }

    // Sets name of the log file.
    // NOTE: It can not be called when the logger is open.
    virtual int set_log_name(const char *base_name)  CA_NOTNULL(2) /*  = 0 */;

    virtual int name_length_limit(void)/*  = 0 */
    {
        return m_name_len_limit;
    }

    virtual int set_name_length_limit(int limit)/*  = 0 */
    {
        return __adjust_path_length_limit(LOG_NAME_LEN_LIMIT_MAX,
            LOG_NAME_LEN_LIMIT_MIN,
            limit,
            m_name_len_limit,
            m_log_name);
    }

    virtual int log_line_limit(void)/*  = 0 */
    {
        return m_line_limit;
    }

    virtual int set_log_line_limit(int limit)/*  = 0 */
    {
        if (is_open())
            return CA_RET(DEVICE_BUSY);

        if (m_line_limit >= LOG_LINE_LIMIT_MIN && m_line_limit <= LOG_LINE_LIMIT_MAX)
            m_line_limit = limit;

        return CA_RET_OK;
    }

/* ===================================
 * status:
 * =================================== */
public:

/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    virtual int __switch_logger_status(void);

    int __update_log_num(void);

    int __innerly_set_log_name(const char *base_name, bool updates_log_num = false);

    int __adjust_path_length_limit(const int max_limit,
        const int min_limit,
        const int target_limit,
        int &path_limit_var,
        char* &path_holder_var);

/* ===================================
 * data:
 * =================================== */
protected:
    DECLARE_CLASS_NAME_VAR();
    //INHERIT_CLASS_NAME_VAR(logger);
    int m_name_len_limit;
    int m_dir_len_limit;
    int m_line_limit;
    char *m_log_name;
    char *m_log_directory;
    char *m_buffer;
};

typedef file_logger flog;

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_FILE_LOGGER_H__

