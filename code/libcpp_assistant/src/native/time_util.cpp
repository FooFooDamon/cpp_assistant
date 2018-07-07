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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include "time_util.h"
#include "base/ca_return_code.h"
#include "base/platforms/threading.h"
#include "private/debug.h"

extern long int timezone; // defined and modified by system

CA_LIB_NAMESPACE_BEGIN

DEFINE_CLASS_NAME(time_util);

static mutex s_lock;
bool time_util::m_timezone_is_initialized = false;
const uint32_t time_util::SECS_FROM_1900_TO_1970 = 2208988800;

time_util::time_util()
{
    ;
}

time_util::~time_util()
{
    ;
}

time_util::time_util(const time_util& src)
{
    ;
}

const time_util& time_util::operator=(const time_util& src)
{
    return *this;
}

/*static */CA_THREAD_SAFE int time_util::get_time_zone(void)
{
    if (!m_timezone_is_initialized)
    {
        lock_guard<mutex> lock(s_lock);
        if (!m_timezone_is_initialized)
        {
            csdebug(time_util, "Initializing time zone ...\n");
            tzset(); // Lets system select the best approximate time zone
            m_timezone_is_initialized = true;
        }
    }

    return -timezone / 3600; //m_timezone_is_initialized;
}

/*static */CA_THREAD_SAFE int time_util::set_time_zone(const int zone_num)
{
    if (zone_num < MIN_TIME_ZONE || zone_num > MAX_TIME_ZONE)
        return CA_RET(VALUE_OUT_OF_RANGE);

    const char *tz_name = "TZ";
    const int overwrite_flag = 1;
    char tz_value[16] = {0};

    if (zone_num > 0)
        snprintf(tz_value, sizeof(tz_value), "GMT-%d", abs(zone_num));
    else
        snprintf(tz_value, sizeof(tz_value), "GMT+%d", abs(zone_num));

    lock_guard<mutex> lock(s_lock);

    if (0 != setenv(tz_name, tz_value, overwrite_flag))
        return -errno;

    tzset(); // Makes system read TZ environment value and change time zone depending on it

    return CA_RET_OK;
}

/*static */int64_t time_util::get_utc_seconds(void)
{
    return (int64_t)(time(NULL)) + get_seconds_from_1900_to_1970();
}

/*static */int64_t time_util::get_utc_microseconds(void)
{
    struct timeval tv;
    int64_t sum = 0;

    gettimeofday(&tv, NULL);

    sum = (int64_t)(tv.tv_sec) + get_seconds_from_1900_to_1970();
    sum *= ONE_SEC_USECS;
    sum += tv.tv_usec;

    return sum;
}

/*static */int64_t time_util::get_local_seconds(void)
{
    return get_utc_seconds() + (int64_t)get_time_zone() * ONE_HOUR_SECS;
    // TODO: Or try this: mktime(localtime(&(time(NULL))). Of course, use their *_r() versions.
}

/*static */int64_t time_util::get_local_microseconds(void)
{
    return get_utc_microseconds() + (int64_t)get_time_zone() * ONE_HOUR_SECS * ONE_SEC_USECS;
}

CA_LIB_NAMESPACE_END
