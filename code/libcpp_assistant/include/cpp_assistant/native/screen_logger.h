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
 * screen_logger.h
 *
 *  Created on: 2017/09/17
 *      Author: wenxiongchang
 * Description: The logger recording log contents to screen.
 */

#ifndef __CPP_ASSISTANT_SCREEN_LOGGER_H__
#define __CPP_ASSISTANT_SCREEN_LOGGER_H__

#include <string.h>

#include <typeinfo>

#include "logger.h"

CA_LIB_NAMESPACE_BEGIN

class screen_logger : public logger
{
/* ===================================
 * constructors:
 * =================================== */
public:
    screen_logger();

/* ===================================
 * copy control:
 * =================================== */
private:
    screen_logger(const screen_logger& src);
    screen_logger& operator=(const screen_logger& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~screen_logger();

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
    //DEFINE_CLASS_NAME_FUNC()

    virtual int directory_length_limit(void)/*  = 0 */
    {
        return LOG_DIR_LEN_LIMIT_MIN;
    }

    virtual int set_directory_length_limit(int limit)/*  = 0 */
    {
        return CA_RET_OK;
    }

    virtual const char *log_name(void) const/*  = 0 */
    {
        return typeid(this).name();
    }

    virtual int set_log_name(const char *base_name = nullptr)/*  = 0 */
    {
        return CA_RET_OK;
    }

    virtual int name_length_limit(void)/*  = 0 */
    {
        return strlen(log_name());
    }

    virtual int set_name_length_limit(int limit)/*  = 0 */
    {
        return CA_RET_OK;
    }

    virtual int log_line_limit(void)/*  = 0 */
    {
        return LOG_LINE_LIMIT_MAX;
    }

    virtual int set_log_line_limit(int limit)/*  = 0 */
    {
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

/* ===================================
 * data:
 * =================================== */
protected:
    //DECLARE_CLASS_NAME_VAR();
};

typedef screen_logger terminal_logger;
typedef terminal_logger termlog;

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_SCREEN_LOGGER_H__

