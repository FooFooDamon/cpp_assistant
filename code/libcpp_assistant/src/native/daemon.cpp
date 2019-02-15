/*
 * Copyright (c) 2017-2019, Wen Xiongchang <udc577 at 126 dot com>
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

#include "daemon.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <typeinfo>

#include "base/ca_return_code.h"

CA_LIB_NAMESPACE_BEGIN

bool daemon::m_is_daemonized = false;
std::set<daemon::clean_func> daemon::m_clean_funcs;

/*static */void daemon::daemonize(bool closes_stdout/* = false*/, bool closes_stderr/* = false*/)
{
    if (m_is_daemonized)
        return;

    pid_t pid;
    struct rlimit rl = { rl.rlim_max = RLIM_INFINITY };
    struct sigaction sa;

    umask(0);

    if ((getrlimit(RLIMIT_NOFILE, &rl) < 0) || (RLIM_INFINITY == rl.rlim_max))
        rl.rlim_max = 1024;

    /*
     * Becomes a session leader to lose controlling TTY
     */

    if ((pid = fork()) < 0)
    {
        perror("fork() at the 1st time failed");
        exit(1);
    }
    else if (0 != pid) // It's the parent process.
    {
        exit(0);
    }
    //else: It's the child process, continue.

    if (setsid() < 0) // Creates a new session and become its leader.
    {
        perror("setsid() failed");
        exit(1);
    }

    /*
     * Ensures future opens won't allocate controlling TTYs
     */

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, nullptr) < 0)
    {
        perror("failed to ignore SIGHUP");
        exit(1);
    }

    if ((pid = fork()) < 0)
    {
        perror("fork() at the 2nd time failed");
        exit(1);
    }
    else if (0 != pid)
    {
        exit(0);
    }

    /*
     * Closes open file descriptors
     */

    for (unsigned int i = 0; i < rl.rlim_max; ++i)
    {
        if (STDOUT_FILENO != i && STDERR_FILENO != i)
            close(i);
    }

    if (closes_stdout) close(STDOUT_FILENO);
    if (closes_stderr) close(STDERR_FILENO);

    // REMENBER to set this flag after all things are done!
    m_is_daemonized = true;
}

/*static */int daemon::register_clean_funtion(clean_func clean_func)
{
    // Registration of a null function with atexit() may succeed
    // and cause a crash at program termination,
    // therefore it's better to check null function explicitly.
    if (nullptr == clean_func)
        return CA_RET(NULL_PARAM);

    if (m_clean_funcs.end() != m_clean_funcs.find(clean_func))
        return CA_RET(OBJECT_ALREADY_EXISTS);

    if (!(m_clean_funcs.insert(clean_func).second))
        return CA_RET(UNDERLYING_ERROR);

    if (0 != atexit(clean_func))
    {
        m_clean_funcs.erase(clean_func);
        return CA_RET(UNDERLYING_ERROR);
    }

    return CA_RET_OK;
}

/*static */bool daemon::is_daemonized(void)
{
    return m_is_daemonized;
}

CA_LIB_NAMESPACE_END
