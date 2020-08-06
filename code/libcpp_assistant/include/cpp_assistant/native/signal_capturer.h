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

/*
 * signal_capturer.h
 *
 *  Created on: 2017/09/23
 *      Author: wenxiongchang
 * Description: Signal capture and handling in single-threading and multi-threading.
 */

#ifndef __CPP_ASSISTANT_SIGNAL_OPERATIONS_H__
#define __CPP_ASSISTANT_SIGNAL_OPERATIONS_H__

#include <signal.h>

#include "base/ca_inner_necessities.h"
#include "base/ca_return_code.h"

CA_LIB_NAMESPACE_BEGIN

enum
{
    MIN_SIGNAL_NUM = SIGHUP,
#ifdef __SIGRTMAX // TODO: This macro seems to be a private macro, it may be not safe.
    MAX_SIGNAL_NUM = __SIGRTMAX,
#else
    MAX_SIGNAL_NUM = 64, // TODO: It may be not this value if the OS kernel improves in future.
#endif
    SIGNAL_COUNT = MAX_SIGNAL_NUM - MIN_SIGNAL_NUM + 1,

    // Actually, numbers beyond the range of [MIN_SIGNAL_NUM, MAX_SIGNAL_NUM] are all invalid,
    // we specify one for the use of some cases that need a concrete value.
    INVALID_SIGNAL_NUM = -1,
};

enum
{
    MAX_SIGNAME_LEN = 15 // '\0' not included
};

enum enum_sigal_status
{
    SIG_NOT_REGISTERED = -1,
    SIG_NOT_TRIGGERED = 0,
    SIG_TRIGGERED = 1
};

// This handler is supposed to return CA_RET_OK on success and return a negative number on failure.
typedef int (*singal_handler)(int sig_num);

class signal_capturer
{
/* ===================================
 * constructors:
 * =================================== */
private:
    signal_capturer();

/* ===================================
 * copy control:
 * =================================== */
private:
    signal_capturer(const signal_capturer& src);
    signal_capturer& operator=(const signal_capturer& src);

/* ===================================
 * destructor:
 * =================================== */
private:
    ~signal_capturer();

/* ===================================
 * types:
 * =================================== */
public:
    typedef struct settings_t
    {
        int status;
        singal_handler handler;
        bool handled_automatically;
        bool exit_hint;
    } settings_t;

/* ===================================
 * abilities:
 * =================================== */
public:
    // Registers a signal.
    //
    // @param sig_num: Target signal number.
    //
    // @param sig_handler: A handler that contains some operations
    //     to be executed everytime the target signal arises.
    //
    // @param exit_hint: A flag to let the calling process know
    //     whether or not it should exit when the target signal occurs.
    //
    // @param handled_automatically: If it's true,
    //     the @sig_handler is called automatically when target signal arises.
    //     Otherwise, the user should manually call handle_one() or handle_all()
    //     to make @sig_handler come into effect.
    //
    // @return CA_RET_OK on success, and a negative number on failure.
    static int register_one(int sig_num,
        singal_handler sig_handler,
        bool exit_hint = false,
        bool handled_automatically = false);

    // Cancels the registration of signal @sig_num so that this signal would not be monitored and captured.
    static int unregister(int sig_num);

    // Handles a single signal with the number @sig_num.
    // NOTE: If register_one() is called with handled_automatically = false,
    // then the target signal will be marked only everytime it occurs,
    // and will not be handled until this function or handle_all() is called.
    static int handle_one(const int sig_num);
    //[[deprecated("Use handle_one(const int) and then should_exit() instead")]] // NOTE: since c++14
    static inline int handle_one(const int sig_num, bool &should_exit) CA_DEPRECATED
    {
        should_exit = m_should_exit;
        return handle_one(sig_num);
    }

    // Handles all pending signals.
    // Returns the number of signals handled on success, or a negative number on failure.
    static int handle_all(void);
    //[[deprecated("Use handle_all(void) and then should_exit() instead")]] // NOTE: since c++14
    static inline int handle_all(bool &should_exit) CA_DEPRECATED
    {
        should_exit = m_should_exit;
        return handle_all();
    }

    static CA_THREAD_SAFE int get_all_signal_names(char result[SIGNAL_COUNT][MAX_SIGNAME_LEN + 1]);
    static CA_THREAD_SAFE const char** get_all_signal_names(void);

    static CA_THREAD_SAFE const char* get_signal_name(const int sig_num, const char *name_if_num_invalid = "INVALID");

/* ===================================
 * attributes:
 * =================================== */
public:
    static inline CA_REENTRANT int index_of(int sig_num)
    {
        return is_valid(sig_num) ? (sig_num - MIN_SIGNAL_NUM) : -1;
    }

    static inline CA_REENTRANT int signum_at(int index)
    {
        return (index >=0 && index < SIGNAL_COUNT) ? (index + MIN_SIGNAL_NUM) : CA_RET(VALUE_OUT_OF_RANGE);
    }

/* ===================================
 * status:
 * =================================== */
public:
    static bool is_registered(int sig_num);

    static inline CA_REENTRANT bool is_valid(int sig_num)
    {
        return (sig_num >= MIN_SIGNAL_NUM) && (sig_num <= MAX_SIGNAL_NUM);
    }

    static inline CA_REENTRANT bool should_exit(void)
    {
        return m_should_exit;
    }

/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    static void init_once(void);
    static void update_settings(int sig_num);

/* ===================================
 * data:
 * =================================== */
protected:
    static bool m_initialized;
    static settings_t m_settings[SIGNAL_COUNT];
    static bool m_should_exit;
};

typedef signal_capturer sigcap;

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_SIGNAL_OPERATIONS_H__

