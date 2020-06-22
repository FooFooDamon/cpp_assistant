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
 * compiler.h
 *
 *  Created on: 2017/09/20
 *      Author: wenxiongchang
 * Description: Compiler options and directives.
 */

#ifndef __CPP_ASSISTANT_COMPILER_H__
#define __CPP_ASSISTANT_COMPILER_H__

// NOTE: See <cdefs.h> for more compiler macros information.

#ifdef WINDOWS
#include "platforms/windows/compiler.h"
#else
#include "platforms/linux/compiler.h"
#endif

// Tells the compiler that the class or function is not recommended to use
// and may be removed in future.
#define CA_DEPRECATED                               CA_ATTRIBUTE(deprecated)

// Tells the compiler that some pointer-type arguments of a function can not be null.
// Note that the parameter start number of a free function or a static class member function is 1,
// but the parameter start number of a non-static class member function is 2 because of
// the implicit use of "this" pointer.
// Examples:
//     1) void xx(int num1, char *s1, int num2, char *s2) CA_NOTNULL(2, 4);
//        class Foo
//        {
//     2)     static void xx(int num1, char *s1, int num2, char *s2) CA_NOTNULL(2, 4);
//     3)     void xx(int num1, char *s1, int num2, char *s2) CA_NOTNULL(3, 5);
//        };
#define CA_NOTNULL(params, ...)                     CA_ATTRIBUTE(__nonnull__(params, ##__VA_ARGS__))

// Tells the compiler to do format checking to arguments of a function.
// How to determine the number of an argument: See explanation in the comment of CA_NOTNULL() above.
//#define CA_FORMAT_CHECK(params, ...)              CA_ATTRIBUTE(__format__(params, ##__VA_ARGS__))
#define CA_FORMAT_CHECK(check_type, fmt_str_index, first_arg_index) \
        CA_ATTRIBUTE(__format__(check_type, fmt_str_index, first_arg_index))

// Tells the compiler to check the format string argument and arguments afterwards
// of a printf-like function.
#define CA_PRINTF_CHECK(fmt_str_index, first_arg_index)             \
    CA_FORMAT_CHECK(__printf__, fmt_str_index, first_arg_index)

#define CA_BEFORE_CPP_11                            __cplusplus < 201103L
#define CA_SINCE_CPP_11                             __cplusplus >= 201103L

#if CA_BEFORE_CPP_11

#define override

#define final

#define nullptr                                     NULL

#endif // if CA_BEFORE_CPP_11

#ifndef CA_MAX_LEN_IN_STACK
#define CA_MAX_LEN_IN_STACK                        8192
#endif

#ifndef CA_MAX_FILENAME_LEN
#define CA_MAX_FILENAME_LEN                        255
#endif

#ifndef CA_MAX_PATH_LEN
#define CA_MAX_PATH_LEN                            4095
#endif

#endif /* __CPP_ASSISTANT_COMPILER_H__ */
