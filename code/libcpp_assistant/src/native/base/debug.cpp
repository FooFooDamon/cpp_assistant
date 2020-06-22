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
 * debug.cpp
 *
 *  Created on: 2017-09-17
 *      Author: wenxiongchang
 * Description: See debug.h.
 */

#include "base/debug.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

bool debug::m_debug_enabled = true;

/* static */void debug::enable_output_prefix(void)
{
    __enable_output_prefix();
}

/* static */void debug::disable_output_prefix(void)
{
    __disable_output_prefix();
}

/* static */void debug::redirect_debug_output(const FILE *where)
{
    __set_debug_output(where);
}

/* static */const FILE* debug::get_debug_output_holder(void)
{
    return __get_debug_output_holder();
}

/* static */void debug::set_debug_lock(const mutex *lock)
{
    __set_debug_lock(lock);
}

/* static */bool debug::error_report_is_enabled(void)
{
    return __error_report_is_enabled();
}

/* static */void debug::redirect_error_output(const FILE *where)
{
    __set_error_output(where);
}

/* static */const FILE* debug::get_error_output_holder(void)
{
    return __get_error_output_holder();
}

/* static */void debug::set_error_lock(const mutex *lock)
{
    __set_error_lock(lock);
}

CA_LIB_NAMESPACE_END
