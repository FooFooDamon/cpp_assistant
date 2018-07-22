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

#include "easy_debugging.h"

#include <stdlib.h>

#include "basic_types.h"

bool g_is_quiet_mode = false;
calns::screen_logger *g_screen_logger = NULL;
calns::file_logger *g_file_logger = NULL;

namespace cafw
{

int init_logger(
    const int terminal_level,
    const bool enables_file_logger,
    const int file_level,
    const char *log_dir,
    const char *log_name)
{
    if (enables_file_logger)
    {
        if (NULL == log_dir || NULL == log_name)
        {
            GLOG_ERROR_NS("cafw", "null log_dir or log_name\n");
            return RET_FAILED;
        }

        if (NULL == g_file_logger
            && NULL == (g_file_logger = new calns::file_logger))
        {
            GLOG_ERROR_NS("cafw", "failed to allocate memory for g_file_logger\n");
            return RET_FAILED;
        }
        // Note: g_screen_logger was created at the beginning.
    }

    calns::logger *loggers[] = {
        g_screen_logger,
        g_file_logger
    };

    for (size_t i = 0; i < sizeof(loggers) / sizeof(calns::logger*); ++i)
    {
        calns::logger *logger = loggers[i];

        if (NULL == logger)
            continue;

        bool is_terminal_logger = (typeid(calns::screen_logger) == typeid(*logger));
        int log_level = is_terminal_logger ? terminal_level : file_level;
        int ret = CA_RET_GENERAL_FAILURE;

        logger->set_log_level((calns::enum_log_level)log_level);

        //GLOG_DEBUG_NS("cafw", "log_dir: %s\n", log_dir);
        if ((ret = logger->set_log_directory(log_dir)) < 0)
        {
            GLOG_ERROR_NS("cafw", "failed to set log directory to [%s]\n", log_dir);
            goto LOG_INIT_FAILED;
        }

        if ((ret = logger->set_log_name(log_name)) < 0)
        {
            GLOG_ERROR_NS("cafw", "failed to set log name to [%s], ret = %d\n", log_name, ret);
            goto LOG_INIT_FAILED;
        }

        if ((ret = logger->open()) < 0)
        {
            GLOG_ERROR_NS("cafw", "failed to open log file [%s], ret = %d\n", log_name, ret);
            goto LOG_INIT_FAILED;
        }
    }

    /*
     * Register the logger clear function so that system will release
     * the logger resource automatically on exit.
     * You can release the logger resource manually in some other place,
     * but make sure you do this after other resource.
     */

    if (0 != atexit(clear_logger))
    {
        fprintf(stderr, "atexit() failed\n");
        goto LOG_INIT_FAILED;
    }

    if (enables_file_logger && !is_quiet_mode())
        printf("\nFile logger has been enabled, see more details in %s(.tmp)"
            " and its afterward files and its worker thread log files.\n\n", g_file_logger->log_name());

    return RET_OK;

LOG_INIT_FAILED:

    clear_logger();
    return RET_FAILED;
}

void clear_logger(void)
{
    if (NULL != g_file_logger)
    {
        delete g_file_logger;
        g_file_logger = NULL;
    }

    if (NULL != g_screen_logger)
    {
        delete g_screen_logger;
        g_screen_logger = NULL;
    }
}

}
