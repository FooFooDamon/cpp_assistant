/*
 * Copyright (c) 2016-2018, Wen Xiongchang <udc577 at 126 dot com>
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
 * signal_registration.h
 *
 *  Created on: 2015-10-14
 *      Author: wenxiongchang
 * Description: data types and default functions for Unix signal registration
 */

#ifndef __CASDK_FRAMEWORK_SIGNAL_REGISTRATION_H__
#define __CASDK_FRAMEWORK_SIGNAL_REGISTRATION_H__

#include "base/all.h"

namespace cafw
{

typedef struct signal_registration_info
{
    int sig_num;
    const char *sig_name;
    calns::singal_handler sig_func;
    bool exit_after_handling;
    bool handles_now;
}signal_registration_info;

typedef signal_registration_info sig_reg_info;

extern const signal_registration_info g_sig_configs[];

#define DEFAULT_SIG_HANDLER(name)                           default_##name##_handler

#define DECLARE_DEFAULT_SIG_HANDLER(name)                   int DEFAULT_SIG_HANDLER(name)(int name)
#define EXTERN_DECLARE_DEFAULT_SIG_HANDLER(name)            extern DECLARE_DEFAULT_SIG_HANDLER(name)

#ifdef MULTI_THREADING

#define DEFINE_DEFAULT_SIG_HANDLER(name)                    int DEFAULT_SIG_HANDLER(name)(int name){\
    GLOG_INFO("%s captured\n", name); \
\
    bool should_exit = false; \
\
    for (int i = 0; calns::INVALID_SIGNAL_NUM != g_sig_configs[i].sig_num; ++i) \
    {\
        if (name == g_sig_configs[i].sig_num) \
        {\
            should_exit = g_sig_configs[i].exit_after_handling; \
            break; \
        }\
    }\
\
    if (should_exit) \
    {\
        const int kThreadCount = CFG_GET_COUNTER(XNODE_WORKER_THREAD); \
\
        for (int i = 0; i < kThreadCount; ++i) \
        {\
            g_thread_contexts[i].should_exit = true; \
        }\
    }\
\
    return RET_OK;\
}

#else

#define DEFINE_DEFAULT_SIG_HANDLER(name)                    int DEFAULT_SIG_HANDLER(name)(int name){\
    GLOG_INFO("%s captured\n", #name);\
    return RET_OK;\
}

#endif

#define CUSTOMIZED_SIG_HANDLER(name)                        customized_##name##_handler

#define DECLARE_CUSTOMIZED_SIG_HANDLER(name)                int CUSTOMIZED_SIG_HANDLER(name)(int name)
#define EXTERN_DECLARE_CUSTOMIZED_SIG_HANDLER(name)         extern DECLARE_CUSTOMIZED_SIG_HANDLER(name)

#define EXTERN_DECLARE_ALL_CUSTOMIZED_SIG_HANDLER_VARS()    \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigusr2); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sighup); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigterm); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigint); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigpipe); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigusr1); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigchild); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigalarm); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigtrap); \
    extern calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigsegv)

#define SET_ALL_CUSTOMIZED_SIG_HANDLERS_TO_DEFAULT()        \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigusr2) = cafw::DEFAULT_SIG_HANDLER(sigusr2); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sighup) = cafw::DEFAULT_SIG_HANDLER(sighup); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigterm) = cafw::DEFAULT_SIG_HANDLER(sigterm); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigint) = cafw::DEFAULT_SIG_HANDLER(sigint); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigpipe) = cafw::DEFAULT_SIG_HANDLER(sigpipe); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigusr1) = cafw::DEFAULT_SIG_HANDLER(sigusr1); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigchild) = cafw::DEFAULT_SIG_HANDLER(sigchild); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigalarm) = cafw::DEFAULT_SIG_HANDLER(sigalarm); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigtrap) = cafw::DEFAULT_SIG_HANDLER(sigtrap); \
    calns::singal_handler CUSTOMIZED_SIG_HANDLER(sigsegv) = cafw::DEFAULT_SIG_HANDLER(sigsegv)

DECLARE_DEFAULT_SIG_HANDLER(sigusr2);
DECLARE_DEFAULT_SIG_HANDLER(sighup);
DECLARE_DEFAULT_SIG_HANDLER(sigterm);
DECLARE_DEFAULT_SIG_HANDLER(sigint);
DECLARE_DEFAULT_SIG_HANDLER(sigpipe);
DECLARE_DEFAULT_SIG_HANDLER(sigusr1);
DECLARE_DEFAULT_SIG_HANDLER(sigchild);
DECLARE_DEFAULT_SIG_HANDLER(sigalarm);
DECLARE_DEFAULT_SIG_HANDLER(sigtrap);
DECLARE_DEFAULT_SIG_HANDLER(sigsegv);

}

#endif /* __CASDK_FRAMEWORK_SIGNAL_REGISTRATION_H__ */
