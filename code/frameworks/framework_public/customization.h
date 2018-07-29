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
 * customization.h
 *
 *  Created on: 2016/12/29
 *      Author: wenxiongchang
 * Description: Declarations of customized stuff.
 *        Note: Their implementations should be done by user.
 */

#ifndef __CASDK_FRAMEWORK_CUSTOMIZATION_H__
#define __CASDK_FRAMEWORK_CUSTOMIZATION_H__

#include "mutable_customization.h"

#include "timed_task_scheduler.h"

CA_LIB_NAMESPACE_BEGIN

class command_line;

CA_LIB_NAMESPACE_END

#if !defined(USAGE)
#if defined(ENABLES_CHINESE)
#define USAGE                               "用法"
#else
#define USAGE                               "Usage"
#endif
#endif

#if !defined(USAGE_TITLE)
#if defined(ENABLES_CHINESE)
#define USAGE_TITLE                         "<此处填用法介绍的标题>"
#else
#define USAGE_TITLE                         "<Fill your usage title here>"
#endif
#endif

#if !defined(USAGE_FORMAT)
#if defined(ENABLES_CHINESE)
#define USAGE_FORMAT                        "[选项 ...] [目标 ...]\n其中命令行选项说明如下："
#else
#define USAGE_FORMAT                        "[options ...] [targets ...]\nOptions are listed below:"
#endif
#endif

#if !defined(HELP_DESC)
#if defined(ENABLES_CHINESE)
#define HELP_DESC                           "显示此帮助内容。"
#else
#define HELP_DESC                           "Shows this [h]elp."
#endif
#endif

#if !defined(VERSION_DESC)
#if defined(ENABLES_CHINESE)
#define VERSION_DESC                        "显示版本信息。"
#else
#define VERSION_DESC                        "Shows [v]ersion info."
#endif
#endif

#if !defined(CONFIG_LOADING_DESC)
#if defined(ENABLES_CHINESE)
#define CONFIG_LOADING_DESC                 "指定启动时加载的配置文件。"
#else
#define CONFIG_LOADING_DESC                 "Loads a [c]onfiguration file on startup."
#endif
#endif

#if !defined(CONFIG_FILE_COUNT)
#define CONFIG_FILE_COUNT                   1
#endif

#if !defined(CONFIG_ASSIGN_EXPRESSION)
#if defined(ENABLES_CHINESE)
#define CONFIG_ASSIGN_EXPRESSION            "=<配置文件路径>"
#else
#define CONFIG_ASSIGN_EXPRESSION            "=<configuration file>"
#endif
#endif

#if !defined(DEFAULT_CONFIG_FILES)
#define DEFAULT_CONFIG_FILES                "./conf/config.xml"
#endif

#if !defined(DAEMON_DESC)
#if defined(ENABLES_CHINESE)
#define DAEMON_DESC                         "程序以守护进程形式来运行。"
#else
#define DAEMON_DESC                         "Program will run as a [d]aemon."
#endif
#endif

#if !defined(QUIET_MODE_DESC)
#if defined(ENABLES_CHINESE)
#define QUIET_MODE_DESC                     "程序以静默模式运行，（启动时）尽量少输出信息。"
#else
#define QUIET_MODE_DESC                     "Program will run in [q]uiet mode and output as little messages as possible(on startup)."
#endif
#endif

extern int check_private_commandline_options(calns::command_line &cmdline, bool &should_exit);

extern int init_extra_config(struct extra_config_t **extra_items);
extern int destroy_extra_config(struct extra_config_t **extra_items);
extern int load_extra_config(struct extra_config_t *extra_items);
extern int reload_partial_extra_config(struct extra_config_t *extra_items);

extern int prepare_extra_resource(const void *condition, struct extra_resource_t *target);
extern void release_extra_resource(struct extra_resource_t **target);

namespace cafw
{

typedef struct timed_task_info_t
{
    const char *name;
    struct timed_task_config config;
}timed_task_info_t;

struct handler_component;

}

extern cafw::timed_task_info_t g_customized_timed_tasks[];

extern struct cafw::handler_component g_packet_handler_components[];

extern int init_business(void);
extern int run_private_business(bool &should_exit);
extern void finalize_business(void);

#endif /* __CASDK_FRAMEWORK_CUSTOMIZATION_H__ */
