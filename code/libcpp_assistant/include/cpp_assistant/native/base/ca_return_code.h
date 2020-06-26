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
 * Description: Return codes management for cpp-assistant library.
 */


#ifndef __CPP_ASSISTANT_BASE_CA_RETURN_CODE_H__
#define __CPP_ASSISTANT_BASE_CA_RETURN_CODE_H__

#include <string>

#include "ca_inner_necessities.h"

enum
{
    CA_RET_OK = 0,
    CA_RET_GENERAL_FAILURE = -1,
    CA_RET_OPERATION_NOT_PERMITTED = -1
};

#define CA_RET(error_name)      (-CA_LIB_NAMESPACE::error_name)

CA_LIB_NAMESPACE_BEGIN

enum
{
    __USER_RET_CODE_BEGIN = 1001,

    NULL_PARAM = __USER_RET_CODE_BEGIN,
    FILE_OR_STREAM_NOT_OPEN,
    INVALID_PATH,
    SIGNAL_REGISTRATION_FAILED,
    MEMORY_ALLOC_FAILED,
    INVALID_PARAM_VALUE,
    OBJECT_ALREADY_EXISTS,
    INNER_DATA_STRUCT_ERROR,
    RESOURCE_NOT_AVAILABLE,
    CONNECTION_BROKEN,
    CONNECTION_NOT_READY,
    EXCESS_OBJECT_COUNT,
    OPERATION_TIMED_OUT,
    OBJECT_DOES_NOT_EXIST,
    FILE_OR_STREAM_OPEN_FAILED,
    VALUE_OUT_OF_RANGE,
    UNDERLYING_ERROR,
    STL_ERROR,
    TCP_SELF_CONNECT,
    PATH_TOO_LONG,
    DEVICE_BUSY,
    SIGNAL_NOT_REGISTERED,
    INVALID_SIGNAL_NUMBER,
    GETTING_TIMEZONE_FAILED,
    SIGNAL_CAPTURE_NOT_ALLOWED,
    SPACE_NOT_ENOUGH,
    POINTER_OUT_OF_BOUND,
    OPERATION_ALREADY_DONE,
    CMD_LINE_PARAM_INSUFFICIENT,
    UNKNOWN_CMD_LINE_OPTION,
    INVALID_CMD_LINE_FORMAT,
    INVALID_PARAM_TYPE,
    OBJECT_MISMATCHED,
    LENGTH_TOO_BIG,
    LENGTH_TOO_SMALL,
    TARGET_NOT_READY,
    SIGNAL_NOT_ARISING,

    // NOTE: New return codes should be added before __USER_RET_CODE_END!
    __USER_RET_CODE_END
};

enum
{
    OK = 0,
    GENERAL_FAILURE = 1,
    OPERATION_NOT_PERMITTED = 1,

    SYS_RET_CODE_BEGIN = -2,
    ESTIMATED_SYS_RET_CODE_END = -150,
    USER_RET_CODE_BEGIN = -__USER_RET_CODE_BEGIN,
    USER_RET_CODE_END = -__USER_RET_CODE_END,
    USER_RET_CODE_COUNT = __USER_RET_CODE_END - __USER_RET_CODE_BEGIN
};

// Translates a return code into a readable text message.
CA_REENTRANT int parse_return_code(const int retcode, const int msg_capacity, char *msg) CA_NOTNULL(3);

// Same as parse_return_code() above, except that this one returns a std::string value directly,
// which is more convenient to use.
CA_REENTRANT inline std::string what(const int retcode)
{
    char msg[1024] = {0};

    parse_return_code(retcode, sizeof(msg), msg);

    return msg;
}

//   USER_RET_CODE_END ... USER_RET_CODE_BEGIN ... SYS_RET_CODE_END ... SYS_RET_CODE_BEGIN  -1  0  1  2  3 ...
//
// __________________|_...___________________|______  ... ________|______________________|___|__|__|__|__|__...
//                   | ---- continuous ----- |-- not continuous --| ---- continuous ---- |
//
// As the diagram above shows that return codes are not continuous, thus you have to tell the function
// where the system end point is, to avoid printing unused codes.
CA_REENTRANT void print_all_return_code_descriptions(int sys_retcode_end = ESTIMATED_SYS_RET_CODE_END);

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_BASE_CA_RETURN_CODE_H__

