/*
 * Copyright (c) 2016-2019, Wen Xiongchang <udc577 at 126 dot com>
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

#include "main_app.h"

int main(int argc, char **argv)
{
    cafw::main_app *app = cafw::main_app::get_instance();

    if (NULL == app)
    {
        fprintf(stderr, "**** Failed to fetch main application instance.\n");
        return -1;
    }

    if (app->prepare_prerequisites())
    {
        fprintf(stderr, "**** Failed to prepare prerequisites.\n");
        return -1;
    }

    if (app->parse_command_line(argc, argv) < 0)
    {
        fprintf(stderr, "**** Errors occurred while parsing command line.\n");
        return -1;
    }

#ifdef HAS_CONFIG_FILES
    if (app->load_configurations() < 0)
    {
        fprintf(stderr, "**** Errors occurred while loading configurations.\n");
        return -1;
    }
#endif
    if (app->prepare_resources() < 0)
    {
        fprintf(stderr, "**** Errors occurred while preparing resource.\n");
        return -1;
    }

    if (app->register_signals() < 0)
    {
        fprintf(stderr, "**** Errors occurred while registering signals.\n");
        return -1;
    }

    if (app->register_timed_tasks() < 0)
    {
        fprintf(stderr, "**** Errors occurred while registering timed tasks.\n");
        app->release_resources();
        return -1;
    }

    if (app->initialize_business(argc, argv) < 0)
    {
        fprintf(stderr, "**** Errors occurred while initializing business.\n");
        app->release_resources();
        return -1;
    }

    int ret = app->run_business();

    app->finalize_business();

    app->release_resources();

    return ret;
}
