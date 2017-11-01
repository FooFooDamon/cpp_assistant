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
 * basic_info.cpp
 *
 *  Created on: 2017-09-10
 *      Author: wenxiongchang
 * Description: See basic_info.h.
 */

#include "base/basic_info.h"

CA_LIB_NAMESPACE_BEGIN

#define CPP_ASSISTANT_START_DATE    "20170910"

#ifndef CA_NEWEST_MOD_DATE
#define CA_NEWEST_MOD_DATE          "20170921"
#endif

#ifndef CA_NEWEST_MAIN_VER
#define CA_NEWEST_MAIN_VER          "0.00.01"
#endif

#ifdef CA_SVN_VER
#define CPP_ASSISTANT_VERSION       "v"CA_NEWEST_MAIN_VER".r"CA_SVN_VER"."CA_NEWEST_MOD_DATE
#else
#define CPP_ASSISTANT_VERSION       "v"CA_NEWEST_MAIN_VER".r012345."CA_NEWEST_MOD_DATE".[Approximate]"
#endif

CA_REENTRANT const char *get_library_name(void)
{
    return CPP_ASSISTANT_LIB_NAME;
}

CA_REENTRANT const char* get_library_version(void)
{
    return CPP_ASSISTANT_VERSION;
}

CA_LIB_NAMESPACE_END
