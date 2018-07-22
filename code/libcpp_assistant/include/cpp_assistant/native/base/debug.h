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
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include "ca_inner_necessities.h"
#include "platforms/threading.h"

/*
 * NOTE: The logging macros below are convenient to use when writing simple programs,
 * but you probably need to redefine them in your own file when writing complex programs and needing high performance.
 */

/*
 * [F]ormatted logging macros. x could be:
 *     d, debug, D, DEBUGGING (DEBUG should not be used because it's generally used as a debug controlling macro somewhere).
 *     i, info, I, INFO,
 *     w, warn, W, WARN,
 *     e, error, E, ERROR,
 *     c, critical, C, CRITICAL.
 */

// R is short for raw, which means outputting log contents as little as possible.
#ifdef RLOGF
#undef RLOGF
#endif
#define RLOGF(x, fmt, ...)                              CA_LIB_NAMESPACE::singleton<CA_LIB_NAMESPACE::screen_logger>::get_instance()->x(fmt, ##__VA_ARGS__)

#ifdef MULTI_THREADING

#define LOGF_BASE(x, class_str, ns_delim, fmt, ...)     do {\
	CA_LIB_NAMESPACE::lock_guard<CA_LIB_NAMESPACE::mutex> lock(*CA_LIB_NAMESPACE::singleton<CA_LIB_NAMESPACE::mutex>::get_instance()); \
	RLOGF(x, "[%d#%d] %s:%d, %s%s%s(): " fmt, getpid(), CA_LIB_NAMESPACE::gettid(), __FILE__, __LINE__, class_str, ns_delim, __FUNCTION__, ##__VA_ARGS__); \
} while(0)

#else

#define LOGF_BASE(x, class_str, ns_delim, fmt, ...)     RLOGF(x, "%s:%d, %s%s%s(): " fmt, __FILE__, __LINE__, class_str, ns_delim, __FUNCTION__, ##__VA_ARGS__)

#endif

#ifdef LOGF
#undef LOGF
#endif
#define LOGF(x, fmt, ...)                               LOGF_BASE(x, "", "", fmt, ##__VA_ARGS__)

// C is short for class, which means outputting name of the class calling this logging function.
#ifdef LOGF_C
#undef LOGF_C
#endif
#define LOGF_C(x, fmt, ...)                             LOGF_BASE(x, typeid(*this).name(), "::", fmt, ##__VA_ARGS__)

// NS is short for namespace, which means outputting the current namespace.
#ifdef LOGF_NS
#undef LOGF_NS
#endif
#define LOGF_NS(_namespace_, fmt, ...)                  LOGF_BASE(x, #_namespace_, "::", fmt, ##__VA_ARGS__)

/*
 * [S]tream-style logging macros. Value of x is the same as formatted logging.
 */

// R means the the same as the one in the formatted logging above.
#ifdef RLOGS
#undef RLOGS
#endif
#ifdef MULTI_THREADING // TODO: has not supported multi-threading so far, because locking is not easy.
#define RLOGS(x)
#else
#define RLOGS(x)                                        CA_LIB_NAMESPACE::singleton<CA_LIB_NAMESPACE::screen_logger>::get_instance()->get_stream(x)
#endif

#ifdef LOGS
#undef LOGS
#endif
#define LOGS(x)                                         RLOGS(x) << __FILE__ << ":" << __LINE__ << ", " << __FUNCTION__ << "(): "

// C means the the same as the one in the formatted logging above.
#ifdef LOGS_C
#undef LOGS_C
#endif
#define LOGS_C(x)                                       RLOGS(x) << __FILE__ << ":" << __LINE__ << ", " << typeid(*this).name() << "::" << __FUNCTION__ << "(): "

// NS means the the same as the one in the formatted logging above.
#ifdef LOGS_NS
#undef LOGS_NS
#endif
#define LOGS_NS(_namespace_)                            RLOGS(x) << __FILE__ << ":" << __LINE__ << ", " << #_namespace_ << "::" << __FUNCTION__ << "(): "

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
