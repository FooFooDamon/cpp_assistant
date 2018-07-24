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

typedef int (*format_output_func)(const char *fmt, ...);

class debug : public no_instance
{
public:
    /*
     * It's easy to understand that cpp-assistant library may output debug, warning or error info
     * when exceptions occur or for some other purposes.
     *
     * APIs below can be used to do such settings as:
     *     enabling or disabling the output prefix displaying,
     *     enabling or disabling the output,
     *     setting the output destinations (e.g. to stdout, stderr or to files)
     * and so on. See comments of each API for more details.
     */

    // Makes the debug or warning/error output display with prefix, like:
    // [2018-01-01 00:00:00.000000][PID:1234] I'm the output with prefix...
    // or something else.
    static void enable_output_prefix(void);

    // Makes the debug or warning/error output display without prefix.
    static void disable_output_prefix(void);

    /*
     * xx_is_enabled(): Checks whether the specified output is enabled.
     *
     * redirect_xx_output(): Redirects the specified output to stdout, stderr or a file.
     *      If @where is nullptr, then the specified output will be disabled.
     *
     * get_xx_output_holder(): Gets the output file pointer.
     *
     * set_xx_lock(): Sets a lock for the specified output, only required in multi-threading environment.
     *
     */

    static inline bool debug_report_is_enabled(void)
    {
        return m_debug_enabled;
    }

    static void redirect_debug_output(const FILE *where);

    static const FILE* get_debug_output_holder(void);

    static void set_debug_lock(const mutex *lock);

    static bool error_report_is_enabled(void);

    static void redirect_error_output(const FILE *where);

    static const FILE* get_error_output_holder(void);

    static void set_error_lock(const mutex *lock);

private:
    static bool m_debug_enabled;

    friend bool __debug_is_enabled(void);
    friend void __set_debug_output(const FILE *);

}; // class debug

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_DEBUG_H__ */
