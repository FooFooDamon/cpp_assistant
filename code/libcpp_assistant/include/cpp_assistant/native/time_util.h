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
 * time_util.h
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

class time_util
{
/* ===================================
 * constructors:
 * =================================== */
public:
    time_util();

/* ===================================
 * copy control:
 * =================================== */
private:
    time_util(const time_util& src);
    const time_util& operator=(const time_util& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~time_util();

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
    static CA_THREAD_SAFE int get_time_zone(void);
    static CA_THREAD_SAFE int set_time_zone(const int zone_num);

    static inline CA_REENTRANT uint32_t get_seconds_from_1900_to_1970(void)
    {
        return SECS_FROM_1900_TO_1970;
    }

    /*
     * get_utc_xxx(): Gets current time value of a specified type in time zone 0,
     * which starts from 1900 AD.
     */
    static int64_t get_utc_seconds(void);
    static int64_t get_utc_microseconds(void);

    /*
     * Gets current time value of a specified type in local place,
     * which starts from 1900 AD.
     */
    static int64_t get_local_seconds(void);
    static int64_t get_local_microseconds(void);

/* ===================================
 * attributes:
 * =================================== */
public:

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
    static bool m_timezone_is_initialized;
    static const uint32_t SECS_FROM_1900_TO_1970;
};

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_TIME_H__ */
