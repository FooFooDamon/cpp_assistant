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
 * ca_time.h
 *
 *  Created on: 2017/09/22
 *      Author: wenxiongchang
 * Description: For time usage.
 */

#ifndef __CPP_ASSISTANT_TIME_H__
#define __CPP_ASSISTANT_TIME_H__

#include <stdint.h>

#include <typeinfo>

#include "base/ca_inner_necessities.h"

CA_LIB_NAMESPACE_BEGIN

class Time
{
/* ===================================
 * constructors:
 * =================================== */
public:
    Time();

/* ===================================
 * copy control:
 * =================================== */
private:
    Time(const Time& src);
    const Time& operator=(const Time& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~Time();

/* ===================================
 * types:
 * =================================== */
public:
    enum
    {
        ONE_DAY_HOURS = 24,
        ONE_DAY_SECS = 24 * 60 * 60,
        ONE_HOUR_SECS = 60 * 60,
        ONE_MIN_SECS = 60,
        ONE_MIN_USECS = 60000000,
        ONE_SEC_USECS = 1000000,
        ONE_WEEK_DAYS = 7,
        LEAP_MONTH_DAYS = 31
    };

    enum
    {
        MIN_TIME_ZONE = -12,
        MAX_TIME_ZONE = 12
    };

/* ===================================
 * abilities:
 * =================================== */
public:
    static CA_THREAD_SAFE int GetTimeZone(void);
    static CA_THREAD_SAFE int SetTimeZone(const int zone_num);

    static inline CA_REENTRANT uint32_t GetSecondsFrom1900To1970(void)
    {
        return SECS_FROM_1900_TO_1970;
    }

    /*
     * GetUtcXXX(): Gets current time value of a specified type in time zone 0,
     * which starts from 1900 AD.
     */
    static int64_t GetUtcSeconds(void);
    static int64_t GetUtcMicroseconds(void);

    /*
     * Gets current time value of a specified type in local place,
     * which starts from 1900 AD.
     */
    static int64_t GetLocalSeconds(void);
    static int64_t GetLocalMicroseconds(void);

/* ===================================
 * attributes:
 * =================================== */
public:
    DEFINE_CLASS_NAME_FUNC()

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
    DECLARE_CLASS_NAME_VAR();
    static bool m_timezone_is_initialized;
    static const uint32_t SECS_FROM_1900_TO_1970;
};

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_TIME_H__ */
