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
 * ca_return_code.h
 *
 *  Created on: 2017/10/17
 *      Author: wenxiongchang
 * Description: See ca_return_code.h.
 */

#include "base/ca_return_code.h"

// Disables the macro below in order to use XSI-compliant strerror_r()
// NOTE: it does not always work,some systems or compilers may keep using
//     the GNU-specific strerror_r() no matter this macro is disabled or not.
/*#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif*/

#include <stdio.h>
#include <string.h>

#include "private/ca_return_code.h"

CA_LIB_NAMESPACE_BEGIN

static const char *S_DESCRIPTIONS[USER_RET_CODE_COUNT] = {
    "Null parameters",
    "File or stream not open",
    "Invalid path",
    "Signal registration failed",
    "Memory allocation failed",
    "Invalid parameter values",
    "Object already exists",
    "Inner data structure error",
    "Resource not available",
    "Connection broken",
    "Connection not ready",
    "Excess object count",
    "Operation timed out",
    "Object does not exist",
    "Failed to open file or stream",
    "Value out of range",
    "Underlying error",
    "STL error",
    "TCP self-connection",
    "Path too long",
    "Device busy",
    "Signal not registered",
    "Invalid signal",
    "Failed to get time zone",
    "Signal capture not allowed",
    "Space not enough",
    "Pointer out of bound",
    "Operation already done",
    "Command line parameters insufficient",
    "Unknown command line option",
    "Invalid command line format",
    "Invalid parameter type",
    "Object mismatched",
    "Length too big",
    "Length too small",
    "Target not ready",
    "Signal not arising"
};

CA_REENTRANT int parse_return_code(const int retcode, const int msg_capacity, char *msg) /* CA_NOTNULL(3) */
{
    if (/* will cause compiler [-Wnonnull-compare] warning: nullptr == msg || */msg_capacity <= 0)
        return -1;

    memset(msg, 0, msg_capacity);

    const int MIN_BUF_LEN = MIN_RET_BUF_LEN();

    if (msg_capacity < MIN_BUF_LEN)
        return -1;

    int ret = -1;

    if (retcode > 0)
        ret = snprintf(msg, msg_capacity, "%d units data processed or a code from a sub-function", retcode);
    else if (0 == retcode)
        ret = snprintf(msg, msg_capacity, "%s", SUCCESS_RET_STRING);
    else
    {
        if (-1 == retcode)
            ret = snprintf(msg, msg_capacity, "%s", "General failure or Operation not permitted");
        else if (retcode <= SYS_RET_CODE_BEGIN && retcode > USER_RET_CODE_BEGIN)
        {
#if 0 // See the explanation about the macro _GNU_SOURCE ahead in this file.
            ret = strerror_r(-retcode, buf, buflen); // The XSI-compliant strerror_r() whose return value type is int.
            return (0 == ret) ? 0 : -1;
#else
            char *msg_ptr = reinterpret_cast<char*>(strerror_r(-retcode, msg, msg_capacity));

            if ('\0' != msg[0]) // XSI version is used, or GNU version is used and @buf is used
                return 0;

            ret = snprintf(msg, msg_capacity, "%s", msg_ptr); // GNU version is used and @buf is not used
#endif
        }
        else if (retcode < USER_RET_CODE_END)
            ret = snprintf(msg, msg_capacity, UNKNOWN_RET_STRING" %d", retcode);
        else
            ret = snprintf(msg, msg_capacity, "%s", S_DESCRIPTIONS[USER_RET_CODE_BEGIN - retcode]);
    }
    msg[msg_capacity - 1] = '\0';

    return (ret > 0) ? 0 : -1;
}

CA_REENTRANT void print_all_return_code_descriptions(int sys_retcode_end/* = ESTIMATED_SYS_RET_CODE_END*/)
{
    const int SYS_RET_BEGIN = SYS_RET_CODE_BEGIN;
    const int SYS_RET_END = (sys_retcode_end >= USER_RET_CODE_BEGIN)
        ? sys_retcode_end : USER_RET_CODE_BEGIN;
    char desc[512] = {0};
    int desc_len = sizeof(desc);

    parse_return_code(CA_RET_OK, desc_len, desc);
    fprintf(stdout, "%d:\t%s\n", CA_RET_OK, desc);

    parse_return_code(CA_RET_GENERAL_FAILURE, desc_len, desc);
    fprintf(stdout, "%d:\t%s\n", CA_RET_GENERAL_FAILURE, desc);

    for (int i = SYS_RET_BEGIN; i > SYS_RET_END; --i)
    {
        parse_return_code(i, desc_len, desc);
        fprintf(stdout, "%d:\t%s\n", i, desc);
    }

    for (int i = USER_RET_CODE_BEGIN; i > USER_RET_CODE_END; --i)
    {
        parse_return_code(i, desc_len, desc);
        fprintf(stdout, "%d:\t%s\n", i, desc);
    }
}

CA_LIB_NAMESPACE_END
