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

CA_LIB_NAMESPACE_BEGIN

class command_line;

CA_LIB_NAMESPACE_END

#if defined(ENABLES_CHINESE)
#ifndef HELP_DESC
#define HELP_DESC                           "显示此帮助内容。"
#endif

#ifndef VERSION_DESC
#define VERSION_DESC                        "显示版本信息。"
#endif

#ifndef CONFIG_LOADING_DESC
#define CONFIG_LOADING_DESC                 "指定启动时加载的配置文件。"
#endif

#ifndef CONFIG_ASSIGN_EXPRESSION
#define CONFIG_ASSIGN_EXPRESSION            "=<配置文件名称>"
#endif

#ifndef DAEMON_DESC
#define DAEMON_DESC                         "程序以守护进程形式来运行。"
#endif

#ifndef QUIET_MODE_DESC
#define QUIET_MODE_DESC                     "程序以静默模式运行，（启动时）尽量少输出信息。"
#endif

#ifndef USAGE_FORMAT
#define USAGE_FORMAT                        "[选项 ...] [目标 ...]"
#endif
#endif // defind(ENABLES_CHINESE)

extern int check_private_commandline_options(cal::command_line &cmdline, bool &should_exit);

extern int init_extra_config(struct extra_config_t **extra_items);
extern int destroy_extra_config(struct extra_config_t **extra_items);
extern int load_extra_config(struct extra_config_t *extra_items);

extern int prepare_extra_resource(const void *condition, struct extra_resource_t *target);
extern void release_extra_resource(struct extra_resource_t **target);

namespace cafw
{

struct timed_task_config;

struct handler_component;

}

extern const char *g_timed_task_names[];
extern struct cafw::timed_task_config g_timed_task_settings[];

extern struct cafw::handler_component g_packet_handler_components[];

extern int init_business(void);
extern int run_private_business(bool &should_exit);
extern void finalize_business(void);

#endif /* __CASDK_FRAMEWORK_CUSTOMIZATION_H__ */
