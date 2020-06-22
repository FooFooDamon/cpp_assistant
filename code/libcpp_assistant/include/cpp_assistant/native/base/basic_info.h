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

// NOTE: The original author also use (short/code) names listed below,
//       for convenience or for a certain purpose, at different places:
//       wenxiongchang, wxc, Damon Wen, udc577

/*
 * basic_info.h
 *
 *  Created on: 2017-09-10
 *      Author: wenxiongchang
 * Description: Basic info of cpp-assistant library.
 */

#ifndef __CPP_ASSISTANT_BASIC_INFO_H__
#define __CPP_ASSISTANT_BASIC_INFO_H__

#define CPP_ASSISTANT_NAME                          "cpp-assistant"
#define CPP_ASSISTANT_LIB_NAME                      "cpp-assistant library"

// You can change the namespace value by running "make CA_LIB_NAMESPACE=xxxx" on compilation,
// and add "-DCA_LIB_NAMESPACE=xxxx" into your application Makefile.
#ifndef CA_LIB_NAMESPACE
#define CA_LIB_NAMESPACE                            cal
#endif

#ifndef CA_LIB_NAMESPACE_STR
#define CA_LIB_NAMESPACE_STR                        "cal"
#endif

#define CA_LIB_NAMESPACE_BEGIN                      namespace CA_LIB_NAMESPACE {
#define CA_LIB_NAMESPACE_END                        }

#ifdef CA_THREAD_SAFE
#undef CA_THREAD_SAFE
#endif
#define CA_THREAD_SAFE

#ifdef CA_REENTRANT
#undef CA_REENTRANT
#endif
#define CA_REENTRANT

CA_LIB_NAMESPACE_BEGIN

CA_REENTRANT const char *get_library_name(void);

CA_REENTRANT const char* get_library_version(void);

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_BASIC_INFO_H__ */
