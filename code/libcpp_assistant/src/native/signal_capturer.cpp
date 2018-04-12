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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../../include/signal_capturer.h"
#include "base/ca_return_code.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

DEFINE_CLASS_NAME(signal_capturer);

static signal_setting_t s_signal_settings[SIGNAL_COUNT];
static bool s_settings_initialized = false;

#define GET_SIGNUM_BY_INDEX(index)          (index + MIN_SIGNAL_NUM)
#define GET_SETTING_BY_INDEX(index)         (s_signal_settings[index])

#define GET_INDEX_BY_SIGNUM(sig_num)        (sig_num - MIN_SIGNAL_NUM)
#define GET_SETTING_BY_SIGNUM(sig_num)      (s_signal_settings[GET_INDEX_BY_SIGNUM(sig_num)])

#define GET_SETTING_CAPACITY()              (sizeof(s_signal_settings) / sizeof(signal_setting_t))

#define INIT_SIG_SETTINGS_ONCE()            do {\
    if (!s_settings_initialized) { \
        init_signal_settings(); \
        /*csdebug(Signal, "Signal initialization successful.\n");*/\
        s_settings_initialized = true; \
    } \
}while(0)

static void init_signal_settings(void)
{
    for (unsigned int i = 0; i < GET_SETTING_CAPACITY(); ++i)
    {
        signal_setting_t &setting = GET_SETTING_BY_INDEX(i);

        setting.status = SIG_NOT_REGISTERED;
        setting.handler = NULL;
        setting.exits_after_handling = false;
        setting.handles_now = false;
    }
}

static void set_signal_trigger_status(int sig_num)
{
    signal_setting_t &setting = GET_SETTING_BY_SIGNUM(sig_num);

    setting.status = SIG_TRIGGERED;
    if (setting.handles_now)
        setting.handler(sig_num);
}

static bool capture_is_forbidden(int sig_num)
{
    return (SIGKILL == sig_num
        || SIGSTOP == sig_num
        || 32 == sig_num /* signal name unknown */
        || 33 == sig_num /* signal name unknown */);
}

/*static */int signal_capturer::register_one(int sig_num,
    singal_handler sig_handler,
    bool exits_after_handling/* = false*/,
    bool handles_now/* = false*/)
{
    INIT_SIG_SETTINGS_ONCE();

    if (!is_valid(sig_num))
        return CA_RET(INVALID_SIGNAL_NUMBER);

    if (capture_is_forbidden(sig_num))
        return CA_RET(SIGNAL_CAPTURE_NOT_ALLOWED);

    struct sigaction act, oact;

    act.sa_handler = set_signal_trigger_status; // just an operation of recording which signal was received just now
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#if 0 // defined(SA_INTERRUPT) || defined(SA_RESTART) // Seems to be no use to sigaction().
    act.sa_flags |= SA_RESTART;
#endif

    if (sigaction(sig_num, &act, &oact) < 0)
        return CA_RET(SIGNAL_REGISTRATION_FAILED);

    signal_setting_t &setting = GET_SETTING_BY_SIGNUM(sig_num);

    setting.status = SIG_NOT_TRIGGERED;
    setting.handler = sig_handler; // the actual handler
    setting.exits_after_handling = exits_after_handling;
    setting.handles_now = handles_now;

    return CA_RET_OK;
}

/*static */int signal_capturer::unregister(int sig_num)
{
    if (!is_registered(sig_num))
        return CA_RET_OK; // CA_RET(SIGNAL_NOT_REGISTERED);

    struct sigaction act, oact;

    act.sa_handler = SIG_DFL;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(sig_num, &act, &oact) < 0)
        return CA_RET(SIGNAL_REGISTRATION_FAILED);

    signal_setting_t &setting = GET_SETTING_BY_INDEX(sig_num);

    setting.status = SIG_NOT_REGISTERED;
    setting.handler = NULL;

    return CA_RET_OK;
}

/*static */int signal_capturer::handle_one(const int sig_num, bool &should_exit)
{
    if (!is_valid(sig_num))
        return CA_RET(INVALID_SIGNAL_NUMBER);

    if (!is_registered(sig_num))
        return CA_RET(SIGNAL_NOT_REGISTERED);

    signal_setting_t &setting = GET_SETTING_BY_SIGNUM(sig_num);

    if (SIG_NOT_TRIGGERED == setting.status)
        return CA_RET_OK;

    if (setting.exits_after_handling)
        should_exit = true;

    setting.status = SIG_NOT_TRIGGERED;

    if (!setting.handles_now /* avoids repetition, see set_signal_trigger_status() */
        && NULL != setting.handler)
    {
        int handle_ret = (setting.handler)(sig_num);

        if (CA_RET_OK != handle_ret)
            return handle_ret;
    }

    return CA_RET_OK;
}

/*static */int signal_capturer::handle_all(bool &should_exit)
{
    INIT_SIG_SETTINGS_ONCE();

    int handled_count = 0;
    int registered_count = 0;

    for (unsigned int i = 0; i < GET_SETTING_CAPACITY(); ++i)
    {
        signal_setting_t &setting = GET_SETTING_BY_INDEX(i);

        if (SIG_NOT_REGISTERED == setting.status)
            continue;

        ++registered_count;
        //csdebug(Signal, "Checking signal %d ...\n", GET_SIGNUM_BY_INDEX(i));

        if (SIG_TRIGGERED != setting.status)
            continue;

        setting.status = SIG_NOT_TRIGGERED;

        if (!setting.handles_now /* avoids repetition, see set_signal_trigger_status() */
            && NULL != setting.handler)
            (setting.handler)(GET_SIGNUM_BY_INDEX(i));

        if (setting.exits_after_handling)
            should_exit = true;

        ++handled_count;

    }

    if (handled_count > 0)
        csdebug(signal_capturer, "%d signals total, %d triggered and handled\n", registered_count, handled_count);

    return handled_count;
}

/*static */bool signal_capturer::is_registered(int sig_num)
{
    INIT_SIG_SETTINGS_ONCE();

    if (!is_valid(sig_num))
        return false;

    if (SIG_NOT_REGISTERED == GET_SETTING_BY_SIGNUM(sig_num).status)
        return false;

    return true;
}

CA_LIB_NAMESPACE_END
