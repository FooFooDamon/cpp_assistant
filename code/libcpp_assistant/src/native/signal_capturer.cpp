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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "signal_capturer.h"
#include "base/ca_return_code.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

/*static */__thread bool signal_capturer::m_initialized = false;
/*static */__thread signal_capturer::settings_t signal_capturer::m_settings[SIGNAL_COUNT];
/*static */bool signal_capturer::m_should_exit = false;

#define GET_SIGNUM_BY_INDEX(index)          (index + MIN_SIGNAL_NUM)
#define GET_SETTING_BY_INDEX(index)         (m_settings[index])

#define GET_INDEX_BY_SIGNUM(sig_num)        (sig_num - MIN_SIGNAL_NUM)
#define GET_SETTING_BY_SIGNUM(sig_num)      (m_settings[GET_INDEX_BY_SIGNUM(sig_num)])

#define GET_SETTING_CAPACITY()              (sizeof(m_settings) / sizeof(settings_t))

static bool capture_is_forbidden(int sig_num)
{
    return (SIGKILL == sig_num
        || SIGSTOP == sig_num
        || 32 == sig_num /* signal name unknown */
        || 33 == sig_num /* signal name unknown */);
}

/*static */int signal_capturer::register_one(int sig_num,
    singal_handler sig_handler,
    bool exit_hint/* = false*/,
    bool handled_automatically/* = false*/)
{
    init_once();

    if (!is_valid(sig_num))
        return CA_RET(INVALID_SIGNAL_NUMBER);

    if (capture_is_forbidden(sig_num))
        return CA_RET(SIGNAL_CAPTURE_NOT_ALLOWED);

    struct sigaction act, oact;

    act.sa_handler = update_settings; // the inner handler
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#if 0 // defined(SA_INTERRUPT) || defined(SA_RESTART) // Seems to be no use to sigaction().
    act.sa_flags |= SA_RESTART;
#endif

    if (sigaction(sig_num, &act, &oact) < 0)
        return CA_RET(SIGNAL_REGISTRATION_FAILED);

    settings_t &setting = GET_SETTING_BY_SIGNUM(sig_num);

    setting.status = SIG_NOT_TRIGGERED;
    setting.handler = sig_handler; // the actual handler
    setting.exit_hint = exit_hint;
    setting.handled_automatically = handled_automatically;

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

    settings_t &setting = GET_SETTING_BY_INDEX(sig_num);

    setting.status = SIG_NOT_REGISTERED;
    setting.handler = nullptr;

    return CA_RET_OK;
}

/*static */int signal_capturer::handle_one(const int sig_num)
{
    if (!is_valid(sig_num))
        return CA_RET(INVALID_SIGNAL_NUMBER);

    if (!is_registered(sig_num))
        return CA_RET(SIGNAL_NOT_REGISTERED);

    settings_t &setting = GET_SETTING_BY_SIGNUM(sig_num);

    if (SIG_NOT_TRIGGERED == setting.status)
        return setting.handled_automatically ? CA_RET_OK : CA_RET(SIGNAL_NOT_ARISING);

    setting.status = SIG_NOT_TRIGGERED;

    if ((!setting.handled_automatically) && nullptr != setting.handler)
        return setting.handler(sig_num);

    return CA_RET_OK;
}

/*static */int signal_capturer::handle_all(void)
{
    init_once();

    int handled_count = 0;
    int registered_count = 0;

    for (unsigned int i = 0; i < GET_SETTING_CAPACITY(); ++i)
    {
        settings_t &setting = GET_SETTING_BY_INDEX(i);

        if (SIG_NOT_REGISTERED == setting.status)
            continue;

        ++registered_count;
        //nsdebug(signal_capturer, "Checking signal %d ...\n", GET_SIGNUM_BY_INDEX(i));

        if (SIG_NOT_TRIGGERED == setting.status)
            continue;

        setting.status = SIG_NOT_TRIGGERED;

        if ((!setting.handled_automatically) && nullptr != setting.handler)
            setting.handler(GET_SIGNUM_BY_INDEX(i));

        ++handled_count;

    }

    if (handled_count > 0)
        nsdebug(signal_capturer, "%d signals total, %d triggered and handled\n", registered_count, handled_count);

    return handled_count;
}

/*static */CA_REENTRANT int signal_capturer::get_all_signal_names(char result[SIGNAL_COUNT][MAX_SIGNAME_LEN + 1])
{
    FILE *fp = popen("kill -l", "r");
    if (NULL == fp)
        return CA_RET(FILE_OR_STREAM_OPEN_FAILED);

    bool before_min_signum = true;

    for (int idx = 0; idx < SIGNAL_COUNT; ++idx) // NOTE: Output of "kill -l" starts from 0.
    {
        if (feof(fp))
            break;

        if (NULL != fgets(result[idx], MAX_SIGNAME_LEN + 1, fp))
            result[idx][strlen(result[idx]) - 1] = '\0'; // replaces the '\n'

        if (before_min_signum && MIN_SIGNAL_NUM == idx)
        {
            before_min_signum = false;
            if (MIN_SIGNAL_NUM > 0)
                strncpy(result[0], result[idx], MAX_SIGNAME_LEN + 1);
            idx = 0;
        }
    }
    pclose(fp);

    return CA_RET(OK);
}

static __thread char s_signames[SIGNAL_COUNT][MAX_SIGNAME_LEN + 1] = {{0}};

/*static */CA_REENTRANT const char** signal_capturer::get_all_signal_names(void)
{
    if ('\0' == s_signames[0][0])
        get_all_signal_names(s_signames);

    return (const char **)s_signames;
}

/*static */CA_REENTRANT const char* signal_capturer::get_signal_name(const int sig_num, const char *name_if_num_invalid/* = "INVALID"*/)
{
    if ('\0' == s_signames[0][0])
        get_all_signal_names(s_signames);

    if (!is_valid(sig_num))
        return name_if_num_invalid;

    return s_signames[GET_INDEX_BY_SIGNUM(sig_num)];
}

/*static */bool signal_capturer::is_registered(int sig_num)
{
    init_once();

    if (!is_valid(sig_num))
        return false;

    if (SIG_NOT_REGISTERED == GET_SETTING_BY_SIGNUM(sig_num).status)
        return false;

    return true;
}

/*static */void signal_capturer::init_once(void)
{
    if (m_initialized)
        return;

    for (unsigned int i = 0; i < GET_SETTING_CAPACITY(); ++i)
    {
        settings_t &setting = GET_SETTING_BY_INDEX(i);

        setting.status = SIG_NOT_REGISTERED;
        setting.handler = nullptr;
        setting.exit_hint = false;
        setting.handled_automatically = false;
    }

    //nsdebug(signal_capturer, "Signal initialization successful.\n");
    m_initialized = true;
}

/*static */void signal_capturer::update_settings(int sig_num)
{
    settings_t &setting = GET_SETTING_BY_SIGNUM(sig_num);

    setting.status = SIG_TRIGGERED;

    if (setting.exit_hint)
        m_should_exit = true;

    if (setting.handled_automatically)
        setting.handler(sig_num);
}

CA_LIB_NAMESPACE_END
