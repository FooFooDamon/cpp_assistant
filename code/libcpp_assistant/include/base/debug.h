/*
 * Copyright (c) 2017, Wen Xiongchang <udc577 at 126 dot com>
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
 * debug.h
 *
 *  Created on: 2017-09-17
 *      Author: wenxiongchang
 * Description: Providing easy ways of outputting readable and well-formatted info
 *              during debugging.
 */

#ifndef __CPP_ASSISTANT_DEBUG_H__
#define __CPP_ASSISTANT_DEBUG_H__

#include <stdio.h>

#include "ca_inner_necessities.h"
#include "platforms/threading.h"

CA_LIB_NAMESPACE_BEGIN

namespace debug
{

extern bool g_debug_enabled;

void enable_output_prefix(void);

void disable_output_prefix(void);

inline bool debug_is_enabled(void)
{
    return g_debug_enabled;
}

void redirect_debug_output(const FILE *where);

const FILE* get_debug_output_holder(void);

void set_debug_lock(const mutex_t *lock);

bool error_report_is_enabled(void);

void redirect_error_output(const FILE *where);

const FILE* get_error_output_holder(void);

void set_error_lock(const mutex_t *lock);

} // namespace debug

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_DEBUG_H__ */
