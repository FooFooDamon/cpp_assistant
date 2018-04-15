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

#include "time_util.h"
#include "common_headers.h"

#include "base/ca_return_code.h"

TEST(time_util, AllInOne)
{
    int init_timezone = calib::time_util::get_time_zone();
    int after_timezone = 0;
    uint32_t sec_difference = calib::time_util::get_seconds_from_1900_to_1970();

    printf("Seconds elapsed from 1900 to 1970: %u\n", sec_difference);
    //ASSERT_EQ(calib::time::SECS_FROM_1900_TO_1970, sec_difference);

    printf("Initial time zone: %d\n", init_timezone);
    ASSERT_TRUE(init_timezone >= -12 && init_timezone <= 12);
    for (int i = -13; i <= 13; ++i)
    {
        int before_timezone = calib::time_util::get_time_zone();
        int set_ret = calib::time_util::set_time_zone(i);
        after_timezone = calib::time_util::get_time_zone();

        if (-13 == i || 13 == i)
        {
            ASSERT_EQ(CA_RET(VALUE_OUT_OF_RANGE), set_ret);
            ASSERT_EQ(before_timezone, after_timezone);
            continue;
        }

        ASSERT_EQ(CA_RET(OK), set_ret);
        ASSERT_EQ(i, after_timezone);

        int64_t utc_secs = calib::time_util::get_utc_seconds();
        int64_t utc_usecs = calib::time_util::get_utc_microseconds();
        int64_t local_secs = calib::time_util::get_local_seconds();
        int64_t local_usecs = calib::time_util::get_local_microseconds();
        int difference_secs = local_secs - utc_secs;

        printf("================ Time zone %d ================\n", after_timezone);
        printf("UTC seconds:         %ld\n", utc_secs);
        printf("UTC micronseconds:   %ld\n", utc_usecs);
        printf("Local seconds:       %ld\n", local_secs);
        printf("Local micronseconds: %ld\n", local_usecs);
        printf("Difference between time zone 0 and local area: %02d:%02d:%02d\n",
            difference_secs / 3600, abs(difference_secs % 3600 / 60), abs(difference_secs % 60));
        ASSERT_GE(utc_usecs, utc_secs * 1000000);
        ASSERT_GE(local_usecs, local_secs * 1000000);
        ASSERT_EQ(i, difference_secs / 3600);
        if (i >= 0)
            ASSERT_GE(local_secs, utc_secs + i * 3600);
        else
            ASSERT_GE(utc_secs, local_secs + i * 3600);
    }
}

