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
 * daemon.h
 *
 *  Created on: 2017/09/23
 *      Author: wenxiongchang
 * Description: APIs for executing daemon process operations.
 */

#ifndef __CPP_ASSISTANT_DAEMON_H__
#define __CPP_ASSISTANT_DAEMON_H__

#include <set>

#include "base/ca_inner_necessities.h"

CA_LIB_NAMESPACE_BEGIN

class daemon
{
/* ===================================
 * constructors:
 * =================================== */
private:
    daemon();

/* ===================================
 * copy control:
 * =================================== */
private:
    daemon(const daemon& src);
    daemon& operator=(const daemon& src);

/* ===================================
 * destructor:
 * =================================== */
private:
    ~daemon();

/* ===================================
 * types:
 * =================================== */
public:
    typedef void (*clean_func)(void);

/* ===================================
 * abilities:
 * =================================== */
public:
    // Makes the calling process be a daemon, which runs in background.
    // Better execute it before any other functions,
    // because some resources such as file descriptors may be released.
    // The calling process continues on success, or dies on failure.
    // Note: A process may exit several times before it become a daemon.
    static void daemonize(bool closes_stdout = false, bool closes_stderr = false);

    // Registers a function which will be called to clean resources
    // before the calling process exits.
    static int register_clean_funtion(clean_func clean_func);

/* ===================================
 * attributes:
 * =================================== */
public:

/* ===================================
 * status:
 * =================================== */
public:
    // Checks if the calling process has daemonized.
    static bool is_daemonized(void);

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
    static bool m_is_daemonized;
    static std::set<clean_func> m_clean_funcs;
};

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_DAEMON_H__

