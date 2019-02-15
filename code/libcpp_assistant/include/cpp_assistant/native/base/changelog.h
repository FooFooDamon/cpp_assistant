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
 * changelog.h
 *
 *  Created on: 2017-09-10
 *      Author: wenxiongchang(AKA: wxc)
 * Description: Change log of cpp-assistant library.
 */

#ifndef __CPP_ASSISTANT_CHANGELOG_H__
#define __CPP_ASSISTANT_CHANGELOG_H__

/*
 * Change log of cpp-assistant (in reversed order !!!)
 *
 * Log format is defined as below:
 * Author, Date, Version:
 *      Descriptions.
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * WARNING: DO NOT change the format of these comments
 *     because commands in Makefiles depend on them to determine some compile info!!!
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * wxc, 2019/02/15, 0.04.04:
 *      1. Added str::split() with std::string type input parameter.
 *      2. Added default parameter since_1900 = true into time_util::get_utc/local_xxx().
 *      3. Fixed the compile flags: Debug: -O0 -g -ggdb; Release: -O3
 *
 * wxc, 2018/11/25, 0.04.03:
 *      1. Added str::dirname(), basename(), split_dir_and_basename(),
 *          get_absolute_path() and get_self_absolute_path().
 *      2. Changed return value (with type of int) of str::get_directory().
 *
 * wxc, 2018/08/03, 0.04.02:
 *      1. Added NO_CA_LOG macro to control if LOG*(x) could be used.
 *      2. Added str::get_directory() series.
 *      3. Added str::split() with a return type of vector<string>.
 *      4. Added NO_TINYXML compilation switch.
 *
 * wxc, 2018/07/23, 0.04.01:
 *      1. Removed TLOG_XX(), GLOG_XX() and CA_XX(), and added LOGF(x)
 *          to do the same things instead.
 *      2. Defined LOGS(x), and fixed the newline bug of the first call
 *          of logger::get_stream().
 *      3. Replaced pthread_self() with gettid() within some APIs.
 *      4. Added mutable version of logger::output_holder().
 *
 * wxc, 2018/07/21, 0.04.00:
 *      1. Added CA_LIB_NAMESPACE customization on compilation.
 *      2. Removed the definition and using of m_class_name.
 *      3. Simplified the series of CA_XX() logging APIs.
 *
 * wxc, 2018/07/14, 0.03.01:
 *      1. Added the conversion between bool and string in str::to_string()
 *          and str::from_string().
 *      2. include -> include/cpp_assistant.
 *      3. Added more aliases for logger::debug/info/warn/error/critical(),
 *          stream-style logger interfaces (logger::<< operator overloading).
 *      4. Adjusted some compilation parameters.
 *
 * wxc, 2018/07/08, 0.03.00:
 *      1. Added supports for some points of c++11 standard, such as override,
 *          nullptr, mutex and lock_guard.
 *      2. Added no_instance and noncopyable class.
 *      3. Turned some namespaces into classes.
 *      4. Tried Git hash code while SVN version number was not available.
 *
 * wxc, 2018/04/21, 0.02.00:
 *      Some new APIs, and code readability improvements.
 *
 * wxc, 2018/04/19, 0.01.02:
 *      Removed: STL adapters.
 *
 * wxc, 2018/04/15, 0.01.01:
 *      Added: tcp_client, tcp_server and their relative classes or APIs.
 *
 * wxc, 2018/04/12, 0.01.00:
 *      Refactoring under the rules of new code style.
 *
 * wxc, 2017/09/10, 0.00.01:
 *      Created.
 */

#endif /* __CPP_ASSISTANT_CHANGELOG_H__ */
