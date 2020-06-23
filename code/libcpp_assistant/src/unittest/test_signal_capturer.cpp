/*
 * Copyright (c) 2017-2020, Wen Xiongchang <udc577 at 126 dot com>
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

#include "signal_capturer.h"
#include "common_headers.h"

#include "base/ca_return_code.h"

static bool is_critical_signal(int sig_num)
{
    return (SIGINT == sig_num
        || SIGQUIT == sig_num
        || SIGABRT == sig_num
        || SIGKILL == sig_num
        || SIGSEGV == sig_num
        || SIGTERM == sig_num
        || SIGSTOP == sig_num);
}

static bool signal_capture_is_forbidden(int sig_num)
{
    return (SIGKILL == sig_num
        || SIGSTOP == sig_num
        || 32 == sig_num
        || 33 == sig_num);
}

static int show_signal(int sig_num)
{
    printf("[PID:%d] Signal %d received.%s\n", getpid(), sig_num,
        is_critical_signal(sig_num) ? " It's a critical signal!!!" : "");

    return CA_RET_OK;
}

TEST(signal_capturer, all_in_one)
{
    const int INVALID_SIGNAL_MIN = calib::MIN_SIGNAL_NUM - 1;
    const int INVALID_SIGNAL_MAX = calib::MAX_SIGNAL_NUM + 1;

    for (int i = INVALID_SIGNAL_MIN; i <= INVALID_SIGNAL_MAX; ++i)
    {
        bool exits_after_handling = is_critical_signal(i);
        int register_ret = -1;
        bool should_exit = false;
        bool init_should_exit = should_exit;

        ASSERT_FALSE(calib::signal_capturer::is_registered(i));

        if (INVALID_SIGNAL_MIN == i || INVALID_SIGNAL_MAX == i)
        {
            ASSERT_FALSE(calib::signal_capturer::is_valid(i));
            register_ret = calib::signal_capturer::register_one(i, show_signal, exits_after_handling);
            ASSERT_EQ(CA_RET(INVALID_SIGNAL_NUMBER), register_ret);
            ASSERT_FALSE(calib::signal_capturer::is_registered(i));
            ASSERT_EQ(CA_RET(INVALID_SIGNAL_NUMBER), calib::signal_capturer::handle_one(i, should_exit));
            ASSERT_EQ(init_should_exit, should_exit);
            continue;
        }

        ASSERT_TRUE(calib::signal_capturer::is_valid(i));
        ASSERT_EQ(CA_RET(SIGNAL_NOT_REGISTERED), calib::signal_capturer::handle_one(i, should_exit));

        printf("Registering signal %d\n", i);
        register_ret = calib::signal_capturer::register_one(i, show_signal, exits_after_handling);
        if (signal_capture_is_forbidden(i))
        {
            printf("Registration for signal %d failed, ret = %d\n", i, register_ret);
            ASSERT_EQ(CA_RET(SIGNAL_CAPTURE_NOT_ALLOWED), register_ret);
            ASSERT_FALSE(calib::signal_capturer::is_registered(i));
            ASSERT_EQ(CA_RET(SIGNAL_NOT_REGISTERED), calib::signal_capturer::handle_one(i, should_exit));
            ASSERT_EQ(init_should_exit, should_exit);
        }
        else
        {
            ASSERT_EQ(CA_RET_OK, register_ret);
            printf("Registration for signal %d successful\n", i);
            ASSERT_TRUE(calib::signal_capturer::is_registered(i));
        }
    }

    pid_t pid;

    if ((pid = fork()) < 0)
    {
        perror("fork() failed");
        exit(1);
    }
    else if (0 == pid) // In child process, sends signals.
    {
        pid_t ppid = getppid();
        bool sends_critical_signals = false;

        pid = getpid();
        sleep(2);

SEND_SIGNALS:

        for (int i = calib::MIN_SIGNAL_NUM; i <= calib::MAX_SIGNAL_NUM; ++i)
        {
            if (signal_capture_is_forbidden(i))
                continue;

            if (sends_critical_signals && !is_critical_signal(i))
                continue;

            if (!sends_critical_signals && is_critical_signal(i))
                continue;

            char cmd[512] = {0};

            snprintf(cmd, sizeof(cmd), "kill -%d %d", i, ppid);
            system(cmd);
            printf("[PID:%d] %s\n", pid, cmd);
            system(cmd);
            printf("[PID:%d] %s\n", pid, cmd);
        }

        if (!sends_critical_signals)
        {
            sends_critical_signals = true;
            sleep(2);
            goto SEND_SIGNALS;
        }

        //system("kill -l"); // For convenient reference.

        exit(0);
    }

    /*
     * In parent process, waits signals.
     */

    int normal_handle_count = 0;
    int critical_handle_count = 0;
    const time_t kStartTime = time(nullptr);
    const int kTimtoutSeconds = 5;

    pid = getpid();

    while (true)
    {
        printf("[PID:%d] Waiting for signals ...\n", pid);
        sleep(1);
        if (time(nullptr) - kStartTime > kTimtoutSeconds)
        {
            printf("[PID:%d] Signal waiting timed out, program will abort.\n", pid);
            break;
        }

        bool should_exit = false;
        int handle_ret = calib::signal_capturer::handle_all(should_exit);

        ASSERT_GE(handle_ret, 0);
        if (0 == handle_ret)
            continue;

        if (should_exit)
            ++critical_handle_count;
        else
            ++normal_handle_count;
    }
    printf("[PID:%d] %d signals were handled totally\n", pid, critical_handle_count + normal_handle_count);

    char result[calib::MAX_SIGNAL_NUM][calib::MAX_SIGNAME_LEN + 1];
    ASSERT_EQ(calib::signal_capturer::get_all_signal_name(result), CA_RET_OK);
    for (int i = 0; i < calib::MAX_SIGNAL_NUM; ++i)
    {
        printf("signal-name[%d]: %s\n", i, result[i]);
    }
}
