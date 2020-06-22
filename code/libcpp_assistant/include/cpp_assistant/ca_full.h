/*
 * Copyright (c) 2018-2020, Wen Xiongchang <udc577 at 126 dot com>
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
 * ca_full.h
 *
 *  Created on: 2018/04/14
 *      Author: wenxiongchang
 */

#ifndef __CPP_ASSISTANT_FULL_H__
#define __CPP_ASSISTANT_FULL_H__

/*
 * General design principles
 *
 * For return codes:
 *
 *     (1) Return type of integer:
 *         Return value is CA_RET_OK (that is: 0) on success, or a negative number,
 *         which can be described by parse_retcode(), on error.
 *
 *     (2) Return type of pointer:
 *         Return value is a pointer to proper contents depending on the calling function on success,
 *         or NULL or a pointer to business-invalid contents on error.
 *
 * For thread-safe requirement:
 *
 *     Functions with CA_THREAD_SAFE are thread-safe, but are not necessarily reentrant.
 *     Functions with CA_REENTRANT are both reentrant and thread-safe, and obviously, they're
 *         thread-safe only if their output parameters are non-static local variables.
 *     Others are neither reentrant nor thread-safe.
 *
 * For comments:
 *
 *     (1) A block-comment "/ * * /" is used for the statement block under/after it.
 *     (2) A line-comment "//" is used for a single statement at the same line or under/after it.
 */

#include "ca_3rdparty.h"

#include "ca_native.h"

#endif /* __CPP_ASSISTANT_FULL_H__ */
