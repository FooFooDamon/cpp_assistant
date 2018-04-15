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

#include <stdio.h>
#include <cpp_assistant/ca_full.h>

#include "framework_public/base/all.h"
#include "framework_public/signal_registration.h"
#include "framework_public/timed_task_scheduler.h"
#include "framework_public/customization.h"

static const cal::command_line::user_option s_kPrivateOptions[]
/*const cal::command_line::user_option g_kPrivateOptions[]*/ = {
    {
        /* .name = */"x,extended-option",
        /* .desc = */"扩展选项。",
        /* .least_value_count = */1,
        /* .value_count_is_fixed = */true,
        /* .assign_expression = */" [选项值1 选项值2 ...]",
        /* .default_values = */NULL
    },
    // TODO: Add your own options here, or do nothing if there is none.
    { NULL }
};

const cal::command_line::user_option *g_kPrivateOptions = s_kPrivateOptions;
const std::vector<std::string> *g_extended_params = NULL;

int check_private_commandline_options(cal::command_line &cmdline, bool &should_exit)
{
    const cal::command_line::option_entry *op_val = NULL;

    op_val = cmdline.get_option_entry("extended-option");
    if (op_val->is_specified)
    {
        printf("Value(s) of extended option:");
        if (op_val->is_specified)
        {
            for (size_t i = 0; i < op_val->values.size(); ++i)
            {
                printf(" [%s]", op_val->values[i].c_str());
            }
            printf("\n");
        }
        g_extended_params = &(op_val->values);
    }

    // TODO: Add your own stuff here, or do nothing if there is none.

    return 0;
}

// TODO: If you want to define your own customized signal handlers,
//     delete this line and write your handlers.
SET_ALL_CUSTOMIZED_SIG_HANDLERS_TO_DEFAULT();

int init_extra_config(struct extra_config_t **extra_items)
{
    if (NULL == extra_items)
    {
        GLOG_ERROR_NS("", "null param\n");
        return RET_FAILED;
    }

    if (NULL != *extra_items)
    {
        GLOG_INFO_NS("", "extra configuration structure already initialized\n");
        return RET_OK;
    }

    if (NULL == (*extra_items = new extra_config_t))
    {
        GLOG_ERROR_NS("", "new extra_conf_t failed\n");
        return RET_FAILED;
    }

    // TODO: Allocate and initialize resources for your own configuration items here,
    //     or do nothing if unnecessary.

    return RET_OK;
}

int destroy_extra_config(struct extra_config_t **extra_items)
{
    if (NULL == extra_items)
    {
        GLOG_ERROR_NS("", "null param\n");
        return RET_FAILED;
    }

    if (NULL == extra_items)
    {
        GLOG_INFO_NS("", "extra configuration structure already destroyed\n");
        return RET_OK;
    }

    // TODO: Clean or release resources of your own configuration items here,
    //     or do nothing if unnecessary.

    delete (*extra_items);
    *extra_items = NULL;

    return RET_OK;
}

int load_extra_config(struct extra_config_t *extra_items)
{
    // TODO: Load values of your own configuration items here, or do nothing if there is none.
    return RET_OK;
}

int prepare_extra_resource(const void *condition, struct extra_resource_t *target)
{
    // TODO: Prepare your own resources here, or do nothing if there is none.
    return RET_OK;
}

void release_extra_resource(struct extra_resource_t **target)
{
    ; // TODO: Release your own resources here, or do nothing if there is none.
}

const char *g_timed_task_names[] = {
    // TODO: Add your own stuff, or do nothing if there is none.
    NULL
};

cafw::timed_task_config g_timed_task_settings[] = {
    // trigger_type, event_time/last_op_time, time_offset/time_interval, operation
    // TODO: Add your own stuff, or do nothing if there is none.
    { cafw::timed_task_config::TRIGGERED_PERIODICALLY,   true,   {0},    {0},    NULL }
};

int init_business(void)
{
    // TODO: Add your own stuff, or do nothing if there is none.
    return RET_OK;
}

int run_private_business(bool &should_exit)
{
    /*GLOG_INFO_NS("", "========== Begin of SIGSEGV test ==========\n");
    strcpy(NULL, "foo");
    GLOG_INFO_NS("", "========== End of SIGSEGV test ==========\n");*/

    static int s_business_counter = 0;

    GLOG_INFO_NS("", "This is a demo test, s_business_counter = %d\n", s_business_counter);
    sleep(1);
    ++s_business_counter;

    return RET_OK;
}

void finalize_business(void)
{
    ; // TODO: Add your own stuff, or do nothing if there is none.
}

int make_session_id(const void *condition, const int sid_holder_size, char *sid_result)
{
    ; // TODO: Implement it!
    return RET_OK;
}
