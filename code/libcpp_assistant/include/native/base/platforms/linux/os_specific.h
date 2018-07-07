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

/*
 * os_specific.h
 *
 *  Created on: 2017/09/17
 *      Author: wenxiongchang
 * Description: Linux version of os_specific.h.
 */

#ifndef __CPP_ASSISTANT_PLATFORMS_LINUX_OS_SPECIFIC_H__
#define __CPP_ASSISTANT_PLATFORMS_LINUX_OS_SPECIFIC_H__

#include <unistd.h>

#include "native/base/ca_inner_necessities.h"

CA_LIB_NAMESPACE_BEGIN

inline char get_directory_delimiter(void)
{
    return '/';
}

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_PLATFORMS_LINUX_OS_SPECIFIC_H__ */
