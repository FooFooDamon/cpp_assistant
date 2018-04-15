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
 * basic_types.h
 *
 *  Created on: 2016-11-14
 *      Author: wenxiongchang
 * Description: Some basic type definitions.
 */

#ifndef __CASDK_BASIC_TYPES_H__
#define __CASDK_BASIC_TYPES_H__

#include <framework_base_info.h>

#ifndef MODULE_VERSION
#define MODULE_VERSION                      "r012345.[Approximative]"
#endif

#ifndef CASDK_NEWEST_MOD_DATE
#define CASDK_NEWEST_MOD_DATE               "20161114"
#endif

#ifndef CASDK_NEWEST_MAIN_VER
#define CASDK_NEWEST_MAIN_VER               "0.0.01"
#endif

#ifdef CASDK_SVN_VER
#define CASDK_FRAMEWORK_VERSION             "v"CASDK_NEWEST_MAIN_VER".r"CASDK_SVN_VER"."CASDK_NEWEST_MOD_DATE
#else
#define CASDK_FRAMEWORK_VERSION             "v"CASDK_NEWEST_MAIN_VER".r012345."CASDK_NEWEST_MOD_DATE".[Approximative]"
#endif

#ifndef CASDK_FRAMEWORK_NAME
#define CASDK_FRAMEWORK_NAME                "casdk"
#endif

#define DEFAULT_HELP_DESC                   "Shows this [h]elp."

#define DEFAULT_VERSION_DESC                "Shows [v]ersion info."

#define DEFAULT_CONFIG_LOADING_DESC         "Loads a [c]onfiguration file on startup."
#define DEFAULT_CONFIG_FILE_COUNT           1
#define DEFAULT_CONFIG_ASSIGN_EXPRESSION    "=<configuration file>"
#define DEFAULT_CONFIG_FILES                "config.xml"

#define DEFAULT_DAEMON_DESC                 "Program will run as a [d]aemon."

#define DEFAULT_QUIET_MODE_DESC             "Program will run in [q]uiet mode and output as little messages as possible(on startup)."

#define DEFAULT_USAGE_NAME                  "Usage"
#define DEFAULT_USAGE_FORMAT                "[options ...] [targets ...]"

#ifndef THREAD_TERMINATION_TIMEOUT_SECS
#define THREAD_TERMINATION_TIMEOUT_SECS     60
#endif

enum enum_return_value
{
    RET_OK = 0,
    RET_FAILED = -1
};

enum
{
    FLAG_NO = 0,
    FLAG_YES = 1
};

#endif /* __CASDK_BASIC_TYPES_H__ */
