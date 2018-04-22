/*
 * Copyright (c) 2017-2018, Wen Xiongchang <udc577 at 126 dot com>
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

#include "screen_logger.h"

#include <typeinfo>

CA_LIB_NAMESPACE_BEGIN

DEFINE_CLASS_NAME(screen_logger);

screen_logger::screen_logger()
{
    m_to_screen = true;
    set_log_name(NULL);
    open();
}

screen_logger::~screen_logger()
{
    close();
}

/*virtual */int screen_logger::open(int cache_buf_size/* = DEFAULT_LOG_CACHE_SIZE*/)/*  = 0 */
{
    if (is_open())
        return CA_RET_OK;

    m_output_holder = stdout;
    m_is_open = true;
    m_cur_line = 0;

    return CA_RET_OK;
}

/*virtual */int screen_logger::close(bool release_buffer/* = true*/)/*  = 0 */
{
    m_output_holder = NULL; // Waring: Never fclose(m_output_holder) or fclose(stdout) !
    m_is_open = false;

    return CA_RET_OK;
}

CA_LIB_NAMESPACE_END
