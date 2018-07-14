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

#include "common_headers.h"

#include "base/ca_return_code.h"
#include "daemon.h"

static void func_before_daemonization(void)
{
    printf("%s(): Register me before daemonization and see what will happen.\n", __FUNCTION__);
}

static void func_after_daemonization(void)
{
    printf("%s(): Register me after daemonization and see what will happen.\n", __FUNCTION__);
}

static int func_with_int_ret(void)
{
    printf("%s(): Register me and see what will happen.\n", __FUNCTION__);
    return 0;
}

static int func_with_ret_and_param(const char *param)
{
    if (nullptr == param)
    {
        printf("%s(): param is null.\n", __FUNCTION__);
        return -1;
    }

    printf("%s(): param is [%s].\n", __FUNCTION__, param);
    return 0;
}

TEST(daemon, AllInOne)
{
    ASSERT_FALSE(calib::daemon::is_daemonized());

    ASSERT_EQ(CA_RET(NULL_PARAM), calib::daemon::register_clean_funtion(nullptr));
    ASSERT_EQ(CA_RET(OK), calib::daemon::register_clean_funtion(func_before_daemonization));

    calib::daemon::daemonize(/*true, true*/);
    calib::daemon::daemonize(/*true, true*/); // Does it again and should have no effect.

    ASSERT_TRUE(calib::daemon::is_daemonized());
    ASSERT_EQ(CA_RET(OK), calib::daemon::register_clean_funtion(func_after_daemonization));
    ASSERT_EQ(CA_RET(OK), calib::daemon::register_clean_funtion(reinterpret_cast<calib::daemon::clean_func>(func_with_int_ret)));
    ASSERT_EQ(CA_RET(OK), calib::daemon::register_clean_funtion(reinterpret_cast<calib::daemon::clean_func>(func_with_ret_and_param)));
}

